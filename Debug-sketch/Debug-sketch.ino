// Used to test all connections on board
// iron and fan replaced by voltmeter
// Heater replaced by light bolb

// ----------------  Pinout ----------------------
// iron control
const int pinPwmIron = 6; // pwm to mosfet
const int pinTempIron = A0; // input from termal sensor in iron

// air control
// zero cross detector pin = 2
const int pinPwmAir = 3; // pwm to tirac
const int pinPwmAirFan = 5;
const int pinTempAir = A1; // input from termal sensor in iron

// control buttons
const int ironPowerToggle = 10;
const int airPowerToggle = 11;

// Button board
const int controllButtons = A6;

// -------- LiquidCrystal 16x2 LCD display. --------
/* The circuit:
 * LCD RS pin to digital pin 4
 * LCD Enable pin to digital pin 7
 * LCD D4 pin to digital pin 8
 * LCD D5 pin to digital pin 12
 * LCD D6 pin to digital pin 13
 * LCD D7 pin to digital pin 19 (A5)
 * LCD R/W pin to ground
 * 10K resistor:
 * ends to +5V and ground
 * wiper to LCD VO pin (pin 3)
 */
 
// include the library code:
#include <LiquidCrystal.h>

// initialize the library with the numbers of the interface pins
LiquidCrystal lcd(4, 7, 8, 12, 13, 19);

const int lcdRefreshTime = 250;

//  Buzzer pin
const int buzzerPin = 3;

int dimming = 128;  // Dimming level (0-128)  0 = ON, 128 = OFF


void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);

  pinMode(pinPwmIron, OUTPUT);
  pinMode(pinPwmAir, OUTPUT);
  pinMode(pinPwmAirFan, OUTPUT);
  pinMode(ironPowerToggle, INPUT);
  pinMode(airPowerToggle, INPUT);

  analogWrite(pinPwmIron, 0);

  analogWrite(pinPwmAirFan, 0);

  lcd.begin(16, 2);
  // Print a welcome message to the LCD.
  lcd.setCursor(0, 0);
  lcd.print("Debug");
// play tone
  tone(buzzerPin, 900, 150);
  delay(150);
  tone(buzzerPin, 1000, 150);
  delay(150);
  tone(buzzerPin, 1100, 150);
  delay(150);
  noTone(buzzerPin);
  
  delay(1500);
  // clear display from welcome message
  lcd.clear();


  attachInterrupt(0, zero_crosss_int, RISING);  // Choose the zero cross interrupt # from the table above
}


void zero_crosss_int()  // function to be fired at the zero crossing to dim the light
{
  // Firing angle calculation : 1 full 50Hz wave =1/50=20ms 
  // Every zerocrossing thus: (50Hz)-> 10ms (1/2 Cycle) For 60Hz => 8.33ms 
  // 10ms=10000us
  // (10000us - 10us) / 128 = 75 (Approx) For 60Hz =>65
  int dimtime = (75*dimming);    // For 60Hz =>65    
  delayMicroseconds(dimtime);    // Off cycle
  digitalWrite(pinPwmAir, HIGH);   // triac firing
  delayMicroseconds(10);         // triac On propogation delay (for 60Hz use 8.33)
  digitalWrite(pinPwmAir, LOW);    // triac Off
}

  

void loop() {
  // put your main code here, to run repeatedly:


int sensorVariableIron = analogRead(pinTempIron); //get iron sensor data
int sensorVariableAir  = analogRead(pinTempAir); 
int sensorVariableButtons = analogRead(controllButtons);
int ironToggleState = digitalRead(ironPowerToggle);
int airToggleState = digitalRead(airPowerToggle);

Serial.print("A0: ");
Serial.print(sensorVariableIron);
Serial.print(" | ");
Serial.print("A1: ");
Serial.print(sensorVariableAir);
Serial.print(" | ");
Serial.print("A6: ");
Serial.print(sensorVariableButtons);
Serial.print(" | ");
Serial.print("Iron ON: ");
Serial.print(ironToggleState);
Serial.print(" | ");
Serial.print("Air ON: ");
Serial.print(airToggleState);
Serial.print(" | ");

Serial.println("");

lcd.setCursor(0, 0);
   lcd.print("A0:");
   lcd.setCursor(4, 0);
   lcd.print(sensorVariableIron);
   lcd.setCursor(7, 0);
   lcd.print("A0");
   lcd.setCursor(10, 0);
   lcd.print(sensorVariableAir);

if (ironToggleState == 1)
{
  lcd.setCursor(0, 1);
  lcd.print("Iron ON");
}
else {
  lcd.setCursor(0, 1);
  lcd.print("IronOFF");
}

if (airToggleState == 1)  
{
   lcd.setCursor(8, 1);
   lcd.print("Air ON");
}
else {
  lcd.setCursor(8, 1);
  lcd.print("AirOFF");
}


  for (int i=5; i <= 128; i++){
    dimming=i;
      delay(10);
   }


for (int i=0; i<=255; i++)
{
  analogWrite(pinPwmIron, i);
  analogWrite(pinPwmAirFan, i);
  delay(100);
}


delay(500);


}






