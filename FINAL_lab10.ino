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

uint8_t buttons;
int state = 1; //correspond to the states in the state diagram
int errorCount = 0;
float kPlus = -2.21;//calibrated k values
float kMinus = 2.21;
float normalEx = 0; //initialize all measured values and calculated values as 0 first
float normalIn = 0;
float forceEx = 0;
float forceIn = 0; 
float normalEx_dry = 0; 
float normalIn_dry = 0;
float forceEx_dry = 0; 
float forceIn_dry = 0;  
float VC = 0; 
float Vt = 0;  
float IC = 0;  
float EC = 0;  
float IRV = 0;  
float ERV = 0; 

void setup() {
  Serial.begin(9600);//initialize the LCD screen and serial
  lcd.begin(16, 2);
  lcd.setBacklight(WHITE);
}

void loop() {
  
  if(state == 1){//check if the spirometer is "zeroed", which means the voltage is around 2.5V
//    float voltage = (float)analogRead(1)*5/1023;
//    Serial.println(voltage);  //easy to debug
    delay(20);
    checkZero(); //function call
    errorCount++; 
    if (errorCount > 5){ //if check doesn't pass in 5 times, go to state 2, which displays the repair message
      state++; 
    }
  }
  
  if (state == 2){ //displays repair message
    repairMessage(); 
  }

  if (state == 3){ //prompts the user to choose manual or default calibration
    caliChoice(); 
  }

  if (state == 4){ //manual calibration; push the 3L syringe, then pull it
    pushCalc(); 
    pullCalc(); 
    state++; 
  }

  if (state == 5){ //measures and records normal exhale and normal inhale volume
    normalBreath(); 
  }

  if (state == 6){ //measures hte records forced exhale and forced inhale volume
    forceBreath(); 
  }

  if (state == 7){ //shows the result for the 6 calculated values
    showResults();
  }
}

void checkZero(){
    lcd.clear();
    lcd.setBacklight(WHITE);
    lcd.setCursor(0,0); 
    lcd.print("Checking if zero"); 
    delay(1000); //checks once in a second
    float total = 0; 
    for (int j=0; j<100; j++){ //averages 100 readings of voltage
      float voltage = (float)analogRead(1)*5/1023; 
      Serial.println(voltage); 
      total = total + voltage; 
    }
    float average = total / 100; 
    if (average > 2.45 && average < 2.55){ //if voltage is within acceptable range...
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Check passed"); //...pass the check and go to the calibration choice stage
      state = state + 2;
      delay(2000);
      lcd.clear(); 
    }
    lcd.clear(); 
}

void repairMessage(){
  lcd.setCursor(0,0); 
  lcd.print("Need repair"); //displays the "need repair" message for 2 seconds in red
  lcd.setBacklight(RED);
  delay(2000);
  state = state - 1;  //then go back to check again
}

void caliChoice(){
  lcd.clear(); //prompts the user to choose manual or default calibration
  lcd.setCursor(0, 0); 
  lcd.print("L: manual"); 
  lcd.setCursor(0, 1); 
  lcd.print("R: default"); 
  buttons = lcd.readButtons(); 
  while(!buttons){ //if user doesn't choose a button, waits here until a button is pressed
    buttons = lcd.readButtons(); 
  }
  if (buttons & BUTTON_LEFT){ //if left button is pressed...
    lcd.clear(); 
    lcd.setCursor(0,0); 
    lcd.print("Manual selected"); 
    delay(1000); 
    state = state + 1; //...go to the manual calibration state
  }
  if (buttons & BUTTON_RIGHT){ //if right button is pressed...
    lcd.clear(); 
    lcd.setCursor(0,0); 
    lcd.print("Default selected"); 
    delay(1000); 
    state = state + 2; //...chose default calibration, goes to the normal breathing state
  }
}

void pushCalc(){ //mainly prompts the user to push syringe and displays result
    lcd.clear(); 
    lcd.setCursor(0,0); 
    lcd.print("Push syringe");
    lcd.setCursor(0,1); 
    lcd.print("in 10s");  
    kMinus = manualCali(); //the actual calculation is in another function
    lcd.clear(); 
    lcd.setCursor(0,0); 
    lcd.print("K-: ");
    lcd.print(kMinus); 
    delay(3000); 
}

