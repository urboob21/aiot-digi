#include "server_main.h"
#include "CCamera.h"
#include <string>
#include "ServerHelper.h"
#include "server_tflite.h"

httpd_handle_t server = NULL;

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
        filetosend = filetosend + "/html" + std::string(req->uri);
        _pos = filetosend.find("?");
        if (_pos > -1)
        {
            filetosend = filetosend.substr(0, _pos);
        }
    }

    if (filetosend == "/sdcard/html/index.html" && isSetupModusActive())
    {
        printf("System ist im Setupmodus --> index.html --> setup.html");
        filetosend = "/sdcard/html/setup.html";
    }

    printf("Filename: %s\n", filename);
    printf("File requested: %s\n", filetosend.c_str());

    if (!filename)
    {
        ESP_LOGE(TAG_MAIN_SERVER, "Filename is too long");
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

    return ESP_OK;
}


esp_err_t img_tmp_handler(httpd_req_t *req)
{
    char filepath[50];
    printf("uri: %s\n", req->uri);

    char *base_path = (char*) req->user_ctx;
    std::string filetosend(base_path);

    const char *filename = getPathFromUri(filepath, base_path,
                                             req->uri  + sizeof("/img_tmp/") - 1, sizeof(filepath));    
    printf("1 uri: %s, filename: %s, filepath: %s\n", req->uri, filename, filepath);

    filetosend = filetosend + "/img_tmp/" + std::string(filename);
    printf("File to upload: %s\n", filetosend.c_str());    

    esp_err_t res = sendFile(req, filetosend);
    if (res != ESP_OK)
        return res;

    /* Respond with an empty chunk to signal HTTP response completion */
    httpd_resp_send_chunk(req, NULL, 0);
    return ESP_OK;
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


esp_err_t img_tmp_virtual_handler(httpd_req_t *req)
{
    char filepath[50];

    printf("uri: %s\n", req->uri);

    char *base_path = (char*) req->user_ctx;
    std::string filetosend(base_path);

    const char *filename = getPathFromUri(filepath, base_path,
                                             req->uri  + sizeof("/img_tmp/") - 1, sizeof(filepath));    
    printf("1 uri: %s, filename: %s, filepath: %s\n", req->uri, filename, filepath);

    filetosend = std::string(filename);
    printf("File to upload: %s\n", filetosend.c_str()); 

    if (filetosend == "raw.jpg")
    {
        return getRawJPG(req); 
    } 

    esp_err_t zw = getJPG(filetosend, req);

    if (zw == ESP_OK)
        return ESP_OK;

    // File wird nicht intern bereit gestellt --> klassischer weg:
    return img_tmp_handler(req);
}


void registerServerMainUri(httpd_handle_t server, const char *basePath)
{

    httpd_uri_t img_tmp_handle = {
        .uri = "/img_tmp/*", // Match all URIs of type /path/to/file
        .method = HTTP_GET,
        .handler = img_tmp_virtual_handler,
        .user_ctx = (void *)basePath // Pass server data as context
    };
    httpd_register_uri_handler(server, &img_tmp_handle);

    httpd_uri_t main_rest_handle = {
        .uri = "/*", // Match all URIs of type /path/to/file
        .method = HTTP_GET,
        .handler = index_main_handler,
        .user_ctx = (void *)basePath // Pass server data as context
    };
    httpd_register_uri_handler(server, &main_rest_handle);
}
