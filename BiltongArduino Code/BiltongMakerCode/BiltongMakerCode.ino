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


//--- WiFi Info/Config ---
const char SSID[] = "T8-Arduino";
const char PASSWORD[] = "T8-Arduino";

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

//Pins
byte FanPIN = 9;
byte LampPIN = 10;

//Settings
float TempMin;
float TempMax;
float HumidMin;
float HumidMax;

bool autoControl = true;

//Info
float TempValue; //Read from AHT, in C units
float HumidtyValue; //Read from AHT, % RH
//float PreasureValue; //Read from BMP, in hPA units

bool isHeating; //Is it currently trying to heat with the lamp
bool isDrying; //Is it currenntly trying to dry with the fan

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
void handleWebSocketMessage(void* arg, uint8_t* data, size_t len) 
{
  AwsFrameInfo* info = (AwsFrameInfo*)arg;

  Serial.println("received");

  if (info->final && info->index == 0 && info->len == len && info->opcode == WS_TEXT) 
  {

    // Convert the received bytes into a null-terminated string.
    String message;
    message.reserve(len);
    for (size_t i = 0; i < len; i++) {
      message += (char)data[i];
    }

    // Handle the original request for sensor readings.
    if (message == "getStates") {
      notifyClients(getOutputStates());
      return;
    }

    // Convert the JSON text into a JSON object.
    JSONVar json = JSON.parse(message);

    // Ignore invalid JSON.
    if (JSON.typeof(json) == "undefined") 
    {
      Serial.println("Invalid JSON received");
      return;
    }

    // Check whether this is a settings update.
    if (JSON.typeof(json["type"]) == "string") 
    {
      String messageType = (const char*)json["type"];

      if (messageType == "settings") 
      {
          TempMin = (double)json["TempMin"];
          TempMax = (double)json["TempMax"];
          HumidMin = (double)json["HumidMin"];
          HumidMax = (double)json["HumidMax"];
          autoControl = (bool)json["autoControl"];

          Serial.println("Settings updated:");
          Serial.printf("Temperature: %.1f - %.1f\n", TempMin, TempMax);
          Serial.printf("Humidity: %.1f - %.1f\n", HumidMin, HumidMax);
          Serial.printf("Automatic control: %s\n", autoControl ? "ON" : "OFF");
      }
    }
  }
}

