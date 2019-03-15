#include <Servo.h>
#include "IRremote.h"
#include <FastLED.h>

Servo myservoLeft;
Servo myservoRight;

IRrecv irrecv(2); // указываем вывод, к которому подключен приемник
decode_results results;

//IR codes for remote
//1 - E318261B
//2 - 511DBB
//3 - FFE21D
//UP - 3D9AE3F7
//OK - FF38C7 or 488F3CBB
//DOWN - FF4AB5
//LEFT - FF10EF or 8C22657B
//RIGHT - FF5AA5 or 449E79F
#define IR1 0xE318261B
#define IR2 0x511DBB

#define IRLeft1 0xFF10EF
#define IRLeft2 0x8C22657B
#define IRRight1 0xFF5AA5
#define IRRight2 0x449E79F
#define IROk1 0xFF38C7
#define IROk2 0x488F3CBB

#define NUM_LEDS 32
#define DATA_PIN 5
#define CLOCK_PIN 6

CRGB leds[NUM_LEDS];
    
void setup() {
  // put your setup code here, to run once:
  myservoLeft.attach(4);
  myservoRight.attach(3);

  Serial.begin(9600); // выставляем скорость COM порта
  irrecv.enableIRIn(); // запускаем прием

  FastLED.addLeds<WS2801, DATA_PIN, CLOCK_PIN>(leds, NUM_LEDS);
}

void servoslow( Servo num, int pos, int time, int start)  // robotday.ru *** Функция для управления скоростью сервопривода ***
{
  num.write(start);

  if (pos>start) {
    for ( int i = start; i < pos; i++)
    { 
      num.write(i);
      delay(time);
    }
  } else {
    for ( int i = start; i > pos; i--)
    { 
      num.write(i);
      delay(time);
    }
  }
}

void servoslowDouble(int pos, int time, int start)  // robotday.ru *** Функция для управления скоростью сервопривода ***
{
  myservoLeft.write(start);
  myservoRight.write(start);

  if (pos>start) {
    for ( int i = start; i < pos; i++)
    { 
      myservoLeft.write(i);
      myservoRight.write(i);
      delay(time);
    }
  } else {
    for ( int i = start; i > pos; i--)
    { 
      myservoLeft.write(i);
      myservoRight.write(i);
      delay(time);
    }
  }
}

void loop() {

  
  if ( irrecv.decode( &results )) { // если данные пришли
    Serial.println( results.value, HEX ); // печатаем данные
    switch ( results.value ) {
      case IROk1:
      case IROk2:
          servoslowDouble(70, 5, 70);
          break;
      case IR2:      
        for(int dot = 0; dot < NUM_LEDS; dot++) { 
            if (dot>1) {
              leds[dot-1] = CRGB::Blue;
            }
            leds[dot] = CRGB::Green;
            FastLED.show();
            // clear this led for the next time around the loop
            leds[dot] = CRGB::Black;
            leds[dot-1] = CRGB::Black;
            delay(60);
        }
        break;
      //left
      case IRLeft1:
      case IRLeft2:
          servoslowDouble(150, 5, 70);
          break;
      //right          
      case IRRight1:        
      case IRRight2:
        servoslowDouble(20, 5, 70);
        break;
      case IR1:
        servoslowDouble(150, 20, 10);
        delay(1000);
        servoslowDouble(80, 5, 150);
        delay(500);
        servoslowDouble(150, 10, 10);
        break;
    } 
    irrecv.resume(); // принимаем следующую команду
  }
}
