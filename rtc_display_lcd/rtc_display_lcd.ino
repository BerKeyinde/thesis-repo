// Sketch by brainstorm ABC for I2C Based Clock Using RTC and 16x2 I2c Display
#include <Wire.h>
#include "RTClib.h"
#include <LCD.h>
#include <LiquidCrystal_I2C.h>

RTC_DS1307 RTC;

#define I2C_ADDR    0x3F  // Define I2C Address for the PCF8574T 
//---(Following are the PCF8574 pin assignments to LCD connections )----
// This are different than earlier/different I2C LCD displays

#define Rs_pin  0
#define Rw_pin  1
#define En_pin  2
#define BACKLIGHT_PIN  3
#define D4_pin  4
#define D5_pin  5
#define D6_pin  6
#define D7_pin  7

#define  LED_OFF  1
#define  LED_ON  0

/*-----( Declare objects )-----*/  
LiquidCrystal_I2C  lcd(I2C_ADDR,En_pin,Rw_pin,Rs_pin,D4_pin,D5_pin,D6_pin,D7_pin);



void setup() {
  Wire.begin();
  RTC.begin();

 lcd.begin (20,4);  // initialize the lcd 
// Switch on the backlight
 lcd.setBacklightPin(BACKLIGHT_PIN,POSITIVE);
 lcd.setBacklight(LED_ON);
}

void loop() {
  DateTime now = RTC.now();
// Print our characters on the LCD
 lcd.backlight();  //Backlight ON if under program control
    
    lcd.setCursor(0,0); //Start at character 0 on line 0
    lcd.print("TIME :");
//  lcd.setCursor(6,0); 
    print2digits(now.hour());
    lcd.print(':');
    print2digits(now.minute());
    lcd.print(':');
    print2digits(now.second());
    
    lcd.setCursor(0,1); //Start at character 0 on line 1
    lcd.print("DATE :"); 
    lcd.print(now.month());
    lcd.print('/');
    lcd.print(now.day());
    lcd.print('/');
    lcd.print(now.year());

    if (now.second() >= 30)
    {
      lcd.setCursor(0,3);
      lcd.print("IT'S WORKING!");
    }
    else
    {
      lcd.setCursor(0,3);
      lcd.print("Not yet time");  
    }
      
    
  delay(990);
}

void print2digits(int number) {
  if (number >= 0 && number < 10) {
    lcd.print('0');
  }
  lcd.print(number);
}
