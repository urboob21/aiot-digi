#pragma once
#include <string>
#include "esp_http_server.h"


const char* getPathFromUri(char *dest, const char *base_path, const char *uri, size_t destsize);

esp_err_t sendFile(httpd_req_t *req, std::string filename);

esp_err_t setContentTypeFromFile(httpd_req_t *req, const char *filename);
