//Dimmer by alex.marinenko

#include <CyberLib.h>  //attach lib Библиотека от Cyber-Place.ru
volatile uint8_t tic, Dimmer1 = 220; // 220=мин. 0=макс 
uint8_t data;
uint8_t dir = 1;

int fanPWMPin = 5;
//int AirControlPin = 3;
// zero detect pin 2 (timer 0)

int fanPWM = 0;

#define CALIB

#ifdef CALIB
// Buttons
const int BUTTON_ERROR_WINDOW = 5;  // +/- this value 
const int BUTTON_DELAY = 210;       // delay to debounce button
const int controlButtonsPin = A4;   // switch circuit input connected to analog pin 4
long buttonLastChecked = 0; // variable to limit the button getting checked every cycle


int airAnalog = 0;
const int pinTempAir = A1; // input from termal sensor

#endif

void setup() {
  #ifdef CALIB
  Serial.begin(115200);
  #endif
  
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

analogWrite(fanPWMPin, fanPWM);
   

#ifdef CALIB
airAnalog = analogRead(pinTempAir);

Serial.print("A1: ");
Serial.print(airAnalog);
Serial.print(" | ");
Serial.print("PWM: ");
Serial.print(fanPWM);
Serial.print(" | ");
Serial.print("DIM: ");
Serial.print(Dimmer1);
Serial.println();


//---------------- buttons ---------------//

if( buttonLastChecked == 0 ) // see if this is the first time checking the buttons
   buttonLastChecked = millis()+BUTTON_DELAY;  // force a check this cycle
 if( millis() - buttonLastChecked > BUTTON_DELAY ) { // make sure a reasonable delay passed
   if( int buttNum = buttonPushed(controlButtonsPin) ) {
     

     // ------ fan  -----------------------------
     if (buttNum == 1) // fan Down
       {
     if ( fanPWM <= 0 || (fanPWM-5) <= 0 )
      {
        fanPWM = 0;
      }

     else {
          fanPWM=fanPWM-5;
          }
      }
 
    else if (buttNum == 2) // fan Up
      {
      if ( fanPWM >= 255 )
        {
          fanPWM = 255;
        }
      else {
          fanPWM=fanPWM+5;
           }
      }
     //------------------------------------------

     // ------ dimmer  -----------------------------
     if (buttNum == 3) // dimmer Down
       {
     if ( Dimmer1 <= 0 || (Dimmer1-5) <= 0 )
      {
        Dimmer1 = 0;
      }

     else {
          Dimmer1=Dimmer1-5;
          }
      }
 
    else if (buttNum == 4) // dimmer Up
      {
      if ( Dimmer1 >= 220 )
        {
          Dimmer1 = 220;
        }
      else {
          Dimmer1=Dimmer1+5;
           }
      }
     //------------------------------------------
   
   }
   buttonLastChecked = millis(); // reset the lastChecked value
 }
//---------------------------------------------//

#endif

  
}


#ifdef CALIB
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
#endif




