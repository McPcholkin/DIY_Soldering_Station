/*
 * Arduino soldering iron test 
 * control temperature with variable resistor
 * get amplified sidnal to serial
 */

int pot = A1; // pot for testing
int pinPwmIron = 10; // pwm to mosfet
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


int increment = 0; //start value of sensor


//debug buttons
int buttonValue = 5;

void setup() {
  Serial.begin(115200);
  pinMode(pinPwmIron, OUTPUT);
  pinMode(buttonUp, INPUT);
  pinMode(buttonDown, INPUT);

  analogWrite(pinPwmIron, tempPwmReal); //Вывод  шим в нагрузку паяльника 
                                    //(выводим 0 - старт с выключеным паяльником- 
                                    // пока не опредилим состояние температуры)

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

void tempIronControl(int value) // deboucet control
{
 static unsigned long last_interrupt_time = 0;
 unsigned long interrupt_time = millis();
 // If interrupts come faster than 200ms, assume it's a bounce and ignore
 if (interrupt_time - last_interrupt_time > 100)
 {
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
          //show(increment);   // Вывести значение переменной на экран(LED)
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
    //show(increment);   // Вывести значение переменной на экран(LED)
    }
 }
 last_interrupt_time = interrupt_time;
}

