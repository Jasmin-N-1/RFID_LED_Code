/*
   Typical pin layout used:
   -------------------------------------------------------------------------------------------------------
               MFRC522      Arduino       Arduino   Arduino    Arduino          Arduino     Raspberry Pico
               Reader/PCD   Uno/101       Mega      Nano v3    Leonardo/Micro   Pro Micro
   Signal      Pin          Pin           Pin       Pin        Pin              Pin         Pin
   -------------------------------------------------------------------------------------------------------
   RST/Reset   RST          9             5         D9         RESET/ICSP-5     RST         GP11
   SPI SS      SDA(SS)      10            53        D10        10               10          GP8
   SPI MOSI    MOSI         11 / ICSP-4   51        D11        ICSP-4           16          GP19
   SPI MISO    MISO         12 / ICSP-1   50        D12        ICSP-1           14          GP16
   SPI SCK     SCK          13 / ICSP-3   52        D13        ICSP-3           15          GP18
*/

/*
Libraries needed:
- USBHID by Various
- MFRC522 by GithubCommunity
*/

/* iPad Pro key release issue:
Add delay(20) in USBKeyboard.cpp

if (!send(&report)) {
        _mutex.unlock();
        return false;
    }

    delay(20);

    report.data[1] = 0;
    report.data[3] = 0;

*/

#include <SPI.h>
#include <MFRC522.h>
#include "PluggableUSBHID.h"
#include "USBKeyboard.h"

#include <Adafruit_NeoPixel.h>
//#include <seesaw_neopixel.h>

// RFID / HID
USBKeyboard Keyboard;

#define RST_PIN 11  // Pico //9          // Configurable, see typical pin layout above
#define SS_PIN 8    // Pico //10         // Configurable, see typical pin layout above

// Neopixel
//#define PIN_NEODRIVER 15         // Neodriver pin
#define PIN        0 // GP0
#define NUMPIXELS 60
Adafruit_NeoPixel pixels(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);

//seesaw_NeoPixel pixels = seesaw_NeoPixel(NUMPIXELS, PIN_NEODRIVER, NEO_GRB + NEO_KHZ800);


MFRC522 mfrc522(SS_PIN, RST_PIN);  // Create MFRC522 instance

long code = 0;
long last_code = 0;
long card = 0;


int startColor = pixels.Color(30,0,255);
int ernteColor = pixels.Color(5,0,248);
int spinnenColor = pixels.Color(0,148,255);
int webenColor = pixels.Color(7,249,104);
int naehenColor = pixels.Color(0,147,23);

int ernteDelay = 30000; //hier wird die länge in Millisekunden definiert, an Länge der Figma Animation anpassen
int spinnenDelay = 30000;
int webenDelay = 28000;
int naehenDelay = 28000;

#define brightness 30
#define speedstart 150 //Geschwindigkeit der Start Animation
#define speedevent 100 //Geschwindigkeit der Event Animationen


void setup() {
  Serial.begin(9600);  // Initialize serial communications with the PC

  SPI.begin();                        // Init SPI bus
  mfrc522.PCD_Init();                 // Init MFRC522
  delay(4);                           // Optional delay. Some board do need more time after init to be ready, see Readme
  mfrc522.PCD_DumpVersionToSerial();  // Show details of PCD - MFRC522 Card Reader details
  //Serial.println(F("Scan PICC to see UID, SAK, type, and data blocks..."));

  pixels.begin();
}

void loop() {
  // Reset the loop if no new card present on the sensor/reader. This saves the entire process when idle.
  card = 0;

  start(speedstart);
  pixels.show();

  if (mfrc522.PICC_IsNewCardPresent()) {
    card = 1;
  } else {
    if (mfrc522.PICC_IsNewCardPresent())
      card = 1;
    else {
      if (card == 1)
        Keyboard.printf("x");
      card = 0;
    }
  }

  if (!mfrc522.PICC_ReadCardSerial()) {
    return;
  }

  if (card == 1) {
    long code = 0;
    for (byte i = 0; i < mfrc522.uid.size; i++) {
      code = ((code + mfrc522.uid.uidByte[i]) * 10);  // Nun werden wie auch vorher die vier Blöcke ausgelesen und in jedem Durchlauf wird der Code mit dem Faktor 10 „gestreckt“. (Eigentlich müsste man hier den Wert 1000 verwenden, jedoch würde die Zahl dann zu groß werden.
    }
    Serial.println(code);

    // if code then key
    if (code == 1518040) {
      Keyboard.printf("A");
      pixels.clear();
      event(speedevent, ernteColor, ernteDelay);
      //pixels.setPixelColor(1, pixels.Color(0, 150, 0)); //1 war i
      pixels.show();
    } else if (code == 1000540) {
      Keyboard.printf("B");
      pixels.clear();
      event(speedevent, spinnenColor, spinnenDelay);
      pixels.show();
    } else if (code == 1179440) {
      Keyboard.printf("C");
       pixels.clear();
      event(speedevent, webenColor, webenDelay);
      pixels.show();
    } else if (code == 2483440) {
      pixels.clear();
      event(speedevent, naehenColor, naehenDelay);
      pixels.show();
      Keyboard.printf("D");
    } else if (code == 1009240) {
      Keyboard.printf("E");
    } else if (code == 2186790) {
      Keyboard.printf("F");
    } else if (code == 1793450) {
      Keyboard.printf("G");
    } else if (code == 1674360) {
      Keyboard.printf("H");
    }

    mfrc522.PICC_HaltA();

    last_code = code;
  } else if (card == 0) {
    //Serial.print("No Card: ");
  }

/*start(speedstart); //immer an wenn kein Gegenstand im RFID Scanner liegt, in den Loop rein

event(speedevent, ernteColor, ernteDelay); //die events sind die jeweiligen LED Animationen zu den einzelnen Figma animationen, diese kommen dann jeweils in die richtige If Abfrage mit rein
event(speedevent, spinnenColor, spinnenDelay);
event(speedevent, webenColor, webenDelay);
event(speedevent, naehenColor, naehenDelay);
*/

  delay(500);
}


void start(uint16_t delay_ms){
  pixels.setBrightness(brightness);
  for(uint8_t i = 0; i<NUMPIXELS/2+2; i++){ //LEDs werden nacheinander angemacht, bis zur Mitte
    pixels.setPixelColor(i, startColor); //animation von erster LED aus 5,0,248
    pixels.setPixelColor(NUMPIXELS-1-i, startColor); //animation von letzter LED aus
    if(i>=3){
      pixels.setPixelColor(i-3,0,0,0); //Led wieder aus
      pixels.setPixelColor(NUMPIXELS-i+2, 0,0,0); //Led wieder aus
    }
    pixels.show();
    delay(delay_ms);
  }
  clearPixels();
}


void event(uint16_t animationspeed, uint32_t color, int animationdelay){
  pixels.setBrightness(brightness);
  for(uint8_t i = 0; i<NUMPIXELS/2+1; i++){ //LEDs werden nacheinander angemacht
    pixels.setPixelColor(i, color); //animation von erster LED aus 0,148,255
    pixels.setPixelColor(NUMPIXELS-1-i, color); //animation von letzter LED aus
    pixels.show();
    delay(animationspeed);
  }
  delay(animationdelay);
}

void clearPixels(){
  for(uint8_t i = 0; i<NUMPIXELS; i++){
    pixels.setPixelColor(i,0,0,0);
  }
  pixels.show();
}