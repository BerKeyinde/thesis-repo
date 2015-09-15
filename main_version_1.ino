#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <RTClib.h>

RTC_DS1307 RTC;
LiquidCrystal_I2C lcd(0x3F, 2, 1, 0, 4, 5, 6, 7, 3, POSITIVE);

int PWM_PIN = 5;
int LOAD_PIN = 8;

byte rotor[8] = //icon for rotor
{
  0b11111,0b10101,0b11111,0b10101,0b11111,0b10101,0b11111,0b00000
};
byte battery[8] =  //icon for battery
{
  0b01110,0b11011,0b10001,0b10001,0b10001,0b10001,0b10001,0b11111
};

byte load[8] =  // icon for power
{
  0b00010,0b00100,0b01000,0b11111,0b00010,0b00100,0b01000,0b00000
};

byte charge[8] = // icon for battery charge
{
  0b01010,0b11111,0b10001,0b10001,0b10001,0b01110,0b00100,0b00100,
};
byte not_charge[8]=
{
  0b00000,0b10001,0b01010,0b00100,0b01010,0b10001,0b00000,0b00000,
};

const int numReadings = 50;
//Rotor Current Reading Variables
int rotorCurrentReadings[numReadings];
int rotorCurrentIndex = 0;
int rotorCurrentTotal = 0.0;
float rotorCurrentAverage = 0.0;
int rotorCurrentInputPin = A0;


//Rotor Voltage Reading Variables
int rotorVoltageReadings[numReadings];
int rotorVoltageIndex = 0;
int rotorVoltageTotal = 0;
int rotorVoltageAverage = 0;
int rotorVoltageInputPin = A1;
float pinVoltage = 0.0;
float rotorActual = 0.0;


//Battery Voltage Reading Variables
int battVoltageReadings[numReadings];
int battVoltageIndex = 0;
int battVoltageTotal = 0;
int battVoltageAverage = 0;
int battVoltageInputPin = A2;
float battPinVoltage = 0.0;
float battActual = 0.0;


//Battery Current Reading Variables
int battCurrentReadings[numReadings];
int battCurrentIndex = 0;
int battCurrentTotal = 0;
float battCurrentAverage = 0;
int battCurrentInputPin = A3;

//State of Charge
const float battFull = 12.7;
const float battEmpty = 10.5;
int battCharge = 0;
const float battCap = 100.0;
float chargingTime = 0.0;
int duty = 0;
float battEnergy = 0.0;

//Load Variables
float loadPower = 0.0;
float loadEnergy = 0.0;

void setup()
{
  // initialize serial communication with computer:
  Serial.begin(9600);                  
  // initialize all the readings to 0:
  for (int thisReading = 0; thisReading < numReadings; thisReading++) {
    rotorCurrentReadings[thisReading] = 0;
    rotorVoltageReadings[thisReading] = 0;
    battVoltageReadings[thisReading] = 0;
    battCurrentReadings[thisReading] = 0; 
  }

  //initialize LCD
  lcd.begin(20,4);   // initialize the lcd for 16 chars 2 lines, turn on backlight
  lcd.backlight(); // finish with backlight on  
  lcd.createChar(1,rotor);
  lcd.createChar(2,battery);
  lcd.createChar(3,charge);
  lcd.createChar(4,load);
  lcd.createChar(5,not_charge);
  lcd.clear();

  //initialize Wire
  Wire.begin();

  //initialize RTC
  RTC.begin();

  //pinModes
  pinMode(LOAD_PIN, OUTPUT); 
}

void loop() {
  //Get Current DateTime
  DateTime now = RTC.now();
  int timeHour = (now.hour(), DEC);
  int timeMin = (now.minute(), DEC);

  rotorVoltageRead();
  rotorCurrentRead();
  battVoltageRead();
  battCurrentRead();
  computeData();
  printLCD();
  loadControl(timeMin, timeHour);
  Serial.print("RV: ");
  Serial.print(rotorActual);
  Serial.print("\tRC: ");
  Serial.print(rotorCurrentAverage);
  Serial.print("\tBV: ");
  Serial.print(battActual);
  Serial.print("\tBC: ");
  Serial.print(battCurrentAverage);
  delay(200);
}

/*Smoothing algorithm are applied on all sensor reading functions
Data Sampling Frequency = 5 Hz
Time Interval (delay) = 200 ms
*/

void rotorVoltageRead()
{
  // subtract the last reading:
  rotorVoltageTotal= rotorVoltageTotal - rotorVoltageReadings[rotorVoltageIndex];        
  // read from the sensor:  
  rotorVoltageReadings[rotorVoltageIndex] = analogRead(rotorVoltageInputPin);
  // add the reading to the total:
  rotorVoltageTotal= rotorVoltageTotal + rotorVoltageReadings[rotorVoltageIndex];      
  // advance to the next position in the array:  
  rotorVoltageIndex = rotorVoltageIndex + 1;                    

  // if we're at the end of the array...
  if (rotorVoltageIndex >= numReadings)              
    // ...wrap around to the beginning:
    rotorVoltageIndex = 0;                          

  // calculate the average:
  rotorVoltageAverage = rotorVoltageTotal / numReadings;  
  
  pinVoltage = rotorVoltageAverage * (5.0 / 1024.0);
  rotorActual = (pinVoltage * 23.3) / 3.3;
    
  // send it to the computer as ASCII digits
 // Serial.println(rotorActual);
//  delay(1);        // delay in between reads for stability  
}

