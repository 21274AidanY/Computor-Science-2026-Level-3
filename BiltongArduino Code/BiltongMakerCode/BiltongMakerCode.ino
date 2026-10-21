//21274 - Aidan Young
//Biltong Maker

#include <WiFi.h>
#include <Adafruit_GFX.h>
#include <LittleFS.h>
#include <Wire.h>
#include <SPI.h>
#include <Adafruit_AHTX0.h>
#include <Adafruit_BMP280.h>
#include <Adafruit_ST7789.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <Arduino_JSON.h>
#include <ESPmDNS.h>


//--- WiFi Info/Config ---
const char SSID[] = "KANDY";
const char PASSWORD[] = "Br33andA1dan";

//------------- WebServer setup --------------
//Create an async web server on port 80
AsyncWebServer server(80);
// Create a WebSocket object
AsyncWebSocket ws("/ws");

//--- Screen Info/Config---
Adafruit_ST7789 TFTScreen = Adafruit_ST7789(TFT_CS, TFT_DC, TFT_RST);

//--- AHT and BMP sensors ---
//Uses I2C interface, no pin declaration needed
Adafruit_AHTX0 aht; //Tempreature and humidity
Adafruit_BMP280 bmp; //Tempreature and preassure

byte FanPIN;
byte LampPIN;

//Info
float TempValue; //Read from AHT, in C units
float HumidtyValue; //Read from AHT, % RH
float PreasureValue; //Read from BMP, in hPA units

//LittleFS setup - wants to be first method for some reason?
void initLittleFS() 
{
  if (!LittleFS.begin(true)) 
  {
    Serial.println("LittleFS mount failed!");
    return;
  }

  Serial.println("LittleFS mounted successfully!");

  Serial.print("index.html exists: ");
  Serial.println(LittleFS.exists("/index.html"));

  Serial.print("Total filesystem bytes: ");
  Serial.println(LittleFS.totalBytes());
}

