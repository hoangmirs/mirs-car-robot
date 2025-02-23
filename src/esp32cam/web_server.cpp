#include "web_server.h"
#include "esp_timer.h"
#include "img_converters.h"
#include "fb_gfx.h"
#include "esp32-hal-ledc.h"

// External variables - declared here, defined in main.cpp
extern int gpLed;
extern String WiFiAddr;
extern byte txdata[3];

// Constants from original code
const int Forward = 92;
const int Backward = 163;
const int Turn_Left = 149;
const int Turn_Right = 106;
const int Top_Left = 20;
const int Bottom_Left = 129;
const int Top_Right = 72;
const int Bottom_Right = 34;
const int Stop = 0;
const int Clockwise = 83;
const int Contrarotate = 172;
const int Moedl1 = 25;
const int Moedl2 = 26;
const int Moedl3 = 27;
const int Moedl4 = 28;
const int MotorLeft = 230;
const int MotorRight = 231;

#define PART_BOUNDARY "123456789000000000000987654321"
static const char *_STREAM_CONTENT_TYPE = "multipart/x-mixed-replace;boundary=" PART_BOUNDARY;
static const char *_STREAM_BOUNDARY = "\r\n--" PART_BOUNDARY "\r\n";
static const char *_STREAM_PART = "Content-Type: image/jpeg\r\nContent-Length: %u\r\nX-Timestamp: %d.%06d\r\n\r\n";

char WebServer::part_buf[128];

WebServer::WebServer(Camera &camera) : camera(camera), stream_httpd(nullptr), camera_httpd(nullptr), ssid(nullptr), password(nullptr) {}

void WebServer::setWiFiCredentials(const char *ssid, const char *password)
{
  this->ssid = ssid;
  this->password = password;
}

bool WebServer::connectToWiFi()
{
  if (!ssid || !password)
  {
    return false;
  }

  WiFi.begin(ssid, password);
  WiFi.setSleep(false);

  // Wait for connection
  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 20)
  {
    delay(500);
    attempts++;
  }

  if (WiFi.status() == WL_CONNECTED)
  {
    wifiAddress = WiFi.localIP().toString();
    return true;
  }
  return false;
}

String WebServer::getWiFiAddress() const
{
  return wifiAddress;
}

void WebServer::start()
{
  httpd_config_t config = HTTPD_DEFAULT_CONFIG();
  config.max_uri_handlers = 30;
  config.max_resp_headers = 30;

  Serial.println("Starting web server on port 80");
  esp_err_t err = httpd_start(&camera_httpd, &config);
  if (err != ESP_OK)
  {
    Serial.printf("Failed to start web server: %d\n", err);
    return;
  }
  registerHandlers();

  config.server_port = 81;
  config.ctrl_port = config.ctrl_port + 1;

  Serial.println("Starting stream server on port 81");
  err = httpd_start(&stream_httpd, &config);
  if (err != ESP_OK)
  {
    Serial.printf("Failed to start stream server: %d\n", err);
    return;
  }
  setupStreamServer();
}

// Helper methods
esp_err_t WebServer::parseGet(httpd_req_t *req, char **obuf)
{
  char *buf = nullptr;
  size_t buf_len = httpd_req_get_url_query_len(req) + 1;

  if (buf_len > 1)
  {
    buf = (char *)malloc(buf_len);
    if (!buf)
    {
      httpd_resp_send_500(req);
      return ESP_FAIL;
    }
    if (httpd_req_get_url_query_str(req, buf, buf_len) == ESP_OK)
    {
      *obuf = buf;
      return ESP_OK;
    }
    free(buf);
  }
  httpd_resp_send_404(req);
  return ESP_FAIL;
}

