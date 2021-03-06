/*
 * Arduino soldering station project
 * McPcholkin https://github.com/McPcholkin/DIY_Soldering_Station
 * 
 * Also used examples from:
 * Doug LaRu, alex.marinenko, David A. Mellis, Paul Stoffregen, Scott Fitzgerald, Arturo Guadalupi
 */
// enable debug serial output  //
//#define DEBUGIRON
//#define DEBUGAIR
//#define DEBUGBUTTONS
//#define DEBUG_ON
//#define SERIALDEBUG

// FW Version
const float fwVersion = 0.9;

// enable sound
#define SOUND 1
 
// ----------------  Pinout ----------------------
// Iron control
const int pinPwmIron = 6;   // pwm to mosfet
const int pinTempIron = A0; // input from termal sensor in iron

// Air control
// zero cross detector pin = 2
// const int pinControlAir = 3; // to tirac
const int pinControlAirFan = 5; // pwm to fan
const int pinTempAir = A1;      // input from termal sensor in фшк пгт

// Power toggles
const int ironPowerToggle = 11; // Iron ON\OFF switch
const int airPowerToggle = 10;  // Ati  ON\OFF switch

//  Buzzer pin
const int buzzerPin = 9;

// Buttons (analog input)
const int BUTTON_ERROR_WINDOW = 5;  // +/- this value 
const int BUTTON_DELAY = 210;       // delay to debounce button
const int controlButtonsPin = A4;   // buttons circuit input connected to analog pin 4
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

//--------------- iron temp control ------------------------------------
int ironTempSet = 230;           //default set temp
const int ironTempMin = 200;     //minimum temp
const int ironTempMax = 310;    //max temp
int ironTempReal = 0;            //val termal sensor analog
int ironTempRealC = 0;           //val termal sensor in celsius
const int ironTempPwmMin = 50;   //minimal value PWM
const int ironTempPwmHalf = 125; //half value PWM
const int ironTempPwmMax = 220;  //maximum value PWM
int ironTempPwmReal = 0;         //current PWM value

// Iron Calibration (may need some tweking)
const int minIronTempValue = 20;    // min temperature in celsius
const int maxIronTempValue = 330;   // max heater temperature
const int minIronAnalogValue = 5;   // sensor value in room temperature
const int maxIronAnalogValue = 640; // sensor value on max heater temperature

// phisical power switch state
boolean ironPowerState = 0; // iron ON state var
// disconnected detection
boolean ironDisconnected = 0;
//-------------------------------------------------------------------------

// Lib tu use AC tirac controll
#include <CyberLib.h>  //attach lib by Cyber-Place.ru for dimming
volatile uint8_t tic, Dimmer1 = 230; // 220 = min/off. 0 = max 
uint8_t data;
uint8_t dir = 1;

// Air temp control
int airTempSet = 300;          //default set temp
const int airTempMin = 100;    //minimum temp
const int airTempMax = 580;    //max temp
int airTempReal = 0;           //val termal sensor var
int airTempRealC = 0;          // termal sensor var in celsius
const int airDimmerMin = 155;  //minimal value PWM
const int airDimmerHalf = 64;  //half value PWM
const int airDimmerMax = 0;    //maximum value PWM
const int airDimmerOff = 230;  //maximum value PWM

// Air Calibration
const int minAirTempValue = 40;    // room temperature
const int maxAirTempValue = 580;   // max heater temperature
const int minAirAnalogValue = 5;   // sensor value in room temperature
const int maxAirAnalogValue = 702; // sensor value on max heater temperature

// Air disconnected trashhold
const int airDisconnectedTrashold = 720; // analog value from sensor when air disconnected.

// phisical power switch state
boolean airPowerState = 0; // Air ON state var
// disconnected detection
boolean airDisconnected = 0;
//-------------------------------------------------------------------------

//-------------  Air Fan control  ----------------------
int fanSpeedSet = 50;           //default set fan speed in %
const int fanSpeedMin = 30;     // min fan speed in %
const int fanSpeedMax = 100;    // max fan speed in %
const int fanSpeedPwmMin = 40;  // min PWM value
const int fanSpeedPwmMax = 240; // max PWM value
int fanSpeedPwmReal = 0;        // current PWM value

// colling state
boolean airCooldownState = 0; // air gun cooling down 
//------------------------------------------------------

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
const int numReadingsIron = 30;     // if system become unstable decreace value

