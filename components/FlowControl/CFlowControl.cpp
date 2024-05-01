#include "CFlowControl.h"

#include "connect_wlan.h"
#include "freertos/task.h"

#include <sys/stat.h>
#include <dirent.h>
#include "time_sntp.h"
#include "helper.h"

#include "ServerHelper.h"

static const char* TAG = "flow_controll";


std::string CFlowControl::doSingleStep(std::string _stepname, std::string _host){
    std::string _classname = "";
    std::string result = "";
    if ((_stepname.compare("[MakeImage]") == 0) || (_stepname.compare(";[MakeImage]") == 0)){
        _classname = "CFlowMakeImage";
    }
    if ((_stepname.compare("[Alignment]") == 0) || (_stepname.compare(";[Alignment]") == 0)){
        _classname = "CFlowAlignment";
    }
    if ((_stepname.compare("[Digits]") == 0) || (_stepname.compare(";[Digits]") == 0)){
        _classname = "CFlowDigit";
    }
    if ((_stepname.compare("[Analog]") == 0) || (_stepname.compare(";[Analog]") == 0)){
        _classname = "CFlowAnalog";
    }
    if ((_stepname.compare("[MQTT]") == 0) || (_stepname.compare(";[MQTT]") == 0)){
        _classname = "CFlowMQTT";
    }

    for (int i = 0; i < FlowControll.size(); ++i)
        if (FlowControll[i]->name().compare(_classname) == 0){
            if (!(FlowControll[i]->name().compare("CFlowMakeImage") == 0))      // falls es ein MakeImage ist, braucht das Bild nicht extra aufgenommen zu werden, dass passiert bei html-Abfrage automatisch
                FlowControll[i]->doFlow("");
            result = FlowControll[i]->getHTMLSingleStep(_host);
        }

    return result;
}

std::vector<HTMLInfo*> CFlowControl::GetAllDigital()
{
    for (int i = 0; i < FlowControll.size(); ++i)
        if (FlowControll[i]->name().compare("CFlowDigit") == 0)
            return ((CFlowDigit*) (FlowControll[i]))->GetHTMLInfo();

    std::vector<HTMLInfo*> empty;
    return empty;
}

void CFlowControl::SetInitialParameter(void)
{
    AutoStart = false;
    SetupModeActive = false;
    AutoIntervall = 10;
    flowdigit = NULL;
    flowpostprocessing = NULL;
    disabled = false;
    aktRunNr = 0;
    aktstatus = "Startup";

}

bool CFlowControl::isAutoStart(long &_intervall)
{
    _intervall = AutoIntervall * 60 * 1000; // AutoIntervall: Minuten -> ms
    return AutoStart;
}

CFlow* CFlowControl::CreateCFlow(std::string _type)
{
    CFlow* cfc = NULL;

    _type = trim(_type);

    if (toUpper(_type).compare("[MAKEIMAGE]") == 0)
    {
        cfc = new CFlowMakeImage(&FlowControll);
        flowmakeimage = (CFlowMakeImage*) cfc;
    }
    if (toUpper(_type).compare("[ALIGNMENT]") == 0)
    {
        cfc = new CFlowAlignment(&FlowControll);
        flowalignment = (CFlowAlignment*) cfc;
    }
    if (toUpper(_type).compare("[DIGITS]") == 0)
    {
        cfc = new CFlowDigit(&FlowControll);
        flowdigit = (CFlowDigit*) cfc;
    }
    if (toUpper(_type).compare("[MQTT]") == 0)
        // cfc = new CFlowMQTT(&FlowControll);
    if (toUpper(_type).compare("[POSTPROCESSING]") == 0)
    {
        cfc = new CFlowPostProcessing(&FlowControll); 
        flowpostprocessing = (CFlowPostProcessing*) cfc;
    }

    if (cfc)                            // Wird nur angehangen, falls es nicht [AutoTimer] ist, denn dieses ist für FlowControll
        FlowControll.push_back(cfc);

    if (toUpper(_type).compare("[AUTOTIMER]") == 0)
        cfc = this;    

    if (toUpper(_type).compare("[DEBUG]") == 0)
        cfc = this;  

    if (toUpper(_type).compare("[SYSTEM]") == 0)
        cfc = this;          

    return cfc;
}

void CFlowControl::InitFlow(std::string config)
{
    string line;

    flowpostprocessing = NULL;

    CFlow* cfc;
    FILE* pFile;
    config = FormatFileName(config);
    pFile = OpenFileAndWait(config.c_str(), "r");

    line = "";

    char zw[1024];
    if (pFile != NULL)
    {
        fgets(zw, 1024, pFile);
        printf("%s", zw);
        line = std::string(zw);
    }

    while ((line.size() > 0) && !(feof(pFile)))
    {
        // create flows base on config.ini
        cfc = CreateCFlow(line);
        if (cfc)
        {
            printf("Start ReadParameter\n");
            cfc->ReadParameter(pFile, line);
        }
        else
        {
            fgets(zw, 1024, pFile);
            printf("%s", zw);
            line = std::string(zw);
        }
    }

    fclose(pFile);

}

