/*
 * Arduino soldering station project
 * McPcholkin https://github.com/McPcholkin/DIY_Soldering_Station
 * 
 * Also used examples from:
 * Doug LaRu, 
 */
// enable debug serial output
//#define DEBUG 1
// enable sound
//#define SOUND 1
 
// ----------------  Pinout ----------------------
// iron control
const int pinPwmIron = 6; // pwm to mosfet
const int pinTempIron = A0; // input from termal sensor in iron

// air control
// zero cross detector pin = 2
const int pinControlAir = 3; // to tirac
const int pinControlAirFan = 5;
const int pinTempAir = A1; // input from termal sensor in iron

// control buttons
const int ironPowerToggle = 10;
const int airPowerToggle = 11;

//  Buzzer pin
const int buzzerPin = 9;

// Buttons
const int BUTTON_ERROR_WINDOW = 5;  // +/- this value 
const int BUTTON_DELAY = 210;       // delay to debounce button
const int controlButtonsPin = A4;   // switch circuit input connected to analog pin 4
long buttonLastChecked = 0; // variable to limit the button getting checked every cycle

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

const int lcdRefreshTime = 250; //refresh LCD every milisec 

// -----------------------------------------------
// -----------------------------------------------

// iron temp control 
int ironTempSet = 200; //default set temp
const int ironTempMin = 200; //minimum temp
const int  ironTempMax = 280; //max temp
int ironTempReal = 230; //val termal sensor var
const int ironTempPwmMin = 45; //minimal value PWM
const int ironTempPwmHalf = 99; //half value PWM
const int ironTempPwmMax = 230; //maximum value PWM
int ironTempPwmReal = 0; //current PWM value

// Iron Calibration
const int minIronTempValue = 25;  // room temperature
const int maxIronTempValue = 400; // max heater temperature
const int minIronAnalogValue = 0; // sensor value in room temperature
const int maxIronAnalogValue = 764; // sensor value on max heater temperature

// phisical power switch
boolean ironPowerState = 0; // iron ON state var

// int tempError = -50; // difference temp (set to real)  -- not used?
// int tempDiff = 0; //variable to diff temp (set to current) -- not used?

// Air temp control
int airTempSet = 200;          //default set temp
const int airTempMin = 200;    //minimum temp
const int airTempMax = 280;    //max temp
int airTempReal = 200;         //val termal sensor var
const int airTempPwmMin = 100; //minimal value PWM
const int airTempPwmHalf = 64; //half value PWM
const int airTempPwmMax = 0;   //maximum value PWM
int airTempPwmReal = 0;        //current PWM value

// Air Calibration
const int minAirTempValue = 25;    // room temperature
const int maxAirTempValue = 400;   // max heater temperature
const int minAirAnalogValue = 0;   // sensor value in room temperature
const int maxAirAnalogValue = 764; // sensor value on max heater temperature

// phisical power switch
boolean airPowerState = 0; // Air ON state var

// Air Fan control
int fanSpeedSet = 50;       //default set fan speed in %
const int fanSpeedMin = 30; // min fan speed in %
const int fanSpeedMax = 100; // max fan speed in %
//int fanSpeedReal = 50;      // current fan speed in %
const int fanSpeedPwmMin = 20;  // min PWM value
const int fanSpeedPwmMax = 255; // max PWM value
int fanSpeedPwmReal = 0; // current PWM value

// colling state
boolean airCooldownState = 0; // air gun cooling down 
//unsigned long airCooldownStartTime; // air gun cooling start time
//unsigned long const airCooldownTime = 90000; // 90 sec to cool heater

// increment to save current temp value
int incrementIron = 000; //start value of iron sensor
int incrementAir = 000; //start value of air sensor
//int incrementFan = 00; //start value of Fan %

// -------- make some custom characters 5x8 pix:
byte degree[8] = {
  0b11100,
  0b10100,
  0b11100,
  0b00000,
  0b00000,
  0b00000,
  0b00000,
  0b00000
};
byte term[8] = {
  0b00100,
  0b01010,
  0b01010,
  0b01010,
  0b01010,
  0b10001,
  0b10001,
  0b01110
};
byte fan[8] = {
  0b00000,
  0b11001,
  0b01011,
  0b00100,
  0b11010,
  0b10011,
  0b00000,
  0b00000,
};
byte ironTop[8] = {
  0b00001,
  0b00010,
  0b00100,
  0b01110,
  0b01110,
  0b01110,
  0b01110,
  0b01110
};
byte ironBottom[8] = {
  0b01110,
  0b11111,
  0b11111,
  0b00100,
  0b00100,
  0b00100,
  0b00100,
  0b00100
};
byte heat[8] = {
  0b11111,
  0b10101,
  0b10001,
  0b10101,
  0b11111,
  0b00000,
  0b00000,
  0b00000
};
byte linesLeft[8] = {
  0b00011,
  0b11011,
  0b11011,
  0b11011,
  0b11011,
  0b11011,
  0b11011,
  0b11011
};
byte linesFull[8] = {
  0b11011,
  0b11011,
  0b11011,
  0b11011,
  0b11011,
  0b11011,
  0b11011,
  0b11011
};
// ---------------------------------------------


