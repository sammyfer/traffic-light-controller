//I really don't know
#include "soc/soc.h"           // Disable brownour problems
#include "soc/rtc_cntl_reg.h"  // Disable brownour problems
#include "driver/rtc_io.h"

//String
#include "string.h"

//Camera
#include "esp_camera.h"
#define CAMERA_MODEL_AI_THINKER // Has PSRAM
#include "camera_pins.h"

//Wifi
#include <WiFi.h>

//FTP
#include "ESP32_FTPClient.h"

//HTTP Client
#include <ArduinoHttpClient.h>

// Wifi vars
const char* ssid = "Fernandes";
const char* password = "17071970";
WiFiClient wifi;

//HTTP Client
char serverAddress[] = "ec2-18-230-151-174.sa-east-1.compute.amazonaws.com";  // server address
int port = 80;
HttpClient client = HttpClient(wifi, serverAddress, port);
int status = WL_IDLE_STATUS;

//Traffic light var
String trafficLightID = "2";

//FTP vars
char* ftp_server = "ec2-18-230-151-174.sa-east-1.compute.amazonaws.com"; //Public IPv4 DNS
char* ftp_user = "esp32user";
char* ftp_pass = "esp32user123";
char* ftp_path = "/semaforo_2";
ESP32_FTPClient ftp (ftp_server, ftp_user, ftp_pass, 5000, 2);

//LED vard
const byte VERDE = 12;
const byte AMARELO = 2;
const byte VERMELHO = 15;

void startCameraServer();

void setup() {
  Serial.begin(115200);
  Serial.setDebugOutput(true);
  Serial.println();

  //Initiate camera
  initCamera();

  //Open FTP connection
  ftp.OpenConnection();

  pinMode(VERDE, OUTPUT);
  pinMode(AMARELO, OUTPUT);
  pinMode(VERMELHO, OUTPUT);
}

void loop() {
  // put your main code here, to run repeatedly:
  abreSemaforo(trafficLightID);
  fechaSemaforo(trafficLightID);
}

void abreSemaforo(String tfID) {
  String own = getOwnStatus(tfID);
  String op = getOppositeStatus(tfID);
  if (strcmp(own.c_str(), "1\n") == 0 && strcmp(op.c_str(), "0\n") == 0) {
    delay(2000);
    Serial.println("Abrindo Semaforo");
    digitalWrite(VERMELHO, LOW);
    digitalWrite(VERDE, HIGH);
    while (strcmp(own.c_str(), "1\n") == 0 && strcmp(op.c_str(), "0\n") == 0) {
      takePhoto();
      delay(1000);
      own = getOwnStatus(tfID);
      op = getOppositeStatus(tfID);
    }
  }
}

void fechaSemaforo(String tfID) {
  String own = getOwnStatus(tfID);
  String op = getOppositeStatus(tfID);
  if (strcmp(own.c_str(), "0\n") == 0 && strcmp(op.c_str(), "1\n") == 0) {
    digitalWrite(VERDE, LOW);
    digitalWrite(AMARELO, HIGH);
    delay(2000);
    digitalWrite(AMARELO, LOW);
    digitalWrite(VERMELHO, HIGH);
    while (strcmp(own.c_str(), "0\n") == 0 && strcmp(op.c_str(), "1\n") == 0) {
      takePhoto();
      delay(1000);
      own = getOwnStatus(tfID);
      op = getOppositeStatus(tfID);
    }
  }
}

String getOwnStatus(String tfID) {
  //Serial.println("making GET request");
  String getCommand = "/resp_semaforo_";
  getCommand.concat(tfID);
  getCommand.concat(".txt");
  client.get(getCommand);

  // read the status code and body of the response
  int statusCode = client.responseStatusCode();
  String response = client.responseBody();

  Serial.print("Semaforo "+tfID+": ");
  Serial.println(response.c_str());
  return response;
}

String getOppositeStatus(String tfID) {
  String opposite;
  if (tfID == "1"){
    opposite = "2";
  } else {
    opposite = "1";
  }
  //Serial.println("making GET request");
  String getCommand = "/resp_semaforo_";
  getCommand.concat(opposite);
  getCommand.concat(".txt");
  client.get(getCommand);

  // read the status code and body of the response
  int statusCode = client.responseStatusCode();
  String response = client.responseBody();
  
  Serial.print("Semaforo "+opposite+": ");
  Serial.print(response.c_str());
  return response;
}

void takePhoto() {
  camera_fb_t * fb = NULL;

  // Take Picture with Camera
  fb = esp_camera_fb_get();
  if (!fb) {
    Serial.println("Camera capture failed");
    return;
  }

  //Upload to ftp server

  ftp.ChangeWorkDir(ftp_path);
  ftp.InitFile("Type I");

  //String nombreArchivo = timeClient.getFormattedTime()+".jpg"; // AAAAMMDD_HHMMSS.jpg
  String nombreArchivo = "imagem.jpg"; // AAAAMMDD_HHMMSS.jpg
  Serial.println("Subiendo " + nombreArchivo);
  int str_len = nombreArchivo.length() + 1;

  char char_array[str_len];
  nombreArchivo.toCharArray(char_array, str_len);

  ftp.NewFile(char_array);
  ftp.WriteData( fb->buf, fb->len );
  ftp.CloseFile();

  /*
     Free
  */
  esp_camera_fb_return(fb);
}

void initCamera() {
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

  // if PSRAM IC present, init with UXGA resolution and higher JPEG quality
  //                      for larger pre-allocated frame buffer.
  if (psramFound()) {
    config.frame_size = FRAMESIZE_UXGA;
    config.jpeg_quality = 10;
    config.fb_count = 2;
  } else {
    config.frame_size = FRAMESIZE_SVGA;
    config.jpeg_quality = 12;
    config.fb_count = 1;
  }

#if defined(CAMERA_MODEL_ESP_EYE)
  pinMode(13, INPUT_PULLUP);
  pinMode(14, INPUT_PULLUP);
#endif

  // camera init
  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("Camera init failed with error 0x%x", err);
    return;
  }

  sensor_t * s = esp_camera_sensor_get();
  // initial sensors are flipped vertically and colors are a bit saturated
  if (s->id.PID == OV3660_PID) {
    s->set_vflip(s, 1); // flip it back
    s->set_brightness(s, 1); // up the brightness just a bit
    s->set_saturation(s, -2); // lower the saturation
  }
  // drop down frame size for higher initial frame rate
  s->set_framesize(s, FRAMESIZE_UXGA);

#if defined(CAMERA_MODEL_M5STACK_WIDE) || defined(CAMERA_MODEL_M5STACK_ESP32CAM)
  s->set_vflip(s, 1);
  s->set_hmirror(s, 1);
#endif

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");

  startCameraServer();

  Serial.print("Camera Ready! Use 'http://");
  Serial.print(WiFi.localIP());
  Serial.println("' to connect");
}