/**
Event listener handles different asyncronous steps from the web socket
*/
void onEvent(AsyncWebSocket* server, AsyncWebSocketClient* client, AwsEventType type, void* arg, uint8_t* data, size_t len) 
{
  switch (type) 
  {
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

//This method initilizes and sets up the following in order: pin configuration, serial monitor, WIFI, LittleFS, ensures website files, webserver, AHT + BMP sensors and the inbuilt TFT screen.
void setup() 
{
  pinMode(FanPIN, OUTPUT);
  pinMode(LampPIN, OUTPUT);

  digitalWrite(FanPIN, LOW);
  digitalWrite(LampPIN, LOW);

  //Initilize serial monitor and wait
  Serial.begin(115200);
  while (!Serial) 
  {
    delay(100);
  }

  Serial.println("- Serial monitor initilized successfully - ");

  Serial.println("- Initilizing Screen -");
  pinMode(TFT_I2C_POWER, OUTPUT);
  digitalWrite(TFT_I2C_POWER, HIGH);
  pinMode(TFT_BACKLITE, OUTPUT);
  digitalWrite(TFT_BACKLITE, HIGH);

  TFTScreen.init(135, 240);
  TFTScreen.setRotation(3);
  TFTScreen.fillScreen(ST77XX_WHITE);
  TFTScreen.setTextSize(1);
  TFTScreen.setTextColor(ST77XX_RED);
  TFTScreen.setCursor(0, 0);
  Serial.println("- Screen initilized successfully -");

  Serial.println("Attempting to connect to WIFI");
  WiFi.mode(WIFI_STA);
  WiFi.begin(SSID, PASSWORD);
  
  Serial.print("Loading.");
  TFTScreen.println("Connecting to WIFI");
  while (WiFi.status() != WL_CONNECTED) 
  {
    delay(1000);
    Serial.print(".");
  }
  TFTScreen.fillScreen(ST77XX_WHITE);
  Serial.println("..");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());

  Serial.println("- WiFi has been initilized successfully -");

  Serial.println("- Initilizing LittleFS -");
  initLittleFS();

  Serial.println("Checking website files:");

  Serial.print("index.html exists: ");
  Serial.println(LittleFS.exists("/index.html"));

  Serial.print("File system total bytes: ");
  Serial.println(LittleFS.totalBytes());

  Serial.println("- Initilizing Sensors -");

  bool tempSensorOK = true;
  bool pressureSensorOK = true;

  // Initialise AHT20 temperature and humidity sensor
  if (!aht.begin())
  {
    Serial.println("ERROR: Tempreature sensor (AHT20) not found!");
    TFTScreen.println("ERROR: AHT20 not found!");
    TFTScreen.println("Check connections.");
    tempSensorOK = false;
  }

  // Initialise BMP280 pressure sensor
  if (!bmp.begin(0x77))
  {
    Serial.println("ERROR: Humidity sensor (BMP280) not found at 0x77!");
    TFTScreen.println("ERROR: BMP280 not found!");
    TFTScreen.println("Check connections.");
    pressureSensorOK = false;
  }

  // Stop if either required sensor failed
  if (!tempSensorOK || !pressureSensorOK)
  {
    Serial.println("Sensor initialisation failed. System halted.");
    while (true)
    {
      delay(1000);
    }
  }

  Serial.println("- Sensors initilized successfully -");

  Serial.println("- Initilizing WebServer -");

  server.serveStatic("/", LittleFS, "/").setDefaultFile("index.html");

  initWebSocket();
  server.begin();
  Serial.println("- WebServer initilized successfully -");

  Serial.println();
  Serial.println("--- SETUP COMPLETED ---");
  Serial.println();
}

//HAN COMMENT tell me the overview of what happens in this loop

//This loop firstly reads the AHT and BMP sensor values. It then comapres those results to the range settings receieved from the website. Depending on its results it will activate/deactivate the fan and/or heat lamp to attempt to bring the values within the set range.
//The sensor values are then sent off to the website so it can update its displays.
void loop()
{
  // Read current temperature and humidity
  ReadSensors();

  if (autoControl)
  {
    // HEAT LAMP:
    // Turn on below minimum, stay on until maximum is reached.
    if (TempValue <= TempMin)
    {
      isHeating = true;
    }
    else if (TempValue >= TempMax)
    {
      isHeating = false;
    }

    // FAN:
    // Turn on above maximum, stay on until minimum is reached.
    if (HumidtyValue >= HumidMax)
    {
      isDrying = true;
    }
    else if (HumidtyValue <= HumidMin)
    {
      isDrying = false;
    }

    // Apply the output states
    digitalWrite(LampPIN, isHeating ? HIGH : LOW);
    digitalWrite(FanPIN, isDrying ? HIGH : LOW);
  }
  else
  {
    // Automatic control disabled: turn both outputs off
    isHeating = false;
    isDrying = false;

    digitalWrite(LampPIN, LOW);
    digitalWrite(FanPIN, LOW);
  }

  // Send current readings and output states to the website
  notifyClients(getOutputStates());

  delay(2000);
}


//This method checks the data of choice and compares them to its ranges. It then reports back with a number to signify how it should react. (e.g. 1, increase data to fit range. -1, decrease data to fit range. 0 data fits range)
int CheckRanges(float Data, float Min, float Max)
{
  if (Data >= Max)
  {
    return -1; // Data is too high
  }
  else if (Data <= Min)
  {
    return 1; // Data is too low
  }

  return 0; // Data is within range
}

// This method reads all the values from the AHT and BMP sensor and then prints it to the serial monitor and TFT screen
void ReadSensors()
{
  //Read sensors
  sensors_event_t humidity, temp;
  aht.getEvent(&humidity, &temp);

  //Update sensor values
  TempValue = temp.temperature;
  HumidtyValue = humidity.relative_humidity;
  //PreasureValue = bmp.readPressure() / 100.0;

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

  // Clear the TFT screen
  TFTScreen.fillScreen(ST77XX_WHITE);

  // Set text appearance
  TFTScreen.setTextColor(ST77XX_BLACK);
  TFTScreen.setTextSize(2);

  // Print temperature
  TFTScreen.setCursor(10, 20);
  TFTScreen.print("Temperature:");
  TFTScreen.setCursor(10, 45);
  TFTScreen.print(TempValue, 1);
  TFTScreen.print(" C");

  // Print humidity
  TFTScreen.setCursor(10, 80);
  TFTScreen.print("Humidity:");
  TFTScreen.setCursor(10, 105);
  TFTScreen.print(HumidtyValue, 1);
  TFTScreen.print(" %");
}