//---------- analog smoothing Iron -----------------
const int numReadingsIron = 100;

int readingsIron[numReadingsIron];      // the readings from the analog input
int readIndexIron = 0;              // the index of the current reading
int totalIron = 0;                  // the running total
int averageIron = 0;                // the average
//------------------------------------------


#ifdef DEBUG
// --------------- debug --------------------
int sensorVariable = 0; // iron sensor data
//------------------------------------------
#endif

void setup() {
  //debug
  #ifdef DEBUG
    Serial.begin(115200);
  #endif
    
  pinMode(pinPwmIron, OUTPUT);
  pinMode(pinControlAir, OUTPUT);
  pinMode(pinControlAirFan, OUTPUT);
  pinMode(ironPowerToggle, INPUT);
  pinMode(airPowerToggle, INPUT);

  analogWrite(pinPwmIron, ironTempPwmReal); //Вывод  шим в нагрузку паяльника 
                                    //(выводим 0 - старт с выключеным паяльником- 
                                    // пока не опредилим состояние температуры)

  analogWrite(pinControlAirFan, fanSpeedPwmReal);                          

  // ------------------ LCD -----------------
  // set up the LCD's number of columns and rows:
  lcd.begin(16, 2);

  // create a new character
  lcd.createChar(0, degree);
  lcd.createChar(1, fan);
  lcd.createChar(2, heat);
  lcd.createChar(3, ironTop);
  lcd.createChar(4, ironBottom);
  lcd.createChar(5, term);
  lcd.createChar(6, linesLeft);
  lcd.createChar(7, linesFull);
  
  // Print a welcome message to the LCD.
  lcd.setCursor(0, 0);
  lcd.print("Soldering");
  lcd.setCursor(0, 1);
  lcd.print("Station FW:0.1");
  lcd.setCursor(15, 0);
  lcd.write(byte(3));
  lcd.setCursor(15, 1);
  lcd.write(byte(4));

  #ifdef SOUND
  // play tone
    tone(buzzerPin, 900, 150);
    delay(150);
    tone(buzzerPin, 1000, 150);
    delay(150);
    tone(buzzerPin, 1100, 150);
    delay(150);
    noTone(buzzerPin);
  #endif
  
  delay(1500);
  // clear display from welcome message
  lcd.clear();

  //---------------------- LCD --------------

  // initialize all the readings iron temp to 0:
  for (int thisReadingIron = 0; thisReadingIron < numReadingsIron; thisReadingIron++) {
    readingsIron[thisReadingIron] = 0;
  }

}