int readingsIron[numReadingsIron];  // the readings from the analog input
int readIndexIron = 0;              // the index of the current reading
int totalIron = 0;                  // the running total
int averageIronTemp = 0;            // the average
int averageIronTempPretty = 0;      //  just pretty value to display
//------------------------------------------

//---------- analog smoothing Air -----------------
const int numReadingsAir = 30;     // if system become unstable decreace value

int readingsAir[numReadingsAir];   // the readings from the analog input
int readIndexAir = 0;              // the index of the current reading
int totalAir = 0;                  // the running total
int averageAirTemp = 0;            // the average
int averageAirTempPretty = 0;      //  just pretty value to display
//------------------------------------------

//--- beep without delay ---
int beepState = 0; // last beep state
unsigned long previousMillisBeep = 0;  // will store last time Beep was updated
const long intervalBeep = 1000;        // interval at which to Beep (milliseconds)
//--------------------------

//--- cooldown without delay ---
unsigned long previousMillisCooldown = 0;  // will store last time Cooldown was updated
const long cooldownTime = 120000;           // Cooldown time  (milliseconds) (120 SEC)
//--------------------------


// Setup some stuff
void setup() {
  //debug
  #ifdef SERIALDEBUG
    Serial.begin(115200);
  #endif
  
  // Config pins  
  pinMode(pinPwmIron, OUTPUT);
  pinMode(pinControlAirFan, OUTPUT);
  pinMode(ironPowerToggle, INPUT);
  pinMode(airPowerToggle, INPUT);
  
  analogWrite(pinPwmIron, ironTempPwmReal); // Write PWM to Iron output
                                            // (Write 0 - start with iron OFF -
                                            // until get iron temp)

  analogWrite(pinControlAirFan, fanSpeedPwmReal); // Write PWM to Air fan
                                                  // (Write 0 - start with fan OFF -
                                                  // until get air temp)


  // --------  Dimmer for air setup -------
  D3_Out; // set ports to output (pin 3)
  D3_Low; // set LOW signal level to output
  D2_In;  // set port to input fot detect zero-cross signal

  //CHANGE  – interrupt call on ANY value chandge on input 
  //RISING  – interrupt call on chandge level from LOW to HIGH on input
  //FALLING – interrupt call on chandge level from HIGH to LOW on input
  //LOW     – interrupt call on HIGH level on input
  //HIGH    – interrupt call on LOW level on input
  attachInterrupt(0, detect_up, LOW); // set interrupt triggering - "interrupt0" on "pin 2" to LOW
  StartTimer1(halfcycle, 40);         // time to one PWM rate
  StopTimer1();                       // stop timer
  // -------------------------------------

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
  lcd.print("Station FW:");
  lcd.setCursor(11, 1);
  lcd.print(fwVersion);
  lcd.setCursor(14, 1);
  lcd.print(" ");
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

  // ----- set smooth to zeros --------
  // initialize all the readings iron temp to 0:
  for (int thisReadingIron = 0; thisReadingIron < numReadingsIron; thisReadingIron++) {
    readingsIron[thisReadingIron] = 0;
  }
  // initialize all the readings air temp to 0:
  for (int thisReadingAir = 0; thisReadingAir < numReadingsAir; thisReadingAir++) {
    readingsAir[thisReadingAir] = 0;
  }
  // ----------------------------------
}
// setup done


// ----------------------------- interrupt hendlers -----------------------------
void halfcycle()  // timer interrupt
{ 
  tic++;          // counter
  if(Dimmer1 < tic ) D3_High; // output controll
}

void  detect_up() // handle outside interrupt. start by up front
{  
 tic=0;           // reset counter
 ResumeTimer1();  // start counter
 attachInterrupt(0, detect_down, HIGH); // reprogram interrupt to other hendler
}  

void  detect_down() // handle outside interrupt. start by down front
{   
 StopTimer1();      // stop timer
 D3_Low;            // logical zero to output 
 tic=0;             // reset counter 
 attachInterrupt(0, detect_up, LOW);   // reprogram interrupt to other hendler
} 
// ----------------------------------------------------------------------------


