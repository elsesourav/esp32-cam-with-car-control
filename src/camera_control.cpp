#include "camera_control.h"

#include "board_config.h"
#include "camera_pins.h"
#include "esp_camera.h"
#include "esp_timer.h"
#include "img_converters.h"
#include "config.h"

namespace {
#define PART_BOUNDARY "123456789000000000000987654321"
static const char *kStreamContentType = "multipart/x-mixed-replace;boundary=" PART_BOUNDARY;
static const char *kStreamBoundary = "\r\n--" PART_BOUNDARY "\r\n";
static const char *kStreamPart = "Content-Type: image/jpeg\r\nContent-Length: %u\r\n\r\n";

httpd_handle_t g_stream_httpd = NULL;
volatile bool g_stream_enabled = false;

static esp_err_t stream_handler(httpd_req_t *req) {
  esp_err_t res = ESP_OK;
  size_t _jpg_buf_len = 0;
  uint8_t *_jpg_buf = NULL;
  char part_buf[128];

  res = httpd_resp_set_type(req, kStreamContentType);
  if (res != ESP_OK) {
    return res;
  }
  
  httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");

  while (true) {
    if (!g_stream_enabled) {
      break;
    }
    
    camera_fb_t *fb = esp_camera_fb_get();
    if (!fb) {
      Serial.println("Camera capture failed");
      res = ESP_FAIL;
      break;
    }

    if (fb->format != PIXFORMAT_JPEG) {
      bool jpeg_converted = frame2jpg(fb, 80, &_jpg_buf, &_jpg_buf_len);
      esp_camera_fb_return(fb);
      fb = NULL;
      if (!jpeg_converted) {
        Serial.println("JPEG compression failed");
        res = ESP_FAIL;
      }
    } else {
      _jpg_buf_len = fb->len;
      _jpg_buf = fb->buf;
    }

    if (res == ESP_OK) {
      res = httpd_resp_send_chunk(req, kStreamBoundary, strlen(kStreamBoundary));
    }
    if (res == ESP_OK) {
      size_t hlen = snprintf(part_buf, sizeof(part_buf), kStreamPart, _jpg_buf_len);
      res = httpd_resp_send_chunk(req, (const char *)part_buf, hlen);
    }
    if (res == ESP_OK) {
      res = httpd_resp_send_chunk(req, (const char *)_jpg_buf, _jpg_buf_len);
    }
    
    if (fb) {
      esp_camera_fb_return(fb);
      fb = NULL;
      _jpg_buf = NULL;
    } else if (_jpg_buf) {
      free(_jpg_buf);
      _jpg_buf = NULL;
    }
    
    if (res != ESP_OK) {
      break;
    }

    // Minimal yield to let other tasks (motor, websocket) run.
    // No artificial frame-pacing delay — run as fast as WiFi allows.
    delay(1);
  }
  return res;
}
}  // namespace

static camera_config_t g_camera_config;
static bool g_camera_is_inited = false;

bool camera_control_init() {
  g_camera_config.ledc_channel = LEDC_CHANNEL_7;
  g_camera_config.ledc_timer = LEDC_TIMER_3;
  g_camera_config.pin_d0 = Y2_GPIO_NUM;
  g_camera_config.pin_d1 = Y3_GPIO_NUM;
  g_camera_config.pin_d2 = Y4_GPIO_NUM;
  g_camera_config.pin_d3 = Y5_GPIO_NUM;
  g_camera_config.pin_d4 = Y6_GPIO_NUM;
  g_camera_config.pin_d5 = Y7_GPIO_NUM;
  g_camera_config.pin_d6 = Y8_GPIO_NUM;
  g_camera_config.pin_d7 = Y9_GPIO_NUM;
  g_camera_config.pin_xclk = XCLK_GPIO_NUM;
  g_camera_config.pin_pclk = PCLK_GPIO_NUM;
  g_camera_config.pin_vsync = VSYNC_GPIO_NUM;
  g_camera_config.pin_href = HREF_GPIO_NUM;
  g_camera_config.pin_sccb_sda = SIOD_GPIO_NUM;
  g_camera_config.pin_sccb_scl = SIOC_GPIO_NUM;
  g_camera_config.pin_pwdn = PWDN_GPIO_NUM;
  g_camera_config.pin_reset = RESET_GPIO_NUM;
  g_camera_config.xclk_freq_hz = 20000000;

  // Hardware JPEG: camera sensor compresses internally, zero CPU cost.
  // QVGA (320x240) is the sweet spot for speed — 4x less data than VGA
  // while still being perfectly usable for driving.
  g_camera_config.frame_size = FRAMESIZE_QVGA;    // if jpeg not support then use FRAMESIZE_QQVGA
  g_camera_config.pixel_format = PIXFORMAT_JPEG;   // if jpeg not support then use PIXFORMAT_RGB565
  // GRAB_LATEST discards stale buffered frames, always sends the newest capture
  g_camera_config.grab_mode = CAMERA_GRAB_LATEST;
  g_camera_config.fb_location = CAMERA_FB_IN_PSRAM;
  // Higher number = more compression = smaller files = faster WiFi transfer
  g_camera_config.jpeg_quality = 20;
  // 2 framebuffers: camera captures into one while we transmit the other (double-buffering)
  g_camera_config.fb_count = 2;

  if (!psramFound()) {
    g_camera_config.fb_location = CAMERA_FB_IN_DRAM;
  }

  // Ensure camera is powered OFF at boot
  pinMode(PWDN_GPIO_NUM, OUTPUT);
  digitalWrite(PWDN_GPIO_NUM, HIGH);

  return true;
}

void camera_control_start_stream_server(uint16_t port) {
  httpd_config_t config = HTTPD_DEFAULT_CONFIG();
  config.server_port = port;
  config.ctrl_port = port + 1;

  httpd_uri_t stream_uri = {
    .uri = "/stream",
    .method = HTTP_GET,
    .handler = stream_handler,
    .user_ctx = NULL
  };

  if (httpd_start(&g_stream_httpd, &config) == ESP_OK) {
    httpd_register_uri_handler(g_stream_httpd, &stream_uri);
  }
}

void camera_control_set_stream_enabled(bool enabled) {
  if (enabled && !g_camera_is_inited) {
    Serial.println("Powering ON camera hardware...");
    esp_err_t err = esp_camera_init(&g_camera_config);
    if (err == ESP_OK) {
      g_camera_is_inited = true;
      sensor_t *s = esp_camera_sensor_get();
      if (s) {
        if (s->id.PID == OV2640_PID) {
          s->set_framesize(s, FRAMESIZE_QVGA);
        }
        // 180-degree flip at the hardware level (zero CPU cost)
        s->set_vflip(s, 1);
        s->set_hmirror(s, 1);
      }
    } else {
      Serial.println("Camera hardware init failed!");
      return;
    }
  } else if (!enabled && g_camera_is_inited) {
    Serial.println("Powering OFF camera hardware...");
    esp_camera_deinit();
    g_camera_is_inited = false;
    
    // Explicitly drive PWDN HIGH to cut power to the sensor completely
    pinMode(PWDN_GPIO_NUM, OUTPUT);
    digitalWrite(PWDN_GPIO_NUM, HIGH);
  }

  g_stream_enabled = enabled;
  Serial.print("Camera Stream | Action ON: ");
  Serial.println(enabled);
}

bool camera_control_is_stream_enabled() {
  return g_stream_enabled;
}
