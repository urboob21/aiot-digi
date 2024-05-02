#include "server_tflite.h"
#include <string>
#include <vector>
#include "string.h"
#include "esp_log.h"
#include <iomanip>
#include <sstream>
#include "Helper.h"
#include "esp_camera.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "CFlowControl.h"
#include "time_sntp.h"
#include "esp_timer.h"
#include "connect_wlan.h"

long autoIntervall = 0;
bool autoIsrunning = false;
int countRounds = 0;
bool flowIsRunning = false;

TaskHandle_t xHandletaskAutodoFlow = NULL;
TaskHandle_t xHandleblink_task_doFlow = NULL;

CFlowControl tfliteFlow;

// load the params from config.ini
// init the tensor flow lite
/*
[MakeImage]
;LogImageLocation = /log/source
;LogfileRetentionInDays = 15
WaitBeforeTakingPicture = 5
ImageQuality = 5
ImageSize = VGA
;Brightness = -2
FixedExposure = false

[Alignment]
InitialRotate=180
/config/ref0.jpg 119 273
/config/ref1.jpg 456 138
SearchFieldX = 20
SearchFieldY = 20
InitialMirror= false
AlignmentAlgo = Default

[Digits]
Model = /config/dig0820s2q.tflite
;LogImageLocation = /log/digit
;LogfileRetentionInDays = 3
ModelInputSize = 20 32
digit1 306 120 37 67
digit2 355 120 37 67
digit3 404 120 37 67

[Analog]
Model = /config/ana0700s1lq.tflite
;LogImageLocation = /log/analog
;LogfileRetentionInDays = 3
ModelInputSize = 32 32
analog1 444 225 92 92
analog2 391 329 92 92
analog3 294 369 92 92
analog4 168 326 92 92
ExtendedResolution = false

[PostProcessing]
DecimalShift = 0
PreValueUse = true
PreValueAgeStartup = 720
AllowNegativeRates = false
MaxRateValue = 0.1
ErrorMessage = true
CheckDigitIncreaseConsistency = false

[MQTT]
;Uri = mqtt://IP-ADRESS:1883
;Topic = wasserzaehler/zaehlerstand
;TopicError = wasserzaehler/error
;ClientID = wasser
;user = USERNAME
;password = PASSWORD

[AutoTimer]
AutoStart = true
Intervall = 4.85

[Debug]
Logfile = false
LogfileRetentionInDays = 3

[System]
TimeZone = CET-1CEST,M3.5.0,M10.5.0/3
;TimeServer = fritz.box
;hostname = watermeter
SetupMode = true

[Ende]
*/
void doInit(void)
{
    string config = "/sdcard/config/config.ini";
    tfliteFlow.InitFlow(config);
}

bool doFlow(void)
{
    std::string zw_time = getTimeString(LOGFILE_TIME_FORMAT);
    printf("doflow - start %s\n", zw_time.c_str());
    flowIsRunning = true;
    tfliteFlow.doFlow(zw_time);
    flowIsRunning = false;

    return true;
}

void blink_task_doFlow(void *pvParameter)
{
    if (!flowIsRunning)
    {
        flowIsRunning = true;
        doFlow();
        flowIsRunning = false;
    }
    vTaskDelete(NULL); // Delete this task if it exits from the loop above
    xHandleblink_task_doFlow = NULL;
}

