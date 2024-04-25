#include "server_camera.h"
#include <string.h>

#include <hal/gpio_ll.h>
void powerResetCamera()
{
    ESP_LOGD(TAG_SERVER_CAMERA, "Resetting camera by power down line PIN32");
    gpio_config_t conf = {0};
    conf.pin_bit_mask = 1LL << GPIO_NUM_32;
    conf.mode = GPIO_MODE_OUTPUT;
    gpio_config(&conf);

    // carefull, logic is inverted compared to reset pin
    gpio_set_level(GPIO_NUM_32, 1);
    vTaskDelay(1000 / portTICK_PERIOD_MS);
    gpio_set_level(GPIO_NUM_32, 0);
    vTaskDelay(1000 / portTICK_PERIOD_MS);
}

void registerServerCameraUri(httpd_handle_t server)
{
    
}
