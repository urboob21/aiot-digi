#pragma once
#include <esp_http_server.h>
#include <string>

static const char *TAGTFLITE = "server_tflite";

esp_err_t getJPG(std::string _filename, httpd_req_t *req);

esp_err_t getRawJPG(httpd_req_t *req);

bool isSetupModusActive();
void startTFLiteFlow();
void registerServerTFliteUri(httpd_handle_t server);
