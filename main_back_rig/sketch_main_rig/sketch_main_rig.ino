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
#define IR3 0xFFE21D

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

uint8_t max_bright = 128;                                      // Overall brightness definition. It can be changed on the fly.

CRGBPalette16 currentPalette=LavaColors_p;
CRGBPalette16 targetPalette=OceanColors_p;
TBlendType    currentBlending;                                // NOBLEND or LINEARBLEND 
 
static int16_t dist;                                          // A moving location for our noise generator.
uint16_t xscale = 30;                                         // Wouldn't recommend changing this on the fly, or the animation will be really blocky.
uint16_t yscale = 30;                                         // Wouldn't recommend changing this on the fly, or the animation will be really blocky.

uint8_t maxChanges = 24;    

int current_mode = 0;     
    
void setup() {
  // put your setup code here, to run once:
  myservoLeft.attach(4);
  myservoRight.attach(3);

  Serial.begin(9600); // выставляем скорость COM порта
  irrecv.enableIRIn(); // запускаем прием

  FastLED.addLeds<WS2801, DATA_PIN, CLOCK_PIN>(leds, NUM_LEDS);
  FastLED.setBrightness(max_bright);
  set_max_power_in_volts_and_milliamps(5, 500);  

  dist = random16(12345);
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

 
void inoise8_mover() {

  for (int i=0; i<20; i++) {
    uint8_t locn = inoise8(xscale, dist+yscale+i*200);  // Get a new pixel location from moving noise. locn rarely goes below 48 or above 192, so let's remove those ends.
    locn = constrain(locn,48,192);                      // Ensure that the occasional value outside those limits is not used.
    uint8_t pixlen = map(locn,48,192,0,NUM_LEDS-1);     // Map doesn't constrain, so we now map locn to the the length of the strand.
    leds[pixlen] = ColorFromPalette(currentPalette, pixlen, 255, LINEARBLEND);   // Use that value for both the location as well as the palette index colour for the pixel.
  }

  dist += beatsin8(10,1,4);                                                // Moving along the distance (that random number we started out with). Vary it a bit with a sine wave.                                             

} // inoise8_mover()

void loop() {  

  /*EVERY_N_MILLISECONDS(20) {
    if (current_mode == IR3) {
    nblendPaletteTowardPalette(currentPalette, targetPalette, maxChanges);   // AWESOME palette blending capability.
    inoise8_mover(); // Update the LED array with noise at the new location
    fadeToBlackBy(leds, NUM_LEDS, 16);  
    }   
  }

  EVERY_N_SECONDS(5) { // Change the target palette to a random one every 5 seconds.
    Serial.println( "EVERY_N_SECONDS" );
    Serial.println( results.value, HEX );
    Serial.println( IR3, HEX );
    Serial.println( results.value );
    if (current_mode == IR3) {
    targetPalette = CRGBPalette16(CHSV(random8(), 255, random8(128,255)), CHSV(random8(), 255, random8(128,255)), CHSV(random8(), 192, random8(128,255)), CHSV(random8(), 255, random8(128,255)));
    }
  }*/
  
  if ( irrecv.decode( &results )) { // если данные пришли
    Serial.println( results.value, HEX ); // печатаем данные
    current_mode = results.value;
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
      case IR3:   
        FastLED.show();
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

    /*if (current_mode == IR3) {      
      FastLED.show();
    }*/
  }
}
