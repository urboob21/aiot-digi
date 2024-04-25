#pragma once
#include <esp_http_server.h>
#include <esp_log.h>
#include <driver/gpio.h>
static const char* TAG_SERVER_CAMERA = "T_server_camera";

// Reset PIN32 which connected with the power CAM pin
void powerResetCamera();

void registerServerCameraUri(httpd_handle_t server);
