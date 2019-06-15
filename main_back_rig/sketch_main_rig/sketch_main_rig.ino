#define FASTLED_ALLOW_INTERRUPTS 0
#include <Servo.h>
#include "IRremote.h"
#include <FastLED.h>

#include <SPI.h>
#include <nRF24L01.h>
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
//////////////////////////////////////////

//WS LED settings - how many leds and pins
#define NUM_LEDS 17
#define DATA_PIN 5
#define CLOCK_PIN 6

CRGB leds[NUM_LEDS];

uint8_t max_bright = 128; // Overall brightness definition. It can be changed on the fly.

CRGBPalette16 currentPalette = LavaColors_p;
CRGBPalette16 targetPalette = OceanColors_p;
TBlendType currentBlending; // NOBLEND or LINEARBLEND

static int16_t dist;  // A moving location for our noise generator.
uint16_t xscale = 30; // Wouldn't recommend changing this on the fly, or the animation will be really blocky.
uint16_t yscale = 30; // Wouldn't recommend changing this on the fly, or the animation will be really blocky.

uint8_t maxChanges = 24;

int current_mode = 0;

RF24 radio(9, 10); // CE, CSN
const byte address[6] = "00001";

int currentLeft = 0;
int currentRight = 0;

void attachServos()
{
  myservoLeft.deattach(4);
  myservoRight.deattach(3);

  //set start position for servos
  servoSlowDouble(5, 5, 5);
}

void deattachServos()
{
  myservoLeft.attach(4);
  myservoRight.attach(3);
}

void setup()
{
  //attach servo to pins
  attachServos();

  Serial.begin(9600);  // выставляем скорость COM порта
  irrecv.enableIRIn(); // запускаем прием

  //ste type of led, it's data pin and led's quantity
  FastLED.addLeds<WS2801, DATA_PIN, CLOCK_PIN>(leds, NUM_LEDS);
  FastLED.setBrightness(max_bright);
  set_max_power_in_volts_and_milliamps(5, 500);

  dist = random16(12345);

  /*radio.begin();
  radio.openWritingPipe(address);
  radio.setPALevel(RF24_PA_MIN);
  radio.stopListening();*/
}

void servoSlow(Servo num, int pos, int time, int start)
{
  num.write(start);

  if (pos > start)
  {
    for (int i = start; i < pos; i++)
    {
      num.write(i);
      delay(time);
    }
  }
  else
  {
    for (int i = start; i > pos; i--)
    {
      num.write(i);
      delay(time);
    }
  }
}

/**
 * 
 */
void servoSlowDouble(int pos, int time, int start)
{
  //move position to start
  myservoLeft.write(start);
  myservoRight.write(start);

  if (pos > start)
  {
    for (int i = start; i < pos; i++)
    {
      myservoLeft.write(i);
      myservoRight.write(i);
      delay(time);
    }
  }
  else
  {
    for (int i = start; i > pos; i--)
    {
      myservoLeft.write(i);
      myservoRight.write(i);
      delay(time);
    }
  }
  currentRight = pos;
  currentLeft = pos;
}

void inoise8_mover()
{
  for (int i = 0; i < 20; i++)
  {
    uint8_t locn = inoise8(xscale, dist + yscale + i * 200);                   // Get a new pixel location from moving noise. locn rarely goes below 48 or above 192, so let's remove those ends.
    locn = constrain(locn, 48, 192);                                           // Ensure that the occasional value outside those limits is not used.
    uint8_t pixlen = map(locn, 48, 192, 0, NUM_LEDS - 1);                      // Map doesn't constrain, so we now map locn to the the length of the strand.
    leds[pixlen] = ColorFromPalette(currentPalette, pixlen, 255, LINEARBLEND); // Use that value for both the location as well as the palette index colour for the pixel.
  }

  dist += beatsin8(10, 1, 4); // Moving along the distance (that random number we started out with). Vary it a bit with a sine wave.
}

