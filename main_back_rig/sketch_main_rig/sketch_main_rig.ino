#include <Servo.h>
#include "IRremote.h"

Servo myservo;
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



void setup() {
  // put your setup code here, to run once:
  myservo.attach(8);

  Serial.begin(9600); // выставляем скорость COM порта
  irrecv.enableIRIn(); // запускаем прием
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

void loop() {
  if ( irrecv.decode( &results )) { // если данные пришли
    Serial.println( results.value, HEX ); // печатаем данные
    switch ( results.value ) {
      case 0xFF38C7:
      case 0x488F3CBB:
          servoslow(myservo, 70, 5, 70);
          break;
      //left
      case 0xFF10EF:
      case 0x8C22657B:
          servoslow(myservo, 150, 5, 70);
          break;
      //right          
      case 0xFF5AA5:        
      case 0x449E79F:
        servoslow(myservo, 20, 5, 70);
          break;
    } 
    irrecv.resume(); // принимаем следующую команду
  }

  //servoslow(myservo, 150, 10, 0);
}
