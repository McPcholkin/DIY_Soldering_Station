/*
 * Arduino soldering station project
 * McPcholkin https://github.com/McPcholkin/DIY_Soldering_Station
 */

// iron control
int pinPwmIron = 9; // pwm to mosfet
int pinTempIron = A0; // input from termal sensor in iron
//int pwmControlIron = 0; //var to save pwm value -- not used?

// iron control
int pinPwmAir = 6; // pwm to mosfet
int pinPwmAirFan = 10;
int pinTempAir = A1; // input from termal sensor in iron
//int pwmControl = 0; //var to save pwm value -- not used?

// control buttons
int ironButtonUp = 7;
int ironButtonDown = 8;
//int airButtonUp = 5;
//int airButtonDown = 6;


// iron temp control 
int ironTempSet = 200; //default set temp
int ironTempMin = 200; //minimum temp
int ironTempMax = 280; //max temp
int ironTempReal = 230; //val termal sensor var
int ironTempPwmMin = 40; //minimal value PWM
int ironTempPwmMax = 180; //maximum value PWM
int ironTempPwmReal = 0; //current PWM value

// int tempError = -50; // difference temp (set to real)  -- not used?

// int tempDiff = 0; //variable to diff temp (set to current) -- not used?

int incrementIron = 000; //start value of iron sensor
int incrementAir = 000; //start value of air sensor


// -------- LiquidCrystal 16x2 LCD display. --------
/* The circuit:
 * LCD RS pin to digital pin 12
 * LCD Enable pin to digital pin 11
 * LCD D4 pin to digital pin 5
 * LCD D5 pin to digital pin 4
 * LCD D6 pin to digital pin 3
 * LCD D7 pin to digital pin 2
 * LCD R/W pin to ground
 * 10K resistor:
 * ends to +5V and ground
 * wiper to LCD VO pin (pin 3)
 */

// include the library code:
#include <LiquidCrystal.h>

// initialize the library with the numbers of the interface pins
LiquidCrystal lcd(12, 11, 5, 4, 3, 2);

// make some custom characters 5x8 pix:

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

int lcdRefreshTime = 250;

// ---------------------------------------------


//---------- analog smoothing Iron -----------------
const int numReadingsIron = 100;

int readingsIron[numReadingsIron];      // the readings from the analog input
int readIndexIron = 0;              // the index of the current reading
int totalIron = 0;                  // the running total
int averageIron = 0;                // the average
//------------------------------------------



// --------------- debug --------------------
int sensorVariable = 0; // iron sensor data
//------------------------------------------

void setup() {
  Serial.begin(115200);
  pinMode(pinPwmIron, OUTPUT);
  pinMode(ironButtonUp, INPUT);
  pinMode(ironButtonDown, INPUT);

  analogWrite(pinPwmIron, ironTempPwmReal); //Вывод  шим в нагрузку паяльника 
                                    //(выводим 0 - старт с выключеным паяльником- 
                                    // пока не опредилим состояние температуры)

  // ---- LCD ----
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
  delay(2000);
  // clear display from welcome message
  lcd.clear();
  //---- LCD ----


  // initialize all the readings to 0:
  for (int thisReadingIron = 0; thisReadingIron < numReadingsIron; thisReadingIron++) {
    readingsIron[thisReadingIron] = 0;
  }

}

void loop() {

// ------------- debug  --------------------//
  
sensorVariable = analogRead(pinTempIron); //get iron sensor data
//int potVariable = analogRead(A1);    //get pot data
//pwmControl=map(potVariable,0,1023,0,255); // map pot 0-1023 as 0-255

int buttonUpState=digitalRead(ironButtonUp);    //get buttons state
int buttonDownState=digitalRead(ironButtonDown);

// Debug output
//Serial.print("PWM Pot: ");
//Serial.print(pwmControl);
Serial.print("Increment: ");
Serial.print(incrementIron);
Serial.print(" | ");
Serial.print("Sensor: ");
Serial.print(sensorVariable);
Serial.print(" | ");
Serial.print("Set Value: ");
Serial.print(ironTempSet);
//Serial.print(" | ");
//Serial.print("Button Down: ");
//Serial.print(buttonDownState);
//Serial.print(" | ");

Serial.println("");
//delay(100);
// set pwm value
//analogWrite(pinPwmIron, pwmControl);

// ------------- debug end --------------------//

// smooth iron meshure values
smoothIron();

// refresh LCD screen
show();   


// ------------------------------  Iron temp control  -------------------------------------------------------
if (ironTempReal < ironTempSet ){   // Если температура паяльника ниже установленной температуры то:
  if ((ironTempSet - ironTempReal) < 16 & (ironTempSet - ironTempReal) > 6 )       // Проверяем разницу между 
                                               // установленной температурой и текущей паяльника,
                                               // Если разница меньше 10 градусов то 
      {
        ironTempPwmReal = 99; // Понижаем мощность нагрева (шим 0-255  мы делаем 99)  - 
                          // таким образом мы убираем инерцию перегрева
      }

  else if ((ironTempSet - ironTempReal) < 4 )
    {
      ironTempPwmReal = 45; 
    }

  else 
    {
      ironTempPwmReal = 230; // Иначе Подымаем мощность нагрева(шим 0-255  мы делаем 230) на максимум 
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

ironTempReal=map(ironTempReal,0,764,25,400);       // нужно вычислить
                             // 0 sens is 25 on iron - 764 is 295 on iron
                             // 400 - get 228-232 on iron when ironTempSet = 230
incrementIron=ironTempReal;
//----------------------------------------------------------------------------------------------------


//---------------- buttons ---------------//
/*
if (digitalRead(ironButtonUp) == 1) // Если нажата вниз кнопка то понизить температуру на 5
  {
   tempIronControl(1);
  }

if (digitalRead(ironButtonDown) == 1) // Если нажата вниз кнопка то понизить температуру на 5
  {
   tempIronControl(0);
  }
// End loop
*/
}



//----------------- iron temp buttons control -----------------------
void tempIronControl(int value) // debouce control iron temp
{
 static unsigned long last_interrupt_time = 0;
 unsigned long interrupt_time = millis();
 // If interrupts come faster than 200ms, assume it's a bounce and ignore
 if (interrupt_time - last_interrupt_time > 50)
 {
  if (value == 0) // temp Down
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
 
  else if (value == 1) // temp Up
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
 last_interrupt_time = interrupt_time;
}
}
//----------------------------------------------------------------



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

// ----------  LCD ------------------------
void show()
{
  static unsigned long last_lcd_refresh_time = 0;
 unsigned long lcd_refresh_time = millis();
 // If interrupts come faster than 200ms, assume it's a bounce and ignore
 if (lcd_refresh_time - last_lcd_refresh_time > lcdRefreshTime)
 {
  // iron temp 
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
 lcd.print(ironTempSet);
 lcd.setCursor(11, 1);
 lcd.write(byte(0));
 
 // HeatGun fan speed
 lcd.setCursor(12, 1);
 lcd.write(byte(1));
 //lcd.setCursor(12, 1);
 //lcd.print("S:");
 lcd.setCursor(13, 1);
 lcd.print(50);
 lcd.setCursor(15, 1);
 lcd.print("%");



 last_lcd_refresh_time = lcd_refresh_time;
}
 //delay(10);
}