void WebServer::registerHandlers()
{
  httpd_uri_t index_uri = {
      .uri = "/",
      .method = HTTP_GET,
      .handler = indexHandler,
      .user_ctx = this};
  httpd_register_uri_handler(camera_httpd, &index_uri);

  httpd_uri_t status_uri = {
      .uri = "/status",
      .method = HTTP_GET,
      .handler = statusHandler,
      .user_ctx = this};
  httpd_register_uri_handler(camera_httpd, &status_uri);

  httpd_uri_t cmd_uri = {
      .uri = "/control",
      .method = HTTP_GET,
      .handler = cmdHandler,
      .user_ctx = this};
  httpd_register_uri_handler(camera_httpd, &cmd_uri);

  httpd_uri_t capture_uri = {
      .uri = "/capture",
      .method = HTTP_GET,
      .handler = captureHandler,
      .user_ctx = this};
  httpd_register_uri_handler(camera_httpd, &capture_uri);

  // Register all robot control handlers
  const httpd_uri_t movement_handlers[] = {
      {"/go", HTTP_GET, goHandler, this},
      {"/back", HTTP_GET, backHandler, this},
      {"/left", HTTP_GET, leftHandler, this},
      {"/right", HTTP_GET, rightHandler, this},
      {"/stop", HTTP_GET, stopHandler, this},
      {"/leftup", HTTP_GET, leftUpHandler, this},
      {"/leftdown", HTTP_GET, leftDownHandler, this},
      {"/rightup", HTTP_GET, rightUpHandler, this},
      {"/rightdown", HTTP_GET, rightDownHandler, this},
      {"/clockwise", HTTP_GET, clockwiseHandler, this},
      {"/contrario", HTTP_GET, contrarioHandler, this},
      {"/ledon", HTTP_GET, ledOnHandler, this},
      {"/ledoff", HTTP_GET, ledOffHandler, this},
      {"/model1", HTTP_GET, model1Handler, this},
      {"/model2", HTTP_GET, model2Handler, this},
      {"/model3", HTTP_GET, model3Handler, this},
      {"/model4", HTTP_GET, model4Handler, this},
      {"/motorleft", HTTP_GET, motorLeftHandler, this},
      {"/motorright", HTTP_GET, motorRightHandler, this}};

  for (const auto &handler : movement_handlers)
  {
    httpd_register_uri_handler(camera_httpd, &handler);
  }
}

void WebServer::setupStreamServer()
{
  httpd_uri_t stream_uri = {
      .uri = "/stream",
      .method = HTTP_GET,
      .handler = streamHandler,
      .user_ctx = this};

  esp_err_t ret = httpd_register_uri_handler(stream_httpd, &stream_uri);
  if (ret != ESP_OK)
  {
    Serial.printf("Failed to register stream handler: %d\n", ret);
  }
  else
  {
    Serial.println("Stream server started on port 81");
  }
}

// Movement handlers implementation
esp_err_t WebServer::goHandler(httpd_req_t *req)
{
  txdata[1] = Forward;
  Serial.write(txdata, 3);
  Serial.println("Go");
  httpd_resp_set_type(req, "text/html");
  return httpd_resp_send(req, "OK", 2);
}

esp_err_t WebServer::backHandler(httpd_req_t *req)
{
  txdata[1] = Backward;
  Serial.write(txdata, 3);
  Serial.println("Back");
  httpd_resp_set_type(req, "text/html");
  return httpd_resp_send(req, "OK", 2);
}

esp_err_t WebServer::leftHandler(httpd_req_t *req)
{
  txdata[1] = Turn_Left;
  Serial.write(txdata, 3);
  Serial.println("Left");
  httpd_resp_set_type(req, "text/html");
  return httpd_resp_send(req, "OK", 2);
}

esp_err_t WebServer::rightHandler(httpd_req_t *req)
{
  txdata[1] = Turn_Right;
  Serial.write(txdata, 3);
  Serial.println("Right");
  httpd_resp_set_type(req, "text/html");
  return httpd_resp_send(req, "OK", 2);
}

esp_err_t WebServer::stopHandler(httpd_req_t *req)
{
  txdata[1] = Stop;
  Serial.write(txdata, 3);
  Serial.println("Stop");
  httpd_resp_set_type(req, "text/html");
  return httpd_resp_send(req, "OK", 2);
}

esp_err_t WebServer::leftUpHandler(httpd_req_t *req)
{
  txdata[1] = Top_Left;
  Serial.write(txdata, 3);
  Serial.println("LeftUp");
  httpd_resp_set_type(req, "text/html");
  return httpd_resp_send(req, "OK", 2);
}

esp_err_t WebServer::leftDownHandler(httpd_req_t *req)
{
  txdata[1] = Bottom_Left;
  Serial.write(txdata, 3);
  Serial.println("LeftDown");
  httpd_resp_set_type(req, "text/html");
  return httpd_resp_send(req, "OK", 2);
}

esp_err_t WebServer::rightUpHandler(httpd_req_t *req)
{
  txdata[1] = Top_Right;
  Serial.write(txdata, 3);
  Serial.println("RightUp");
  httpd_resp_set_type(req, "text/html");
  return httpd_resp_send(req, "OK", 2);
}

