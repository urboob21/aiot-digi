#ifndef _SERVER_FILE_H

#define _SERVER_FILE_H
#include <esp_http_server.h>

void registerServerFileUri(httpd_handle_t server, const char *base_path);

#endif