#include "camera.h"

Camera::Camera() : sensor(nullptr) {}

bool Camera::init()
{
  camera_config_t config;
  initCameraConfig(config);

  // Initialize camera
  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK)
  {
    Serial.printf("Camera init failed with error 0x%x\n", err);
    return false;
  }

  sensor = esp_camera_sensor_get();
  if (!sensor)
  {
    Serial.println("Failed to get camera sensor");
    return false;
  }

  if (sensor->id.PID == OV3660_PID)
  {
    sensor->set_vflip(sensor, 1);
    sensor->set_brightness(sensor, 1);
    sensor->set_saturation(sensor, -2);
  }

  if (config.pixel_format == PIXFORMAT_JPEG)
  {
    sensor->set_framesize(sensor, FRAMESIZE_QVGA);
    sensor->set_quality(sensor, 10);
  }

  return true;
}

void Camera::initCameraConfig(camera_config_t &config)
{
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;
  config.pin_d0 = Y2_GPIO_NUM;
  config.pin_d1 = Y3_GPIO_NUM;
  config.pin_d2 = Y4_GPIO_NUM;
  config.pin_d3 = Y5_GPIO_NUM;
  config.pin_d4 = Y6_GPIO_NUM;
  config.pin_d5 = Y7_GPIO_NUM;
  config.pin_d6 = Y8_GPIO_NUM;
  config.pin_d7 = Y9_GPIO_NUM;
  config.pin_xclk = XCLK_GPIO_NUM;
  config.pin_pclk = PCLK_GPIO_NUM;
  config.pin_vsync = VSYNC_GPIO_NUM;
  config.pin_href = HREF_GPIO_NUM;
  config.pin_sccb_sda = SIOD_GPIO_NUM;
  config.pin_sccb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;
  config.xclk_freq_hz = 20000000;
  config.frame_size = FRAMESIZE_QVGA;
  config.pixel_format = PIXFORMAT_JPEG;
  config.grab_mode = CAMERA_GRAB_LATEST;
  config.fb_location = CAMERA_FB_IN_PSRAM;
  config.jpeg_quality = 12;
  config.fb_count = 2;

  if (config.pixel_format == PIXFORMAT_JPEG)
  {
    if (psramFound())
    {
      config.jpeg_quality = 10;
      config.fb_count = 2;
      config.grab_mode = CAMERA_GRAB_LATEST;
    }
    else
    {
      config.frame_size = FRAMESIZE_SVGA;
      config.fb_location = CAMERA_FB_IN_DRAM;
    }
  }
  else
  {
    config.frame_size = FRAMESIZE_240X240;
  }
}

sensor_t *Camera::getSensor()
{
  return sensor;
}

camera_fb_t *Camera::capture()
{
  return esp_camera_fb_get();
}

void Camera::returnFrame(camera_fb_t *fb)
{
  esp_camera_fb_return(fb);
}

void Camera::setFrameSize(framesize_t size)
{
  if (sensor)
    sensor->set_framesize(sensor, size);
}

void Camera::setQuality(int quality)
{
  if (sensor)
    sensor->set_quality(sensor, quality);
}

void Camera::setXclk(int xclk)
{
  if (sensor)
    sensor->set_xclk(sensor, LEDC_TIMER_0, xclk);
}

int Camera::setReg(uint8_t reg, uint8_t mask, uint8_t value)
{
  return sensor ? sensor->set_reg(sensor, reg, mask, value) : -1;
}

int Camera::getReg(uint8_t reg, uint8_t mask)
{
  return sensor ? sensor->get_reg(sensor, reg, mask) : -1;
}

int Camera::setPLL(int bypass, int mul, int sys, int root, int pre, int seld5, int pclken, int pclk)
{
  return sensor ? sensor->set_pll(sensor, bypass, mul, sys, root, pre, seld5, pclken, pclk) : -1;
}
