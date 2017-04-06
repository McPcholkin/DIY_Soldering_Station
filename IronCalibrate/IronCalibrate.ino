// get calibration data 
// pwm - analog - temp   values table 


const int pinPwmIron = 6; // pwm to mosfet
const int pinTempIron = A0; // input from termal sensor in iron

// Buttons
const int BUTTON_ERROR_WINDOW = 5;  // +/- this value 
const int BUTTON_DELAY = 210;       // delay to debounce button
const int controlButtonsPin = A4;   // switch circuit input connected to analog pin 4
long buttonLastChecked = 0; // variable to limit the button getting checked every cycle

int ironPWM = 0;  
int ironAnalog = 0;



void setup() {
  Serial.begin(115200);
}

void loop() {


ironAnalog = analogRead(pinTempIron);

Serial.print("A0: ");
Serial.print(ironAnalog);
Serial.print(" | ");
Serial.print("PWM: ");
Serial.print(ironPWM);
Serial.println();





  
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
     if ( ironPWM <= 0 || (ironPWM-5) <= 0 )
      {
        ironPWM = 0;
      }

     else {
          ironPWM=ironPWM-5;
          }
      }
 
    else if (buttNum == 2) // Iron temp Up
      {
      if ( ironPWM >= 255 )
        {
          ironPWM = 255;
        }
      else {
          ironPWM=ironPWM+5;
           }
      }
     //------------------------------------------
   
   }
   buttonLastChecked = millis(); // reset the lastChecked value
 }
//---------------------------------------------//


analogWrite(pinPwmIron, ironPWM);

delay(100);


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


