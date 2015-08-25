#include <Wire.h>
#include <LiquidCrystal_I2C.h>

LiquidCrystal_I2C lcd(0x3F, 2, 1, 0, 4, 5, 6, 7, 3, POSITIVE);

byte solar[8] = //icon for solar panel
{
  0b11111,0b10101,0b11111,0b10101,0b11111,0b10101,0b11111,0b00000
};
byte battery[8] =  //icon for battery
{
  0b01110,0b11011,0b10001,0b10001,0b10001,0b10001,0b10001,0b11111
};

byte energy[8] =  // icon for power
{
  0b00010,0b00100,0b01000,0b11111,0b00010,0b00100,0b01000,0b00000
};
byte alarm[8] =  // icon for alarm
{
 0b00000,0b00100,0b01110,0b01110,0b01110,0b11111,0b00000,0b00100
};
byte temp[8] = //icon for termometer
{
 0b00100,0b01010,0b01010,0b01110,0b01110,0b11111,0b11111,0b01110
};

byte charge[8] = // icon for battery charge
{
  0b01010,0b11111,0b10001,0b10001,0b10001,0b01110,0b00100,0b00100,
};
byte not_charge[8]=
{
  0b00000,0b10001,0b01010,0b00100,0b01010,0b10001,0b00000,0b00000,
};

const int numReadings = 10;

int rotorCurrentReadings[numReadings];      // the readings from the analog input
int rotorCurrentIndex = 0;                  // the index of the current reading
int rotorCurrentTotal = 0;                  // the running total
int rotorCurrentAverage = 0;                // the average

int rotorCurrentInputPin = A0;
float rawVoltage = 0.0;
int acoffset = 2500;
int mvPerAmp = 185;
float rotorActualCurrent = 0.0;


float rotorVoltageReadings[numReadings];      // the readings from the analog input
int rotorVoltageIndex = 0;                  // the index of the current reading
float rotorVoltageTotal = 0;                  // the running total
float rotorVoltageAverage = 0;                // the average

float rotorVoltageInputPin = A1;
float pinVoltage = 0.0;
float rotorActual = 0.0;

int battVoltageReadings[numReadings];      // the readings from the analog input
int battVoltageIndex = 0;                  // the index of the current reading
int battVoltageTotal = 0;                  // the running total
int battVoltageAverage = 0;                // the average

int battVoltageInputPin = A2;
float battPinVoltage = 0.0;
float battActual = 0.0;

int battCurrentReadings[numReadings];      // the readings from the analog input
int battCurrentIndex = 0;                  // the index of the current reading
int battCurrentTotal = 0;                  // the running total
int battCurrentAverage = 0;                // the average

int battCurrentInputPin = A3;

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
  lcd.begin(20,4);   // initialize the lcd for 16 chars 2 lines, turn on backlight
  lcd.backlight(); // finish with backlight on  
  lcd.createChar(1,solar);
  lcd.createChar(2, battery);
  lcd.createChar(3, energy);
  lcd.createChar(4,alarm);
  lcd.createChar(5,temp);
  lcd.createChar(6,charge);
  lcd.createChar(7,not_charge);
  lcd.clear();  
}

void loop() {
  rotorVoltageRead();
  rotorCurrentRead();
  battVoltageRead();
  battCurrentRead();
  printLCD();
}

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
  Serial.println(rotorActual);
  delay(1);        // delay in between reads for stability  
}

void rotorCurrentRead()
{
  rotorCurrentTotal= rotorCurrentTotal - rotorCurrentReadings[rotorCurrentIndex];        
  // read from the sensor:  
  rotorCurrentReadings[rotorCurrentIndex] = analogRead(rotorCurrentInputPin);
  // add the reading to the total:
  rotorCurrentTotal= rotorCurrentTotal + rotorCurrentReadings[rotorCurrentIndex];      
  // advance to the next position in the array:  
  rotorCurrentIndex = rotorCurrentIndex + 1;                    

  // if we're at the end of the array...
  if (rotorCurrentIndex >= numReadings)              
    // ...wrap around to the beginning:
    rotorCurrentIndex = 0;                          

  // calculate the average:
  rotorCurrentAverage = rotorCurrentTotal / numReadings;
  rawVoltage = (rotorCurrentAverage / 1024.0) * 5000;
  rotorActualCurrent = ((rawVoltage - acoffset) / mvPerAmp);

  Serial.println(rotorActualCurrent);
  delay(1);

}

void battCurrentRead()
{
  // subtract the last reading:
  battCurrentTotal= battCurrentTotal - battCurrentReadings[battCurrentIndex];        
  // read from the sensor:  
  battCurrentReadings[battCurrentIndex] = analogRead(battCurrentInputPin);
  //Data processing:510-raw data from analogRead when the input is 0; 5-5v; the first 0.04-0.04V/A(sensitivity); the second 0.04-offset val;
  battCurrentReadings[battCurrentIndex] = (battCurrentReadings[battCurrentIndex]-510)*5/1024/0.04-0.04;
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
  // send it to the computer as ASCII digits
  Serial.println(battCurrentAverage);  
  delay(1);        // delay in between reads for stability  
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
  delay(1);        // delay in between reads for stability  
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
  lcd.setCursor(10, 0);
  lcd.print(rotorActualCurrent);
  lcd.setCursor(15, 0);
  lcd.print("A");
  
  //2nd line
  lcd.setCursor(0, 1);
  lcd.write(2);
  lcd.setCursor(2, 1);
  lcd.print(battActual);
  lcd.setCursor(7,1);
  lcd.print("V/");
  lcd.setCursor(10, 1);
  lcd.print(battCurrentAverage);
  lcd.setCursor(15, 1);
  lcd.print("A");
  
  //3rd line
  lcd.setCursor(0, 2);
  lcd.write(3);
  lcd.setCursor(2, 2);
  lcd.print("100Ah/98%");
  lcd.setCursor(0, 3);
  lcd.write(6);
  lcd.setCursor(2, 3);
  lcd.print("CHARGING...");
//  lcd.setCursor(12, 0);
//  lcd.write(7);
}
