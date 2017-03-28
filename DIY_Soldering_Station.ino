/*
 * Arduino soldering iron test 
 * control temperature with variable resistor
 * get amplified sidnal to serial
 */

int pot = A1; // pot for testing
int pinPwmIron = 11; // pwm to mosfet
int tempIron = A0; // input from termal sensor in iron
int pwmControl = 0; //var to save pwm value

// control buttons
int buttonUp = 13;
int buttonDown = 12;


// temp control 
int tempSet = 230; //default set temp
int tempMin = 200; //minimum temp
int tempMax = 350; //max temp
int tempReal = 250; //val termal sensor var
int tempPwmMin = 40; //minimal value PWM
int tempPwmMax = 180; //maximum value PWM
int tempPwmReal = 0; //current PWM value

int tempError = -50; // difference temp (set to real)

int tempDiff = 0; //variable to diff temp (set to current)


int increment = 000; //start value of sensor


//debug buttons
int buttonValue = 5;

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

//---------- analog smoothing ---
const int numReadings = 100;

int readings[numReadings];      // the readings from the analog input
int readIndex = 0;              // the index of the current reading
int total = 0;                  // the running total
int average = 0;                // the average
//------------------------------------------




void setup() {
  Serial.begin(115200);
  pinMode(pinPwmIron, OUTPUT);
  pinMode(buttonUp, INPUT);
  pinMode(buttonDown, INPUT);

  analogWrite(pinPwmIron, tempPwmReal); //Вывод  шим в нагрузку паяльника 
                                    //(выводим 0 - старт с выключеным паяльником- 
                                    // пока не опредилим состояние температуры)

  pinMode(latchPin, OUTPUT);
  pinMode(clockPin, OUTPUT);
  pinMode(dataPin, OUTPUT);
  pinMode(oneDigitPin, OUTPUT);
  pinMode(twoDigitPin, OUTPUT);
  pinMode(threeDigitPin, OUTPUT);
  digitalWrite(latchPin, HIGH);

  // initialize all the readings to 0:
  for (int thisReading = 0; thisReading < numReadings; thisReading++) {
    readings[thisReading] = 0;
  }

}

void loop() {

  
int sensorVariable = analogRead(tempIron); //get iron sensor data
int potVariable = analogRead(A1);    //get pot data

pwmControl=map(potVariable,0,1023,0,255); // map pot 0-1023 as 0-255


int buttonUpState=digitalRead(buttonUp);    //get buttons state
int buttonDownState=digitalRead(buttonDown);


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
Serial.print(tempSet);
//Serial.print(" | ");
//Serial.print("Button Down: ");
//Serial.print(buttonDownState);
//Serial.print(" | ");

Serial.println("");
//delay(100);


// set pwm value
//analogWrite(pinPwmIron, pwmControl);

// ------------- debug end --------------------//

//----------------  smooth -----------------
  // subtract the last reading:
  total = total - readings[readIndex];
  // read from the sensor:
  readings[readIndex] = increment;
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

  show(average);

//------------------------------------------------------------------  

//show(increment);   // Вывести значение переменной на экран(LED)

if (tempReal < tempSet ){   // Если температура паяльника ниже установленной температуры то:
  if ((tempSet - tempReal) < 16 & (tempSet - tempReal) > 6 )       // Проверяем разницу между 
                                               // установленной температурой и текущей паяльника,
                                               // Если разница меньше 10 градусов то 
      {
        tempPwmReal = 99; // Понижаем мощность нагрева (шим 0-255  мы делаем 99)  - 
                          // таким образом мы убираем инерцию перегрева
      }

  else if ((tempSet - tempReal) < 4 )
    {
      tempPwmReal = 45; 
    }

  else 
    {
      tempPwmReal = 230; // Иначе Подымаем мощность нагрева(шим 0-255  мы делаем 230) на максимум 
                         // для быстрого нагрева до нужной температуры
    }

analogWrite(pinPwmIron, tempPwmReal); // Вывод в шим порт (на транзистор) значение мощности

}

else { // Иначе (если температура паяльника равняется или выше установленной) 
       tempPwmReal = 0;  // Выключаем мощность нагрева (шим 0-255  мы делаем 0)  - 
                         // таким образом мы отключаем паяльник
       analogWrite(pinPwmIron, tempPwmReal); // Вывод в шим порт (на транзистор) значение 
     }

tempReal = analogRead(tempIron); // считываем текущую температуру

tempReal=map(tempReal,0,764,25,400);       // нужно вычислить
                             // 0 sens is 25 on iron - 764 is 295 on iron
                             // 325 - get 228-232 on iron when tempSet = 230
increment=tempReal;



//---------------- buttons ---------------//

if (digitalRead(buttonUp) == 1) // Если нажата вниз кнопка то понизить температуру на 5
  {
   tempIronControl(1);
  }

if (digitalRead(buttonDown) == 1) // Если нажата вниз кнопка то понизить температуру на 5
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
  cyclesToLed = 100; // show value for 100 cycles
  if (value == 0) // temp Down
    {
    if ( tempSet <= tempMin || (tempSet-5) <= tempMin )
    {
      tempSet = tempMin;
      increment = tempSet;
    }

    else {
          tempSet=tempSet-5;
          increment = tempSet;
          show(increment);   // Вывести значение переменной на экран(LED)
          //delay(100);
         }
    }
 
  else if (value == 1) // temp Up
    {
    if ( tempSet >= tempMax )
      {
        tempSet = tempMax;
      }
    else {
         tempSet=tempSet+5;
         }
    increment = tempSet;
    //cyclesTolLed = 100;
    show(increment);   // Вывести значение переменной на экран(LED)
    //cyclesTolLed = 0;
    //delay(100);
    }
    cyclesToLed = 0;
 }
 last_interrupt_time = interrupt_time;
}
//----------------------------------------------------------------


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

