#include "handles.h"
#include <Arduino.h>

const char* html = "<!DOCTYPE html><head><meta name=\"viewport\" content=\"width=device-width,initial-scale=1,user-scalable=no\"><title>Calibration Server</title><style>table,td,th{border:1px solid #000}.button-on:active{background-color:#2980b9}</style><table style=\"width:100%\"><tr><th>ADC Value</th><th>Volume</th><th>Voltage 1</th><th>Voltage 2</th></tr></table>";

const char* serve_html() {
    Serial.println("Get Home.");
    return html;
}

esp_err_t Handles::home_get(httpd_req_t *r) {
    httpd_resp_send(r, serve_html(), HTTPD_RESP_USE_STRLEN);
}

esp_err_t Handles::calibration_get(httpd_req_t *r) {

}

esp_err_t Handles::calibration_post(httpd_req_t *r) {

}