void loop() {
#ifdef DEBUG
// ------------- debug  --------------------//
sensorVariable = analogRead(pinTempIron); //get iron sensor data
//sensorVariable = analogRead(pinTempAir); //get iron sensor data
//int potVariable = analogRead(A1);    //get pot data
//pwmControl=map(potVariable,0,1023,0,255); // map pot 0-1023 as 0-255

//int buttonUpState=digitalRead(ironButtonUp);    //get buttons state
//int buttonDownState=digitalRead(ironButtonDown);

// Debug output
//Serial.print("PWM Pot: ");
//Serial.print(pwmControl);
Serial.print("Inc Iron: ");
Serial.print(incrementIron);
Serial.print(" | ");
Serial.print("Sensor Iron: ");
Serial.print(sensorVariable);
Serial.print(" | ");
Serial.print("Set Iron: ");
Serial.print(ironTempSet);
Serial.print(" | ");
Serial.print("Iron On: ");
Serial.print(ironPowerState);
Serial.println("");
Serial.print("Inc Air: ");
Serial.print(incrementAir);
//Serial.print(" | ");

Serial.println("");
//delay(100);
// set pwm value
//analogWrite(pinPwmIron, pwmControl);
// ------------- debug end --------------------//
#endif

// ---------------------  Main code --------------------------------------------------------------

// smooth iron meshure values
smoothIron();

// refresh LCD screen
show();   


// ------------------------------  Iron temp control  -------------------------------------------------------

if ( digitalRead(ironPowerToggle == HIGH)){ // if iron "ON" switch is enabled
  ironPowerState = 1; // chandge power state of iron to ON
  
  if (ironTempReal < ironTempSet ){   // Если температура паяльника ниже установленной температуры то:
    if ((ironTempSet - ironTempReal) < 16 & (ironTempSet - ironTempReal) > 6 )  // Проверяем разницу между 
                                                         // установленной температурой и текущей паяльника,
                                                         // Если разница меньше 10 градусов то 
      {
        ironTempPwmReal = ironTempPwmHalf; // Понижаем мощность нагрева (шим 0-255  мы делаем 99)  - 
                                           // таким образом мы убираем инерцию перегрева
      }

  else if ((ironTempSet - ironTempReal) < 4 ) // if difference less 4 degree use min temp
    {
      ironTempPwmReal = ironTempPwmMin; 
    }

  else 
    {
      ironTempPwmReal = ironTempPwmMax; // Иначе Подымаем мощность нагрева(шим 0-255  мы делаем 230) на максимум 
                                        // для быстрого нагрева до нужной температуры
    }

  analogWrite(pinPwmIron, ironTempPwmReal); // Вывод в шим порт (на транзистор) значение мощности
  }

  else { // Иначе (если температура паяльника равняется или выше установленной) 
       ironTempPwmReal = 0;  // Выключаем мощность нагрева (шим 0-255  мы делаем 0)  - 
                         // таким образом мы отключаем паяльник
       analogWrite(pinPwmIron, ironTempPwmReal); // Вывод в шим порт (на транзистор) значение 
       }

  ironTempReal = analogRead(pinTempIron); // считываем текущую температуру

  // scale heater temperature to sensor values
  ironTempReal=map(ironTempReal, minIronAnalogValue, maxIronAnalogValue, minIronTempValue, maxIronTempValue); 
                             // нужно вычислить
                             // 0 sens is 25 on iron - 764 is 295 on iron
                             // 400 - get 228-232 on iron when ironTempSet = 230
  incrementIron=ironTempReal; 
}
else
{
  analogWrite(pinPwmIron, 0); // Disable iron heater if switch off
  ironPowerState = 0;        // chandge iron power state to OFF
}

//----------------------------------------------------------------------------------------------------

// ------------------------------  Air temp control  -------------------------------------------------------

if ( digitalRead(airPowerToggle == HIGH)){ // if iron "ON" switch is enabled
  airPowerState = 1; // chandge power state of iron to ON
   
  if (airTempReal < airTempSet ){   // if temp of AirGun less set temp than:
    if ((airTempSet - airTempReal) < 16 & (airTempSet - airTempReal) > 6 )  // check difference between
                                                         // set air temp and current temp,
                                                         // If difference less 10 degree than 
      {
        airTempPwmReal = airTempPwmHalf; // set heat power to half (pwm 128-0  we set  64)  - 
                                         // таким образом мы убираем инерцию перегрева
      }

  else if ((airTempSet - airTempReal) < 4 ) // if difference less 4 degree use min temp
    {
      airTempPwmReal = airTempPwmMin; 
    }

  else 
    {
      airTempPwmReal = airTempPwmMax; // Иначе Подымаем мощность нагрева(шим 0-255  мы делаем 230) на максимум 
                                        // для быстрого нагрева до нужной температуры
    }

  analogWrite(pinControlAir, airTempPwmReal); // Вывод в шим порт (на транзистор) значение мощности
  }

  else { // Иначе (если температура паяльника равняется или выше установленной) 
       airTempPwmReal = 0;  // Выключаем мощность нагрева (шим 0-255  мы делаем 0)  - 
                         // таким образом мы отключаем паяльник
       analogWrite(pinControlAir, airTempPwmReal); // Вывод в шим порт (на транзистор) значение 
       }

  airTempReal = analogRead(pinTempAir); // считываем текущую температуру

  // scale heater temperature to sensor values
  airTempReal=map(airTempReal, minAirAnalogValue, maxAirAnalogValue, minAirTempValue, maxAirTempValue); 
                             // нужно вычислить
                             // 0 sens is 25 on iron - 764 is 295 on iron
                             // 400 - get 228-232 on iron when ironTempSet = 230
  incrementAir=airTempReal;
}
else
{
  analogWrite(pinControlAir, 0); // Disable iron heater if switch off
  airPowerState = 0;        // chandge iron power state to OFF

  if ( airCooldownState == 0 && incrementAir > minAirTempValue ) // if cooling not start and air temp 
  {                                                             // more room temp
    airCooldownState = 1;        // cooldown started
  }
}


//----------------------------------------------------------------------------------------------------



// --------------  Air Fan control  --------------------------
if ( airPowerState == 1 && airCooldownState == 0 ) // if cooling not start - normal work
{
  // get PWM value from % value
  fanSpeedPwmReal=map(fanSpeedSet, fanSpeedMin, fanSpeedMax, fanSpeedPwmMin, fanSpeedPwmMax);
  analogWrite(pinControlAirFan, fanSpeedPwmReal); 
}
else if ( airPowerState == 0 ) // if air switch off
{
  if ( airCooldownState = 1 ) //  if cooling trigered
  {
   if (incrementAir > minAirTempValue) // if air switch off and temp more than room temp
    {
      analogWrite(pinControlAirFan, fanSpeedPwmMax); // run fan on max speed to cooling
    }
    else
    { // when temp down to room temp disable cooling triger
      airCooldownState = 0;        // cooldown stoped, air temp eq room temp
    }
  }
  else
  { // if air is off and cooling not start just off fan
    analogWrite(pinControlAirFan, 0);
  }
  
}
//------------------------------------------------------

//---------------- buttons ---------------//

if( buttonLastChecked == 0 ) // see if this is the first time checking the buttons
   buttonLastChecked = millis()+BUTTON_DELAY;  // force a check this cycle
 if( millis() - buttonLastChecked > BUTTON_DELAY ) { // make sure a reasonable delay passed
   if( int buttNum = buttonPushed(controlButtonsPin) ) {
     
     #ifdef DEBUG
     Serial.print("Button "); Serial.print(buttNum); Serial.println(" was pushed."); 
     #endif

     // ------ iron  -----------------------------
     if (buttNum == 1) // Iron temp Down
       {
     if ( ironTempSet <= ironTempMin || (ironTempSet-5) <= ironTempMin )
      {
        ironTempSet = ironTempMin;
        incrementIron = ironTempSet;
      }

     else {
          ironTempSet=ironTempSet-5;
          incrementIron = ironTempSet;
          }
      }
 
    else if (buttNum == 2) // Iron temp Up
      {
      if ( ironTempSet >= ironTempMax )
        {
          ironTempSet = ironTempMax;
        }
      else {
          ironTempSet=ironTempSet+5;
           }
     incrementIron = ironTempSet;
      }
     //------------------------------------------

     // --- air ----------------------------------
     if (buttNum == 3) // Air temp Down
       {
     if ( airTempSet <= airTempMin || (airTempSet-5) <= airTempMin )
      {
        airTempSet = airTempMin;
        incrementAir = airTempSet;
      }

     else {
          airTempSet=airTempSet-5;
          incrementAir = airTempSet;
          }
      }
 
    else if (buttNum == 4) // Air temp Up
      {
      if ( airTempSet >= airTempMax )
        {
          airTempSet = airTempMax;
        }
      else {
          airTempSet=airTempSet+5;
           }
     incrementAir = airTempSet;
      }
    //--------------------------------------------

    //-------------- Fan ------------------
     if (buttNum == 5) // Fan Speed Down
       {
     if ( fanSpeedSet <= fanSpeedMin || (fanSpeedSet-5) <= fanSpeedMin )
      {
        fanSpeedSet = fanSpeedMin;
      }

     else {
          fanSpeedSet=fanSpeedSet-5;
          }
      }
 
    else if (buttNum == 6) // Fan Speed Up
      {
      if ( fanSpeedSet >= fanSpeedMax )
        {
          fanSpeedSet = fanSpeedMax;
        }
      else {
          fanSpeedSet=fanSpeedSet+5;
           }
      }
     //------------------------------------
   
   }
   buttonLastChecked = millis(); // reset the lastChecked value
 }
//---------------------------------------------//

// end loop
}