esp_err_t WebServer::rightDownHandler(httpd_req_t *req)
{
  txdata[1] = Bottom_Right;
  Serial.write(txdata, 3);
  Serial.println("RightDown");
  httpd_resp_set_type(req, "text/html");
  return httpd_resp_send(req, "OK", 2);
}

esp_err_t WebServer::clockwiseHandler(httpd_req_t *req)
{
  txdata[1] = Clockwise;
  Serial.write(txdata, 3);
  Serial.println("Clockwise");
  httpd_resp_set_type(req, "text/html");
  return httpd_resp_send(req, "OK", 2);
}

esp_err_t WebServer::contrarioHandler(httpd_req_t *req)
{
  txdata[1] = Contrarotate;
  Serial.write(txdata, 3);
  Serial.println("Contrario");
  httpd_resp_set_type(req, "text/html");
  return httpd_resp_send(req, "OK", 2);
}

esp_err_t WebServer::ledOnHandler(httpd_req_t *req)
{
  digitalWrite(gpLed, HIGH);
  Serial.println("LED ON");
  httpd_resp_set_type(req, "text/html");
  return httpd_resp_send(req, "OK", 2);
}

esp_err_t WebServer::ledOffHandler(httpd_req_t *req)
{
  digitalWrite(gpLed, LOW);
  Serial.println("LED OFF");
  httpd_resp_set_type(req, "text/html");
  return httpd_resp_send(req, "OK", 2);
}

esp_err_t WebServer::model1Handler(httpd_req_t *req)
{
  txdata[1] = Moedl1;
  Serial.write(txdata, 3);
  Serial.println("Model1");
  httpd_resp_set_type(req, "text/html");
  return httpd_resp_send(req, "OK", 2);
}

esp_err_t WebServer::model2Handler(httpd_req_t *req)
{
  txdata[1] = Moedl2;
  Serial.write(txdata, 3);
  Serial.println("Model2");
  httpd_resp_set_type(req, "text/html");
  return httpd_resp_send(req, "OK", 2);
}

esp_err_t WebServer::model3Handler(httpd_req_t *req)
{
  txdata[1] = Moedl3;
  Serial.write(txdata, 3);
  Serial.println("Model3");
  httpd_resp_set_type(req, "text/html");
  return httpd_resp_send(req, "OK", 2);
}

esp_err_t WebServer::model4Handler(httpd_req_t *req)
{
  txdata[1] = Moedl4;
  Serial.write(txdata, 3);
  Serial.println("Model4");
  httpd_resp_set_type(req, "text/html");
  return httpd_resp_send(req, "OK", 2);
}

esp_err_t WebServer::motorLeftHandler(httpd_req_t *req)
{
  txdata[1] = MotorLeft;
  Serial.write(txdata, 3);
  Serial.println("MotorLeft");
  httpd_resp_set_type(req, "text/html");
  return httpd_resp_send(req, "OK", 2);
}

esp_err_t WebServer::motorRightHandler(httpd_req_t *req)
{
  txdata[1] = MotorRight;
  Serial.write(txdata, 3);
  Serial.println("MotorRight");
  httpd_resp_set_type(req, "text/html");
  return httpd_resp_send(req, "OK", 2);
}

// Camera control handlers
esp_err_t WebServer::captureHandler(httpd_req_t *req)
{
  WebServer *server = (WebServer *)req->user_ctx;
  Camera &camera = server->camera;

  camera_fb_t *fb = camera.capture();
  if (!fb)
  {
    Serial.println("Camera capture failed");
    httpd_resp_send_500(req);
    return ESP_FAIL;
  }

  httpd_resp_set_type(req, "image/jpeg");
  httpd_resp_set_hdr(req, "Content-Disposition", "inline; filename=capture.jpg");
  httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");

  char ts[32];
  snprintf(ts, 32, "%lld.%06ld", fb->timestamp.tv_sec, fb->timestamp.tv_usec);
  httpd_resp_set_hdr(req, "X-Timestamp", (const char *)ts);

  esp_err_t res = httpd_resp_send(req, (const char *)fb->buf, fb->len);
  camera.returnFrame(fb);

  return res;
}