//LittleFS reading a file 
String readFile(fs::FS& fs, const char* path) 
{
  Serial.print("Reading file: ");
  Serial.println(path);
  File file = fs.open(path);
  if (!file || file.isDirectory()) 
  {
    Serial.println("readFile failed to open file");
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

//LittleFS writing a file
void writeFile(fs::FS& fs, const char* path, const char* message) 
{
  Serial.print("Writing file: ");
  Serial.println(path);
  File file = fs.open(path, FILE_WRITE);
  if (!file) 
  {
    Serial.println("writeFile failed to write to file");
    return;
  }
  if (file.print(message)) 
  {
    Serial.println("- writeFile success");
  } 
  else 
  {
    Serial.println("- writeFile failed");
  }
}

/**
set up a JSON array and hold the states of all values we care about
pins and their current states variables and their current values
*/
String getOutputStates() 
{
  JSONVar myArray;

  myArray["values"][0]["humidity"] = String(HumidtyValue);
  myArray["values"][0]["temp"] = String(TempValue);

  String jsonString = JSON.stringify(myArray);
  return jsonString;
}

/**
Sends a message to all clients connected to ip address with a JSON message
*/
void notifyClients(String state) {
  ws.textAll(state);
}

/**
Handles messages from clients in the if statements below
*/
void handleWebSocketMessage(void* arg, uint8_t* data, size_t len) {
  AwsFrameInfo* info = (AwsFrameInfo*)arg;
  if (info->final && info->index == 0 && info->len == len && info->opcode == WS_TEXT) {
    data[len] = 0;
    //if message is states send the state of all gpio pins
    if (strcmp((char*)data, "getStates") == 0) {
      notifyClients(getOutputStates());
    } else {
      //HAN COMMENT is the next 10 lines of code still needed? or does it need some adjustment?
      //(char*)data should be in this format "rgb(255,87,51)" when it isn't 'getStates'
      std::string rgb = (char*)data;
      byte firstComma = rgb.find(',');
      byte secondComma = rgb.find(',', firstComma + 1);
      byte lastBracket = rgb.find(')');
    }
    //Just to alternate the led on and off to show we can digitalRead and digitalWrite here
    //digitalWrite(LEDPIN, !digitalRead(LEDPIN)); //AM I AN ISSUE NEED TO TEST--------------------------------------
  }
}

/**
Event listener handles different asyncronous steps from the web socket
*/
void onEvent(AsyncWebSocket* server, AsyncWebSocketClient* client, AwsEventType type, void* arg, uint8_t* data, size_t len) {
  switch (type) {
    case WS_EVT_CONNECT:
      //Serial.printf("WebSocket client #%u connected from %s\n", client->id(), client->remoteIP().toString().c_str());
      Serial.print("WebSocket client ");
      Serial.print(client->id());
      Serial.print(" connected from ");
      Serial.println(client->remoteIP().toString().c_str());
      break;
    case WS_EVT_DISCONNECT:
      //Serial.printf("WebSocket client #%u disconnected\n", client->id());
      Serial.print("WebSocket client ");
      Serial.print(client->id());
      Serial.println(" disconnected\n");
      break;
    case WS_EVT_DATA:
      handleWebSocketMessage(arg, data, len);
      break;
    case WS_EVT_PONG:
    case WS_EVT_ERROR:
      break;
  }
}


/**
WebSocket setup
What to do when an event happens on the ws socket and adding a handler to the server
*/
void initWebSocket() {
  ws.onEvent(onEvent);
  server.addHandler(&ws);
  Serial.println("websocket should be running now");
}

//HAN COMMENT tell me what setup is setting up
void setup() 
{
  //Initilize serial monitor and wait
  Serial.begin(115200);
  while (!Serial) 
  {
    delay(100);
  }

  Serial.println("- Serial monitor initilized successfully - ");

  Serial.println("Attempting to connect to WIFI");
  WiFi.mode(WIFI_STA);
  WiFi.begin(SSID, PASSWORD);
  
  Serial.print("Loading.");
  while (WiFi.status() != WL_CONNECTED) 
  {
    delay(1000);
    Serial.print(".");
  }
  Serial.println("..");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());

  Serial.println("- WiFi has been initilized successfully -");

  // --- mDNS setup ---
  if (!MDNS.begin("biltongmaker")) {  // <-- this becomes "biltongmaker.local"
    Serial.println("Error starting mDNS");
  } else {
    Serial.println("mDNS responder started: http://biltongmaker.local");
  }

  Serial.println("- Initilizing LittleFS -");
  initLittleFS();

  Serial.println("Checking website files:");

  Serial.print("index.html exists: ");
  Serial.println(LittleFS.exists("/index.html"));

  Serial.print("File system total bytes: ");
  Serial.println(LittleFS.totalBytes());

  Serial.println("- Initilizing Sensors -");
  // AHT20 initilize
  if (!aht.begin()) 
  {
    Serial.println("AHT20 not found!");
    while (1);
  }

  // BMP280 init (0x76 or 0x77)
  if (!bmp.begin(0x77)) 
  {
    Serial.println("BMP280 not found at 0x77!");
    while (1);
  }

  Serial.println("- Initilizing WebServer -");

  server.serveStatic("/", LittleFS, "/").setDefaultFile("index.html");

  initWebSocket();
  server.begin();
  Serial.println("- WebServer initilized successfully -");

  Serial.println("- Sensors initilized successfully -");

  pinMode(TFT_I2C_POWER, OUTPUT);
  digitalWrite(TFT_I2C_POWER, HIGH);
  pinMode(TFT_BACKLITE, OUTPUT);
  digitalWrite(TFT_BACKLITE, HIGH);

  Serial.println("- Initilizing Screen -");
  TFTScreen.init(135, 240);
  TFTScreen.setRotation(3);
  TFTScreen.fillScreen(ST77XX_WHITE);
  Serial.println("- Screen initilized successfully -");

  Serial.println();
  Serial.println("--- SETUP COMPLETED ---");
  Serial.println();
}

//HAN COMMENT tell me the overview of what happens in this loop
void loop() 
{
  //read all sensor values
  ReadSensors();
  
  byte TempResponse = CheckRanges(TempValue,0,0);
  byte HumidtyResponse = CheckRanges(HumidtyValue,0,0);
  byte PressureResponse = CheckRanges(PreasureValue,0,0);

  notifyClients(getOutputStates());

  //Read measurement and check limits
  
  delay(2000);
}

//This method checks the data of choice and compares them to its ranges. It then reports back with a number to signify how it should react. (e.g. 1, increase data to fit range. -1, decrease data to fit range. 0 data fits range)
byte CheckRanges(float Data, float Min, float Max)
{
  if (Data >= Max)
  {
    return -1;
  }
  else if (Data <= Min)
  {
    return 1;
  }

  return 0;
}

//HAN COMMENT what does this method do? and do we need to do anything with the tft screen?
void ReadSensors()
{
  //Read sensors
  sensors_event_t humidity, temp;
  aht.getEvent(&humidity, &temp);

  //Update sensor values
  TempValue = temp.temperature;
  HumidtyValue = humidity.relative_humidity;
  PreasureValue = bmp.readPressure() / 100.0;

  //Print to serial
  Serial.print("AHT20 Temp: ");
  Serial.print(temp.temperature);
  Serial.print(" °C, Humidity: ");
  Serial.print(humidity.relative_humidity);
  Serial.println(" %");

  Serial.print("BMP280 Temp: ");
  Serial.print(bmp.readTemperature());
  Serial.print(" °C, Pressure: ");
  Serial.print(bmp.readPressure() / 100.0);
  Serial.println(" hPa");
}
