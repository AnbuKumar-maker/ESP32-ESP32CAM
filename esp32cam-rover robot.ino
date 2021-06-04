/*
  ESP32CAM Rover Robot 
  esp32cam-robot.ino (requires app_httpd.cpp)
  Based upon Espressif ESP32CAM Examples
  Uses TBA6612FNG H-Bridge Controller
  
  CodingScientist 2021-2022
  https://www.codingscientist.com
*/

#include "esp_wifi.h"
#include "esp_camera.h"
#include <WiFi.h>
#include "soc/soc.h"
#include "soc/rtc_cntl_reg.h"

// Setup Access Point Credentials
const char* ssid1 = "ESP32-CAM Robot";
const char* password1 = "1234567890";

extern volatile unsigned int  motor_speed;
extern void robot_stop();
extern void robot_setup();
extern uint8_t robo;
extern volatile unsigned long previous_time;        
extern volatile unsigned long move_interval; 

#define CAMERA_MODEL_AI_THINKER
#define PWDN_GPIO_NUM     32
#define RESET_GPIO_NUM    -1
#define XCLK_GPIO_NUM      0
#define SIOD_GPIO_NUM     26
#define SIOC_GPIO_NUM     27

#define Y9_GPIO_NUM       35
#define Y8_GPIO_NUM       34
#define Y7_GPIO_NUM       39
#define Y6_GPIO_NUM       36
#define Y5_GPIO_NUM       21
#define Y4_GPIO_NUM       19
#define Y3_GPIO_NUM       18
#define Y2_GPIO_NUM        5
#define VSYNC_GPIO_NUM    25
#define HREF_GPIO_NUM     23
#define PCLK_GPIO_NUM     22

void startCameraServer();

void setup() 
{
  WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0); // prevent brownouts by silencing them
  
  Serial.begin(115200);
  Serial.setDebugOutput(true);
  Serial.println();



  camera_config_t config;
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
  config.pin_sscb_sda = SIOD_GPIO_NUM;
  config.pin_sscb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;
  config.xclk_freq_hz = 20000000;
  config.pixel_format = PIXFORMAT_JPEG;
  //init with high specs to pre-allocate larger buffers
  if(psramFound()){
    config.frame_size = FRAMESIZE_QVGA;
    config.jpeg_quality = 10;
    config.fb_count = 2;
  } else {
    config.frame_size = FRAMESIZE_QVGA;
    config.jpeg_quality = 12;
    config.fb_count = 1;
  }

  // camera init
  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("Camera init failed with error 0x%x", err);
    return;
  }

  //drop down frame size for higher initial frame rate
  sensor_t * s = esp_camera_sensor_get();
  s->set_framesize(s, FRAMESIZE_QVGA);
  s->set_vflip(s, 1);
  s->set_hmirror(s, 1);

  WiFi.softAP(ssid1, password1);
  IPAddress myIP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(myIP);
  
  startCameraServer();

  ledcSetup(7, 5000, 8);
  ledcAttachPin(4, 7);  //pin4 is LED
  robot_setup();
  
  for (int i=0;i<5;i++) 
  {
    ledcWrite(7,10);  // flash led
    delay(50);
    ledcWrite(7,0);
    delay(50);    
  }
      
  previous_time = millis();
}

void loop() {
  if(robo)
  {
    unsigned long currentMillis = millis();
    if (currentMillis - previous_time >= move_interval) {
      previous_time = currentMillis;
      robot_stop();
      char rsp[32];
      sprintf(rsp,"SPPED: %d",motor_speed);
      Serial.println("Stop");
      robo=0;
    }
  }
  delay(1);
  yield();
}
