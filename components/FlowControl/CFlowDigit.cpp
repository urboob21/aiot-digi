#include "CFlowDigit.h"

#include "CTfLiteClass.h"

static const char *TAG = "flow_digital";

void CFlowDigit::SetInitialParameter(void)
{
    string cnnmodelfile = "";
    modelxsize = 1;
    modelysize = 1;
    ListFlowControll = NULL;
    previousElement = NULL;
    SaveAllFiles = false;
    disabled = false;
}

CFlowDigit::CFlowDigit() : CFlowImage(TAG)
{
    SetInitialParameter();
}

CFlowDigit::CFlowDigit(std::vector<CFlow *> *lfc) : CFlowImage(lfc, TAG)
{
    SetInitialParameter();
    ListFlowControll = lfc;

    for (int i = 0; i < ListFlowControll->size(); ++i)
    {
        if (((*ListFlowControll)[i])->name().compare("CFlowAlignment") == 0)
        {
            flowpostalignment = (CFlowAlignment *)(*ListFlowControll)[i];
        }
    }
}

CFlowDigit::CFlowDigit(std::vector<CFlow *> *lfc, CFlow *_prev) : CFlowImage(lfc, _prev, TAG)
{
    SetInitialParameter();
    ListFlowControll = lfc;
    previousElement = _prev;

    for (int i = 0; i < ListFlowControll->size(); ++i)
    {
        if (((*ListFlowControll)[i])->name().compare("CFlowAlignment") == 0)
        {
            flowpostalignment = (CFlowAlignment *)(*ListFlowControll)[i];
        }
    }
}

string CFlowDigit::getReadout()
{
    string rst = "";

    for (int i = 0; i < ROI.size(); ++i)
    {
        if (ROI[i]->resultklasse == 10)
            rst = rst + "N";
        else
            rst = rst + std::to_string(ROI[i]->resultklasse);
    }

    return rst;
}

bool CFlowDigit::ReadParameter(FILE *pfile, string &aktparamgraph)
{
    std::vector<string> zerlegt;

    aktparamgraph = trim(aktparamgraph);

    if (aktparamgraph.size() == 0)
        if (!this->GetNextParagraph(pfile, aktparamgraph))
            return false;

    if ((aktparamgraph.compare("[Digits]") != 0) && (aktparamgraph.compare(";[Digits]") != 0)) // Paragraph passt nich zu MakeImage
        return false;

    if (aktparamgraph[0] == ';')
    {
        disabled = true;
        while (getNextLine(pfile, &aktparamgraph) && !isNewParagraph(aktparamgraph))
            ;
        printf("[Digits] is disabled !!!\n");
        return true;
    }

    while (getNextLine(pfile, &aktparamgraph) && !isNewParagraph(aktparamgraph))
    {
        zerlegt = this->ZerlegeZeile(aktparamgraph);
        if ((zerlegt[0] == "LogImageLocation") && (zerlegt.size() > 1))
        {
            LogImageLocation = "/sdcard" + zerlegt[1];
            isLogImage = true;
        }
        if ((zerlegt[0] == "Model") && (zerlegt.size() > 1))
        {
            cnnmodelfile = zerlegt[1];
        }
        if ((zerlegt[0] == "ModelInputSize") && (zerlegt.size() > 2))
        {
            modelxsize = std::stoi(zerlegt[1]);
            modelysize = std::stoi(zerlegt[2]);
        }
        if (zerlegt.size() >= 5)
        {
            roi *neuroi = new roi;
            neuroi->name = zerlegt[0];
            neuroi->posx = std::stoi(zerlegt[1]);
            neuroi->posy = std::stoi(zerlegt[2]);
            neuroi->deltax = std::stoi(zerlegt[3]);
            neuroi->deltay = std::stoi(zerlegt[4]);
            neuroi->resultklasse = -1;
            neuroi->image = NULL;
            neuroi->image_org = NULL;
            ROI.push_back(neuroi);
        }

        if ((toUpper(zerlegt[0]) == "SAVEALLFILES") && (zerlegt.size() > 1))
        {
            if (toUpper(zerlegt[1]) == "TRUE")
                SaveAllFiles = true;
        }
    }

    for (int i = 0; i < ROI.size(); ++i)
    {
        ROI[i]->image = new CImageBasis(modelxsize, modelysize, 3);
        ROI[i]->image_org = new CImageBasis(ROI[i]->deltax, ROI[i]->deltay, 3);
    }

    return true;
}

