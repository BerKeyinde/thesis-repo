#include <SoftwareSerial.h>
SoftwareSerial SIM900(10, 11);
 
void setup()
{
  pinMode(9,OUTPUT);
  Serial.begin(9600);
  SIM900.begin(9600);
  
  SIM900power();  
  delay(20000);  // give time to log on to network. 
}
 
void SIM900power()
// software equivalent of pressing the GSM shield "power" button
{
  digitalWrite(9, LOW);
  delay(1000);
  digitalWrite(9, HIGH);
  delay(5000);
}
 
void sendSMS()
{
  SIM900.print("AT+CMGF=1\r");                                                        // AT command to send SMS message
  delay(1000);
//  SIM900.print("AT+CMGS=");  // recipient's mobile number, in international format
//  SIM900.print('"');
//  SIM900.print("+639056814564");
//  SIM900.println('"');
//  SIM900.println("\r");
  SIM900.println("AT+CMGS=\"+639056814564\"\r");
  delay(1000);
  SIM900.println("Hello, world. This is a text message from an Arduino Uno.");        // message to send
  delay(1000);
  SIM900.println((char)26);                       // End AT command with a ^Z, ASCII code 26
  delay(1000); 
  SIM900.println();
  Serial.println("Sending SMS");
  delay(10000);  // give module time to send SMS
  Serial.println("SMS Sent");
  SIM900power();                                   // turn off module
}
 
void loop()
{
    if(SIM900.available())
  {
    Serial.println("StatusL Ready");
  }
  else
  {
    Serial.println("StatusL Not Ready");
  }
  sendSMS();
  do {} while (1);
}