void runBottomAndBackLeds(int speed = 50)
{
  for (int dot = 0; dot < NUM_LEDS; dot++)
  {
    Serial.println(dot); // печатаем данные
    if (dot > 1)
    {
      leds[dot - 1] = CRGB(0, 255, 0);
    }
    leds[dot] = CRGB(0, 150, 150);

    //mirrored leds
    leds[NUM_LEDS - dot - 1] = CRGB(0, 150, 150);
    leds[NUM_LEDS - dot] = CRGB(0, 150, 150);

    FastLED.show();
    // clear this led for the next time around the loop

    leds[NUM_LEDS - dot - 1] = CRGB::Black;
    leds[NUM_LEDS - dot] = CRGB::Black;
    delay(speed);

    leds[dot] = CRGB::Black;
    leds[dot - 1] = CRGB::Black;
  }
}

void runTopToBottomLeds(int speed = 50)
{
  for (int dot = 0; dot < NUM_LEDS / 2; dot++)
  {
    Serial.println(dot); // печатаем данные
    if (dot > 1)
    {
      leds[dot - 1] = CRGB(0, 255, 0);
    }
    leds[dot] = CRGB(0, 150, 150);

    //mirrored leds
    leds[NUM_LEDS - dot - 1] = CRGB(0, 150, 150);
    leds[NUM_LEDS - dot] = CRGB(0, 150, 150);

    FastLED.show();
    // clear this led for the next time around the loop

    leds[NUM_LEDS - dot - 1] = CRGB::Black;
    leds[NUM_LEDS - dot] = CRGB::Black;
    delay(speed);

    leds[dot] = CRGB::Black;
    leds[dot - 1] = CRGB::Black;
  }
}

void lightSegment(int segment = 4)
{
  FastLED.show();
  for (int dot = 0; dot < NUM_LEDS; dot++)
  {
    leds[dot] = CRGB::Black;
  }
  switch (segment)
  {
  case 4:
    leds[7] = CRGB(255, 0, 0);
    leds[6] = CRGB(255, 0, 0);
    leds[8] = CRGB(255, 0, 0);
    break;
  case 3:
    leds[7] = CRGB(255, 0, 216);
    leds[6] = CRGB(255, 0, 216);
    leds[8] = CRGB(255, 0, 216);

    leds[5] = CRGB(255, 0, 216);
    leds[9] = CRGB(255, 0, 216);
    break;
  case 2:
    for (int dot = 0; dot < NUM_LEDS; dot++)
    {
      leds[dot] = CRGB(0, 255, 0);
    }
  }

  FastLED.show();
}

void loop()
{

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

  if (irrecv.decode(&results))
  { // если данные пришли
    Serial.println(results.value, HEX); //output data for debugging
    current_mode = results.value;
    irrecv.resume(); //recieve next command (after we finish next actions)

    //if we get any command except reset
    if (current_mode !== IR0) {
      attachServos();
    }

    switch (current_mode)
    {
    case IROk1:
    case IROk2:
      servoSlowDouble(5, 5, 5);
      break;
    case IR2:
    case IR2_ALT:
      runBottomAndBackLeds(50);
      break;
    case IR3:
      //FastLED.show();
      runTopToBottomLeds(50);
      break;
    case IR4:
      lightSegment(4);
      break;
    case IR5:
      lightSegment(3);
      break;
    case IR6:
      lightSegment(2);
      break;
    case IR0:
      //switch off light
      lightSegment(0);
      //deattach servos to lower power consumption
      deattachServos();
      break;
    //left
    case IRLeft1:
    case IRLeft2:
      servoSlowDouble(150, 5, currentRight);
      break;
    //right
    case IRRight1:
    case IRRight2:
      servoSlowDouble(60, 5, currentRight);
      break;
    case IR1:
    case IR1_ALT:
      //set brightness to lower brightness position for lower power consumtion
      //so it wouldn't case problems with both led and servos work
      FastLED.setBrightness(64);
      servoSlowDouble(150, 20, 10);
      delay(1000);
      servoSlowDouble(80, 5, 150);
      delay(500);
      servoSlowDouble(10, 10, 80);
      delay(500);
      servoSlowDouble(150, 10, 10);
      delay(500);
      servoSlowDouble(5, 20, 150);
      FastLED.setBrightness(max_bright);
      break;
    }
  }
}
