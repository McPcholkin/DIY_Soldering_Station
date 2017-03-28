/*
 * Arduino soldering station project
 * get amplified sidnal to serial
 */

// iron control
int pinPwmIron = 10; // pwm to mosfet
int pinTempIron = A0; // input from termal sensor in iron
int pwmControl = 0; //var to save pwm value

// control buttons
int ironButtonUp = 7;
int ironButtonDown = 8;


// iron temp control 
int ironTempSet = 230; //default set temp
int ironTempMin = 200; //minimum temp
int ironTempMax = 350; //max temp
int ironTempReal = 230; //val termal sensor var
int ironTempPwmMin = 40; //minimal value PWM
int ironTempPwmMax = 180; //maximum value PWM
int ironTempPwmReal = 0; //current PWM value

// int tempError = -50; // difference temp (set to real)  -- not used?

// int tempDiff = 0; //variable to diff temp (set to current) -- not used?

int increment = 000; //start value of sensor



/*
//----------  7 segment 

//Пин подключен к ST_CP входу 74HC595
int latchPin = 8;
//Пин подключен к SH_CP входу 74HC595
int clockPin = 9;
//Пин подключен к DS входу 74HC595
int dataPin = 10; 

// Пины катодов дисплея
int oneDigitPin = 5;
int twoDigitPin = 6;
int threeDigitPin = 7;

// бинарные значения цыфр
char numbers[10] = {
  0b11111100, // 0
  0b01100000, // 1
  0b11011010, // 2
  0b11110010, // 3
  0b01100110, // 4
  0b10110110, // 5
  0b10111110, // 6
  0b11100000, // 7 
  0b11111110, // 8
  0b11110110  // 9
};

// Задержка смены сегмента в МС
int flicerDelay = 3;

// delay to show value to display
int cyclesToLed = 0; //

//--------------------------

*/



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


// ---------------------------------------------

//---------- analog smoothing ---
const int numReadings = 100;

int readings[numReadings];      // the readings from the analog input
int readIndex = 0;              // the index of the current reading
int total = 0;                  // the running total
int average = 0;                // the average
//------------------------------------------

// debug
int sensorVariable = 0; // iron sensor data
//------------------

