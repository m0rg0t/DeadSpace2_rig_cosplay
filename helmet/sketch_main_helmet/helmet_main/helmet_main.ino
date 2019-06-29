#define FASTLED_ALLOW_INTERRUPTS 0
#include <Servo.h>
#include "IRremote.h"
//#include <FastLED.h>

#include <SPI.h>
//#include <nRF24L01.h>
#include <RF24.h>

Servo myservoLeft;
Servo myservoRight;

IRrecv irrecv(2); // указываем вывод, к которому подключен приемник
decode_results results;

//Define IR codes /////////////////////////////
#define IR2 0x511DBB
#define IR2_ALT 0xFF629D
#define IR3 0xFFE21D
#define IR3_ALT 0xEE886D7F

#define IR4 0xFF22DD

#define IR6_ALT 0x20FE4DBB
#define IR6 0xFFC23D
#define IR5 0xFF02FD
#define IR5_ALT 0xD7E84B1B
#define IR0 0xFF9867

#define IRHash 0xFFB04F

#define IR1 0xE318261B
#define IR1_ALT 0xFFA25D

#define IRLeft1 0xFF10EF
#define IRLeft2 0x8C22657B

#define IRRight1 0xFF5AA5
#define IRRight2 0x449E79F

#define IROk1 0xFF38C7
#define IROk2 0x488F3CBB

#define IRUp1 0xFF18E7

//////////////////////////////////////////

//WS LED settings - how many leds and pins
#define NUM_LEDS 15
#define DATA_PIN 6
#define CLOCK_PIN 5

//CRGB leds[NUM_LEDS];

uint8_t max_bright = 128;  // Overall brightness definition. It can be changed on the fly.

int current_mode = 0;

RF24 radio(9, 10); // CE, CSN
const byte address[6] = "00002";

int currentLeft = 0;
int currentRight = 0;

void attachServos()
{
  myservoLeft.attach(3);
  myservoRight.attach(4);

  //set start position for servos
  servoSlowDouble(1, 0, 1);
}

void deatachServos() {
  myservoLeft.detach();
  myservoRight.detach();
}
#define LED 6
void setup() {
  digitalWrite(LED, HIGH);
  //attach servo to pins
  attachServos();


  Serial.begin(9600); // выставляем скорость COM порта
  irrecv.enableIRIn(); // запускаем прием


  //dist = random16(12345);

  //set start position for servos
  servoSlowDouble(5, 5, 5);

}

#define DIFF 190

void servoSlow( Servo num, int pos, int time, int start)
{
  num.write(start);

  if (pos > start) {
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

/**

*/
void servoSlowDouble(int pos, int time, int start)
{
 
  //move position to start
  myservoLeft.write(start);
  myservoRight.write(DIFF - start);

  if (pos > start) {
    for ( int i = start; i < pos; i++)
    {
      myservoLeft.write(i);
      myservoRight.write(180 - i);
      delay(time);
    }
  } else {
    for ( int i = start; i > pos; i--)
    {
      myservoLeft.write(i);
      myservoRight.write(180 - i);
      delay(time);


    }
  }
  currentRight = pos;
  currentLeft = pos;
}

void servoFastDouble(int pos, int  time, int start) {
   myservoLeft.write(pos);
  myservoRight.write(DIFF - pos);

  currentRight = pos;
  currentLeft = pos;
}


bool attached = true;

void loop() {

  if ( irrecv.decode( &results )) { // если данные пришли
    Serial.println( results.value, HEX ); // печатаем данные
    current_mode = results.value;
    irrecv.resume(); // принимаем следующую команду

    switch ( current_mode ) {
      case IROk1:
      case IROk2:
        servoFastDouble(6, 0, 6);
        //deatachServos();
        break;
      case IR0:        
        if (attached) {
          attached = false;
          deatachServos();
          digitalWrite(LED, LOW);
        } else {
          attached = true;
          attachServos();
          digitalWrite(LED, HIGH);
        }
        break;
      //left
      case IRLeft1:
      case IRLeft2:
        //Serial.println("Move left");
        servoFastDouble(30, 0, currentRight);
        break;
      //right
      case IRRight1:
      case IRRight2:
        servoSlowDouble(60, 4, currentRight);
        break;
      case IRUp1:
        servoSlowDouble(100, 4, currentRight);
        break;
    }

  }
}
