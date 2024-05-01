#include "CFlowMakeImage.h"
#include "Helper.h"

#include "CImageBasis.h"
#include "CCamera.h"

#include <time.h>

// #define DEBUG_DETAIL_ON 

static const char* TAG = "flow_make_image";

esp_err_t CFlowMakeImage::camera_capture(){
    string nm =  namerawimage;
    cameraESP.captureAndSaveToFile(nm);
    return ESP_OK;
}

void CFlowMakeImage::takePictureWithFlash(int flashdauer)
{
    cameraESP.captureToBasisImage(rawImage, flashdauer);
    if (SaveAllFiles) rawImage->SaveToFile(namerawimage);
}

void CFlowMakeImage::SetInitialParameter(void)
{
    waitbeforepicture = 5;
    isImageSize = false;
    ImageQuality = -1;    
    TimeImageTaken = 0;
    ImageQuality = 5;
    rawImage = NULL;
    ImageSize = FRAMESIZE_VGA;
    SaveAllFiles = false;
    disabled = false;
    FixedExposure = false;
    namerawimage =  "/sdcard/img_tmp/raw.jpg";
}     


CFlowMakeImage::CFlowMakeImage(std::vector<CFlow*>* lfc) : CFlowImage(lfc, TAG)
{
    SetInitialParameter();
}

bool CFlowMakeImage::ReadParameter(FILE* pfile, string& aktparamgraph)
{
    std::vector<string> zerlegt;

    aktparamgraph = trim(aktparamgraph);
    int _brightness = -100;
    int _contrast = -100;
    int _saturation = -100;

    if (aktparamgraph.size() == 0)
        if (!this->GetNextParagraph(pfile, aktparamgraph))
            return false;

    if (aktparamgraph.compare("[MakeImage]") != 0)       // Paragraph passt nich zu MakeImage
        return false;

    while (this->getNextLine(pfile, &aktparamgraph) && !this->isNewParagraph(aktparamgraph))
    {
        zerlegt = this->ZerlegeZeile(aktparamgraph);
        if ((zerlegt[0] ==  "LogImageLocation") && (zerlegt.size() > 1))
        {
            LogImageLocation = "/sdcard" + zerlegt[1];
            isLogImage = true;
        }
        if ((zerlegt[0] == "ImageQuality") && (zerlegt.size() > 1))
            ImageQuality = std::stod(zerlegt[1]);

        if ((zerlegt[0] == "ImageSize") && (zerlegt.size() > 1))
        {
            ImageSize = cameraESP.textToFramesize(zerlegt[1].c_str());
            isImageSize = true;
        }

        if ((toUpper(zerlegt[0]) == "SAVEALLFILES") && (zerlegt.size() > 1))
        {
            if (toUpper(zerlegt[1]) == "TRUE")
                SaveAllFiles = true;
        }

        if ((toUpper(zerlegt[0]) == "BRIGHTNESS") && (zerlegt.size() > 1))
        {
            _brightness = stoi(zerlegt[1]);
        }

        if ((toUpper(zerlegt[0]) == "CONTRAST") && (zerlegt.size() > 1))
        {
            _contrast = stoi(zerlegt[1]);
        }

        if ((toUpper(zerlegt[0]) == "SATURATION") && (zerlegt.size() > 1))
        {
            _saturation = stoi(zerlegt[1]);
        }

        if ((toUpper(zerlegt[0]) == "FIXEDEXPOSURE") && (zerlegt.size() > 1))
        {
            if (toUpper(zerlegt[1]) == "TRUE")
                FixedExposure = true;
        }
    }

    cameraESP.setBrightnessContrastSaturation(_brightness, _contrast, _saturation);
    cameraESP.setQualitySize(ImageQuality, ImageSize);

    image_width = cameraESP.imageWidth;
    image_height = cameraESP.imageHeight;
    rawImage = new CImageBasis();
    rawImage->CreateEmptyImage(image_width, image_height, 3);

    waitbeforepicture_store = waitbeforepicture;
    if (FixedExposure)
    {
        printf("Fixed Exposure enabled!\n");
        int flashdauer = (int) (waitbeforepicture * 1000);
        cameraESP.enableAutoExposure(flashdauer);
        waitbeforepicture = 0.2;
    }

    return true;
}

string CFlowMakeImage::getHTMLSingleStep(string host)
{
    string result;
    result = "Raw Image: <br>\n<img src=\"" + host + "/img_tmp/raw.jpg\">\n";
    return result;
}

bool CFlowMakeImage::doFlow(string zwtime)
{
    string logPath = CreateLogFolder(zwtime);

    int flashdauer = (int) (waitbeforepicture * 1000);

    takePictureWithFlash(flashdauer);

    LogImage(logPath, "raw", NULL, NULL, zwtime, rawImage);

    RemoveOldLogs();

    return true;
}

esp_err_t CFlowMakeImage::SendRawJPG(httpd_req_t *req)
{
    int flashdauer = (int) (waitbeforepicture * 1000);
    return cameraESP.captureImgAndResToHTTP(req, flashdauer);
}


ImageData* CFlowMakeImage::SendRawImage()
{
    CImageBasis *zw = new CImageBasis(rawImage);
    ImageData *id;
    int flashdauer = (int) (waitbeforepicture * 1000);
    cameraESP.captureToBasisImage(zw, flashdauer);
    id = zw->writeToMemoryAsJPG();    
    delete zw;
    return id;  
}

time_t CFlowMakeImage::getTimeImageTaken()
{
    return TimeImageTaken;
}

CFlowMakeImage::~CFlowMakeImage(void)
{
    delete rawImage;
}

