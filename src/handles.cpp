#include "handles.h"
#include <Arduino.h>
#include "Result.h"
#include "EEPROM.h"
#include "vector"
#include "regex.h"

const int sample_count = 5;

const int resistor_pin = 35;

const char* html = "<!DOCTYPE html><head><meta name=\"viewport\" content=\"width=device-width,initial-scale=1,user-scalable=no\"><title>Calibration Server</title><style>table,td,th{border:1px solid #000}.button-on:active{background-color:#2980b9}</style><table style=\"width:100%\">%s<tr><th>ADC Value</th><th>Voltage 1</th><th>Voltage 2</th><th>Submit</th></tr>%s</table>";

const char* build_html() {
    String table;
    String forms;
    char* buf = (char*)malloc(4096);
    for(int i = 0; i < EEPROM.length(); i += sizeof(Result)) {
        Result res;
        EEPROM.get(i, res);

        if(res.result == 0xDEAD) {
            break;
        }

        sprintf(buf, "<tr><td>%d</td></td><td><input type=\"text\" id=\"voltage\" name=\"voltage\" form=\"row_%d\" value=\"%f\"></td><td><input type=\"text\" id=\"voltage2\" name=\"voltage2\" form=\"row_%d\" value=\"%f\"></td><td><input type=\"hidden\" name=\"form_id\" form=\"row_%d\" value=\"%d&\"><input type=\"submit\" value=\"SET\" form=\"row_%d\"></tr>", res.result, i, res.voltage, i, res.voltage2, i, i/sizeof(Result), i);
        table += buf;
        sprintf(buf, "<form method=\"post\" id=\"row_%d\" action=\"/calibration\"></form>", i);
        forms += buf;
    }
    int size = snprintf(NULL, 0, html, forms.c_str(), table.c_str());
    char * a = (char*)malloc(size + 1);
    sprintf(a, html, forms.c_str(), table.c_str());
    free(buf);
    return a;
}

esp_err_t Handles::home_get(httpd_req_t *r) {
    httpd_resp_send(r, build_html(), HTTPD_RESP_USE_STRLEN);
    return ESP_OK;
}

esp_err_t Handles::calibration_get(httpd_req_t *r) {
    return ESP_OK;
}

esp_err_t Handles::calibration_post(httpd_req_t *r) {
    Serial.println("calibration POST");

    char content[2048];
    size_t content_size = r->content_len;

    int ret = httpd_req_recv(r, content, content_size);
    if(ret <= 0) {
        if (ret == HTTPD_SOCK_ERR_TIMEOUT) {
            httpd_resp_send_408(r);
        }

        return ESP_FAIL;
    }

    Result res;
    int idx;

    int i = 0;
    int j = i;
    for(; content[i] != '='; ++i) {}
    for(; content[j] != '&'; ++j) {}

    char buf[255];

    memcpy(buf, &content[i+1], j-i-1);
    buf[j-i-1] = '\0';
    res.voltage = atof(buf);

    i = j;
    for(; content[i] != '='; ++i) {}
    j = i;
    for(; content[j] != '&'; ++j) {}

    memcpy(buf, &content[i+1], j-i-1);
    buf[j-i-1] = '\0';
    res.voltage2 = atof(buf);

    i = j;
    for(; content[i] != '='; ++i) {}
    j = i;
    for(; content[j] != '%'; ++j) {}

    memcpy(buf, &content[i+1], j-i-1);
    buf[j-i-1] = '\0';
    idx = atoi(buf);

    Serial.println(res.voltage);
    Serial.println(res.voltage2);
    Serial.println(idx);

    res.result = 0;
    for(int j = 0; j < sample_count; j++) {
      res.result += analogRead(resistor_pin);
      delay(10);
    }
    res.result = res.result / sample_count;

    EEPROM.put(idx*sizeof(Result), res);
    EEPROM.commit();

    const char resp[] = "Set calibration.";
    httpd_resp_send(r, resp, HTTPD_RESP_USE_STRLEN);

    return ESP_OK;
}