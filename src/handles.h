#include <http_server.h>

namespace Handles {
    esp_err_t home_get(httpd_req_t *r);
    esp_err_t calibration_get(httpd_req_t *r);
    esp_err_t calibration_post(httpd_req_t *r);
}