// main loop
void loop() {

// ------------- debug  --------------------//
#ifdef DEBUGAIR
//int sensorVariable = analogRead(pinTempAir); //get air sensor data

Serial.print("Fan %: ");
Serial.print(fanSpeedSet);
Serial.print(" | ");
Serial.print("Fan PWM: ");
Serial.print(fanSpeedPwmReal);
Serial.print(" | ");
Serial.print("Air On: ");
Serial.print(airPowerState);
Serial.print(" | ");
Serial.print("Air Cool: ");
Serial.print(airCooldownState);
Serial.print(" | ");
Serial.print("Sensor Air: ");
Serial.print(airTempReal);
Serial.print(" | ");
Serial.print("Set Air: ");
Serial.print(airTempSet);
Serial.print(" | ");
Serial.print("Real Air: ");
Serial.print(airTempRealC);
Serial.print(" | ");
Serial.print("Dimmer: ");
Serial.print(Dimmer1);
Serial.print(" | ");
Serial.print("Dsc: ");
Serial.print(airDisconnected);
Serial.print(" | ");
Serial.print("Pretty: ");
Serial.print(averageAirTempPretty);
Serial.println("");
#endif


#ifdef DEBUGIRON
//int sensorVariable = analogRead(pinTempIron); //get iron sensor data

Serial.print("Sensor Iron: ");
Serial.print(ironTempReal);
Serial.print(" | ");
Serial.print("Set Iron: ");
Serial.print(ironTempSet);
Serial.print(" | ");
Serial.print("Iron On: ");
Serial.print(ironPowerState);
Serial.print(" | ");
Serial.print("Iron real: ");
Serial.print(ironTempRealC);
Serial.print(" | ");
Serial.print("Iron PWM: ");
Serial.print(ironTempPwmReal);
Serial.println("");
#endif


#ifdef DEBUGBUTTONS
int analog4 = analogRead(controlButtonsPin);
int gercon = digitalRead(17);

Serial.print("Analog: ");
Serial.print(analog4);
Serial.print(" | ");
Serial.print("Gercon: ");
Serial.print(gercon);

Serial.println("");
delay(200);

#endif
// ------------- debug end --------------------//


// ---------------------  Main code -----------------

// smooth iron meshure values
smoothIron();

// smooth air meshure values
smoothAir();

// refresh LCD screen
show();   

// read iron temp 
GetIronTemp();

// read air temp
GetAirTemp();

// alert when iron or air is diconnected
disconnectAlert();

// ----------------------------------------------------

// ------------------------------  Iron temp control logic ----------------------------------------------------

ironPowerState = digitalRead(ironPowerToggle);
if ( ironPowerState == 1 && ironDisconnected == 0 ){// check if iron "ON" switch is enabled and iron connected
  
  if (ironTempRealC < ironTempSet ){  // if iron temp lower then set temp do:
    if ((ironTempSet - ironTempRealC) < 16 & (ironTempSet - ironTempRealC) > 6 ) // check difference between -
                                                                        // set iron temp and current iron temp,
                                                                        // if difference less 10 degree than: 
      {
        ironTempPwmReal = ironTempPwmHalf; // set heater power to half (PWM 0-255 we set 99) - 
                                           // by that we decrease heater inertia
      }

  else if ((ironTempSet - ironTempRealC) < 4 ) // if difference less 4 degree set minimal PWM
    {
      ironTempPwmReal = ironTempPwmMin; 
    }

  else 
    {
      ironTempPwmReal = ironTempPwmMax; // else set heater power to max (PWM 0-255 we set 230) 
                                        // for fast heating to set temperature 
    }
  }

  else {                      // else (if iron temp equal or higher then set ) 
       ironTempPwmReal = 0;   // disable heater (PWM 0-255 we set 0) - 
                              // by that we disable iron heater
       }
  analogWrite(pinPwmIron, ironTempPwmReal);  // write PWM value to transistor port 
}
else 
{
  analogWrite(pinPwmIron, 0); // Disable iron heater if iron switch is OFF or iron disconnected
}                             // Or iron temp sensor fail

//----------------------------------------------------------------------------------------------------


// ------------------------------  Air temp control logic  -------------------------------------------------------

airPowerState = digitalRead(airPowerToggle);
if ( airPowerState == 1){           // check if air "ON" switch is enabled
 
  if (airTempRealC < airTempSet ){  // if temp of AirGun less then set temp do:
    if ((airTempSet - airTempRealC) < 16 & (airTempSet - airTempRealC) > 6 )  // check difference between -
                                                                        // set air temp and current temp,
                                                                        // If difference less 10 degree than: 
      {
        Dimmer1 = airDimmerHalf;   // set heater power to half (delay 128-0  we set  64)  - 
                                   // by that we decrease heater inertia
      }

  else if ((airTempSet - airTempRealC) < 4 )  // if difference less 4 degree set max delay
    {
      Dimmer1 = airDimmerMin; 
    }

  else 
    {
      Dimmer1 = airDimmerMax;    // else set heater power to max  
                                 // for fast heating to set temperature
    }

  }

  else {                        // else (if air temp equal or higher then set )   
       Dimmer1 = airDimmerOff;  // disable air heater - 
       }
}
else
{
  Dimmer1 = airDimmerOff;      // write OFF value 

  if ( airCooldownState == 0 && averageAirTemp > minAirTempValue+8 &&  airTempReal < airDisconnectedTrashold )  // if cooling not start and air temp -
  {                                                                                         // more then room temp: start cooldown
    airCooldownState = 1;                                                                   // (760 is sensor value on disconnected air gun)
  }
  
}
// ----------------------------------------------------------------------------------------------------


// --------------------------------  Air gun Fan control  -------------------------------------------
if ( airPowerState == 1 && airCooldownState == 0 )    // if cooling not start: normal work
{
  // get PWM value from % value
  fanSpeedPwmReal=map(fanSpeedSet, fanSpeedMin, fanSpeedMax, fanSpeedPwmMin, fanSpeedPwmMax);
  analogWrite(pinControlAirFan, fanSpeedPwmReal); 
}
else if ( airPowerState == 0 && airCooldownState == 1) // if air switch off and cooling started
{                                                      
  Cooldown();                                          // start cooling 
}
else
  {                                                    // if air is off and cooling not start
    fanSpeedPwmReal = 0;                               // just off fan
    analogWrite(pinControlAirFan, fanSpeedPwmReal);
  }  
// ------------------------------------------------------------------------------------------------



//---------------------------------- analog buttons ----------------------------------------------//

if( buttonLastChecked == 0 )                               // check if this is the first time read buttons
   buttonLastChecked = millis()+BUTTON_DELAY;              // force a check this cycle
 if( millis() - buttonLastChecked > BUTTON_DELAY ) {       // make sure a reasonable delay passed
   if( int buttNum = buttonPushed(controlButtonsPin) ) {
     
     #ifdef DEBUG // button debug
     Serial.print("Button "); Serial.print(buttNum); Serial.println(" was pushed."); 
     #endif

     // --------------------- iron control  -----------------------------
     if (buttNum == 8)   // Iron temp Down
       {
     if ( ironTempSet <= ironTempMin || (ironTempSet-5) <= ironTempMin )  
      {                                                                 
        ironTempSet = ironTempMin;     // if iron temp set is equal or less min temp - set temp to min                                    
      }

     else {
          ironTempSet=ironTempSet-5;  // if it`s not decreace temp by 5 degree
          }
      }
 
    else if (buttNum == 7) // Iron temp Up
      {
      if ( ironTempSet >= ironTempMax ) // if iron temp set is equal or more then max temp - set temp to max
        {
          ironTempSet = ironTempMax;
        }
      else {
          ironTempSet=ironTempSet+5;   // if it`s not increace temp by 5 degree
           }
      }
     // -------------------------------------------------------------------

     // ----------------------- air control ----------------------------------
     if (buttNum == 5) // Air temp Down
       {
     if ( airTempSet <= airTempMin || (airTempSet-5) <= airTempMin )
      {
        airTempSet = airTempMin;    // if air temp set is equal or less min temp - set temp to min
      }

     else {
          airTempSet=airTempSet-5;  // if it`s not decreace temp by 5 degree
          }
      }
 
    else if (buttNum == 6) // Air temp Up
      {
      if ( airTempSet >= airTempMax ) // if air temp set is equal or more then max temp - set temp to max
        {
          airTempSet = airTempMax;
        }
      else {
          airTempSet=airTempSet+5;   // if it`s not increace temp by 5 degree
           }
      }
    //-----------------------------------------------------------------------

    //---------------------- Fan speed control ------------------------------
     if (buttNum == 1) // Fan Speed Down
       {
     if ( fanSpeedSet <= fanSpeedMin || (fanSpeedSet-5) <= fanSpeedMin )
      {
        fanSpeedSet = fanSpeedMin;    // if fan speed set is equal or less min speed - set speed to min
      }

     else {
          fanSpeedSet=fanSpeedSet-5;  // if it`s not decreace speed by 5 percent
          }
      }
 
    else if (buttNum == 2) // Fan Speed Up
      {
      if ( fanSpeedSet >= fanSpeedMax ) // if fan speed set is equal or more then max speed - set speed to max
        {
          fanSpeedSet = fanSpeedMax;
        }
      else {
          fanSpeedSet=fanSpeedSet+5;  // if it`s not increace speed by 5 percent
           }
      }
     //--------------------------------------------------------------------
   
   }
   buttonLastChecked = millis(); // reset the lastChecked value
 }
//-------------------------------------------------------------------------------//

// end loop
}




