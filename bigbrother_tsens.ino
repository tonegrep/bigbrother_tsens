#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <ESP8266WebServer.h>
#include <ESP8266HTTPClient.h>
#include <ArduinoJson.h>
#include <ESP8266mDNS.h>
#include <bigbrother_common.h>
#include <Timer.h>

const char* CONTROLLER_UUID = "PefwR97iuzyUPqZZlQ3W";
int humidityPin = D5;
int CONTROLLER_PORT = 302;
int CURRENT_HUMIDITY;
const char* SSID = "aaaaaaaaaaaaaaa";
const char* PASSWORD = "89153510369";
const char* GLOBAL_IP;
const char* LOCAL_IP;
int STATUS_CODE;
MDNSResponder mdns;
ESP8266WebServer server(CONTROLLER_PORT);

Timer timer_send_data;

void setup() {
  Serial.begin(115200);
  pinMode(humidityPin, INPUT);
  
  // Connect to WiFi network
  WiFi.begin(SSID, PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("WiFi connected");
  // Setup mdns
  if (mdns.begin("esp8266WebForm", WiFi.localIP())) {
    Serial.println("MDNS responder started");
  }
  // Bound functions to requests
  server.on("/", page);
  server.on("/GET", get_humidity);

  server.begin();
  Serial.println("Server started");
  Serial.println(WiFi.localIP());
  timer_send_data.every(10000, send_data, 0);
}

void loop() {
  server.handleClient();
  timer_send_data.update();
}

void page() {
   String s = "<!DOCTYPE HTML>\r\n<html>\r\n";
   s += "<h3>current humidity is ";
   s += analogRead(humidityPin);
   s += "</h3>";
   s += "</html>\n";
  respond_ok("text/html", s);
}

void get_humidity() {
  String response = String(analogRead(humidityPin));
  respond_ok("text/plain", response);
}

void send_data(void* context) {
  HTTPClient http;
  http.begin(HOST_TRANSMIT);
  http.addHeader("Content-Type", "text/json");
  int httpCode = http.POST(create_response(JOB_CLEAR, analogRead(humidityPin)));
  String payload = http.getString();
  http.end();
}

void respond_ok(String response_type, String response){
  server.sendHeader("Connection", "close");
  server.sendHeader("Access-Control-Allow-Origin", "*");
  server.send(200, response_type, response);
}

String create_response(int response_code, int data) {
  const int capacity = JSON_OBJECT_SIZE(5);
  StaticJsonDocument<capacity> doc;
  doc["type"] = CONTROLLER_TYPE_SENSOR;
  doc["UUID"] = CONTROLLER_UUID;
  doc["status"] = STATUS_CODE;
  doc["response_code"] = response_code;
  doc["data"] = data;
  String output;
  serializeJson(doc, output);
  return output;
}
