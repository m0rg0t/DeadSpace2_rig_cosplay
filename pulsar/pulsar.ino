#include "FastLED.h"

// оличество светодиодов
#define NUM_LEDS 20 
//---------
//”правл¤ющий пин
#define DATA_PIN 6
//---------
//’« зачем это, осталось от предыдущего проекта
#define CLOCK_PIN 13
//---------
//Ќачальна¤ ¤ркость
int down = 255;
//---------
CRGB leds[NUM_LEDS];

void setup() { 
  LEDS.addLeds<WS2812,DATA_PIN,RGB>(leds,NUM_LEDS);
  LEDS.setBrightness(150);
}

#define MAXV 255
#define MINV 150

void loop() { 
  down = MAXV;
  //ѕервый блок за 20 циклов уменьшаем ¤ркость, с задержкой 100 милисекунд
 
 for(int iq = down; iq > MINV; iq = iq - 1) {
  for(int i = 0; i < NUM_LEDS; i++) {
    leds[i] = CHSV( HUE_GREEN, 255, down);
    
  }
  FastLED.show(); 
  down = iq;
  //down = down - 10;
  delay(10);
  }
  //ѕауза между блоками в милисекундах
  //delay(1000);
  //¬торой блок за 20 циклов повышаем ¤ркость, с задержкой 100 милисекунд
 for(int iq = MINV; iq < MAXV; iq = iq + 1) {
  for(int i = 0; i < NUM_LEDS; i++) {
    leds[i] = CHSV( HUE_GREEN, 255, down);    
  }
  FastLED.show(); 
  down = iq;
  delay(10);
  }
 //ѕауза между блоками в милисекундах
 delay(1000);

}
