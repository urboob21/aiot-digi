#pragma once
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <esp_log.h>
#include <nvs_flash.h>
#include <sys/param.h>
#include "esp_netif.h"
#include "esp_tls_crypto.h"
#include <esp_http_server.h>
#include "esp_event.h"
#include "esp_netif.h"
#include "esp_tls.h"
#include "esp_check.h"

static const char *TAG_MAIN_SERVER = "T_server_main";

extern httpd_handle_t server;

httpd_handle_t startHTTPWebserver(void);

void registerServerMainUri(httpd_handle_t server, const char *basePath);
