#include <WiFi.h>
#include <http_server.h>
#include "handles.h"
#include "server.h"

const char* ssid = "Starling WiFi";
httpd_handle_t server = nullptr;

httpd_uri_t home_get = {
    .uri      = "/",
    .method   = HTTP_GET,
    .handler  = Handles::home_get,
    .user_ctx = NULL
};

httpd_uri_t calibration_post = {
    .uri      = "/calibration",
    .method   = HTTP_POST,
    .handler  = Handles::calibration_post,
    .user_ctx = NULL
};

httpd_uri_t calibration_get = {
    .uri      = "/calibration",
    .method   = HTTP_GET,
    .handler  = Handles::calibration_get,
    .user_ctx = NULL
};

void CalibrationServer::start_server() {
    WiFi.softAP(ssid);
    delay(100);

    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    config.lru_purge_enable = true;

    if(httpd_start(&server, &config) == ESP_OK) {
        httpd_register_uri_handler(server, &home_get);
        httpd_register_uri_handler(server, &calibration_get);
        httpd_register_uri_handler(server, &calibration_post);

        Serial.printf("Started HTTP Server. PORT: %d, IP: %s\n", config.server_port, WiFi.softAPIP().toString().c_str());
    }
}