void taskAutodoFlow(void *pvParameter)
{
    int64_t fr_start, fr_delta_ms;

    // Init the flows
    doInit();

    autoIsrunning = tfliteFlow.isAutoStart(autoIntervall);

    if (isSetupModusActive())
    {
        autoIsrunning = false;
        std::string zw_time = "";
        tfliteFlow.doFlowMakeImageOnly(zw_time);
    }

    while (autoIsrunning)
    {
        std::string _zw = "task_autodoFlow - next round - Round #" + std::to_string(++countRounds);
        printf("Autoflow: start\n");
        fr_start = esp_timer_get_time();
        if (!flowIsRunning)
        {
            flowIsRunning = true;
            // do the flows
            doFlow();
        }

        // delay time
        fr_delta_ms = (esp_timer_get_time() - fr_start) / 1000;
        if (autoIntervall > fr_delta_ms)
        {
            const TickType_t xDelay = (autoIntervall - fr_delta_ms) / portTICK_PERIOD_MS;
            printf("Autoflow: sleep for : %ldms\n", (long)xDelay);
            vTaskDelay(xDelay);
        }
    }
        printf("Debug2\n");
    
    vTaskDelete(NULL); // Delete this task if it exits from the loop above
    xHandletaskAutodoFlow = NULL;
}

esp_err_t getJPG(std::string _filename, httpd_req_t *req)
{
    return tfliteFlow.GetJPGStream(_filename, req);
}

esp_err_t getRawJPG(httpd_req_t *req)
{
    return tfliteFlow.SendRawJPG(req);
}

bool isSetupModusActive() {
    return tfliteFlow.getStatusSetupModus();
    return false;
}

void startTFLiteFlow()
{
    xTaskCreate(&taskAutodoFlow, "task_autodoFlow", configMINIMAL_STACK_SIZE * 64, NULL, tskIDLE_PRIORITY + 1, &xHandletaskAutodoFlow);
}

// "/setPreValue.html"
esp_err_t handler_prevalue(httpd_req_t *req)
{
    const char *resp_str;
    string zw;

    char _query[100];
    char _size[10] = "";

    if (strlen(_size) == 0)
        zw = tfliteFlow.GetPrevalue();
    else
        zw = "SetPrevalue to " + tfliteFlow.UpdatePrevalue(_size);

    resp_str = zw.c_str();

    httpd_resp_send(req, resp_str, strlen(resp_str));
    /* Respond with an empty chunk to signal HTTP response completion */
    httpd_resp_send_chunk(req, NULL, 0);
    return ESP_OK;
};

// "/doinit"
esp_err_t handler_init(httpd_req_t *req)
{
    char *resp_str = "Init started<br>";
    httpd_resp_send(req, resp_str, strlen(resp_str));

    doInit();

    resp_str = "Init done<br>";
    httpd_resp_send(req, resp_str, strlen(resp_str));
    /* Respond with an empty chunk to signal HTTP response completion */
    httpd_resp_send_chunk(req, NULL, 0);
    return ESP_OK;
};

// "/doflow"
esp_err_t handler_doflow(httpd_req_t *req)
{
    char *resp_str;

    printf("handler_doFlow uri: ");
    printf(req->uri);
    printf("\n");

    if (flowIsRunning)
    {
        const char *resp_str = "doFlow läuft bereits und kann nicht nochmal gestartet werden";
        httpd_resp_send(req, resp_str, strlen(resp_str));
        return 2;
    }
    else
    {
        xTaskCreate(&blink_task_doFlow, "blink_doFlow", configMINIMAL_STACK_SIZE * 64, NULL, tskIDLE_PRIORITY + 1, &xHandleblink_task_doFlow);
    }
    resp_str = "doFlow gestartet - dauert ca. 60 Sekunden";
    httpd_resp_send(req, resp_str, strlen(resp_str));
    /* Respond with an empty chunk to signal HTTP response completion */
    httpd_resp_send_chunk(req, NULL, 0);
    return ESP_OK;
};

