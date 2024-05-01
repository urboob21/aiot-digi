#pragma once
#include <esp_http_server.h>

static const char *TAGTFLITE = "server_tflite";


void startTFLiteFlow();
void registerServerTFliteUri(httpd_handle_t server);
