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
const int  ironTempMax = 310;    //max temp
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
const int airTempMin = 200;    //minimum temp
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
const long cooldownTime = 60000;           // Cooldown time  (milliseconds) (60 SEC)
//--------------------------


// Setup some stuff
void setup() {
  //debug
  #ifdef SERIALDEBUG
    Serial.begin(115200);
  #endif
    
  pinMode(pinPwmIron, OUTPUT);
  pinMode(pinControlAirFan, OUTPUT);
  pinMode(ironPowerToggle, INPUT);
  pinMode(airPowerToggle, INPUT);

  analogWrite(pinPwmIron, ironTempPwmReal); //Вывод  шим в нагрузку паяльника 
                                    //(выводим 0 - старт с выключеным паяльником- 
                                    // пока не опредилим состояние температуры)

  analogWrite(pinControlAirFan, fanSpeedPwmReal); // stop fan at boot

  // --------  Dimmer for air -------
  D3_Out; //Настраиваем порты на выход (pin 3)
  D3_Low; //установить на выходах низкий уровень сигнала
  D2_In;  //настраиваем порт на вход для отслеживания прохождения сигнала через ноль

  //CHANGE – прерывание вызывается при любом изменении значения на входе; 
  //RISING – вызов прерывания при изменении уровня напряжения с низкого (Low) на высокий(HIGH) 
  //FALLING – вызов прерывания при изменении уровня напряжения с высокого (HIGH)
  //LOW - вызов прерывания при высоком уровне напряжения
  //HIGH - вызов прерывания при низком уровне напряжения
  attachInterrupt(0, detect_up, LOW); // настроить срабатывание прерывания interrupt0 на pin 2 на низкий
  StartTimer1(halfcycle, 40); //время для одного разряда ШИМ
  StopTimer1(); //остановить таймер
  // ---------------------------------

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
  lcd.print("Station FW:0.6");
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
  // initialize all the readings air temp to 0:
  for (int thisReadingAir = 0; thisReadingAir < numReadingsAir; thisReadingAir++) {
    readingsAir[thisReadingAir] = 0;
  }

}

//********************обработчики прерываний*******************************
void halfcycle()  //прерывания таймера
{ 
  tic++;  //счетчик  
  if(Dimmer1 < tic ) D3_High; //управляем выходом
}

void  detect_up()  // обработка внешнего прерывания. Сработает по переднему фронту
{  
 tic=0;             //обнулить счетчик
 ResumeTimer1();   //запустить таймер
 attachInterrupt(0, detect_down, HIGH);  //перепрограммировать прерывание на другой обработчик
}  

void  detect_down()  // обработка внешнего прерывания. Сработает по заднему фронту
{   
 StopTimer1(); //остановить таймер
 D3_Low; //логический ноль на выходы
 tic=0;       //обнулить счетчик
 attachInterrupt(0, detect_up, LOW); //перепрограммировать прерывание на другой обработчик
} 
//***********************************************************************



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


// ---------------------  Main code --------------

// smooth iron meshure values
smoothIron();

// smooth air meshure values
smoothAir();

// refresh LCD screen
show();   

// read iron temp 
GetIronTemp();

// alert when iron or air is diconnected
disconnectAlert();

// read air temp
GetAirTemp();



// ------------------------------------------------

// ------------------------------  Iron temp control  -------------------------------------------------------

ironPowerState = digitalRead(ironPowerToggle);
if ( ironPowerState == 1){ // if iron "ON" switch is enabled
  
  if (ironTempRealC < ironTempSet ){   // Если температура паяльника ниже установленной температуры то:
    if ((ironTempSet - ironTempRealC) < 16 & (ironTempSet - ironTempRealC) > 6 )  // Проверяем разницу между 
                                                         // установленной температурой и текущей паяльника,
                                                         // Если разница меньше 10 градусов то 
      {
        ironTempPwmReal = ironTempPwmHalf; // Понижаем мощность нагрева (шим 0-255  мы делаем 99)  - 
                                           // таким образом мы убираем инерцию перегрева
      }

  else if ((ironTempSet - ironTempRealC) < 4 ) // if difference less 4 degree use min temp
    {
      ironTempPwmReal = ironTempPwmMin; 
    }

  else 
    {
      ironTempPwmReal = ironTempPwmMax; // Иначе Подымаем мощность нагрева(шим 0-255  мы делаем 230) на максимум 
                                        // для быстрого нагрева до нужной температуры
    }
  }

  else { // Иначе (если температура паяльника равняется или выше установленной) 
       ironTempPwmReal = 0;  // Выключаем мощность нагрева (шим 0-255  мы делаем 0)  - 
                         // таким образом мы отключаем паяльник
       }
  analogWrite(pinPwmIron, ironTempPwmReal); // Вывод в шим порт (на транзистор) значение 
}
else 
{
  analogWrite(pinPwmIron, 0); // Disable iron heater if switch off
}