//------------------------------------ Buttons analog values check -------------------------------------
int buttonPushed(int pinNum) {
 int val = 0;                  // init variable to store the read value
   val = analogRead(pinNum);   // read the input pin
   
   #ifdef DEBUG_ON //some debug
     Serial.println(val);
   #endif
   
   // check values (meshured in debug mode)
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
   else if( val >= (612-BUTTON_ERROR_WINDOW) and val <= (625+BUTTON_ERROR_WINDOW) )  { // 612-625
     #ifdef DEBUG_ON
     Serial.println("switch 7 pressed/triggered");    
     #endif
     return 7;
   }
   else if( val >= (653-BUTTON_ERROR_WINDOW) and val <= (660+BUTTON_ERROR_WINDOW) )  { // 653-660
     #ifdef DEBUG_ON
     Serial.println("switch 8 pressed/triggered");    
     #endif
     return 8;
   }
   else
     return 0;  // no button found to have been pushed
}
// --------------------------------------------------------------------------------------------------


//----------------  smooth iron -----------------
void smoothIron()
{
  // subtract the last reading:
  totalIron = totalIron - readingsIron[readIndexIron];
  // read from the sensor:
  readingsIron[readIndexIron] = ironTempRealC; // bouncing value !
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
  averageIronTemp = totalIron / numReadingsIron;
  averageIronTempPretty = averageIronTemp+3; // to get more pretty value (230 when avarage 227) just for perfect 
}
//----------------------------------------------  

