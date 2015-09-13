float amps = 0.0;
float lastAmps=0.0;
float maxAmps = 0.0;
float minAmps = 0.0;
float noise = 0.0;

void setup() {
  // put your setup code here, to run once:

  Serial.begin(9600);
}

void loop() {
  // put your main code here, to run repeatedly:
  amps = ((514-analogRead(A0))*27.03)/1023; //FOR ACS712 5A
  amps = (amps + lastAmps)/2;
  lastAmps = amps;
  maxAmps = max(maxAmps,amps);
  minAmps = min(minAmps,amps);
  noise = maxAmps - minAmps;
  Serial.print(amps);
  Serial.print(" ");
  Serial.println(noise);
  if (Serial.read() != -1){
    maxAmps = amps;
    minAmps = amps;
  }
  
  delay(250);
}
