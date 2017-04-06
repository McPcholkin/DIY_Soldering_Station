
int fanPWMPin = 5;
int AirControlPin = 3;
// zero detect pin 2 (timer 0)

int fanPWM = 0;
int dimmer = 255; // start dimmer value (255 - full off, 0 - full ON)
int tic = 0;
boolean timer = 0;
int previousMicros = 0; //save micros

//#define CALIB

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
  
   attachInterrupt(0, detect_up, LOW); // detect UP on zero cross
}

//********************обработчики прерываний*******************************

void halfcycle()  //прерывания таймера
{
  tic++;  //счетчик 
  if(dimmer < tic )
  {
    digitalWrite(AirControlPin, HIGH);
  }
}

void  detect_up()  // обработка внешнего прерывания. Сработает по переднему фронту
{ 
 tic=0;             //обнулить счетчик
 timer=true;
 //ResumeTimer1();   //запустить таймер
 attachInterrupt(0, detect_down, HIGH);  //перепрограммировать прерывание на другой обработчик
} 

void  detect_down()  // обработка внешнего прерывания. Сработает по заднему фронту
{  
 //StopTimer1(); //остановить таймер
 timer=false;
 //D4_Low;  //логический ноль на выходы
 digitalWrite(AirControlPin, LOW);
 tic=0;       //обнулить счетчик
 attachInterrupt(0, detect_up, LOW); //перепрограммировать прерывание на другой обработчик
}
//*************************************************************************



void loop() {

     if (timer==true)
     {  
       int currentMicros = micros();
       if(currentMicros - previousMicros > 25) 
       {
        // сохраняем время последнего переключения
        previousMicros = currentMicros;
        halfcycle();
       }
    }

#ifdef CALIB
airAnalog = analogRead(pinTempAir);

Serial.print("A1: ");
Serial.print(airAnalog);
Serial.print(" | ");
Serial.print("PWM: ");
Serial.print(fanPWM);
Serial.print(" | ");
Serial.print("DIM: ");
Serial.print(dimmer);
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
     if ( dimmer <= 0 || (dimmer-5) <= 0 )
      {
        dimmer = 0;
      }

     else {
          dimmer=dimmer-5;
          }
      }
 
    else if (buttNum == 4) // dimmer Up
      {
      if ( dimmer >= 255 )
        {
          dimmer = 255;
        }
      else {
          dimmer=dimmer+5;
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




