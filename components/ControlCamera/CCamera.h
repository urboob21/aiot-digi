#ifndef CCAMERA_H
#define CCAMERA_H

#include "CImageBasis.h"
#include <esp_err.h>
#include <esp_camera.h>
#include <esp_http_server.h>
#include <string>
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
    int waitbeforepictureOrg;

public:
    int imageHeight, imageWidth;
    CCamera();
    void lightOnOff(bool status);
    esp_err_t initCamera();

    // uri handlers
    void getCameraParamFromHttpRequest(httpd_req_t *req, int &qual, framesize_t &resol);
    void setQualitySize(int qual, framesize_t resol);
    esp_err_t captureImgAndResToHTTP(httpd_req_t *req, int delay = 0);
    esp_err_t captureAndSaveToFile(std::string nm, int delay = 0);
    esp_err_t captureToBasisImage(CImageBasis *_Image, int delay = 0);

    framesize_t textToFramesize(const char *text);
    void enableAutoExposure(int flashdauer);
    bool setBrightnessContrastSaturation(int _brightness, int _contrast, int _saturation);
};

extern CCamera cameraESP;

#endif