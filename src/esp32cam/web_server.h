#pragma once

#include "esp_http_server.h"
#include "camera.h"
#include <WiFi.h>

class WebServer
{
public:
  WebServer(Camera &camera);
  void start();
  void setWiFiCredentials(const char *ssid, const char *password);
  bool connectToWiFi();
  String getWiFiAddress() const;

private:
  Camera &camera;
  httpd_handle_t stream_httpd;
  httpd_handle_t camera_httpd;
  String wifiAddress;
  const char *ssid;
  const char *password;

  static char part_buf[128]; // Buffer for stream parts

  void registerHandlers();
  void setupStreamServer();

  // Handler methods
  static esp_err_t indexHandler(httpd_req_t *req);
  static esp_err_t streamHandler(httpd_req_t *req);
  static esp_err_t captureHandler(httpd_req_t *req);
  static esp_err_t cmdHandler(httpd_req_t *req);
  static esp_err_t statusHandler(httpd_req_t *req);
  static esp_err_t xclkHandler(httpd_req_t *req);
  static esp_err_t regHandler(httpd_req_t *req);
  static esp_err_t gregHandler(httpd_req_t *req);
  static esp_err_t pllHandler(httpd_req_t *req);
  static esp_err_t winHandler(httpd_req_t *req);

  // Robot control handlers
  static esp_err_t goHandler(httpd_req_t *req);
  static esp_err_t backHandler(httpd_req_t *req);
  static esp_err_t leftHandler(httpd_req_t *req);
  static esp_err_t rightHandler(httpd_req_t *req);
  static esp_err_t stopHandler(httpd_req_t *req);
  static esp_err_t leftUpHandler(httpd_req_t *req);
  static esp_err_t leftDownHandler(httpd_req_t *req);
  static esp_err_t rightUpHandler(httpd_req_t *req);
  static esp_err_t rightDownHandler(httpd_req_t *req);
  static esp_err_t clockwiseHandler(httpd_req_t *req);
  static esp_err_t contrarioHandler(httpd_req_t *req);
  static esp_err_t ledOnHandler(httpd_req_t *req);
  static esp_err_t ledOffHandler(httpd_req_t *req);
  static esp_err_t model1Handler(httpd_req_t *req);
  static esp_err_t model2Handler(httpd_req_t *req);
  static esp_err_t model3Handler(httpd_req_t *req);
  static esp_err_t model4Handler(httpd_req_t *req);
  static esp_err_t motorLeftHandler(httpd_req_t *req);
  static esp_err_t motorRightHandler(httpd_req_t *req);

  // Helper methods
  static esp_err_t parseGet(httpd_req_t *req, char **obuf);
  static int parseGetVar(char *buf, const char *key, int def);

  static esp_err_t gamepadHandler(httpd_req_t *req);
};