esp_err_t handler_editflow(httpd_req_t *req)
{
    printf("handler_editflow uri: ");
    printf(req->uri);
    printf("\n");

    char _query[200];
    char _valuechar[30];
    string _task;

    if (httpd_req_get_url_query_str(req, _query, 200) == ESP_OK)
    {
        if (httpd_query_key_value(_query, "task", _valuechar, 30) == ESP_OK)
        {
            _task = string(_valuechar);
        }
    }

    if (_task.compare("copy") == 0)
    {
        string in, out, zw;

        httpd_query_key_value(_query, "in", _valuechar, 30);
        in = string(_valuechar);
        httpd_query_key_value(_query, "out", _valuechar, 30);
        out = string(_valuechar);

        in = "/sdcard" + in;
        out = "/sdcard" + out;

        CopyFile(in, out);
        zw = "Copy Done";
        httpd_resp_sendstr_chunk(req, zw.c_str());
    }

    if (_task.compare("cutref") == 0)
    {
        string in, out, zw;
        int x, y, dx, dy;
        bool enhance = false;

        httpd_query_key_value(_query, "in", _valuechar, 30);
        in = string(_valuechar);

        httpd_query_key_value(_query, "out", _valuechar, 30);
        out = string(_valuechar);

        httpd_query_key_value(_query, "x", _valuechar, 30);
        zw = string(_valuechar);
        x = stoi(zw);

        httpd_query_key_value(_query, "y", _valuechar, 30);
        zw = string(_valuechar);
        y = stoi(zw);

        httpd_query_key_value(_query, "dx", _valuechar, 30);
        zw = string(_valuechar);
        dx = stoi(zw);

        httpd_query_key_value(_query, "dy", _valuechar, 30);
        zw = string(_valuechar);
        dy = stoi(zw);

        if (httpd_query_key_value(_query, "enhance", _valuechar, 10) == ESP_OK)
        {
            zw = string(_valuechar);
            if (zw.compare("true") == 0)
            {
                enhance = true;
            }
        }

        in = "/sdcard" + in;
        out = "/sdcard" + out;

        string out2 = out.substr(0, out.length() - 4) + "_org.jpg";

        CAlignAndCutImage *caic = new CAlignAndCutImage(in);
        caic->CutAndSave(out2, x, y, dx, dy);
        delete caic;

        CImageBasis *cim = new CImageBasis(out2);
        if (enhance)
        {
            cim->Contrast(90);
        }

        cim->SaveToFile(out);
        delete cim;

        zw = "CutImage Done";
        httpd_resp_sendstr_chunk(req, zw.c_str());
    }

    if (_task.compare("test_take") == 0)
    {
        std::string _host = "";
        std::string _bri = "";
        std::string _con = "";
        std::string _sat = "";
        int bri = -100;
        int sat = -100;
        int con = -100;

        if (httpd_query_key_value(_query, "host", _valuechar, 30) == ESP_OK)
        {
            _host = std::string(_valuechar);
        }
        if (httpd_query_key_value(_query, "bri", _valuechar, 30) == ESP_OK)
        {
            _bri = std::string(_valuechar);
            bri = stoi(_bri);
        }
        if (httpd_query_key_value(_query, "con", _valuechar, 30) == ESP_OK)
        {
            _con = std::string(_valuechar);
            con = stoi(_con);
        }
        if (httpd_query_key_value(_query, "sat", _valuechar, 30) == ESP_OK)
        {
            _sat = std::string(_valuechar);
            sat = stoi(_sat);
        }

        bool changed = cameraESP.setBrightnessContrastSaturation(bri, con, sat);
        std::string zw = tfliteFlow.doSingleStep("[MakeImage]", _host);
        httpd_resp_sendstr_chunk(req, zw.c_str());
    }

    if (_task.compare("test_align") == 0)
    {
        std::string _host = "";
        if (httpd_query_key_value(_query, "host", _valuechar, 30) == ESP_OK)
        {
            _host = std::string(_valuechar);
        }

        std::string zw = tfliteFlow.doSingleStep("[Alignment]", _host);
        httpd_resp_sendstr_chunk(req, zw.c_str());
    }
    if (_task.compare("test_analog") == 0)
    {
        std::string _host = "";
        if (httpd_query_key_value(_query, "host", _valuechar, 30) == ESP_OK)
        {
            _host = std::string(_valuechar);
        }
        std::string zw = tfliteFlow.doSingleStep("[Analog]", _host);
        httpd_resp_sendstr_chunk(req, zw.c_str());
    }
    if (_task.compare("test_digits") == 0)
    {
        std::string _host = "";
        if (httpd_query_key_value(_query, "host", _valuechar, 30) == ESP_OK)
        {
            _host = std::string(_valuechar);
        }

        std::string zw = tfliteFlow.doSingleStep("[Digits]", _host);
        httpd_resp_sendstr_chunk(req, zw.c_str());
    }

    /* Respond with an empty chunk to signal HTTP response completion */
    httpd_resp_sendstr_chunk(req, NULL);

    return ESP_OK;
};