//----------------  smooth Air -----------------
void smoothAir()
{
  // subtract the last reading:
  totalAir = totalAir - readingsAir[readIndexAir];
  // read from the sensor:
  readingsAir[readIndexAir] = airTempRealC; // bouncing value !
  // add the reading to the total:
  totalAir = totalAir + readingsAir[readIndexAir];
  // advance to the next position in the array:
  readIndexAir = readIndexAir + 1;

  // if we're at the end of the array...
  if (readIndexAir >= numReadingsAir) {
    // ...wrap around to the beginning:
    readIndexAir = 0;
  }
  // calculate the average:
  averageAirTemp = totalAir / numReadingsAir;
  averageAirTempPretty = averageAirTemp+2; // to get more pretty value (300 when avarage 298) just for perfect 
}
//----------------------------------------------  


// ---------------------------------- LCD control ------------------------------------------------
void show()
{
  static unsigned long last_lcd_refresh_time = 0; // init value to store last refresh time
 unsigned long lcd_refresh_time = millis();       // write time
 // If interrupts come faster than 200ms, assume it's a bounce and ignore
 if (lcd_refresh_time - last_lcd_refresh_time > lcdRefreshTime)
 {
 
 if ( ironDisconnected == 1) // check if iron disconnected
 {
  lcd.setCursor(0, 0);
  lcd.print("I:");
  lcd.setCursor(2, 0);
  lcd.print("Disconnected");
 }
 else // iron connected
 {
  if (ironPowerState == 1)
  {
   // iron temp when iron is ON
   lcd.setCursor(0, 0);
   if (averageIronTempPretty <= 99) // if temp is two digit value
   {
    lcd.print("I: ");
    lcd.setCursor(3, 0);
   }
   else
   {
    lcd.print("I:");
    lcd.setCursor(2, 0);
   }
   lcd.print(averageIronTempPretty); // print iron current temp
   lcd.setCursor(5, 0);
   lcd.write(byte(0));               // custom symbol
   lcd.setCursor(6, 0);
   lcd.print("> ");
   lcd.setCursor(8, 0);
   lcd.print(ironTempSet);           // print iron set temp
   lcd.setCursor(11, 0);
   lcd.write(byte(0));               // custom symbol
  }
  else if ( ironPowerState == 0 && (averageIronTemp - minIronTempValue) > 10)  // iron is off and temp more than
  {                                                                            // min temp, iron is cooling
   lcd.setCursor(0, 0);
   if (averageIronTemp <= 99) // if temp is two digit value
   {
    lcd.print("I: ");
    lcd.setCursor(3, 0);
   }
   else
   {
    lcd.print("I:");
    lcd.setCursor(2, 0);
   }
   lcd.print(averageIronTempPretty); // print iron current temp
   lcd.setCursor(5, 0);
   lcd.write(byte(0));               // custom symbol
   lcd.setCursor(6, 0);
   lcd.print("> ");
   lcd.setCursor(8, 0);
   lcd.print("COOL");                // print "COOL" insead of set temp
  }
  else                               // if iron temp is room temp - is off
  {
    lcd.setCursor(0, 0);
    lcd.print("I:");
    lcd.setCursor(2, 0);
    lcd.print("OFF");               // print "OFF" insead of current temp
    lcd.setCursor(5, 0);
    lcd.write(" ");
    lcd.setCursor(6, 0);
    lcd.print("> ");
    lcd.setCursor(8, 0);
    lcd.print(ironTempSet);         // print iron set temp
    lcd.setCursor(11, 0);
    lcd.write(byte(0));
  }

  lcd.setCursor(12, 0);            // hold space for "Disconnected"
  lcd.print("    ");
 }

    
// HeatGun temp
if ( airDisconnected == 1) // check if air disconnected
 {
  lcd.setCursor(0, 1);
  lcd.print("A:");
  lcd.setCursor(2, 1);
  lcd.print("Disconnected  ");
 }
 else // if air connected
 {

 if (airPowerState == 1 ) // check if air is on
  {
   // print air temp when air is ON
   lcd.setCursor(0, 1);
   if (averageAirTempPretty <= 99) // if temp is two digit value
   {
    lcd.print("A: ");
    lcd.setCursor(3, 1);
   }
   else
   {
    lcd.print("A:");
    lcd.setCursor(2, 1);
   }
   lcd.print(averageAirTempPretty);  // print air temp
   lcd.setCursor(5, 1);
   lcd.write(byte(0));               // custom symbol
   lcd.setCursor(6, 1);
   lcd.print("> ");
   lcd.setCursor(8, 1);
   lcd.print(airTempSet);            // print set temp
   lcd.setCursor(11, 1);
   lcd.write(byte(0));               // custom symbol
  }
 else if ( airPowerState == 0 && airCooldownState == 1)  // if cooling start
 {
  lcd.setCursor(0, 1);
  if (averageAirTempPretty <= 99) // if temp is two digit value
   {
    lcd.print("A: ");
    lcd.setCursor(3, 1);
   }
   else
   {
    lcd.print("A:");
    lcd.setCursor(2, 1);
   }
   lcd.print(averageAirTempPretty);
   lcd.setCursor(5, 1);
   lcd.write(byte(0));
   lcd.setCursor(6, 1);
   lcd.print("> ");
   lcd.setCursor(8, 1);
   lcd.print("COOL");
 }
 else
  {
    lcd.setCursor(0, 1);
    lcd.print("A:");
    lcd.setCursor(2, 1);
    lcd.print("OFF");
    lcd.setCursor(5, 1);
    lcd.write(" ");
    lcd.setCursor(6, 1);
    lcd.print("> ");
    lcd.setCursor(8, 1);
    lcd.print(airTempSet);
    lcd.setCursor(11, 1);
    lcd.write(byte(0));
  }  


// HeatGun fan speed
 lcd.setCursor(12, 1);
 lcd.write(byte(1));
 lcd.setCursor(13, 1);
   if ( fanSpeedSet == 100 )
   {
    lcd.print(fanSpeedSet);  // if fan speed is 100 print only digits
   }
   else 
   {
    lcd.print(fanSpeedSet);  // if fan speed 99 or less print "%" after value
    lcd.setCursor(15, 1);
    lcd.print("%");
   }
 }



 last_lcd_refresh_time = lcd_refresh_time; // write last lcd refresh time 
}
}
// ---------------------------------------------------------------------------------------------


