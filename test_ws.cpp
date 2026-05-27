#include <Arduino.h>
#include "esp_http_server.h"

void check() {
    httpd_ws_frame_t pkt;
    pkt.final = true;
    pkt.fragmented = false;
}
