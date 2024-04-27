#include <server_camera.h>
#include <CCamera.h>
#include "sdcard.h"
#include <esp_log.h>
#include "connect_wlan.h"
CCamera cameraESP;

extern "C" void app_main()
{
    // Init the CAMERA
    printf("Reset Camera\n");
    powerResetCamera();
    cameraESP.initCamera();
    cameraESP.lightOnOff(false); // turn off the light

    // Init the SDCard
    if (!initNVS_SDCard())
    {
        xTaskCreate(&taskNoSDBlink, "task_NoSDBlink", configMINIMAL_STACK_SIZE * 64, NULL, tskIDLE_PRIORITY + 1, NULL);
        return;
    }


    // Tree SD Cardfolder
    // _______________________________________________________________
    // sd-card
    // ├───test (.txt) // testing
    // ├───config  (model.tflite, config.ini, prevalue.ini )   // store config files
    // ├───wlan.ini (wifi local area network config)
    // └───html    (.js, .html, .css)  // store web server files
    initTheContentSDCard();

    // Load Station config from SDCard
    loadWlanFromFile("/sdcard/wlan.ini");

    // 4. Create the Wifi Station Mode

    // 5. Create HTTP Server

    // 6. Config via local HTTP server to config file

    // 7. Done Init

    // 8. Loop take the picture + MQTT task + Loop Server
}
