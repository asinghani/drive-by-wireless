#include <FastLED.h>

#define NUM_LEDS 29
#define DATA_PIN 9

CRGB leds[2*NUM_LEDS];

/*#define NUM_STATES 3
CRGB states[NUM_STATES] = {
  CRGB(10, 0, 60),
  CRGB(50, 0, 30),
  CRGB(80, 0, 0),
};*/
void setup() {
  delay(2000);
  Serial1.begin(115200);
  FastLED.addLeds<WS2812B, DATA_PIN, RGB>(leds, 2*NUM_LEDS);
}

uint8_t m_speed = 0;
int state = 0;

void loop() {
  for (int i = 0; i < NUM_LEDS; i++) {
    CRGB base = CRGB(35, 85, 0);
    CRGB color = base;
    if (((i+state)%28) == 4) color = CRGB(120, 0, 60);
    if (((i+state)%28) == 3) color = CRGB(85, 0, 50);
    if (((i+state)%28) == 2) color = CRGB(65, 0, 40);
    if (((i+state)%28) == 1) color = CRGB(50, 0, 20);
    if (((i+state)%28) == 0) color = CRGB(40, 0, 15);
    //if (i < 4) color = base;
    if (m_speed == 0) color = CRGB(35, 85, 0);
    
    leds[NUM_LEDS-i-1] = color;
    leds[NUM_LEDS+i] = color;
  }

  while (Serial1.available()) m_speed = Serial1.read();
  if (m_speed > 100) m_speed = 100;

  FastLED.show();

  delay(50 - m_speed/3);

  state -= 1;
  if (state <= 0) state = 3584;
}
