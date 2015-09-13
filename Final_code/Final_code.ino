#include <SoftwareSerial.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <RTClib.h>

SoftwareSerial SMS(10,11);
RTC_DS1307 RTC;

#define AVG_NUM 10    // number of iterations of the adc routine to average the adc readings
#define BAT_MIN 10.5  // minimum battery voltage for 12V system
#define BAT_MAX 13.8  // maximum battery voltage for 12V system
#define BULK_CH_SP 14.4 // bulk charge set point for sealed lead acid battery // flooded type set it to 14.6V
#define FLOAT_CH_SP 13.6  //float charge set point for lead acid battery
#define LVD 11.5          //Low voltage disconnect setting for a 12V system
#define PWM_PIN 5         // pin-3 is used to control the charging MOSFET //the default frequency is 490.20Hz
#define LOAD_PIN 2       // pin-2 is used to control the load

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
int duty =0;

int rotorCurrentInputPin = A0;
float rawVoltage = 0.0;
float rawValue = 0.0;
int acsoffset = 2500;
int mvPerAmp = 66;
float rotorActualCurrent = 0.0;
float rotorCurrentReadings[numReadings];      // the readings from the analog input
int rotorCurrentIndex = 0;                  // the index of the current reading
float rotorCurrentTotal = 0.0;                  // the running total
double rotorCurrentAverage = 0.0;                // the average
String str_rotcurr;

float rotorVoltageInputPin = A1;
float pinVoltage = 0.0;
float rotorActualVoltage = 0.0;
float rotorVoltageReadings[numReadings];      // the readings from the analog input
int rotorVoltageIndex = 0;                  // the index of the current reading
float rotorVoltageTotal = 0;                  // the running total
float rotorVoltageAverage = 0;                // the average
String str_rotvolt;

int battVoltageInputPin = A2;
float battPinVoltage = 0.0;
float battActualVoltage = 0.0;
int battVoltageReadings[numReadings];      // the readings from the analog input
int battVoltageIndex = 0;                  // the index of the current reading
int battVoltageTotal = 0;                  // the running total
int battVoltageAverage = 0;                // the average
String str_battvolt;

//float battCurrentReadings[numReadings];      // the readings from the analog input
int battCurrentInputPin = A3;
float battCurrentReadings=0.0;
int battCurrentIndex = 0;                  // the index of the current reading
float battCurrentTotal = 0;                  // the running total
float battCurrentAverage = 0;               // the average
float battCurrentDisplay=0.0;
String str_battcurr;

float watts =0.0;
float msec=0.0;
float sample = 0.0;
float total = 0.0;
float totalCharge = 0.0;
float averageAmps=0.0;
float ampSeconds=0.0;
float ampHours=0.0;
float time=0.0;

String smscontent;
String text;
String stringVal;
char charVal[10];
bool sendtext=true;

void setup()
{
//  Serial.begin(9600);
  SMS.begin(9600);
  Wire.begin();
  RTC.begin();
  
  lcd.begin(20,4);
  lcd.backlight();
  lcd.createChar(1,solar);
  lcd.createChar(2, battery);
  lcd.createChar(3, energy);
  lcd.createChar(4,alarm);
  lcd.createChar(5,temp);
  lcd.createChar(6,charge);
  lcd.createChar(7,not_charge);
  lcd.clear(); 
  
  pinMode(8,OUTPUT);
  
  // initialize all the readings to 0:
  for (int thisReading = 0; thisReading < numReadings; thisReading++) {
    rotorCurrentReadings[thisReading] = 0;
    rotorVoltageReadings[thisReading] = 0;
    battVoltageReadings[thisReading] = 0;
//    battCurrentReadings[thisReading] = 0; 
  } 
}

void loop() {
  DateTime now = RTC.now();
  
  rotorVoltageRead();
  rotorCurrentRead();
  battVoltageRead();
  battCurrentRead();

  printLCD();
  
  if((now.minute()%2) ==0 && now.second() <20 && sendtext == true){
    text = SMScontent(now.hour(),now.minute(),now.second(),now.month(),now.day(),now.year(),str_rotvolt,str_rotcurr,str_battvolt,str_battcurr);
  
    sendSMS(text);
    
    sendtext = false;
  }
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
  rotorActualVoltage = (pinVoltage * 23.3) / 3.3;
    
  // send it to the computer as ASCII digits
  //Serial.println(rotorActualVoltage);
  str_rotvolt = floattostring(rotorActualVoltage);
  delay(1);        // delay in between reads for stability  
}

void rotorCurrentRead()
{
  rotorCurrentTotal= rotorCurrentTotal - rotorCurrentReadings[rotorCurrentIndex];        
  // read from the sensor:  
  rotorCurrentReadings[rotorCurrentIndex] = analogRead(rotorCurrentInputPin);
  rotorCurrentReadings[rotorCurrentIndex] = (rotorCurrentReadings[rotorCurrentIndex]-510)*5/1024/0.066;
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
  
  //Serial.println(rotorCurrentAverage);
  str_rotcurr = floattostring(rotorCurrentAverage);
  delay(1);

}