void pullCalc(){ //mainly prompts the user to pull syringe and displays result
    lcd.clear(); 
    lcd.setCursor(0,0); 
    lcd.print("Pull syringe"); 
    lcd.setCursor(0,1); 
    lcd.print("in 10s"); 
    kPlus = manualCali(); //the actual calculation is in another function
    lcd.clear(); 
    lcd.setCursor(0,0); 
    lcd.print("K+: ");
    lcd.print(kPlus); 
    delay(3000); 
}

float manualCali(){ //calculation for K+ K- calibration
  float voltage = 0; 
  for (int i=0; i<800; i++){ //Reiman-sum the voltage
    float oneVoltage = (float)analogRead(1)*5/1023; 
    voltage = voltage + (oneVoltage-2.5) * 0.02; 
    delay(20); 
  }
  return 3/voltage; //and converts it to a K value with the known volume
}

void normalBreath(){
  lcd.clear(); 
  lcd.setCursor(0,0); 
  lcd.print("Take 3 normal"); 
  lcd.setCursor(0,1); 
  lcd.print("breath in 20s"); 
  float volume = 0; 
  float oneMax = 0; 
  float allMax = 0; 
  float oneMin = 0; 
  float allMin = 0; 
  for (int i=0; i<800; i++){
    float oneVoltage = (float)analogRead(1)*5/1023 - 2.5;
    delay(20); //reads two voltages to determine a local maximum/minimum
    float anotherVoltage = (float)analogRead(1)*5/1023 - 2.5;
    if (oneVoltage > 0.1){ //inhaling, passing the threshold
      volume = volume + kPlus * oneVoltage*0.02; 
      Serial.println(volume); //easy to debug
      if (volume > oneMax){
        oneMax = volume; 
      }
      if (anotherVoltage < 0.1){ //if inhaling stops, then it's a local maximum
         allMax = allMax + oneMax; 
         oneMax = 0; 
      }
    }
    if (oneVoltage < -0.1){ //exhaling, passing the threshold
      volume = volume - kMinus * oneVoltage*0.02; 
      Serial.println(volume); //easy to debug
      if (volume < oneMin){
        oneMin = volume; 
      }
      if (anotherVoltage > -0.1){ //if exhaling stops, then it's a local minimum
        allMin = allMin + oneMin; 
        oneMin = 0; 
      }
    }
  }
  normalEx = allMin/3; //with the three breaths, three max and min are recorded and added
  normalIn = allMax/3; //together; divide by 3 gives the average max and average min
  lcd.clear(); 
  lcd.setCursor(0, 0); 
  lcd.print("Exhale: "); 
  lcd.print(abs(normalEx)); 
  lcd.setCursor(0, 1); 
  lcd.print("Inhale: "); 
  lcd.print(abs(normalIn)); 
  delay(5000); 
  state++; 
}

void forceBreath(){
  lcd.clear(); 
  lcd.setCursor(0,0); 
  lcd.print("Take 3 forced"); 
  lcd.setCursor(0,1); 
  lcd.print("breath in 20s");
  float volume_2 = 0; 
  float oneMax_2 = 0; 
  float allMax_2 = 0; 
  float oneMin_2 = 0; 
  float allMin_2 = 0;  
  for (int i=0; i<500; i++){
    float oneVoltage = (float)analogRead(1)*5/1023 - 2.5;
    delay(20); //reads two voltages to determine a local maximum/minimum
    float anotherVoltage = (float)analogRead(1)*5/1023 - 2.5;
    if (oneVoltage > 0.1){ //inhaling
      volume_2 = volume_2 + kPlus * oneVoltage*0.02; 
      Serial.println(volume_2); //inhaling, passing the threshold
      if (volume_2 > oneMax_2){
        oneMax_2 = volume_2; 
      }
      if (anotherVoltage < 0.1){ //if inhaling stops, then it's a local maximum
         allMax_2 = allMax_2 + oneMax_2; 
         oneMax_2 = 0; 
      }
    }
    if (oneVoltage < -0.1){ //exhaling, passing the threshold
      volume_2 = volume_2 - kMinus * oneVoltage*0.02; 
      Serial.println(volume_2); 
      if (volume_2 < oneMin_2){
        oneMin_2 = volume_2; 
      }
      if (anotherVoltage > -0.1){
        allMin_2 = allMin_2 + oneMin_2; 
        oneMin_2 = 0; 
      }
    }
  }
  forceEx = allMin_2/3; //with the three breaths, three max and min are recorded and added
  forceIn = allMax_2/3; //together; divide by 3 gives the average max and average min
  lcd.clear(); 
  lcd.setCursor(0, 0); 
  lcd.print("Wet Exhale: "); 
  lcd.print(abs(forceEx)); 
  lcd.setCursor(0, 1); 
  lcd.print("Wet Inhale: "); 
  lcd.print(abs(forceIn)); 
  delay(5000); 
  delay(5000); 
  state++;
}

