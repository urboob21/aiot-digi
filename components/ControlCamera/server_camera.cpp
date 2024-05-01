#include "server_camera.h"
#include <string.h>
#include "CCamera.h"
#include <hal/gpio_ll.h>

// =========================================== URI handlers==========================================

// GET "/lighton"
esp_err_t handler_lightOn(httpd_req_t *req)
{
    cameraESP.lightOnOff(true);
    const char *resp_str = (const char *)req->user_ctx;
    httpd_resp_send(req, resp_str, strlen(resp_str));
    return ESP_OK;
};

// GET "/lightoff"
esp_err_t handler_lightOff(httpd_req_t *req)
{
    cameraESP.lightOnOff(false);
    const char *resp_str = (const char *)req->user_ctx;
    httpd_resp_send(req, resp_str, strlen(resp_str));
    return ESP_OK;
};

// GET "/capture"
esp_err_t handler_capture(httpd_req_t *req)
{
    int quality;
    framesize_t fSize; // size of picture

    cameraESP.getCameraParamFromHttpRequest(req, quality, fSize);
    cameraESP.setQualitySize(quality, fSize);

    esp_err_t res;
    res = cameraESP.captureImgAndResToHTTP(req);

    return res;
};

// GET "/save"

esp_err_t handler_capture_save_to_file(httpd_req_t *req)
{
    char _query[100];
    char _delay[10];
    int delay = 0;
    char filename[100];
    std::string fn = "/sdcard/";
    int quality;
    framesize_t res;

    if (httpd_req_get_url_query_str(req, _query, 100) == ESP_OK)
    {
        printf("Query: ");
        printf(_query);
        printf("\n");
        if (httpd_query_key_value(_query, "filename", filename, 100) == ESP_OK)
        {
            fn.append(filename);
        }
        else
            fn.append("noname.jpg");

        if (httpd_query_key_value(_query, "delay", _delay, 10) == ESP_OK)
        {
            delay = atoi(_delay);

            if (delay < 0)
                delay = 0;
        }
    }
    else
        fn.append("noname.jpg");

    cameraESP.getCameraParamFromHttpRequest(req, quality, res);
    cameraESP.setQualitySize(quality, res);

    esp_err_t ressult;
    ressult = cameraESP.captureAndSaveToFile(fn, delay);

    const char *resp_str = (const char *)fn.c_str();
    httpd_resp_send(req, resp_str, strlen(resp_str));
    return ressult;
}

void powerResetCamera()
{
    ESP_LOGD(TAG_SERVER_CAMERA, "Resetting camera by power down line PIN32");
    gpio_config_t conf = {0};
    conf.pin_bit_mask = 1LL << GPIO_NUM_32;
    conf.mode = GPIO_MODE_OUTPUT;
    gpio_config(&conf);

    // carefull, logic is inverted compared to reset pin
    gpio_set_level(GPIO_NUM_32, 1);
    vTaskDelay(1000 / portTICK_PERIOD_MS);
    gpio_set_level(GPIO_NUM_32, 0);
    vTaskDelay(1000 / portTICK_PERIOD_MS);
}

void registerServerCameraUri(httpd_handle_t server)
{
    httpd_uri_t camuri = {};
    camuri.method = HTTP_GET;

    camuri.uri = "/lighton";
    camuri.handler = handler_lightOn;
    camuri.user_ctx = (void *)"Light On";
    httpd_register_uri_handler(server, &camuri);

    camuri.uri = "/lightoff";
    camuri.handler = handler_lightOff;
    camuri.user_ctx = (void *)"Light Off";
    httpd_register_uri_handler(server, &camuri);

    camuri.uri = "/capture";
    camuri.handler = handler_capture;
    camuri.user_ctx = NULL;
    httpd_register_uri_handler(server, &camuri);

    // camuri.uri = "/capture_with_flashlight";
    // camuri.handler = handler_capture_with_ligth;
    // camuri.user_ctx = NULL;
    // httpd_register_uri_handler(server, &camuri);

    camuri.uri = "/save";
    camuri.handler = handler_capture_save_to_file;
    camuri.user_ctx = NULL;
    httpd_register_uri_handler(server, &camuri);
}
