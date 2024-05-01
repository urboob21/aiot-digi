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

long autoIntervall = 0;
bool autoIsrunning = false;

TaskHandle_t xHandletaskAutodoFlow = NULL;
// TFLiteFlow tfliteFlow;

// load the params from config.ini
// init the tensor flow lite
void doInit(void){
    string config = "/sdcard/config/config.ini";
    // tfliteflow.initFlow(config);
}


void taskAutodoFlow(void *pvParameter)
{
    // int64_t fr_start, fr_delta_ms;

    // doInit();

    // autoIsrunning = tfliteflow.isAutoStart(autoIntervall);

    // if (isSetupModusActive())
    // {
    //     auto_isrunning = false;
    //     // std::string zw_time = gettimestring(LOGFILE_TIME_FORMAT);
    //     std::string zw_time = "";
    //     tfliteflow.doFlowMakeImageOnly(zw_time);
    // }

    // while (autoIsrunning)
    // {
    //     std::string _zw = "task_autodoFlow - next round - Round #" + std::to_string(++countRounds);
    //     printf("Autoflow: start\n");
    //     fr_start = esp_timer_get_time();
    //     doflow();

    //     // CPU Temp
    //     float cputmp = temperatureRead();
    //     std::stringstream stream;
    //     stream << std::fixed << std::setprecision(1) << cputmp;
    //     string zwtemp = "CPU Temperature: " + stream.str();
    //     printf("CPU Temperature: %.2f\n", cputmp);
    //     fr_delta_ms = (esp_timer_get_time() - fr_start) / 1000;
    //     if (auto_intervall > fr_delta_ms)
    //     {
    //         const TickType_t xDelay = (auto_intervall - fr_delta_ms) / portTICK_PERIOD_MS;
    //         printf("Autoflow: sleep for : %ldms\n", (long)xDelay);
    //         vTaskDelay(xDelay);
    //     }
    // }


    // vTaskDelete(NULL); // Delete this task if it exits from the loop above
    // xHandletaskAutodoFlow = NULL;
}

void startTFLiteFlow()
{
    xTaskCreate(&taskAutodoFlow, "task_autodoFlow", configMINIMAL_STACK_SIZE * 64, NULL, tskIDLE_PRIORITY + 1, &xHandletaskAutodoFlow);
}