#pragma once
#include "CFlowImage.h"
#include "CCamera.h"

#include <string>

#define BLINK_GPIO GPIO_NUM_4

#define CAMERA_MODEL_AI_THINKER



class CFlowMakeImage :
    public CFlowImage
{
protected:
    float waitbeforepicture;
    float waitbeforepicture_store;
    framesize_t ImageSize;
    bool isImageSize;
    int ImageQuality;
    time_t TimeImageTaken;
    string namerawimage;
    int image_height, image_width;
    bool SaveAllFiles;
    bool FixedExposure;



    void CopyFile(string input, string output);

    esp_err_t camera_capture();
    void takePictureWithFlash(int flashdauer);


    void SetInitialParameter(void);       

public:
    CImageBasis *rawImage;

    CFlowMakeImage(std::vector<CFlow*>* lfc);

    bool ReadParameter(FILE* pfile, string& aktparamgraph);
    bool doFlow(string time);
    string getHTMLSingleStep(string host);
    time_t getTimeImageTaken();
    string name(){return "CFlowMakeImage";};

    ImageData* SendRawImage();
    esp_err_t SendRawJPG(httpd_req_t *req);

    ~CFlowMakeImage(void);
};