void rotorCurrentRead()
{
  rotorCurrentTotal= rotorCurrentTotal - rotorCurrentReadings[rotorCurrentIndex];        
  // read from the sensor:  
  rotorCurrentReadings[rotorCurrentIndex] = analogRead(rotorCurrentInputPin);
  //rotorCurrentReadings[rotorCurrentIndex] = (rotorCurrentReadings[rotorCurrentIndex] * 5/1024.0 - 2.5)/.185; // add the reading to the total:
  rotorCurrentTotal= rotorCurrentTotal + rotorCurrentReadings[rotorCurrentIndex];      
  // advance to the next position in the array:  
  rotorCurrentIndex = rotorCurrentIndex + 1;                    

  // if we're at the end of the array...
  if (rotorCurrentIndex >= numReadings)              
    // ...wrap around to the beginning:
    rotorCurrentIndex = 0;                          
  //Serial.println(rotorCurrentAverage);
  // calculate the average:
  rotorCurrentAverage = rotorCurrentTotal / numReadings;
  rotorCurrentAverage = rotorCurrentAverage - 509;
  rotorCurrentAverage = rotorCurrentAverage * 5 / 1024;
  rotorCurrentAverage = rotorCurrentAverage / 0.1;
//  delay(200);

}

void battCurrentRead()
{
  // subtract the last reading:
  battCurrentTotal= battCurrentTotal - battCurrentReadings[battCurrentIndex];        
  // read from the sensor:  
  battCurrentReadings[battCurrentIndex] = analogRead(battCurrentInputPin);

  // add the reading to the total:
  battCurrentTotal= battCurrentTotal + battCurrentReadings[battCurrentIndex];      
  // advance to the next position in the array:  
  battCurrentIndex = battCurrentIndex + 1;                    

  // if we're at the end of the array...
  if (battCurrentIndex >= numReadings)              
    // ...wrap around to the beginning:
    battCurrentIndex = 0;                          

  // calculate the average:
  battCurrentAverage = battCurrentTotal / numReadings;
  battCurrentAverage = battCurrentAverage - 511;
  battCurrentAverage = battCurrentAverage * 5 / 1024;
  battCurrentAverage = battCurrentAverage / 0.04;        
  // send it to the computer as ASCII digits
  //Serial.println(battCurrentAverage);  
 // delay(200);        // delay in between reads for stability  
}

void battVoltageRead()
{
  // subtract the last reading:
  battVoltageTotal= battVoltageTotal - battVoltageReadings[battVoltageIndex];        
  // read from the sensor:  
  battVoltageReadings[battVoltageIndex] = analogRead(battVoltageInputPin);
  // add the reading to the total:
  battVoltageTotal= battVoltageTotal + battVoltageReadings[battVoltageIndex];      
  // advance to the next position in the array:  
  battVoltageIndex = battVoltageIndex + 1;                    

  // if we're at the end of the array...
  if (battVoltageIndex >= numReadings)              
    // ...wrap around to the beginning:
    battVoltageIndex = 0;                          

  // calculate the average:
  battVoltageAverage = battVoltageTotal / numReadings;  
  
  battPinVoltage = battVoltageAverage * (5.0 / 1024.0);
  battActual = (battPinVoltage * 17) / 5;
    
  // send it to the computer as ASCII digits
  Serial.println(battActual);
 // delay(200);        // delay in between reads for stability  
}

void computeData()
{
  battCharge = ((battActual - battEmpty) / (battFull - battEmpty)) * 100;
  loadPower = battActual * battCurrentAverage;
  float currCap = battCap * (100 - battCharge); //current capacity of battery
  chargingTime = currCap / rotorCurrentAverage;
  battEnergy = battActual * (battCap * battCharge);
  
  if (battCharge >= 80)
  {
    duty= 76;
    analogWrite(PWM_PIN,duty);
  }
  
  else
  {
   duty=0;
   analogWrite(PWM_PIN,duty);
  }
}

void loadControl(int timeMin, int timeHour)
{
  if (timeMin == 30 && timeHour == 17)
    digitalWrite(LOAD_PIN, HIGH);
  else if (timeHour == 6 && timeMin < 1)
    digitalWrite(LOAD_PIN, LOW);
}

void printLCD()
{
  //1st line
  lcd.setCursor(0, 0);
  lcd.write(1);
  lcd.setCursor(2, 0);
  lcd.print(rotorActual);
  lcd.setCursor(7,0);
  lcd.print("V/");
  
  //2nd line
  lcd.setCursor(0, 1);
  lcd.write(2);
  lcd.setCursor(2, 1);
  lcd.print(battEnergy);
  lcd.setCursor(7,1);
  lcd.print("Wh/");
  lcd.setCursor(10, 1);
  lcd.print(battCharge);
  lcd.setCursor(13, 1);
  lcd.print("%");
  
  //3rd line
  lcd.setCursor(0, 2);
  lcd.write(3);
  lcd.setCursor(2, 2);
  lcd.print(loadPower);
  lcd.setCursor(6, 2);
  lcd.print("W");

  //4th Line
  lcd.setCursor(0, 3);
  lcd.write(4);
  lcd.setCursor(2, 3);
  lcd.print(rotorCurrentAverage);
  lcd.setCursor(7, 3);
  lcd.print("A/");
  lcd.setCursor(9, 3);
  lcd.print(chargingTime);
  lcd.setCursor(17, 3);
  lcd.print("HRS");
}
