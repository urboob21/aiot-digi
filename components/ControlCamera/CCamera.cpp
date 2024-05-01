#include "CCamera.h"
#include "userGPIO.h"
#include "esp_camera.h"
#include <esp_http_server.h>
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include <esp_log.h>
#include "esp_timer.h"
#include "Helper.h"
#include "CImageBasis.h"

static camera_config_t camera_config = {
    .pin_pwdn = CAM_PIN_PWDN,
    .pin_reset = CAM_PIN_RESET,
    .pin_xclk = CAM_PIN_XCLK,
    .pin_sscb_sda = CAM_PIN_SIOD,
    .pin_sscb_scl = CAM_PIN_SIOC,

    .pin_d7 = CAM_PIN_D7,
    .pin_d6 = CAM_PIN_D6,
    .pin_d5 = CAM_PIN_D5,
    .pin_d4 = CAM_PIN_D4,
    .pin_d3 = CAM_PIN_D3,
    .pin_d2 = CAM_PIN_D2,
    .pin_d1 = CAM_PIN_D1,
    .pin_d0 = CAM_PIN_D0,
    .pin_vsync = CAM_PIN_VSYNC,
    .pin_href = CAM_PIN_HREF,
    .pin_pclk = CAM_PIN_PCLK,

    // XCLK 20MHz or 10MHz for OV2640 double FPS (Experimental)
    //    .xclk_freq_hz = 20000000,             // Orginalwert
    .xclk_freq_hz = 5000000, // Test, um die Bildfehler los zu werden !!!!
    .ledc_timer = LEDC_TIMER_0,
    .ledc_channel = LEDC_CHANNEL_0,

    .pixel_format = PIXFORMAT_JPEG, // YUV422,GRAYSCALE,RGB565,JPEG
    .frame_size = FRAMESIZE_VGA,    // QQVGA-UXGA Do not use sizes above QVGA when not JPEG
    //    .frame_size = FRAMESIZE_UXGA,    //QQVGA-UXGA Do not use sizes above QVGA when not JPEG

    .jpeg_quality = 5, // 0-63 lower number means higher quality
    .fb_count = 1      // if more than one, i2s runs in continuous mode. Use only with JPEG
};

typedef struct
{
    httpd_req_t *req;
    size_t len;
} jpg_chunking_t;

CCamera::CCamera()
{
    brightness = -5;
    contrast = -5;
    saturation = -5;
    isFixedExposure = false;
}

void CCamera::lightOnOff(bool status)
{
    // Init the GPIO
    gpio_reset_pin(FLASH_GPIO); // gpio_pad_select_gpio() has changed to gpio_reset_pin()

    // Set the push/pull mode output
    gpio_set_direction(FLASH_GPIO, GPIO_MODE_OUTPUT);

    if (status)
        gpio_set_level(FLASH_GPIO, 1);
    else
        gpio_set_level(FLASH_GPIO, 0);
}

framesize_t CCamera::textToFramesize(const char * _size)
{
    if (strcmp(_size, "QVGA") == 0)
        return FRAMESIZE_QVGA;       // 320x240
    if (strcmp(_size, "VGA") == 0)
        return FRAMESIZE_VGA;      // 640x480
    if (strcmp(_size, "SVGA") == 0)
        return FRAMESIZE_SVGA;     // 800x600
    if (strcmp(_size, "XGA") == 0)
        return FRAMESIZE_XGA;      // 1024x768
    if (strcmp(_size, "SXGA") == 0)
        return FRAMESIZE_SXGA;     // 1280x1024
    if (strcmp(_size, "UXGA") == 0)
        return FRAMESIZE_UXGA;     // 1600x1200   
    return actualResolution;
}

esp_err_t CCamera::initCamera()
{
    if (CAM_PIN_PWDN != -1)
    {
        // Init the GPIO
        gpio_reset_pin(CAM_PIN_PWDN);
        gpio_set_direction(CAM_PIN_PWDN, GPIO_MODE_OUTPUT);
        gpio_set_level(CAM_PIN_PWDN, 0);
    }

    printf("Init Camera \n");

    actualQuality = camera_config.jpeg_quality;
    actualResolution = camera_config.frame_size;

    // initialize the camera
    esp_err_t err = esp_camera_init(&camera_config);
    if (err != ESP_OK)
    {
        ESP_LOGE(TAG_CCAMERA, "Camera Init Failed");
        return err;
    }

    return ESP_OK;
}

