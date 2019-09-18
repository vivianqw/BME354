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

int val; 
int setTemp = 37;       //default temperature of the incubator
float hys = 0.5;        //default hysteresis of the incubator
uint8_t buttons; 

void setup() {          //setup code; set pin 7 (controls the circuit) and pin 2 (controls the buzzer) as output pins
  Serial.begin(9600);
  lcd.begin(16, 2);
  lcd.setBacklight(WHITE);
  pinMode(7, OUTPUT); 
  pinMode(2, OUTPUT); 
}


void loop() {
  buttons = lcd.readButtons();    //check if the user has pressed a button
  currentTemperature();           //the functions will be explained below
  checkTemp(); 
  lcd.setCursor(0,1); 
  lcd.print("Set: "); 
  lcd.print(setTemp); 
  if (buttons){
     setTemperature(); 
     setHysteresis(); 
  }
  heat(); 
  delay(100);
}

void currentTemperature(){        //reads the current analogIn and calculates the current temperature
  val = analogRead(1); 
  val = 0.1645 * val - 30.7;      //calculated from the calibration with hot water and ice water
  lcd.clear(); 
  lcd.setCursor(0,0); 
  lcd.print("Current: "); 
  lcd.print(val);                 //print the result for the current temperature on the LCD screen
}

void setTemperature(){            //allow the user to increment or decrease the temperature in 1 degree Celsius
  if (buttons & BUTTON_DOWN) {    //if the user pressed the down button, decrease the temperature...
    lcd.clear(); 
    lcd.setCursor(0, 1); 
    if (setTemp > 35){            //...only if the set temperature is higher than 35 degrees Celsius
      setTemp--; 
      lcd.print("Set: ");
      lcd.print(setTemp);
    }
    else{
      lcd.print("Wrong temp! "); 
    }
  }
  else if (buttons & BUTTON_UP) { //if the user pressed the up button, increase the temperature...
    lcd.clear(); 
    lcd.setCursor(0, 1); 
    if (setTemp < 42){            //...only if the set temperature is lower than 42 degrees Celsius
      setTemp++; 
      lcd.print("Set: ");
      lcd.print(setTemp);
    }
    else{
      lcd.print("Wrong temp! ");
    }  
  }
}

void setHysteresis(){             //allow the user to increase or decrease the hysteresis in 0.5 degree Celsius
  if (buttons & BUTTON_RIGHT) {   //if the user pressed the right button, increase the hysteresis...
    lcd.clear(); 
    lcd.setCursor(0, 1); 
    Serial.println(hys); 
    if (setTemp - hys > 35 && setTemp + hys < 42){   //...only if the set temperature +- hysteresis is within 35 and 42 degrees Celsius
      hys = hys + 0.5; 
      lcd.print("Hys: ");
      lcd.print(hys);
    }
    else{
      lcd.print("Wrong hysteresis! ");
    }
  }
  else if (buttons & BUTTON_LEFT) { //if the user pressed the left button, decrease the hysteresis...
    lcd.clear(); 
    lcd.setCursor(0, 1); 
    Serial.println(hys); 
    if (hys > 0.5){                 //...only if the current hysteresis is larger than 0.5 (prevents zero or negative hysteresis) 
      hys = hys - 0.5; 
      lcd.print("Hys: ");
      lcd.print(hys);
    }
    else{
      lcd.print("Wrong hysteresis! ");
    }  
  }
}

void checkTemp(){                   //if the current temperature is outside the range of 35-42 degrees Celsius, sound the buzzer
  if (val > 42 || val < 35){
    tone(2, 500); 
  }
  else{
    noTone(2); 
  }
}

void heat(){                        //if the current temperature is lower than the set temperature - hysteresis, set pin 7 to high, relay will turn on the circuit and start heating
  if (val + hys < setTemp){
    digitalWrite(7, HIGH); 
  }
  else if (setTemp + hys < val){    //if the current temperature is higher than the set temperature + hysteresis, set pin 7 to low, relay will turn off the circuit and stop heating
    digitalWrite(7, LOW); 
  }
}

