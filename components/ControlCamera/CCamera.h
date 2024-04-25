#ifndef CCAMERA_H
#define CCAMERA_H

#include <esp_err.h>
#include <esp_camera.h>
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
    CCamera();
    void lightOnOff(bool status);
    esp_err_t initCamera();
};

extern CCamera cameraESP;

#endif