esp_err_t WebServer::cmdHandler(httpd_req_t *req)
{
  WebServer *server = (WebServer *)req->user_ctx;
  Camera &camera = server->camera;
  char *buf = nullptr;

  if (server->parseGet(req, &buf) != ESP_OK)
  {
    return ESP_FAIL;
  }

  char variable[32];
  char value[32];

  if (httpd_query_key_value(buf, "var", variable, sizeof(variable)) != ESP_OK ||
      httpd_query_key_value(buf, "val", value, sizeof(value)) != ESP_OK)
  {
    free(buf);
    httpd_resp_send_404(req);
    return ESP_FAIL;
  }
  free(buf);

  int val = atoi(value);
  sensor_t *s = camera.getSensor();

  int res = 0;
  if (!strcmp(variable, "framesize"))
  {
    if (s->pixformat == PIXFORMAT_JPEG)
    {
      res = s->set_framesize(s, (framesize_t)val);
    }
  }
  else if (!strcmp(variable, "quality"))
  {
    res = s->set_quality(s, val);
  }
  // ... Add other camera settings as needed

  if (res)
  {
    return httpd_resp_send_500(req);
  }

  httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");
  return httpd_resp_send(req, NULL, 0);
}

esp_err_t WebServer::statusHandler(httpd_req_t *req)
{
  static char json_response[1024];

  sensor_t *s = ((WebServer *)req->user_ctx)->camera.getSensor();
  char *p = json_response;
  *p++ = '{';

  p += sprintf(p, "\"framesize\":%u,", s->status.framesize);
  p += sprintf(p, "\"quality\":%u,", s->status.quality);
  p += sprintf(p, "\"brightness\":%d,", s->status.brightness);
  p += sprintf(p, "\"contrast\":%d,", s->status.contrast);
  p += sprintf(p, "\"saturation\":%d,", s->status.saturation);
  p += sprintf(p, "\"special_effect\":%u,", s->status.special_effect);
  p += sprintf(p, "\"wb_mode\":%u,", s->status.wb_mode);
  p += sprintf(p, "\"awb\":%u,", s->status.awb);
  p += sprintf(p, "\"awb_gain\":%u,", s->status.awb_gain);
  p += sprintf(p, "\"aec\":%u,", s->status.aec);
  p += sprintf(p, "\"aec2\":%u,", s->status.aec2);
  p += sprintf(p, "\"denoise\":%u,", s->status.denoise);
  p += sprintf(p, "\"ae_level\":%d,", s->status.ae_level);
  p += sprintf(p, "\"aec_value\":%u,", s->status.aec_value);
  p += sprintf(p, "\"agc\":%u,", s->status.agc);
  p += sprintf(p, "\"agc_gain\":%u,", s->status.agc_gain);
  p += sprintf(p, "\"gainceiling\":%u,", s->status.gainceiling);
  p += sprintf(p, "\"bpc\":%u,", s->status.bpc);
  p += sprintf(p, "\"wpc\":%u,", s->status.wpc);
  p += sprintf(p, "\"raw_gma\":%u,", s->status.raw_gma);
  p += sprintf(p, "\"lenc\":%u,", s->status.lenc);
  p += sprintf(p, "\"hmirror\":%u,", s->status.hmirror);
  p += sprintf(p, "\"dcw\":%u,", s->status.dcw);
  p += sprintf(p, "\"colorbar\":%u", s->status.colorbar);
  *p++ = '}';
  *p++ = 0;

  httpd_resp_set_type(req, "application/json");
  httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");
  return httpd_resp_send(req, json_response, strlen(json_response));
}