void setup() {
  Serial.begin(115200);
  pinMode(pinPwmIron, OUTPUT);
  pinMode(ironButtonUp, INPUT);
  pinMode(ironButtonDown, INPUT);

  analogWrite(pinPwmIron, ironTempPwmReal); //Вывод  шим в нагрузку паяльника 
                                    //(выводим 0 - старт с выключеным паяльником- 
                                    // пока не опредилим состояние температуры)

//  pinMode(latchPin, OUTPUT);
 // pinMode(clockPin, OUTPUT);
 // pinMode(dataPin, OUTPUT);
  //pinMode(oneDigitPin, OUTPUT);
 // pinMode(twoDigitPin, OUTPUT);
 // pinMode(threeDigitPin, OUTPUT);
  //digitalWrite(latchPin, HIGH);


  // --- LCD ---
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
  
  // Print a message to the LCD.
  lcd.setCursor(0, 0);
  lcd.print("Soldering");
  lcd.setCursor(0, 1);
  lcd.print("Station FW:0.1");
  lcd.setCursor(15, 0);
  lcd.write(byte(3));
  lcd.setCursor(15, 1);
  lcd.write(byte(4));
  delay(1000);
  // clear display from welcome message
  lcd.clear();

  // initialize all the readings to 0:
  for (int thisReading = 0; thisReading < numReadings; thisReading++) {
    readings[thisReading] = 0;
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
Serial.print(increment);
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

smoothIron();

show();   // Вывести значение переменной на экран(LED)
//lcd.display();
//delay(500);
//lcd.setCursor(0, 0);
//lcd.print(millis() / 100);

//delay(2000);


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
increment=ironTempReal;



//---------------- buttons ---------------//

if (digitalRead(ironButtonUp) == 1) // Если нажата вниз кнопка то понизить температуру на 5
  {
   tempIronControl(1);
  }

if (digitalRead(ironButtonDown) == 1) // Если нажата вниз кнопка то понизить температуру на 5
  {
   tempIronControl(0);
  }




}



//----------------- iron temp buttons control ----
void tempIronControl(int value) // debouce control iron temp
{
 static unsigned long last_interrupt_time = 0;
 unsigned long interrupt_time = millis();
 // If interrupts come faster than 200ms, assume it's a bounce and ignore
 if (interrupt_time - last_interrupt_time > 50)
 {
  //cyclesToLed = 100; // show value for 100 cycles
  if (value == 0) // temp Down
    {
    if ( ironTempSet <= ironTempMin || (ironTempSet-5) <= ironTempMin )
    {
      ironTempSet = ironTempMin;
      increment = ironTempSet;
    }

    else {
          ironTempSet=ironTempSet-5;
          increment = ironTempSet;
          //show(increment);   // Вывести значение переменной на экран(LED)
          //delay(100);
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
    increment = ironTempSet;
    //cyclesTolLed = 100;
    //show(increment);   // Вывести значение переменной на экран(LED)
    //cyclesTolLed = 0;
    //delay(100);
    }
    //cyclesToLed = 0;
 }
 last_interrupt_time = interrupt_time;
}
//----------------------------------------------------------------


//----------------  smooth iron -----------------

void smoothIron()
{
  // subtract the last reading:
  total = total - readings[readIndex];
  // read from the sensor:
  readings[readIndex] = increment; // bouncing value !
  // add the reading to the total:
  total = total + readings[readIndex];
  // advance to the next position in the array:
  readIndex = readIndex + 1;

  // if we're at the end of the array...
  if (readIndex >= numReadings) {
    // ...wrap around to the beginning:
    readIndex = 0;
  }

  // calculate the average:
  average = total / numReadings;
  // send it to the computer as ASCII digits
  //Serial.println(average);
  //delay(1);        // delay in between reads for stability

  //show(average);
}

//------------------------------------------------------------------  

// ----------  LCD ------------------------
void show()
{
  // iron temp 
 lcd.setCursor(0, 0);
 lcd.print("I:");
 lcd.setCursor(2, 0);
 lcd.print(average);
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
 lcd.print(sensorVariable);
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
 lcd.print(ironTempSet);
 lcd.setCursor(15, 1);
 lcd.print("%");


 //lcd.noDisplay();
// delay(1000);
}


/*
//----------------- LED control -----------------

void offAllLedSegments()
{
  digitalWrite(oneDigitPin, HIGH); // off all segments
  digitalWrite(twoDigitPin, HIGH); // to prevent ghosting
  digitalWrite(threeDigitPin, HIGH); 
}

void onLedSegment(int value)
{
  
  if (value == 1) // on first segment
  {
    digitalWrite(oneDigitPin, LOW); 
    digitalWrite(twoDigitPin, HIGH); 
    digitalWrite(threeDigitPin, HIGH);
  }
  else if (value == 2) // on second segment
  {
    digitalWrite(oneDigitPin, HIGH); 
    digitalWrite(twoDigitPin, LOW); 
    digitalWrite(threeDigitPin, HIGH);
  }
  else if (value == 3) // on third segment
  {
   digitalWrite(oneDigitPin, HIGH); 
   digitalWrite(twoDigitPin, HIGH); 
   digitalWrite(threeDigitPin, LOW); 
  }

  delay(flicerDelay);
  
  
}

void show(int value)
  {
    int first=value/100;       //split parameter to 3 digit
    int second=value%100/10;
    int third=value%10;

    
     
    for (int i=0; i<=cyclesToLed; i++)
    {
    offAllLedSegments();

    digitalWrite(latchPin, LOW);  // put bits for first digit
    shiftOut(dataPin, clockPin, LSBFIRST, numbers[first]);
    digitalWrite(latchPin, HIGH);

    onLedSegment(1);

    offAllLedSegments();
  
    digitalWrite(latchPin, LOW);  // put bits for second digit
    shiftOut(dataPin, clockPin, LSBFIRST, numbers[second]);
    digitalWrite(latchPin, HIGH);

    onLedSegment(2); 

    offAllLedSegments();

    digitalWrite(latchPin, LOW);  // put bits for third digit
    shiftOut(dataPin, clockPin, LSBFIRST, numbers[third]);
    digitalWrite(latchPin, HIGH);

    onLedSegment(3);

    offAllLedSegments();
    } 
  }
  
*/
