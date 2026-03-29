//Aidan 21274
//Libarys
#include <Adafruit_GFX.h>
#include <Adafruit_ST7789.h>
#include <SPI.h>

//Create object for the tft screen
Adafruit_ST7789 TFTScreen = Adafruit_ST7789(TFT_CS, TFT_DC, TFT_RST);

//Pins
const byte SEN1PIN = 1;
const byte SEN2PIN = 2;

unsigned long Sen1Triggered = 0;
unsigned long Sen2Triggered = 0;

void setup() 
{
  //Set pin modes
  pinMode(SEN1PIN, INPUT);
  pinMode(SEN2PIN, INPUT);
  pinMode(TFT_BACKLITE, OUTPUT);
  pinMode(TFT_I2C_POWER, OUTPUT);

  //Turn on backlite and power to high
  digitalWrite(TFT_BACKLITE, HIGH);
  digitalWrite(TFT_I2C_POWER, HIGH);
  delay(10);

  //Initlize screen res, rotation and bg color
  TFTScreen.init(135, 240);
  TFTScreen.setRotation(3);
  TFTScreen.fillScreen(ST77XX_BLACK);

  // Start and then wait for serial monitor to intialize
  Serial.begin(115200);
  while (!Serial){}
  Serial.println("- Setup Complete -");
}

void loop() 
{
  //Read sensor values
  bool Inside1Val = digitalRead(SEN1PIN);
  bool Outside2Val = digitalRead(SEN2PIN);

  //If outside triggered update outside millis and call the triggered function
  if (Outside2Val)
  {
    Sen2Triggered = millis();
    Triggered();
  }

  //If inside triggered update inside millis and call the triggered function
  if (Inside1Val)
  {
    Sen1Triggered = millis();
    Triggered();
  }
}

void Triggered()
{
  // If Inside Was Triggered First
  if(Sen1Triggered <= Sen2Triggered)
    {
      // Sets where to write text
      TFTScreen.setCursor(0, 30);
      // Sets text color
      TFTScreen.setTextColor(ST77XX_RED);
      // Sets text scale
      TFTScreen.setTextSize(3);

      // Print Hello World to screen and serial
      TFTScreen.println("Hello World");
      Serial.println("Hello World"); 
    }

    // If Outside Was Triggered First
    if(Sen1Triggered >= Sen2Triggered)
    {
      // Sets where to write text
      TFTScreen.setCursor(0, 30);
      // Sets text color
      TFTScreen.setTextColor(ST77XX_RED);
      // Sets text scale
      TFTScreen.setTextSize(3);

      // Print Bye World to screen and serial
      TFTScreen.println("Bye World");
      Serial.println("Bye World"); 
    }
}