void battCurrentRead()
{
//  // subtract the last reading:
//  battCurrentTotal= battCurrentTotal - battCurrentReadings[battCurrentIndex];        
//  // read from the sensor:  
//  battCurrentReadings[battCurrentIndex] = analogRead(battCurrentInputPin);
//  //Data processing:510-raw data from analogRead when the input is 0; 5-5v; the first 0.04-0.04V/A(sensitivity); the second 0.04-offset val;
//  battCurrentReadings[battCurrentIndex] = (battCurrentReadings[battCurrentIndex]-510)*5/1024/0.04-0.04;
//  // add the reading to the total:
//  battCurrentTotal= battCurrentTotal + battCurrentReadings[battCurrentIndex];      
//  // advance to the next position in the array:  
//  battCurrentIndex = battCurrentIndex + 1;                    
//
//  // if we're at the end of the array...
//  if (battCurrentIndex >= numReadings)              
//    // ...wrap around to the beginning:
//    battCurrentIndex = 0;                          
//
//  // calculate the average:
//  battCurrentAverage = battCurrentTotal / numReadings;        
//  // send it to the computer as ASCII digits

  battCurrentReadings = analogRead(battCurrentInputPin);
  battCurrentAverage = ((398-battCurrentReadings)*75.76)/1023;
  
  //Serial.println(battCurrentAverage);  
  str_battcurr = floattostring(battCurrentAverage);
  delay(250);        // delay in between reads for stability  
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
  battActualVoltage = (battPinVoltage * 17) / 5;

  if (battActualVoltage >= 11.40)
  {
    duty= 76;
    analogWrite(PWM_PIN,duty);
  }
  
  else
  {
   duty=0;
   analogWrite(PWM_PIN,duty);
  }
    
  // send it to the computer as ASCII digits
  //Serial.println(battActualVoltage);
  str_battvolt = floattostring(battActualVoltage);
  delay(250);        // delay in between reads for stability  
}

void chargeRead()
{
  watts = battCurrentAverage * battActualVoltage;
  sample = sample +1;
  msec=millis();
  time = msec/1000.0;
  totalCharge = totalCharge + battCurrentAverage;
  averageAmps=totalCharge/sample;
  ampSeconds = averageAmps*time;
  ampHours = ampSeconds/3600;
}

void printLCD()
{
  //1st line
  lcd.setCursor(0, 0);
  lcd.write(1);
  lcd.setCursor(2, 0);
  lcd.print(rotorActualVoltage);
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
  lcd.print(battActualVoltage);
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
  lcd.print(ampHours);
  lcd.setCursor(0, 3);
  lcd.write(6);
  lcd.setCursor(2, 3);
  lcd.print("CHARGING...");
//  lcd.setCursor(12, 0);
//  lcd.write(7);
}

void GSMpower()
{
  digitalWrite(8,LOW);
  delay(1000);
  digitalWrite(8,HIGH);
  delay(5000);
}
 
void sendSMS(String message)
{
  GSMpower();  
  delay(1000); 
  
  SMS.print("AT+CMGF=1\r");                                                        // AT command to send SMS message
  delay(1000);
  SMS.println("AT+CMGS=\"+639056814564\"\r");
  delay(1000);
  SMS.println(message);        // message to send
  delay(1000);
  SMS.println((char)26);                       // End AT command with a ^Z, ASCII code 26
  delay(1000); 
  SMS.println();
//  Serial.println("Sending SMS");
  delay(2000);  // give module time to send SMS
//  Serial.println("SMS Sent");
  
  GSMpower();                                   // turn off module
}

String SMScontent(int in_hour,int in_minute, int in_second,int in_month, int in_day,int in_year,String rotvolt,String rotcurr, String battvolt, String battcurr)
{
  smscontent = "As of ";
  smscontent.concat(in_hour);
  smscontent = smscontent + ":";
  smscontent.concat(in_minute);
  smscontent = smscontent + ":";
  smscontent.concat(in_second);
  smscontent = smscontent + " ";
  smscontent.concat(in_month);
  smscontent = smscontent + "/";
  smscontent.concat(in_day);
  smscontent = smscontent + "/";
  smscontent.concat(in_year);
  smscontent = smscontent + ": ";
  smscontent = smscontent + "RV: ";
  smscontent.concat(rotvolt);
  smscontent = smscontent + "V";
//  smscontent = smscontent + "Rotor Current: ";
//  smscontent = smscontent + rotcurr;
//  smscontent = smscontent + " A ";
//  smscontent = smscontent + "Battery Voltage: ";
//  smscontent = smscontent + battvolt;
//  smscontent = smscontent + " V ";
//  smscontent = smscontent + "Battery Current: ";
//  smscontent = smscontent + battcurr;
//  smscontent = smscontent + " A ";
  
  
  return smscontent;
}

String floattostring(float floatVal){
  dtostrf(floatVal,4,3,charVal);
  for(int i=0;i<sizeof(charVal);i++)
  {
    stringVal+=charVal[i];
  }
  
  return stringVal;
}
