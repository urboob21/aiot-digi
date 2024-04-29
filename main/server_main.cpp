#include "server_main.h"
#include "CCamera.h"
#include <string>
#include "ServerHelper.h"
// =========================================== URIs main =====================================
// Navigate to the pages
esp_err_t index_main_handler(httpd_req_t *req)
{
    // get file path from URI ?? what
    char filepath[50];
    printf("uri: %s\n", req->uri);
    int _pos;
    esp_err_t res;

    char *base_path = (char *)req->user_ctx;
    std::string filetosend(base_path);

    // output: "folder1/folder2/file.txt"
    const char *filename = getPathFromUri(filepath, base_path,
                                             req->uri - 1, sizeof(filepath));
    printf("1 uri: %s, filename: %s, filepath: %s\n", req->uri, filename, filepath);


    // if URI "/": load index.html file
    if ((strcmp(req->uri, "/") == 0))
    {
        {
            filetosend = filetosend + "/html/index.html";   // "/sdcard" + "/html/index.html"
        }
    }
    else
    {
        // ! TODO:
        filetosend = filetosend + "/html" + std::string(req->uri);
        _pos = filetosend.find("?");
        if (_pos > -1)
        {
            filetosend = filetosend.substr(0, _pos);
        }
    }

    // ! TODO : if tensorfow lite work
    // if (filetosend == "/sdcard/html/index.html" && isSetupModusActive())
    // {
    //     printf("System ist im Setupmodus --> index.html --> setup.html");
    //     filetosend = "/sdcard/html/setup.html";
    // }

    printf("Filename: %s\n", filename);
    printf("File requested: %s\n", filetosend.c_str());

    if (!filename)
    {
        ESP_LOGE(TAG, "Filename is too long");
        /* Respond with 500 Internal Server Error */
        httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Filename too long");
        return ESP_FAIL;
    }

    res = sendFile(req, filetosend);
    if (res != ESP_OK)
        return res;

    // =================================================//
    // respond with HTTP respond
    httpd_resp_send_chunk(req, NULL, 0);
}

// ===========================================================================================
httpd_handle_t startHTTPWebserver(void)
{
    httpd_handle_t server = NULL;
    httpd_config_t config = {
        .task_priority = tskIDLE_PRIORITY + 5,
        .stack_size = 16384,
        .core_id = tskNO_AFFINITY,
        .server_port = 80,
        .ctrl_port = 32768,
        .max_open_sockets = 7,
        .max_uri_handlers = 24,
        .max_resp_headers = 8,
        .backlog_conn = 5,
        .lru_purge_enable = false,
        .recv_wait_timeout = 30,
        .send_wait_timeout = 30,
        .global_user_ctx = NULL,
        .global_user_ctx_free_fn = NULL,
        .global_transport_ctx = NULL,
        .global_transport_ctx_free_fn = NULL,
        .open_fn = NULL,
        .close_fn = NULL,
        .uri_match_fn = httpd_uri_match_wildcard};

    ESP_LOGI(TAG_MAIN_SERVER, "Starting server on port: '%d'", config.server_port);
    if (httpd_start(&server, &config) == ESP_OK)
    {
        // Set URI handlers
        ESP_LOGI(TAG_MAIN_SERVER, "Register URI handlers");
        return server;
    }
    ESP_LOGI(TAG_MAIN_SERVER, "Error starting server!");
    return NULL;
}

void registerServerMainUri(httpd_handle_t server, const char *basePath)
{

    // httpd_uri_t img_tmp_handle = {
    //     .uri = "/img_tmp/*", // Match all URIs of type /path/to/file
    //     .method = HTTP_GET,
    //     .handler = img_tmp_virtual_handler,
    //     .user_ctx = (void *)base_path // Pass server data as context
    // };
    // httpd_register_uri_handler(server, &img_tmp_handle);

    httpd_uri_t main_rest_handle = {
        .uri = "/*", // Match all URIs of type /path/to/file
        .method = HTTP_GET,
        .handler = index_main_handler,
        .user_ctx = (void *)basePath // Pass server data as context
    };
    httpd_register_uri_handler(server, &main_rest_handle);
}