string CFlowDigit::getHTMLSingleStep(string host)
{
    string result, zw;
    std::vector<HTMLInfo *> htmlinfo;

    result = "<p>Found ROIs: </p> <p><img src=\"" + host + "/img_tmp/alg_roi.jpg\"></p>\n";
    result = result + "Digital Counter: <p> ";

    htmlinfo = GetHTMLInfo();
    for (int i = 0; i < htmlinfo.size(); ++i)
    {
        if (htmlinfo[i]->val == 10)
            zw = "NaN";
        else
        {
            zw = to_string((int)htmlinfo[i]->val);
        }
        result = result + "<img src=\"" + host + "/img_tmp/" + htmlinfo[i]->filename + "\"> " + zw;
        delete htmlinfo[i];
    }
    htmlinfo.clear();

    return result;
}

bool CFlowDigit::doFlow(string time)
{
    if (disabled)
        return true;

    if (!doAlignAndCut(time))
    {
        return false;
    };

    doNeuralNetwork(time);

    RemoveOldLogs();

    return true;
}

bool CFlowDigit::doAlignAndCut(string time)
{
    if (disabled)
        return true;

    CAlignAndCutImage *caic = flowpostalignment->GetAlignAndCutImage();

    for (int i = 0; i < ROI.size(); ++i)
    {
        printf("DigitalDigit %d - Align&Cut\n", i);

        caic->CutAndSave(ROI[i]->posx, ROI[i]->posy, ROI[i]->deltax, ROI[i]->deltay, ROI[i]->image_org);
        if (SaveAllFiles)
            ROI[i]->image_org->SaveToFile(FormatFileName("/sdcard/img_tmp/" + ROI[i]->name + ".jpg"));

        ROI[i]->image_org->Resize(modelxsize, modelysize, ROI[i]->image);
        if (SaveAllFiles)
            ROI[i]->image->SaveToFile(FormatFileName("/sdcard/img_tmp/" + ROI[i]->name + ".bmp"));
    }

    return true;
}

bool CFlowDigit::doNeuralNetwork(string time)
{
    if (disabled)
        return true;

    string logPath = CreateLogFolder(time);

    CTfLiteClass *tflite = new CTfLiteClass;
    string zwcnn = FormatFileName("/sdcard" + cnnmodelfile);
    printf(zwcnn.c_str());
    printf("\n");
    tflite->LoadModel(zwcnn);
    tflite->MakeAllocate();

    for (int i = 0; i < ROI.size(); ++i)
    {
        printf("DigitalDigit %d - TfLite\n", i);

        ROI[i]->resultklasse = 0;
        ROI[i]->resultklasse = tflite->GetClassFromImageBasis(ROI[i]->image);

        printf("Result Digit%i: %d\n", i, ROI[i]->resultklasse);

        if (isLogImage)
        {
            LogImage(logPath, ROI[i]->name, NULL, &ROI[i]->resultklasse, time, ROI[i]->image_org);
        }
    }
    delete tflite;
    return true;
}

void CFlowDigit::DrawROI(CImageBasis *_zw)
{
    for (int i = 0; i < ROI.size(); ++i)
        _zw->drawRect(ROI[i]->posx, ROI[i]->posy, ROI[i]->deltax, ROI[i]->deltay, 0, 0, 255, 2);
}

std::vector<HTMLInfo *> CFlowDigit::GetHTMLInfo()
{
    std::vector<HTMLInfo *> result;

    for (int i = 0; i < ROI.size(); ++i)
    {
        HTMLInfo *zw = new HTMLInfo;
        zw->filename = ROI[i]->name + ".bmp";
        zw->filename_org = ROI[i]->name + ".jpg";
        zw->val = ROI[i]->resultklasse;
        zw->image = ROI[i]->image;
        zw->image_org = ROI[i]->image_org;
        result.push_back(zw);
    }

    return result;
}
