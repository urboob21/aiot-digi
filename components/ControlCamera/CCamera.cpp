#include "CCamera.h"
#include "userGPIO.h"
#include "esp_camera.h"
#include <esp_http_server.h>
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include <esp_log.h>

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


esp_err_t CCamera::captureImgAndResToHTTP(httpd_req_t *req, int delay)
{
    return NULL;
}