// get the params from msg : "?size=VGA&quality=50"
void CCamera::getCameraParamFromHttpRequest(httpd_req_t *req, int &qual, framesize_t &resol)
{
    char _query[100];
    char _qual[10];
    char _size[10];

    resol = actualResolution;
    qual = actualQuality;

    if (httpd_req_get_url_query_str(req, _query, 100) == ESP_OK)
    {
        printf("Query: ");
        printf(_query);
        printf("\n");
        if (httpd_query_key_value(_query, "size", _size, 10) == ESP_OK)
        {
            if (strcmp(_size, "QVGA") == 0)
                resol = FRAMESIZE_QVGA; // 320x240
            if (strcmp(_size, "VGA") == 0)
                resol = FRAMESIZE_VGA; // 640x480
            if (strcmp(_size, "SVGA") == 0)
                resol = FRAMESIZE_SVGA; // 800x600
            if (strcmp(_size, "XGA") == 0)
                resol = FRAMESIZE_XGA; // 1024x768
            if (strcmp(_size, "SXGA") == 0)
                resol = FRAMESIZE_SXGA; // 1280x1024
            if (strcmp(_size, "UXGA") == 0)
                resol = FRAMESIZE_UXGA; // 1600x1200
        }
        if (httpd_query_key_value(_query, "quality", _qual, 10) == ESP_OK)
        {
            qual = atoi(_qual);

            if (qual > 63)
                qual = 63;
            if (qual < 0)
                qual = 0;
        }
    };
}

/**
 * @brief Thiết lập chất lượng và kích thước của ảnh cho camera.
 *
 * Hàm này được sử dụng để cập nhật chất lượng và kích thước của hình ảnh cho camera.
 * Nó thực hiện việc thiết lập chất lượng và kích thước mới cho cảm biến camera, cũng như
 * cập nhật các biến theo dõi chất lượng và kích thước thực tế của ảnh.
 *
 * @param qual Giá trị chất lượng mới cho hình ảnh (0-63).
 * @param resol Kích thước khung hình mới cho hình ảnh (giá trị enum framesize_t).
 *
 * @return Không có giá trị trả về.
 */

void CCamera::setQualitySize(int qual, framesize_t resol)
{
    sensor_t *s = esp_camera_sensor_get();
    s->set_quality(s, qual);
    s->set_framesize(s, resol);
    actualResolution = resol;
    actualQuality = qual;

    if (resol == FRAMESIZE_QVGA)
    {
        imageHeight = 240;
        imageWidth = 320;
    }
    if (resol == FRAMESIZE_VGA)
    {
        imageHeight = 480;
        imageWidth = 640;
    }
}

static size_t jpg_encode_stream(void *arg, size_t index, const void *data, size_t len)
{
    jpg_chunking_t *j = (jpg_chunking_t *)arg;
    if (!index)
    {
        j->len = 0;
    }
    if (httpd_resp_send_chunk(j->req, (const char *)data, len) != ESP_OK)
    {
        return 0;
    }
    j->len += len;
    return len;
}

void CCamera::enableAutoExposure(int flashdauer)
{
    // LEDOnOff(true);
    lightOnOff(true);
    const TickType_t xDelay = flashdauer / portTICK_PERIOD_MS;
    vTaskDelay( xDelay );

    camera_fb_t * fb = esp_camera_fb_get();
    if (!fb) {
        ESP_LOGE(TAG_CCAMERA, "Camera Capture Failed");
    }
    esp_camera_fb_return(fb);        

    sensor_t * s = esp_camera_sensor_get(); 
    s->set_gain_ctrl(s, 0);
    s->set_exposure_ctrl(s, 0);


    // LEDOnOff(false);  
    lightOnOff(false);
    isFixedExposure = true;
    waitbeforepictureOrg = flashdauer;
}


bool CCamera::setBrightnessContrastSaturation(int _brightness, int _contrast, int _saturation)
{
    bool result = false;
    sensor_t * s = esp_camera_sensor_get(); 
    if (_brightness > -100)
        _brightness = min(2, max(-2, _brightness));
    if (_contrast > -100)
        _contrast = min(2, max(-2, _contrast));
    if (_contrast > -100)
        s->set_contrast(s, _contrast);
    if (_brightness > -100)
        s->set_brightness(s, _brightness);

    if ((_brightness != brightness) && (_brightness > -100))
        result = true;
    if ((_contrast != contrast) && (_contrast > -100))
        result = true;
    if ((_saturation != saturation) && (_saturation > -100))
        result = true;
    
    if (_brightness > -100)
        brightness = _brightness;
    if (_contrast > -100)
        contrast = _contrast;
    if (_saturation > -100)
       saturation = _saturation;

    if (result && isFixedExposure)
        enableAutoExposure(waitbeforepictureOrg);

    return result;
}

