#ifndef CCAMERA_H
#define CCAMERA_H

#include <esp_err.h>
#include <esp_camera.h>
#include <esp_http_server.h>

static const char *TAG_CCAMERA = "T_CCamera";

class CCamera
{
protected:
    int actualQuality;
    framesize_t actualResolution;
    int brightness;
    int contrast;
    int saturation;
    bool isFixedExposure;

public:
    int imageHeight, imageWidth;
    CCamera();
    void lightOnOff(bool status);
    esp_err_t initCamera();

    // uri handlers
    void getCameraParamFromHttpRequest(httpd_req_t *req, int &qual, framesize_t &resol);
    void setQualitySize(int qual, framesize_t resol);
    esp_err_t captureImgAndResToHTTP(httpd_req_t *req, int delay = 0);
};

extern CCamera cameraESP;

#endif