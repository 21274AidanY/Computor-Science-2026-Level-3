#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <LittleFS.h>
#include <ArduinoJson.h>

const char* SSID = "REPLACE_WITH_YOUR_SSID";
const char* PASSWORD = "REPLACE_WITH_YOUR_PASSWORD";

AsyncWebServer server(80);

const char* PARAM_INPUT_1 = "input1";
const char* PARAM_INPUT_2 = "input2";
const char* PARAM_INPUT_3 = "input3";

String input1;
String input2;
String input3;

const char* INPUT1PATH = "/input1.txt";
const char* INPUT2PATH = "/input2.txt";
const char* INPUT2PATH = "/input3.txt";

void initLittleFs() 
{
  if (!LittleFS.begin(true)) 
  {
    Serial.println("An error has occurred while mounting LittleFS");
  } 
  else 
  {
    Serial.println("LittleFS mounted successfully");
  }
}

String readFile(fs::FS &fs, const char* PATH) {
  Serial.printf("Reading file: %s\r\n", PATH);
  File file = fs.open(PATH);

  if (!file || file.isDirectory()) 
  {
    Serial.println("- failed to open file for reading");
    return String();
  }

  String fileContent;

  while (file.available()) 
  {
    fileContent = file.readStringUntil('\n');
    break;
  }

  return fileContent;
}

void writeFile(fs::FS &fs, const char* PATH, const char* MESSAGE) 
{
  Serial.printf("Writing file: %s\r\n", PATH);
  File file = fs.open(PATH, FILE_WRITE);

  if (!file) 
  {
    Serial.println("- failed to open file for writing");
    return;
  }

  if (file.print(MESSAGE)) 
  {
    Serial.println("- file written");
  } 
  else 
  {
    Serial.println("- write failed");
  }
}

void initWiFi() 
{
  WiFi.mode(WIFI_STA);
  WiFi.begin(SSID, PASSWORD);
  Serial.print("Connecting to WiFi ..");

  while (WiFi.status() != WL_CONNECTED) 
  {
    Serial.print('.');
    delay(1000);
  }

  Serial.println(WiFi.localIP());
}

String getCurrentInputValues() 
{
  JsonDocument values;
  values["launchTextValue"] = input1;
  values["targetTextValue"] = input2;
  values["ftsTextValue"] = input3;

  String jsonString = "";
  serializeJson(values, jsonString);
  return jsonString;
}

void setup() 
{
  Serial.begin(115200);

  input1 = readFile(LittleFS, INPUT1PATH);
  input2 = readFile(LittleFS, INPUT2PATH);
  input3 = readFile(LittleFS, INPUT3PATH);

  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(LittleFS, "/index.html", "text/html");
  });

  server.serveStatic("/", LittleFS, "/");

  server.on("/values", HTTP_GET, [](AsyncWebServerRequest *request) {
    String json = getCurrentInputValues();
    request->send(200, "application/json", json);
    json = String();
  });

  server.on("/", HTTP_POST, [](AsyncWebServerRequest *request) {
    int params = request->params();

    for (int i = 0; i < params; i++) {
      const AsyncWebParameter* p = request->getParam(i);

      if (p->isPost()) {
        if (p->name() == PARAM_INPUT_1) {
          input1 = p->value().c_str();
          Serial.print("Input 1 set to: ");
          Serial.println(input1);
          writeFile(LittleFS, INPUT1PATH, input1.c_str());
        }

        if (p->name() == PARAM_INPUT_2) {
          input2 = p->value().c_str();
          Serial.print("Input 2 set to: ");
          Serial.println(input2);
          writeFile(LittleFS, INPUT2PATH, input2.c_str());
        }

        if (p->name() == PARAM_INPUT_3) {
          input2 = p->value().c_str();
          Serial.print("Input 3 set to: ");
          Serial.println(input3);
          writeFile(LittleFS, INPUT3PATH, input2.c_str());
        }
      }
    }

    request->send(LittleFS, "/index.html", "text/html");
  });

  server.begin();
}

void loop() {
}