// Capture the picture
esp_err_t CCamera::captureImgAndResToHTTP(httpd_req_t *req, int delay)
{
    camera_fb_t *fb = NULL;
    esp_err_t res = ESP_OK;
    size_t fb_len = 0;
    int64_t fr_start = esp_timer_get_time();

    // LEDOnOff(true);
    if (delay > 0)
    {
        lightOnOff(true);
        const TickType_t xDelay = delay / portTICK_PERIOD_MS;
        vTaskDelay(xDelay);
    }

    // take the picture
    fb = esp_camera_fb_get();
    if (!fb)
    {
        ESP_LOGE(TAG_CCAMERA, "Camera capture failed");
        httpd_resp_send_500(req);
        return ESP_FAIL;
    }
    lightOnOff(false);

    // picture to browser
    res = httpd_resp_set_type(req, "image/jpeg");
    if (res == ESP_OK)
    {
        res = httpd_resp_set_hdr(req, "Content-Disposition", "inline; filename=raw.jpg");
    }

    if (res == ESP_OK)
    {
        if (fb->format == PIXFORMAT_JPEG)
        {
            fb_len = fb->len;
            res = httpd_resp_send(req, (const char *)fb->buf, fb->len);
        }
        else
        {
            jpg_chunking_t jchunk = {req, 0};
            res = frame2jpg_cb(fb, 80, jpg_encode_stream, &jchunk) ? ESP_OK : ESP_FAIL;
            httpd_resp_send_chunk(req, NULL, 0);
            fb_len = jchunk.len;
        }
    }

    esp_camera_fb_return(fb);
    int64_t fr_end = esp_timer_get_time();

    ESP_LOGI(TAG_CCAMERA, "JPG: %ldKB %ldms", (uint32_t)(fb_len / 1024), (uint32_t)((fr_end - fr_start) / 1000));

    if (delay > 0)
    {
        lightOnOff(false);
    }

    return res;
}


esp_err_t CCamera::captureToBasisImage(CImageBasis *_Image, int delay)
{
    string ftype;

    uint8_t *zwischenspeicher = NULL;


    // LEDOnOff(true);


    if (delay > 0) 
    {
        lightOnOff(true);
        const TickType_t xDelay = delay / portTICK_PERIOD_MS;
        vTaskDelay( xDelay );
    }


    camera_fb_t * fb = esp_camera_fb_get();
    if (!fb) {
        ESP_LOGE(TAG_CCAMERA, "Camera Capture Failed");
        // LEDOnOff(false);
        return ESP_FAIL;
    }

    int _size = fb->len;
    zwischenspeicher = (uint8_t*) malloc(_size);
    for (int i = 0; i < _size; ++i)
        *(zwischenspeicher + i) = *(fb->buf + i);
    esp_camera_fb_return(fb);        

    // LEDOnOff(false);  

    if (delay > 0) 
        lightOnOff(false);

    uint8_t * buf = NULL;

    CImageBasis _zwImage;
    _zwImage.LoadFromMemory(zwischenspeicher, _size);
    free(zwischenspeicher);

    stbi_uc* p_target;
    stbi_uc* p_source;    
    int channels = 3;
    int width = imageWidth;
    int height = imageHeight;

    for (int x = 0; x < width; ++x)
        for (int y = 0; y < height; ++y)
        {
            p_target = _Image->rgb_image + (channels * (y * width + x));
            p_source = _zwImage.rgb_image + (channels * (y * width + x));
            p_target[0] = p_source[0];
            p_target[1] = p_source[1];
            p_target[2] = p_source[2];
        }

    free(buf);

    return ESP_OK;    
}

esp_err_t CCamera::captureAndSaveToFile(std::string nm, int delay)
{
    std::string ftype;

    //  LEDOnOff(true);              // Abgeschaltet, um Strom zu sparen !!!!!!

    if (delay > 0)
    {
        lightOnOff(true);
        const TickType_t xDelay = delay / portTICK_PERIOD_MS;
        vTaskDelay(xDelay);
    }

    camera_fb_t *fb = esp_camera_fb_get();
    if (!fb)
    {
        ESP_LOGE(TAG_CCAMERA, "Camera Capture Failed");
        // LEDOnOff(false);
        return ESP_FAIL;
    }
    // LEDOnOff(false);
    nm = FormatFileName(nm);
    ftype = toUpper(getFileType(nm));
    uint8_t *buf = NULL;
    size_t buf_len = 0;
    bool converted = false;

    if (ftype.compare("BMP") == 0)
    {
        frame2bmp(fb, &buf, &buf_len);
        converted = true;
    }
    if (ftype.compare("JPG") == 0)
    {
        if (fb->format != PIXFORMAT_JPEG)
        {
            bool jpeg_converted = frame2jpg(fb, actualQuality, &buf, &buf_len);
            converted = true;
            if (!jpeg_converted)
            {
                ESP_LOGE(TAG_CCAMERA, "JPEG compression failed");
            }
        }
        else
        {
            buf_len = fb->len;
            buf = fb->buf;
        }
    }

    FILE *fp = OpenFileAndWait(nm.c_str(), "wb");
    if (fp == NULL) /* If an error occurs during the file creation */
    {
        fprintf(stderr, "fopen() failed for '%s'\n", nm.c_str());
    }
    else
    {
        fwrite(buf, sizeof(uint8_t), buf_len, fp);
        fclose(fp);
    }
    if (converted)
        free(buf);

    esp_camera_fb_return(fb);

    if (delay > 0)
    {
        lightOnOff(false);
    }

    return ESP_OK;
}