#include <Wire.h>
#include <Adafruit_RGBLCDShield.h>
#include <utility/Adafruit_MCP23017.h>

Adafruit_RGBLCDShield lcd = Adafruit_RGBLCDShield();

#define RED 0x1
#define YELLOW 0x3
#define GREEN 0x2
#define TEAL 0x6
#define BLUE 0x4
#define VIOLET 0x5
#define WHITE 0x7

const long samplingFrequency = 100;
const long sineFrequency = 10;
unsigned long sampleCounter = 0;
float samplingPeriod = 1.0 / (float) samplingFrequency * 1000.0;
float sineValue = 0;
float tri = 0;


void setup() {
  Serial.begin(9600);
  lcd.begin(16, 2);
  lcd.setBacklight(WHITE);
  setPwmFrequency(10, 1);
}

void loop() {
  uint8_t buttons = lcd.readButtons();
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Sine");
  while (!buttons) {
    sineWave();
    buttons = lcd.readButtons();
  }
  delay(300);
  buttons = 0;
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Triangle");
  while (!buttons) {
    triangleWave();
    buttons = lcd.readButtons();
  }
  delay(300);
}

void sineWave() {
  sineValue = 122.5 * sin(2 * M_PI * sineFrequency * sampleCounter / 1000.0) + 122.5;
  delay((int) samplingPeriod);
  analogWrite(10, sineValue);
  sampleCounter++;
  analogWrite(10, sineValue); 
  Serial.println(sineValue);
}
void triangleWave() {
  for (int i = 0; i <= 100; i++){
    if(i<=50){
      tri = (float)255 / 50 * i; 
      delay((int) samplingPeriod);
      analogWrite(10, tri); 
      Serial.println(tri);
    }
    else if (i>50){
      tri = 510 - (float)255 / 50 * i; 
      delay((int) samplingPeriod);
      analogWrite(10, tri); 
      Serial.println(tri);
    }
  }
}

void setPwmFrequency(int pin, int divisor) {
  byte mode;
  if (pin == 5 || pin == 6 || pin == 9 || pin == 10) {
    switch (divisor) {
      case 1: mode = 0x01; break;
      case 8: mode = 0x02; break;
      case 64: mode = 0x03; break;
      case 256: mode = 0x04; break;
      case 1024: mode = 0x05; break;
      default: return;
    }
    if (pin == 5 || pin == 6) {
      TCCR0B = TCCR0B & 0b11111000 | mode;
    } else {
      TCCR1B = TCCR1B & 0b11111000 | mode;
    }
  } else if (pin == 3 || pin == 11) {
    switch (divisor) {
      case 1: mode = 0x01; break;
      case 8: mode = 0x02; break;
      case 32: mode = 0x03; break;
      case 64: mode = 0x04; break;
      case 128: mode = 0x05; break;
      case 256: mode = 0x06; break;
      case 1024: mode = 0x7; break;
      default: return;
    }
    TCCR2B = TCCR2B & 0b11111000 | mode;
  }
}

