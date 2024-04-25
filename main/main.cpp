#include <server_camera.h>
#include <CCamera.h>
#include "sdcard.h"
#include <esp_log.h>

CCamera cameraESP;

extern "C" void app_main()
{
    // 1. Settup ESP32CAM
    printf("Reset Camera\n");
    powerResetCamera();
    cameraESP.initCamera();
    cameraESP.lightOnOff(false); // turn off the light

    // 2. Settup SDCard
    if (!initNVS_SDCard())
    {
        xTaskCreate(&taskNoSDBlink, "task_NoSDBlink", configMINIMAL_STACK_SIZE * 64, NULL, tskIDLE_PRIORITY + 1, NULL);
        return;
    }

    // 3. Load Station config from SDCard

    // 4. Create the Wifi Station Mode

    // 5. Create HTTP Server

    // 6. Config via local HTTP server to config file

    // 7. Done Init

    // 8. Loop take the picture + MQTT task + Loop Server

    
}