// Index page handler
esp_err_t WebServer::indexHandler(httpd_req_t *req)
{
  static const char PROGMEM INDEX_HTML[] = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
  <title>ESP32-CAM Robot</title>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <style>
    body { font-family: Arial; text-align: center; margin:0px auto; padding-top: 30px;}
    table { margin-left: auto; margin-right: auto; }
    td { padding: 8 px; }
    .button {
      background-color: lightgrey;
      width: 90px;
      height: 40px;
      border: none;
      color: black;
      text-align: center;
      text-decoration: none;
      display: inline-block;
      font-size: 16px;
      margin: 2px;
      cursor: pointer;
      -webkit-touch-callout: none;
      -webkit-user-select: none;
      -khtml-user-select: none;
      -moz-user-select: none;
      -ms-user-select: none;
      user-select: none;
      -webkit-tap-highlight-color: rgba(0,0,0,0);
    }
    img { width: auto; max-width: 100%; height: auto; }
  </style>
</head>
<body>
  <h1>ESP32-CAM Robot</h1>
  <img src="" id="photo">
  <p align=center>
    <button class="button" onmousedown="toggleCheckbox('leftup');" onmouseup="toggleCheckbox('stop');" ontouchstart="toggleCheckbox('leftup');" ontouchend="toggleCheckbox('stop')"><b>LeftUp</b></button>&nbsp;
    <button class="button" onmousedown="toggleCheckbox('go');" onmouseup="toggleCheckbox('stop');" ontouchstart="toggleCheckbox('go');" ontouchend="toggleCheckbox('stop')"><b>Forward</b></button>&nbsp;
    <button class="button" onmousedown="toggleCheckbox('rightup');" onmouseup="toggleCheckbox('stop');" ontouchstart="toggleCheckbox('rightup');" ontouchend="toggleCheckbox('stop')"><b>RightUp</b></button>
  </p>
  <p align=center>
    <button class="button" onmousedown="toggleCheckbox('left');" onmouseup="toggleCheckbox('stop');" ontouchstart="toggleCheckbox('left');" ontouchend="toggleCheckbox('stop')"><b>Left</b></button>&nbsp;
    <button style="background-color:indianred" class="button" onmousedown="toggleCheckbox('stop');" onmouseup="toggleCheckbox('stop');" ontouchstart="toggleCheckbox('stop');" ontouchend="toggleCheckbox('stop')"><b>Stop</b></button>&nbsp;
    <button class="button" onmousedown="toggleCheckbox('right');" onmouseup="toggleCheckbox('stop');" ontouchstart="toggleCheckbox('right');" ontouchend="toggleCheckbox('stop')"><b>Right</b></button>
  </p>
  <p align=center>
    <button class="button" onmousedown="toggleCheckbox('leftdown');" onmouseup="toggleCheckbox('stop');" ontouchstart="toggleCheckbox('leftdown');" ontouchend="toggleCheckbox('stop')"><b>LeftDown</b></button>&nbsp;
    <button class="button" onmousedown="toggleCheckbox('back');" onmouseup="toggleCheckbox('stop');" ontouchstart="toggleCheckbox('back');" ontouchend="toggleCheckbox('stop')"><b>Backward</b></button>&nbsp;
    <button class="button" onmousedown="toggleCheckbox('rightdown');" onmouseup="toggleCheckbox('stop');" ontouchstart="toggleCheckbox('rightdown');" ontouchend="toggleCheckbox('stop')"><b>RightDown</b></button>
  </p>
  <p align=center>
    <button class="button" onmousedown="toggleCheckbox('clockwise');" onmouseup="toggleCheckbox('stop');" ontouchstart="toggleCheckbox('clockwise');" ontouchend="toggleCheckbox('stop')"><b>Clockwise</b></button>&nbsp;
    <button class="button" onmousedown="toggleCheckbox('contrario');" onmouseup="toggleCheckbox('stop');" ontouchstart="toggleCheckbox('contrario');" ontouchend="toggleCheckbox('stop')"><b>Contrario</b></button>
  </p>
  <p align=center>
    <button class="button" onmousedown="toggleCheckbox('model1');" onmouseup="toggleCheckbox('model1');" ontouchstart="toggleCheckbox('model1');" ontouchend="toggleCheckbox('model1')"><b>Model1</b></button>&nbsp;
    <button class="button" onmousedown="toggleCheckbox('model2');" onmouseup="toggleCheckbox('model2');" ontouchstart="toggleCheckbox('model2');" ontouchend="toggleCheckbox('model2')"><b>Model2</b></button>
  </p>
  <p align=center>
    <button class="button" onmousedown="toggleCheckbox('model3');" onmouseup="toggleCheckbox('model3');" ontouchstart="toggleCheckbox('model3');" ontouchend="toggleCheckbox('model3')"><b>Model3</b></button>&nbsp;
    <button class="button" onmousedown="toggleCheckbox('model4');" onmouseup="toggleCheckbox('model4');" ontouchstart="toggleCheckbox('model4');" ontouchend="toggleCheckbox('model4')"><b>Model4</b></button>
  </p>
  <p align=center>
    <button class="button" onmousedown="toggleCheckbox('motorleft');" onmouseup="toggleCheckbox('stop');" ontouchstart="toggleCheckbox('motorleft');" ontouchend="toggleCheckbox('stop')"><b>MotorLeft</b></button>&nbsp;
    <button class="button" onmousedown="toggleCheckbox('motorright');" onmouseup="toggleCheckbox('stop');" ontouchstart="toggleCheckbox('motorright');" ontouchend="toggleCheckbox('stop')"><b>MotorRight</b></button>
  </p>
  <p align=center>
    <button style="background-color:yellow" class="button" onmousedown="toggleCheckbox('ledon')"><b>Light ON</b></button>&nbsp;
    <button style="background-color:indianred" class="button" onmousedown="toggleCheckbox('stop');" onmouseup="toggleCheckbox('stop')"><b>Stop</b></button>&nbsp;
    <button style="background-color:yellow" class="button" onmousedown="toggleCheckbox('ledoff')"><b>Light OFF</b></button>
  </p>
  <script>
   var xhttp = new XMLHttpRequest();
   function toggleCheckbox(x) {
     xhttp.open('GET', '/' + x + '?' + new Date().getTime(), true);
     xhttp.send();
   }
   window.onload = function() {
     var img = document.getElementById("photo");
     var baseUrl = window.location.href.slice(0, -1).replace(/:80$/, '');
     function startStream() {
       img.src = baseUrl + ":81/stream";
     }
     img.onerror = function() {
       console.log("Stream connection failed, retrying...");
       setTimeout(startStream, 1000);
     };
     startStream();
   };
  </script>
</body>
</html>
)rawliteral";

  httpd_resp_set_type(req, "text/html");
  return httpd_resp_send(req, INDEX_HTML, strlen(INDEX_HTML));
}

