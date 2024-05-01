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

long autoIntervall = 0;
bool autoIsrunning = false;
int countRounds = 0;
bool flowIsRunning = false;

TaskHandle_t xHandletaskAutodoFlow = NULL;
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

void taskAutodoFlow(void *pvParameter)
{
    int64_t fr_start, fr_delta_ms;

    // Init the flows
    doInit();

    autoIsrunning = tfliteFlow.isAutoStart(autoIntervall);

    // if (isSetupModusActive())
    // {
    //     auto_isrunning = false;
    //     // std::string zw_time = gettimestring(LOGFILE_TIME_FORMAT);
    //     std::string zw_time = "";
    //     tfliteflow.doFlowMakeImageOnly(zw_time);
    // }

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

    vTaskDelete(NULL); // Delete this task if it exits from the loop above
    xHandletaskAutodoFlow = NULL;
}

void startTFLiteFlow()
{
    xTaskCreate(&taskAutodoFlow, "task_autodoFlow", configMINIMAL_STACK_SIZE * 64, NULL, tskIDLE_PRIORITY + 1, &xHandletaskAutodoFlow);
}