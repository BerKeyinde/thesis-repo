#include <SoftwareSerial.h>
SoftwareSerial SIM900(11, 12);
 
char incoming_char=0;
 
void setup()
{
  pinMode(9,OUTPUT);
  Serial.begin(9600); // for serial monitor
  SIM900.begin(9600); // for GSM shield
  SIM900power();  // turn on shield
  delay(20000);  // give time to log on to network.
 
  SIM900.print("AT+CMGF=1\r");  // set SMS mode to text
  delay(100);
  SIM900.print("AT+CNMI=2,2,0,0,0\r"); 
  // blurt out contents of new SMS upon receipt to the GSM shield's serial out
  delay(100);
}
 
void SIM900power()
// software equivalent of pressing the GSM shield "power" button
{
  digitalWrite(9, LOW);
  delay(1000);
  digitalWrite(9, HIGH);
  delay(7000);
}
 
void loop()
{
  // Now we simply display any text that the GSM shield sends out on the serial monitor
  if(SIM900.available() >0)
  {
    incoming_char=SIM900.read(); //Get the character from the cellular serial port.
    Serial.print(incoming_char); //Print the incoming character to the terminal.
  }
}