//----------------------- Buttons analog values check ------------------
int buttonPushed(int pinNum) {
 int val = 0;         // variable to store the read value
   val = analogRead(pinNum);   // read the input pin
   
   #ifdef DEBUG_ON
     Serial.println(val);
   #endif
   

   if( val >= (1012-BUTTON_ERROR_WINDOW) and val <= (1015+BUTTON_ERROR_WINDOW) ) {  // 1012-1015
     #ifdef DEBUG_ON
     Serial.println("switch 1 pressed/triggered");
     #endif
     return 1;
   }
   else if ( val >= (966-BUTTON_ERROR_WINDOW) and val <= (971+BUTTON_ERROR_WINDOW) ) { // 966-971
     #ifdef DEBUG_ON
     Serial.println("switch 2 pressed/triggered");
     #endif
     return 2;
   }
   else if ( val >= (883-BUTTON_ERROR_WINDOW) and val <= (886+BUTTON_ERROR_WINDOW) ) { // 883-886
     #ifdef DEBUG_ON
     Serial.println("switch 3 pressed/triggered");
     #endif
     return 3;
   }
   else if ( val >= (812-BUTTON_ERROR_WINDOW) and val <= (818+BUTTON_ERROR_WINDOW) ) { // 812-818
     #ifdef DEBUG_ON
     Serial.println("switch 4 pressed/triggered");
     #endif
     return 4;
   }
   else if( val >= (749-BUTTON_ERROR_WINDOW) and val <= (758+BUTTON_ERROR_WINDOW) )  { // 749-758
     #ifdef DEBUG_ON
     Serial.println("switch 5 pressed/triggered");    
     #endif
     return 5;
   }
   else if( val >= (699-BUTTON_ERROR_WINDOW) and val <= (703+BUTTON_ERROR_WINDOW) )  { // 699-703
     #ifdef DEBUG_ON
     Serial.println("switch 6 pressed/triggered");    
     #endif
     return 6;
   }
   else
     return 0;  // no button found to have been pushed
}
// --------------------------------------------------------------------------------------


