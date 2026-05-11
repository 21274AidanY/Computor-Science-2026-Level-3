//21274 - Aidan Young
//Biltong Maker

#include <WiFi.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ST7789.h>
#include <SPI.h>

//--- WiFi Info/Config ---
const char SSID[] = "SECRET_SSID";
const char PASSWORD[] = "SECRET_PASS";

//--- Screen Info/Config---
Adafruit_ST7789 tftScreen = Adafruit_ST7789(TFT_CS, TFT_DC, TFT_RST);

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
}

void loop() {
  // put your main code here, to run repeatedly:

}