// xử lý các url liên quan đến đồng hồ nước
esp_err_t handler_wasserzaehler(httpd_req_t *req)
{
    bool _rawValue = false;
    bool _noerror = false;
    string zw;

    printf("handler_wasserzaehler uri:\n");
    printf(req->uri);
    printf("\n");

    char _query[100];
    char _size[10];

    if (httpd_req_get_url_query_str(req, _query, 100) == ESP_OK)
    {
        if (httpd_query_key_value(_query, "rawvalue", _size, 10) == ESP_OK)
        {
            _rawValue = true;
        }
        if (httpd_query_key_value(_query, "noerror", _size, 10) == ESP_OK)
        {
            _noerror = true;
        }
    }

    zw = tfliteFlow.getReadout(_rawValue, _noerror);
    if (zw.length() > 0)
        httpd_resp_sendstr_chunk(req, zw.c_str());

    string query = std::string(_query);
    if (query.find("full") != std::string::npos)
    {
        string txt, zw;

        txt = "<p>Aligned Image: <p><img src=\"/img_tmp/alg_roi.jpg\"> <p>\n";
        txt = txt + "Digital Counter: <p> ";
        httpd_resp_sendstr_chunk(req, txt.c_str());

        std::vector<HTMLInfo *> htmlinfo;
        htmlinfo = tfliteFlow.GetAllDigital();
        for (int i = 0; i < htmlinfo.size(); ++i)
        {
            if (htmlinfo[i]->val == 10)
                zw = "NaN";
            else
            {
                zw = to_string((int)htmlinfo[i]->val);
            }
            txt = "<img src=\"/img_tmp/" + htmlinfo[i]->filename + "\"> " + zw;
            httpd_resp_sendstr_chunk(req, txt.c_str());
            delete htmlinfo[i];
        }
        htmlinfo.clear();
    }
    /* Respond with an empty chunk to signal HTTP response completion */
    httpd_resp_sendstr_chunk(req, NULL);
    return ESP_OK;
};

void registerServerTFliteUri(httpd_handle_t server)
{
    ESP_LOGI(TAGTFLITE, "server_part_camera - Registering URI handlers");

    httpd_uri_t camuri = {};
    camuri.method = HTTP_GET;

    camuri.uri = "/doinit";
    camuri.handler = handler_init;
    camuri.user_ctx = (void *)"Light On";
    httpd_register_uri_handler(server, &camuri);

    camuri.uri = "/setPreValue.html";
    camuri.handler = handler_prevalue;
    camuri.user_ctx = (void *)"Prevalue";
    httpd_register_uri_handler(server, &camuri);

    camuri.uri = "/doflow";
    camuri.handler = handler_doflow;
    camuri.user_ctx = (void *)"Light Off";
    httpd_register_uri_handler(server, &camuri);

    camuri.uri = "/editflow.html";
    camuri.handler = handler_editflow;
    camuri.user_ctx = (void *)"EditFlow";
    httpd_register_uri_handler(server, &camuri);

    camuri.uri = "/wasserzaehler.html";
    camuri.handler = handler_wasserzaehler;
    camuri.user_ctx = (void *)"Wasserzaehler";
    httpd_register_uri_handler(server, &camuri);
}