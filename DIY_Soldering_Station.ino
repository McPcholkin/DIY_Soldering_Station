/*
 * Arduino soldering iron test 
 * control temperature with variable resistor
 * get amplified sidnal to serial
 */

int pot = A1; // pot for testing
int pwm = 10; // pwm to mosfet
int temp = A0; // input from termal sensor in iron
int pwmControl = 0; //var to save pwm value

// control buttons
int buttonUp = 13;
int buttonDown = 12;


// temp control 
int tempSet = 230; //default set temp
int tempMin = 200; //minimum temp
int tempMax = 280; //max temp
int tempReal = 250; //val termal sensor var
int tempPwmMin = 40; //minimal value PWM
int tempPwmMax = 180; //maximum value PWM
int tempPwmReal = 0; //current PWM value

int tempError = -50; // difference temp (set to real)

int tempDiff = 0; //variable to diff temp (set to current)




void setup() {
  Serial.begin(9600);
  pinMode(pwm, OUTPUT);
  pinMode(buttonUp, INPUT);
  pinMode(buttonDown, INPUT);

  analogWrite(pinpwm, tempPwmReal); //Вывод  шим в нагрузку паяльника 
                                    //(выводим 0 - старт с выключеным паяльником- 
                                    // пока не опредилим состояние температуры)

}

void loop() {

  
int sensorVariable = analogRead(A0); //get iron sensor data
int potVariable = analogRead(A1);    //get pot data

pwmControl=map(potVariable,0,1023,0,255); // map pot 0-1023 as 0-255


int buttonUpState=digitalRead(buttonUp);    //get buttons state
int buttonDownState=digitalRead(buttonDown);


// Debug output
Serial.print("PWM Pot: ");
Serial.print(pwmControl);
Serial.print(" | ");
Serial.print("Sensor: ");
Serial.print(sensorVariable);
Serial.print(" | ");
Serial.print("Button UP: ");
Serial.print(buttonUpState);
Serial.print(" | ");
Serial.print("Button Down: ");
Serial.print(buttonDownState);
Serial.println("");
delay(100);


// set pwm value
analogWrite(pwm, pwmControl);



}