std::string CFlowControl::getActStatus(){
    return aktstatus;
}

void CFlowControl::doFlowMakeImageOnly(string time){
    std::string zw_time;

    for (int i = 0; i < FlowControll.size(); ++i)
    {
        if (FlowControll[i]->name() == "CFlowMakeImage") {
            zw_time = getTimeString("%Y%m%d-%H%M%S");
            aktstatus = zw_time + ": " + FlowControll[i]->name();
            string zw = "FlowControll.doFlowMakeImageOnly - " + FlowControll[i]->name();
            FlowControll[i]->doFlow(time);
        }
    }
}

bool CFlowControl::doFlow(string time)
{
//    CleanTempFolder();            // dazu muss man noch eine Rolling einführen

    bool result = true;
    std::string zw_time;
    int repeat = 0;

    for (int i = 0; i < FlowControll.size(); ++i)
    {
        zw_time = getTimeString("%Y%m%d-%H%M%S");
        aktstatus = zw_time + ": " + FlowControll[i]->name();
        
       
        string zw = "FlowControll.doFlow - " + FlowControll[i]->name();

        if (!FlowControll[i]->doFlow(time)){
            repeat++;
            i = -1;    // vorheriger Schritt muss wiederholt werden (vermutlich Bilder aufnehmen)
            result = false;
            if (repeat > 5) {
                // TODO :
                // doReboot();
                // Schritt wurde 5x wiederholt --> reboot
            }
        }
        else
        {
            result = true;
        }
    }
    zw_time = getTimeString("%Y%m%d-%H%M%S");    
    aktstatus = zw_time + ": Flow is done";
    return result;
}

void CFlowControl::UpdateAktStatus(std::string _flow)
{
    aktstatus = getTimeString("%Y%m%d-%H%M%S");
    aktstatus = aktstatus + "\t" + std::to_string(aktRunNr) + "\t";
    
    if (_flow == "CFlowMakeImage")
        aktstatus = aktstatus + "Taking Raw Image";
    else
        if (_flow == "CFlowAlignment")
            aktstatus = aktstatus + "Aligning Image";


}


string CFlowControl::getReadout(bool _rawvalue = false, bool _noerror = false)
{
    if (flowpostprocessing)
        return flowpostprocessing->getReadoutParam(_rawvalue, _noerror);

    string zw = "";
    string result = "";

    for (int i = 0; i < FlowControll.size(); ++i)
    {
        zw = FlowControll[i]->getReadout();
        if (zw.length() > 0)
        {
            if (result.length() == 0)
                result = zw;
            else
                result = result + "\t" + zw;
        }
    }

    return result;
}

string CFlowControl::GetPrevalue()	
{
    if (flowpostprocessing)
    {
        return flowpostprocessing->GetPreValue();   
    }

    return std::string();    
}

std::string CFlowControl::UpdatePrevalue(std::string _newvalue)
{
    float zw;
    char* p;

    _newvalue = trim(_newvalue);
//    printf("Input UpdatePreValue: %s\n", _newvalue.c_str());

    if (_newvalue.compare("0.0") == 0)
    {
        zw = 0;
    }
    else
    {
        zw = strtof(_newvalue.c_str(), &p);
        if (zw == 0)
            return "- Error in String to Value Conversion!!! Must be of format value=123.456";
    }
    

    if (flowpostprocessing)
    {
        flowpostprocessing->SavePreValue(zw);
        return _newvalue;    
    }

    return std::string();
}