// -----------------------------------  Read iron temp -----------------------------------------
void GetIronTemp()                        // get iron temp in celsius
{
 ironTempReal = analogRead(pinTempIron);  // read current temp (analog value) 
 // scale heater temperature to sensor values
 ironTempRealC = map(ironTempReal, minIronAnalogValue, maxIronAnalogValue, minIronTempValue, maxIronTempValue);
 ironTempRealC = constrain(ironTempRealC, minIronTempValue, maxIronTempValue); // limit iron temp to real values
}
// ----------------------------------------------------------------------------------------------

// ---------------------------------- Read air temp ---------------------------------------------
void GetAirTemp()                      // get air temp in celsius
{
 airTempReal = analogRead(pinTempAir); // read current temp (analog value)
 // scale heater temperature to sensor values
 airTempRealC = map(airTempReal, minAirAnalogValue, maxAirAnalogValue, minAirTempValue, maxAirTempValue);
 airTempRealC = constrain(airTempRealC, minAirTempValue, maxAirTempValue); // limit air temp  to real values
}
// -----------------------------------------------------------------------------------------------

// ---------------------------------  Disconnected allert ----------------------------------------
void disconnectAlert()            // detect disconnected alert
{
  int alertIron = 0;              // init var to store iron alert status
  int alertAir = 0;               // init var to store air alert status
  
  if ( airTempReal >= airDisconnectedTrashold && airPowerState == 1 ) // check if air analog value is 760 (meshured on disconnected)
  {                                               // and iron power switch is ON
     airDisconnected = 1; // chandge status
     alertAir = 1;        // signal is needed
  }
  else if ( airTempReal >= airDisconnectedTrashold && airPowerState == 0 ) // if air disconnected on off and analog value is 760 (meshured on disconnected)
  {
    airDisconnected = 1; // chandge status
    alertAir = 0;        // signal is NOT needed
  }
  else if ( airTempReal < airDisconnectedTrashold ) // if air connected
  {
    airDisconnected = 0; // chandge status
    alertAir = 0;        // signal is NOT needed
  }
  if ( ironTempReal >= airDisconnectedTrashold && ironPowerState == 1 ) //if iron disconnected on work
  {
     ironDisconnected = 1;  // chandge status
     alertIron = 1;         // signal is needed
  }
  else if ( ironTempReal >= airDisconnectedTrashold && ironPowerState == 0 ) //if iron disconnected on off
  {
    ironDisconnected = 1;  // chandge status
    alertIron = 0;         // signal is NOT needed
  }
  else if ( ironTempReal < airDisconnectedTrashold ) // if iron connected
  {
    ironDisconnected = 0;  // chandge status
    alertIron = 0;         // signal is NOT needed
  }

  int soundAlert = alertIron + alertAir; // plus alert values 
  alertSound(soundAlert);                // call sound func
}
// -------------------------------------------------------------------------------------------