// Stream handler implementation
esp_err_t WebServer::streamHandler(httpd_req_t *req)
{
  WebServer *server = (WebServer *)req->user_ctx;
  Camera &camera = server->camera;
  struct timeval _timestamp;
  esp_err_t res = ESP_OK;
  size_t _jpg_buf_len = 0;
  uint8_t *_jpg_buf = NULL;
  char *part_buf[128];

  static int64_t last_frame = 0;
  if (!last_frame)
  {
    last_frame = esp_timer_get_time();
  }

  res = httpd_resp_set_type(req, _STREAM_CONTENT_TYPE);
  if (res != ESP_OK)
  {
    return res;
  }

  httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");
  httpd_resp_set_hdr(req, "X-Framerate", "60");

  Serial.println("Starting stream");

  while (true)
  {
    camera_fb_t *fb = camera.capture();
    if (!fb)
    {
      Serial.println("Camera capture failed");
      res = ESP_FAIL;
    }
    else
    {
      _timestamp.tv_sec = fb->timestamp.tv_sec;
      _timestamp.tv_usec = fb->timestamp.tv_usec;
      if (fb->format != PIXFORMAT_JPEG)
      {
        bool jpeg_converted = frame2jpg(fb, 80, &_jpg_buf, &_jpg_buf_len);
        camera.returnFrame(fb);
        fb = NULL;
        if (!jpeg_converted)
        {
          Serial.println("JPEG compression failed");
          res = ESP_FAIL;
        }
      }
      else
      {
        _jpg_buf_len = fb->len;
        _jpg_buf = fb->buf;
      }
    }

    if (res == ESP_OK)
    {
      res = httpd_resp_send_chunk(req, _STREAM_BOUNDARY, strlen(_STREAM_BOUNDARY));
    }
    if (res == ESP_OK)
    {
      size_t hlen = snprintf((char *)part_buf, 128, _STREAM_PART, _jpg_buf_len, _timestamp.tv_sec, _timestamp.tv_usec);
      res = httpd_resp_send_chunk(req, (const char *)part_buf, hlen);
    }
    if (res == ESP_OK)
    {
      res = httpd_resp_send_chunk(req, (const char *)_jpg_buf, _jpg_buf_len);
    }

    if (fb)
    {
      camera.returnFrame(fb);
      fb = NULL;
      _jpg_buf = NULL;
    }
    else if (_jpg_buf)
    {
      free(_jpg_buf);
      _jpg_buf = NULL;
    }

    if (res != ESP_OK)
    {
      break;
    }

    int64_t fr_end = esp_timer_get_time();
    int64_t frame_time = fr_end - last_frame;
    last_frame = fr_end;
    frame_time /= 1000;
    Serial.printf("MJPG: %uB %ums (%.1ffps)\n",
                  (uint32_t)(_jpg_buf_len),
                  (uint32_t)frame_time,
                  1000.0 / (uint32_t)frame_time);
  }

  Serial.println("Stream ended");
  return res;
}