//----------------------------------------------------------------------------------------------------

// ------------------------------  Air temp control  -------------------------------------------------------

airPowerState = digitalRead(airPowerToggle);
if ( airPowerState == 1){ // if iron "ON" switch is enabled
 
  if (airTempRealC < airTempSet ){   // if temp of AirGun less set temp than:
    if ((airTempSet - airTempRealC) < 16 & (airTempSet - airTempRealC) > 6 )  // check difference between
                                                         // set air temp and current temp,
                                                         // If difference less 10 degree than 
      {
        Dimmer1 = airDimmerHalf; // set heat power to half (pwm 128-0  we set  64)  - 
                                         // таким образом мы убираем инерцию перегрева
      }

  else if ((airTempSet - airTempRealC) < 4 ) // if difference less 4 degree use min temp
    {
      Dimmer1 = airDimmerMin; 
    }

  else 
    {
      Dimmer1 = airDimmerMax; // Иначе Подымаем мощность нагрева(шим 0-255  мы делаем 230) на максимум 
                                        // для быстрого нагрева до нужной температуры
    }

  }

  else { // Иначе (если температура паяльника равняется или выше установленной) 
       Dimmer1 = airDimmerOff;  // Выключаем мощность нагрева (шим 0-255  мы делаем 0)  - 
                         // таким образом мы отключаем паяльник
       }
}
else
{
  Dimmer1 = airDimmerOff;

  if ( airCooldownState == 0 && averageAirTemp > minAirTempValue+8 &&  airTempReal < 760 )  // if cooling not start and air temp 
  {                                                                // more room temp 
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
else if ( airPowerState == 0 && airCooldownState == 1) // if air switch off
{                                                      // and cooling started
  Cooldown(); // start cooling 
}
else
  { // if air is off and cooling not start just off fan
    fanSpeedPwmReal = 0;
    analogWrite(pinControlAirFan, fanSpeedPwmReal);
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
     if (buttNum == 8) // Iron temp Down
       {
     if ( ironTempSet <= ironTempMin || (ironTempSet-5) <= ironTempMin )
      {
        ironTempSet = ironTempMin;
      }

     else {
          ironTempSet=ironTempSet-5;
          }
      }
 
    else if (buttNum == 7) // Iron temp Up
      {
      if ( ironTempSet >= ironTempMax )
        {
          ironTempSet = ironTempMax;
        }
      else {
          ironTempSet=ironTempSet+5;
           }
      }
     //------------------------------------------

     // --- air ----------------------------------
     if (buttNum == 5) // Air temp Down
       {
     if ( airTempSet <= airTempMin || (airTempSet-5) <= airTempMin )
      {
        airTempSet = airTempMin;
      }

     else {
          airTempSet=airTempSet-5;
          }
      }
 
    else if (buttNum == 6) // Air temp Up
      {
      if ( airTempSet >= airTempMax )
        {
          airTempSet = airTempMax;
        }
      else {
          airTempSet=airTempSet+5;
           }
      }
    //--------------------------------------------

    //-------------- Fan ------------------
     if (buttNum == 1) // Fan Speed Down
       {
     if ( fanSpeedSet <= fanSpeedMin || (fanSpeedSet-5) <= fanSpeedMin )
      {
        fanSpeedSet = fanSpeedMin;
      }

     else {
          fanSpeedSet=fanSpeedSet-5;
          }
      }
 
    else if (buttNum == 2) // Fan Speed Up
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
// --------------------------------------------------------------------------------------


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
//------------------------------------------------------------------  

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
//------------------------------------------------------------------  


// ----------------------------------  LCD ------------------------
void show()
{
  static unsigned long last_lcd_refresh_time = 0;
 unsigned long lcd_refresh_time = millis();
 // If interrupts come faster than 200ms, assume it's a bounce and ignore
 if (lcd_refresh_time - last_lcd_refresh_time > lcdRefreshTime)
 {
 
 if ( ironDisconnected == 1) // if iron disconnected
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
   lcd.print(averageIronTempPretty);
   lcd.setCursor(5, 0);
   lcd.write(byte(0));
   lcd.setCursor(6, 0);
   lcd.print("> ");
   lcd.setCursor(8, 0);
   lcd.print(ironTempSet);
   lcd.setCursor(11, 0);
   lcd.write(byte(0));
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
   lcd.print(averageIronTempPretty);
   lcd.setCursor(5, 0);
   lcd.write(byte(0));
   lcd.setCursor(6, 0);
   lcd.print("> ");
   lcd.setCursor(8, 0);
   lcd.print("COOL");
  }
  else
  {
    lcd.setCursor(0, 0);
    lcd.print("I:");
    lcd.setCursor(2, 0);
    lcd.print("OFF");
    lcd.setCursor(5, 0);
    lcd.write(" ");
    lcd.setCursor(6, 0);
    lcd.print("> ");
    lcd.setCursor(8, 0);
    lcd.print(ironTempSet);
    lcd.setCursor(11, 0);
    lcd.write(byte(0));
  }

  lcd.setCursor(12, 0);  // hold space for "Disconnected"
  lcd.print("    ");
 }
    
   
 // iron state
// lcd.setCursor(13, 0);
// lcd.print("SLP");

 // HeatGun temp
if ( airDisconnected == 1) // if air disconnected
 {
  lcd.setCursor(0, 1);
  lcd.print("A:");
  lcd.setCursor(2, 1);
  lcd.print("Disconnected  ");
 }
 else // air connected
 {

 if (airPowerState == 1 )
  {
   // air temp when air is ON
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
   lcd.print(airTempSet);
   lcd.setCursor(11, 1);
   lcd.write(byte(0));
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
    lcd.print(fanSpeedSet);
   }
   else 
   {
    lcd.print(fanSpeedSet);
    lcd.setCursor(15, 1);
    lcd.print("%");
   }
 }



 last_lcd_refresh_time = lcd_refresh_time;
}
}

void GetIronTemp()  // get iron temp in celsius
{
 ironTempReal = analogRead(pinTempIron); // считываем текущую температуру
 // scale heater temperature to sensor values
 ironTempRealC = map(ironTempReal, minIronAnalogValue, maxIronAnalogValue, minIronTempValue, maxIronTempValue);
 ironTempRealC = constrain(ironTempRealC, minIronTempValue, maxIronTempValue); // limit iron temp    
}

void GetAirTemp()  // get air temp in celsius
{
 airTempReal = analogRead(pinTempAir); // считываем текущую температуру
 // scale heater temperature to sensor values
 airTempRealC = map(airTempReal, minAirAnalogValue, maxAirAnalogValue, minAirTempValue, maxAirTempValue);
 airTempRealC = constrain(airTempRealC, minAirTempValue, maxAirTempValue); // limit air temp    

}


void disconnectAlert() // detect disconnected alert
{
  int alertIron = 0;
  int alertAir = 0;
  
  if ( airTempReal >= 760 && airPowerState == 1 ) //if air disconnected on work
  {
     airDisconnected = 1; // chandge status
     alertAir = 1;        // signal is needed
  }
  else if ( airTempReal >= 760 && airPowerState == 0 ) //if air disconnected on off
  {
    airDisconnected = 1; // chandge status
    alertAir = 0;        // signal is NOT needed
  }
  else if ( airTempReal < 760 ) // if air connected
  {
    airDisconnected = 0; // chandge status
    alertAir = 0;        // signal is NOT needed
  }
  if ( ironTempReal >= 760 && ironPowerState == 1 ) //if iron disconnected on work
  {
     ironDisconnected = 1;  // chandge status
     alertIron = 1;         // signal is needed
  }
  else if ( ironTempReal >= 760 && ironPowerState == 0 ) //if iron disconnected on off
  {
    ironDisconnected = 1;  // chandge status
    alertIron = 0;         // signal is NOT needed
  }
  else if ( ironTempReal < 760 ) // if iron connected
  {
    ironDisconnected = 0;  // chandge status
    alertIron = 0;         // signal is NOT needed
  }

  int soundAlert = alertIron + alertAir; 
  alertSound(soundAlert);
}

void alertSound(int val)
{
  if ( val >= 1) // enable beeper if any of alert is active
  {
   unsigned long currentMillisBeep = millis(); // get current time

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

void Cooldown()
{
  unsigned long currentMillisCooldown = millis(); // get current time

   if (currentMillisCooldown - previousMillisCooldown <= cooldownTime) 
   { // need more cooling
      fanSpeedPwmReal = fanSpeedPwmMax; // run fan on max speed to cooling
      analogWrite(pinControlAirFan, fanSpeedPwmReal); 
   } 
   else // cooling done
   {
    // save the last time you Cooldown
    previousMillisCooldown = currentMillisCooldown;
    // disable cooling
    airCooldownState = 0;        // cooldown stoped, air temp eq room temp
   }        
}