void showResults(){
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Standardized vals");
  forceEx_dry = forceEx*0.83; //conversions to dry volumes
  normalEx_dry = normalEx*0.83;
  forceIn_dry = forceIn*0.87;
  normalIn_dry = normalIn*0.87;
  VC = forceIn_dry - forceEx_dry;  //vital capacity
  Vt = normalIn_dry - normalEx_dry;  //tidal volume
  IC = forceIn_dry - normalEx_dry;   //inspiratory capacity
  EC = normalIn_dry - forceEx_dry;   //expiratory capacity
  IRV = forceIn_dry - normalIn_dry;  //inspiratory reserve volume
  ERV = normalEx_dry - forceEx_dry;  //expiratory reserve volume
  
//  Serial.println("VC: "); //easy to debug and read the values from serial
//  Serial.println(VC); 
//  Serial.println("Vt: "); 
//  Serial.println(Vt); 
//  Serial.println("IC: "); 
//  Serial.println(IC); 
//  Serial.println("EC: "); 
//  Serial.println(EC);
//  Serial.println("IRV: "); 
//  Serial.println(IRV); 
//  Serial.println("ERV: "); 
//  Serial.println(ERV);
  
  delay(1000);
  lcd.clear();
  lcd.setBacklight(BLUE); //display the results in different colors
  lcd.setCursor(0, 0);
  lcd.print("VC: ");
  lcd.print(VC);
  lcd.setCursor(0, 1);
  lcd.print("V_t: ");
  lcd.print(Vt);
  delay(2000);
  lcd.clear();
  lcd.setBacklight(YELLOW);
  lcd.setCursor(0, 0);
  lcd.print("IC: ");
  lcd.print(IC);
  lcd.setCursor(0, 1);
  lcd.print("EC: ");
  lcd.print(EC);
  delay(2000);
  lcd.clear();
  lcd.setBacklight(VIOLET);
  lcd.setCursor(0, 0);
  lcd.print("IRV: ");
  lcd.print(IRV);
  lcd.setCursor(0, 1);
  lcd.print("ERV: ");
  lcd.print(ERV);
  delay(2000);
  lcd.clear();
  lcd.setBacklight(TEAL);
  lcd.setCursor(0,0);
  lcd.print("Doctor code:"); //displays the doctor's number
  lcd.setCursor(0,1);
  lcd.print("214-394-2898");
  delay(3000);

  lcd.clear(); 
  lcd.setBacklight(WHITE); //prompts the user to push buttons
  lcd.setCursor(0, 0); 
  lcd.print("L: Repeat values"); 
  lcd.setCursor(0, 1); 
  lcd.print("R: RESTART"); 
  
  buttons = lcd.readButtons();
  while(!buttons){ //if user doesn't push, keep reading
    buttons = lcd.readButtons();
  }
  if (buttons & BUTTON_RIGHT){ //if user pushes the right button...
    lcd.clear();
    lcd.setBacklight(RED); 
    lcd.setCursor(0,0); 
    lcd.print("Restart selected"); 
    delay(1000); 
    state = state - 6;  //...then it goes back to the first stage (all over again)
    
  }
  if (buttons & BUTTON_LEFT){ //if user pushes the left button...
    lcd.clear(); 
    lcd.setBacklight(GREEN);
    lcd.setCursor(0,0); 
    lcd.print("Repeat selected"); 
    delay(1000);
    state = 7; 
  }
}
