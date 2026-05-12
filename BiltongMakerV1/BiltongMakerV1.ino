//21274 - Aidan Young
//Biltong Maker

#include <WiFi.h>
#include <Adafruit_GFX.h>
//#include <Adafruit_ST7789.h>
#include <LittleFS.h>
#include <Wire.h>
#include <SPI.h>
#include <Adafruit_AHTX0.h>
#include <Adafruit_BMP280.h>

//--- WiFi Info/Config ---
const char SSID[] = "T8-Arduino";
const char PASSWORD[] = "T8-Arduino";

//--- Screen Info/Config---
//Adafruit_ST7789 tftScreen = Adafruit_ST7789(TFT_CS, TFT_DC, TFT_RST);

//--- AHT and BMP sensors ---
//Uses I2C interface, no pin declaration needed
Adafruit_AHTX0 aht; //Tempreature and humidity
Adafruit_BMP280 bmp; //Tempreature and preassure

//LittleFS setup - wants to be first method for some reason?
void initLittleFS() 
{
  if (!LittleFS.begin(true)) 
  {
    Serial.println("- LittleFS failed to initilize -");
  } 
  else 
  {
    Serial.println("- LittleFS has been initilized successfully -");
  }
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
  if (WL_CONNECTED == true) 
  {
    Serial.println(WiFi.localIP());
  } 
  else 
  {
    Serial.println("ERROR: Issues with displaying IP Adress");
  }

  Serial.println("- WiFi has been initilized successfully -");

  Serial.println("- Initilizing LittleFS -");
  initLittleFS();

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

  Serial.println("- Sensors initilized successfully -");
}

void loop() {
  //Test sensor readings
  sensors_event_t humidity, temp;
  aht.getEvent(&humidity, &temp);

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

  delay(2000);
}
