/*
 * Arduino soldering iron test 
 * control temperature with variable resistor
 * get amplified sidnal to serial
 */

int pot = A1;
int pwm = 10;
int temp = A0;
int pwmControl = 0;


void setup() {
  Serial.begin(9600);
  pinMode(pwm,OUTPUT);
  

}

void loop() {
int sensorVariable = analogRead(A0);
int potVariable = analogRead(A1);

pwmControl=map(potVariable,0,1023,0,255); // map pot 0-1023 as 0-255

Serial.print("Pot: ");
Serial.print(pwmControl);
Serial.print(" | ");
Serial.print("Sensor: ");
Serial.print(sensorVariable);
Serial.println("");
delay(100);



analogWrite(pwm, pwmControl);


}
