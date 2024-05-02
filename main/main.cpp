#include <server_camera.h>
#include <CCamera.h>
#include "sdcard.h"
#include <esp_log.h>
#include "connect_wlan.h"
#include "time_sntp.h"
#include "server_main.h"
#include "server_file.h"
#include <server_tflite.h>
#include <CImageBasis.h>
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
    // initTheContentSDCard();

    // Load Station config from SDCard
    loadWlanFromFile("/sdcard/wlan.ini");

    // Set STATION MODE - connet to another WIFI local
    connectToWLAN();

    // Init the VietNam timezone
    setupTime();

    // HTTP Server
    server = startHTTPWebserver();
    registerServerCameraUri(server); // register server with uri camera
    registerServerTFliteUri(server);    
    registerServerFileUri(server, "/sdcard"); // handle server with file
    registerServerMainUri(server, "/sdcard"); // this match with all URIs GET
    printf("Debug1\n");
    startTFLiteFlow();
}