bool CFlowControl::ReadParameter(FILE* pfile, string& aktparamgraph)
{
    std::vector<string> zerlegt;

    aktparamgraph = trim(aktparamgraph);

    if (aktparamgraph.size() == 0)
        if (!this->GetNextParagraph(pfile, aktparamgraph))
            return false;


    if ((toUpper(aktparamgraph).compare("[AUTOTIMER]") != 0) && (toUpper(aktparamgraph).compare("[DEBUG]") != 0) && (toUpper(aktparamgraph).compare("[SYSTEM]") != 0))      // Paragraph passt nicht zu MakeImage
        return false;

    while (this->getNextLine(pfile, &aktparamgraph) && !this->isNewParagraph(aktparamgraph))
    {
        zerlegt = this->ZerlegeZeile(aktparamgraph, " =");
        if ((toUpper(zerlegt[0]) == "AUTOSTART") && (zerlegt.size() > 1))
        {
            if (toUpper(zerlegt[1]) == "TRUE")
            {
                AutoStart = true;
            }
        }
        if ((toUpper(zerlegt[0]) == "INTERVALL") && (zerlegt.size() > 1))
        {
            AutoIntervall = std::stof(zerlegt[1]);
        }
        if ((toUpper(zerlegt[0]) == "LOGFILE") && (zerlegt.size() > 1))
        {
            if (toUpper(zerlegt[1]) == "TRUE")
            {
            }
            if (toUpper(zerlegt[1]) == "FALSE")
            {
            }
        }
        if ((toUpper(zerlegt[0]) == "LOGFILERETENTIONINDAYS") && (zerlegt.size() > 1))
        {
        }      

        if ((toUpper(zerlegt[0]) == "TIMEZONE") && (zerlegt.size() > 1))
        {
            string zw = "Set TimeZone: " + zerlegt[1];
            setTimeZone(zerlegt[1]);
        }      

        if ((toUpper(zerlegt[0]) == "TIMESERVER") && (zerlegt.size() > 1))
        {
            string zw = "Set TimeZone: " + zerlegt[1];
            // reset_servername(zerlegt[1]);    ! TODO:
        }  

        if ((toUpper(zerlegt[0]) == "HOSTNAME") && (zerlegt.size() > 1))
        {
            // !TODO:
            // if (ChangeHostName("/sdcard/wlan.ini", zerlegt[1]))
            // {
            //     // reboot notwendig damit die neue wlan.ini auch benutzt wird !!!
            //     fclose(pfile);
            //     printf("do reboot\n");
            //     esp_restart();
            //     hard_restart();                   
            //     doReboot();
            // }
        }

        if ((toUpper(zerlegt[0]) == "SETUPMODE") && (zerlegt.size() > 1))
        {
            if (toUpper(zerlegt[1]) == "TRUE")
            {
                SetupModeActive = true;
            }        
        }      

        if ((toUpper(zerlegt[0]) == "LOGLEVEL") && (zerlegt.size() > 1))
        {
        }      
    }
    return true;
}

int CFlowControl::CleanTempFolder() {
    const char* folderPath = "/sdcard/img_tmp";
    
    ESP_LOGI(TAG, "Clean up temporary folder to avoid damage of sdcard sectors : %s", folderPath);
    DIR *dir = opendir(folderPath);
    if (!dir) {
        ESP_LOGE(TAG, "Failed to stat dir : %s", folderPath);
        return -1;
    }

    struct dirent *entry;
    int deleted = 0;
    while ((entry = readdir(dir)) != NULL) {
        std::string path = string(folderPath) + "/" + entry->d_name;
		if (entry->d_type == DT_REG) {
			if (unlink(path.c_str()) == 0) {
				deleted ++;
			} else {
				ESP_LOGE(TAG, "can't delete file : %s", path.c_str());
			}
        } else if (entry->d_type == DT_DIR) {
			deleted += removeFolder(path.c_str(), TAG);
		}
    }
    closedir(dir);
    ESP_LOGI(TAG, "%d files deleted", deleted);
    
    return 0;
}


esp_err_t CFlowControl::SendRawJPG(httpd_req_t *req)
{
    return flowmakeimage->SendRawJPG(req);
}


esp_err_t CFlowControl::GetJPGStream(std::string _fn, httpd_req_t *req)
{
    printf("CFlowControl::GetJPGStream %s\n", _fn.c_str());

    CImageBasis *_send = NULL;
    esp_err_t result = ESP_FAIL;
    bool Dodelete = false;    

    if (_fn == "alg.jpg")
    {
        _send = flowalignment->ImageBasis;  
    }



    if (_fn == "alg_roi.jpg")
    {
        CImageBasis* _imgzw = new CImageBasis(flowalignment->ImageBasis);
        flowalignment->DrawRef(_imgzw);
        if (flowdigit) flowdigit->DrawROI(_imgzw);
        _send = _imgzw;
        Dodelete = true;
    }

    std::vector<HTMLInfo*> htmlinfo;
    htmlinfo = GetAllDigital();
    for (int i = 0; i < htmlinfo.size(); ++i)
    {
        if (_fn == htmlinfo[i]->filename)
        {
            if (htmlinfo[i]->image)
                _send = htmlinfo[i]->image;
        }
        if (_fn == htmlinfo[i]->filename_org)
        {
            if (htmlinfo[i]->image_org)
                _send = htmlinfo[i]->image_org;        
        }
    }

    htmlinfo = GetAllAnalog();
    for (int i = 0; i < htmlinfo.size(); ++i)
    {
        if (_fn == htmlinfo[i]->filename)
        {
            if (htmlinfo[i]->image)
                _send = htmlinfo[i]->image;
        }
        if (_fn == htmlinfo[i]->filename_org)
        {
            if (htmlinfo[i]->image_org)
                _send = htmlinfo[i]->image_org;        
        }
    }

    if (_send)
    {
        ESP_LOGI(TAG, "Sending file : %s ...", _fn.c_str());
        setContentTypeFromFile(req, _fn.c_str());
        result = _send->SendJPGtoHTTP(req);
        ESP_LOGI(TAG, "File sending complete");    
        /* Respond with an empty chunk to signal HTTP response completion */
        httpd_resp_send_chunk(req, NULL, 0);
    }

    if (Dodelete) 
    {
        delete _send;
    }

    return result;
}