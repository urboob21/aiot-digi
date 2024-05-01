#include "server_file.h"


#include <stdio.h>
#include <string.h>
#include <string>
#include <sys/param.h>
#include <sys/unistd.h>
#include <sys/stat.h>
#include <dirent.h>

#include "esp_err.h"
#include "esp_log.h"

#include "esp_vfs.h"
#include <esp_spiffs.h>
#include "esp_http_server.h"
#include "ServerHelper.h"

#include "Helper.h"
#include "miniz.h"

void registerServerFileUri(httpd_handle_t server, const char *base_path){

}