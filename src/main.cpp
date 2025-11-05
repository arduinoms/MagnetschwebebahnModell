#include <Arduino.h>
#include <Encoder.h>
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define RELAY_PIN_1 3
int relay_size = 6;

#define CLK 11
#define DT 10
#define SW 12

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

#define OLED_RESET     -1 // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3C ///< See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

Encoder meinEncoder(DT, CLK);
long altePosition = 0;
int selectedrelay = 0;
int mode = 0;
int programmeselect = 0;
boolean running = false;
long jumpTime = 1000;
long old_time1 = millis();
long old_time2 = millis();
long new_time;

volatile unsigned long lastInterruptTime = 0;
const unsigned long debounceDelay = 500; // milliseconds

void overlay() {
  display.clearDisplay();
  display.setTextSize(2);             // Normal 1:1 pixel scale
  display.setTextColor(SSD1306_WHITE);        // Draw white text
  display.setCursor(0,2);             
  display.println(F("Zeit:"));
  display.setCursor(80,2);            
  display.println(jumpTime);
  display.setCursor(0,22);             
  display.println(F("Modus:"));
  display.setCursor(80,22);
  if(mode==0){
    display.println(F("Aus"));
  }
  else if (mode==1)
  {
    display.println(F("Prg"));
  }
  if(programmeselect==0){
    display.fillRect(79,0,50,18,SSD1306_INVERSE);
  }
  else if(programmeselect==1){
    display.fillRect(79,20,37,18,SSD1306_INVERSE);
  }
  display.display();
}

void SwitchInterrupt()
{
  unsigned long interruptTime = millis();
  if (interruptTime - lastInterruptTime > debounceDelay) {
    programmeselect++;
    Serial.println(programmeselect);
    if(programmeselect > 1){
      programmeselect = 0;
    }
    lastInterruptTime = interruptTime;
  }
}

void checkEncoder() {
  long neuePosition = meinEncoder.read();
  if (neuePosition == altePosition+4 || neuePosition == altePosition-4)
  { 
    if(programmeselect == 0) {
      jumpTime = jumpTime + ((neuePosition - altePosition)/4) * 20;
    }
    else if(programmeselect == 1) {
      mode++;
      if(mode > 1){
        mode = 0;
      }
    }
    altePosition = neuePosition;
    overlay();
  }
}

void setup() // the setup function runs once when you press reset or power the board
{ 
  for (size_t i = 0; i < relay_size; i++)
  {
    pin_size_t pin = RELAY_PIN_1 + i;
    pinMode(pin, OUTPUT);
    digitalWrite(pin, HIGH); // Ensure all relays are off at startup
  }
  for (size_t i = 0; i < relay_size; i++)
  {
    digitalWrite(RELAY_PIN_1 + i, LOW);  // Turn on each relay one by one
    delay(250);                          // Wait for half a second
    digitalWrite(RELAY_PIN_1 + i, HIGH); // Turn off the relay
  }
  Serial.begin(9600);
  pinMode(SW, INPUT);
  attachInterrupt(digitalPinToInterrupt(SW), SwitchInterrupt, CHANGE);
  if(!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("SSD1306 allocation failed"));
    for(;;); // Don't proceed, loop forever
  }
  display.clearDisplay();
  display.display();
  overlay();
}

void loop()
{
  checkEncoder();
  new_time = millis();
  if(mode==1 && running==false){
    running = true;
  }
  if (new_time - old_time1 >= jumpTime && running==true) {
    old_time1 = new_time;
    digitalWrite(RELAY_PIN_1 + selectedrelay, LOW); 
    digitalWrite(RELAY_PIN_1 + selectedrelay-1, HIGH);
    selectedrelay++;
    if(selectedrelay > relay_size) {
      digitalWrite(RELAY_PIN_1 + selectedrelay-1, HIGH);
      selectedrelay = 0;
      if(mode==0){
        running = false;
      }
    }
  } 
} 