// --------------------------  Sound function ------------------------------------------------
void alertSound(int val)
{
  if ( val >= 1)                                // enable beeper if any of alert is active 
  {
   unsigned long currentMillisBeep = millis();  // get current time

   if (currentMillisBeep - previousMillisBeep >= intervalBeep) 
     {
       // save the last time you Beep
       previousMillisBeep = currentMillisBeep;

       // if the Beep is off turn it on and vice-versa:
       if (beepState == 0) 
        {
         beepState = 1;
         tone(buzzerPin, 900, 1000);
        } 
        else 
        {
        beepState = 0;
        noTone(buzzerPin);
        }
     }
   
  }
  
  else if (val = 0) // disable beep none of alert is inactive
   {
     beepState = 0;
     noTone(buzzerPin);
   }
}
// ---------------------------------------------------------------------------------------

// -------------------------------  Coooling air -----------------------------------------
void Cooldown()
{
  unsigned long currentMillisCooldown = millis(); // get current time

   if (currentMillisCooldown - previousMillisCooldown <= cooldownTime) 
   {                                              // need more cooling
      fanSpeedPwmReal = fanSpeedPwmMax;           // run fan on max speed to cooling
      analogWrite(pinControlAirFan, fanSpeedPwmReal); 
   } 
   else                                           // cooling done
   {
                                                  // save the last time you Cooldown
    previousMillisCooldown = currentMillisCooldown;
                                                  // disable cooling
    airCooldownState = 0;                         // cooldown stoped, air temp eq room temp
   }        
}
// --------------------------------------------------------------------------------------


