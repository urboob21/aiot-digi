#ifndef _SERVER_FILE_H

#define _SERVER_FILE_H
#include <esp_http_server.h>
#include <string>

void registerServerFileUri(httpd_handle_t server, const char *base_path);
void deleteAllInDirectory(std::string _directory);

#endif