//----------------  smooth iron -----------------

void smoothIron()
{
  // subtract the last reading:
  totalIron = totalIron - readingsIron[readIndexIron];
  // read from the sensor:
  readingsIron[readIndexIron] = incrementIron; // bouncing value !
  // add the reading to the total:
  totalIron = totalIron + readingsIron[readIndexIron];
  // advance to the next position in the array:
  readIndexIron = readIndexIron + 1;

  // if we're at the end of the array...
  if (readIndexIron >= numReadingsIron) {
    // ...wrap around to the beginning:
    readIndexIron = 0;
  }

  // calculate the average:
  averageIron = totalIron / numReadingsIron;

}

//------------------------------------------------------------------  

// ----------------------------------  LCD ------------------------
void show()
{
  static unsigned long last_lcd_refresh_time = 0;
 unsigned long lcd_refresh_time = millis();
 // If interrupts come faster than 200ms, assume it's a bounce and ignore
 if (lcd_refresh_time - last_lcd_refresh_time > lcdRefreshTime)
 {
  if (ironPowerState == 1)
  {
   // iron temp when iron is ON
   lcd.setCursor(0, 0);
   lcd.print("I:");
   lcd.setCursor(2, 0);
   lcd.print(averageIron);
   lcd.setCursor(5, 0);
   lcd.write(byte(0));
   lcd.setCursor(6, 0);
   lcd.print(">");
   lcd.setCursor(8, 0);
   lcd.print(ironTempSet);
   lcd.setCursor(11, 0);
   lcd.write(byte(0));
  }
  else // iron is off
  {
   lcd.setCursor(0, 0);
   lcd.print("I:");
   lcd.setCursor(2, 0);
   lcd.print("OFF");
   lcd.setCursor(5, 0);
   lcd.write(" ");
   lcd.setCursor(6, 0);
   lcd.print(">");
   lcd.setCursor(8, 0);
   lcd.print(ironTempSet);
   lcd.setCursor(11, 0);
   lcd.write(byte(0));
  }
 // iron state
 lcd.setCursor(13, 0);
 lcd.print("SLP");

 // HeatGun temp
 lcd.setCursor(0, 1);
 lcd.print("A:");
 lcd.setCursor(2, 1);
 lcd.print(ironTempPwmReal);
 lcd.setCursor(5, 1);
 lcd.write(byte(0));
 lcd.setCursor(6, 1);
 lcd.print(">");
 lcd.setCursor(8, 1);
 lcd.print(airTempSet);
 lcd.setCursor(11, 1);
 lcd.write(byte(0));
 
 // HeatGun fan speed
 lcd.setCursor(12, 1);
 lcd.write(byte(1));
 //lcd.setCursor(12, 1);
 //lcd.print("S:");
 lcd.setCursor(13, 1);
   if ( fanSpeedSet == 100 )
   {
    lcd.print(fanSpeedSet);
   }
   else 
   {
    lcd.print(fanSpeedSet);
    lcd.setCursor(15, 1);
    lcd.print("%");
   }



 last_lcd_refresh_time = lcd_refresh_time;
}
}


