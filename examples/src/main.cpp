/* WS2812Serial BasicTest Example
   Test LEDs by turning then 7 different colors.
   This example code is in the public domain. */

#include <Arduino.h>
#include <WS2812Serial.h>
#include <Encoder.h>
#include <math.h>
#include <Bounce.h>

const int numled = 20;
const int nx = 8;
const int ny = 8;
const int pin = 1;
const int maxPos = 80;
const int btnPin = 4;
const int nModes = 4;

int mode = 0;
float col[3];

Encoder myEnc(2,3);
Bounce btn = Bounce(btnPin, 10);
//myEnc.write(0);
int pos = myEnc.read();

// Usable pins:
//   Teensy LC:   1, 4, 5, 24
//   Teensy 3.2:  1, 5, 8, 10, 31   (overclock to 120 MHz for pin 8)
//   Teensy 3.5:  1, 5, 8, 10, 26, 32, 33, 48
//   Teensy 3.6:  1, 5, 8, 10, 26, 32, 33

byte drawingMemory[numled*4];         //  3 bytes per LED
DMAMEM byte displayMemory[numled*16]; // 12 bytes per LED

WS2812Serial leds(numled, displayMemory, drawingMemory, pin, WS2812_GRB); //WS2812_GRB
/*
#define R    0x00FF0000
#define G  0x0000FF00
#define B   0x000000FF
#define Y 0x00FFFF00
#define P   0x00FF1088
#define O 0x00E05800
#define W  0xFF000000
*/
// Less intense...

#define R  0x00160000
#define G  0x00001600
#define B  0x00000016
#define Y  0x00101400
#define P  0x00160016 // Purple
#define O  0x00100400
#define W  0x00161616
#define T  0x00106a6b // Teal- Chosen by Lexie

// HSV->RGB conversion based on GLSL version
// expects hsv channels defined in 0.0 .. 1.0 interval
//https://gist.github.com/postspectacular/2a4a8db092011c6743a7
float fract(float x) { return x - int(x); }

float mix(float a, float b, float t) { return a + (b - a) * t; }

float step(float e, float x) { return x < e ? 0.0 : 1.0; }

float* hsv2rgb(float h, float s, float b, float* rgb) {
  rgb[0] = b * mix(1.0, constrain(abs(fract(h + 1.0) * 6.0 - 3.0) - 1.0, 0.0, 1.0), s);
  rgb[1] = b * mix(1.0, constrain(abs(fract(h + 0.6666666) * 6.0 - 3.0) - 1.0, 0.0, 1.0), s);
  rgb[2] = b * mix(1.0, constrain(abs(fract(h + 0.3333333) * 6.0 - 3.0) - 1.0, 0.0, 1.0), s);
  return rgb;
}

float* rgb2hsv(float r, float g, float b, float* hsv) {
  float s = step(b, g);
  float px = mix(b, g, s);
  float py = mix(g, b, s);
  float pz = mix(-1.0, 0.0, s);
  float pw = mix(0.6666666, -0.3333333, s);
  s = step(px, r);
  float qx = mix(px, r, s);
  float qz = mix(pw, pz, s);
  float qw = mix(r, px, s);
  float d = qx - min(qw, py);
  hsv[0] = abs(qz + (qw - py) / (6.0 * d + 1e-10));
  hsv[1] = d / (qx + 1e-10);
  hsv[2] = qx;
  return hsv;
}


void setup() {
  leds.begin();
  myEnc.write(0);
  mode = 4;
  pinMode(btnPin, INPUT_PULLUP);
}


void loop() {
  if (btn.update()) {
    if (btn.fallingEdge()) {
      mode++;
      myEnc.write(0);
    }
  }
  if (mode > nModes) {
    mode = 0;
  }
  
  switch(mode) {
    case 0:
      for (int i=0; i < numled; i++) {
        leds.setPixel(i, 0);
      }
      break;
    
    case 1:
      pos = myEnc.read();
      if (pos > maxPos) {
        myEnc.write(maxPos);
      } else if (pos < 0) {
        myEnc.write(0);
      }
      for (int i=0; i < pos/4; i++) {
        leds.setPixel(i, T);
      }
      for (int i=pos/4; i < numled; i++) {
        leds.setPixel(i, 0);
      }
      break;

    case 2:
      pos = myEnc.read();
      if (pos > maxPos) {
        myEnc.write(maxPos);
      } else if (pos < -maxPos+1) {
        myEnc.write(-maxPos+1);
      }
      leds.setPixel(0, O);
      if (pos >= 0) {
        for (int i=1; i <= pos/4; i++) {
          leds.setPixel(i, G);
        }
        for (int i=pos/4+1; i < numled; i++) {
          leds.setPixel(i, 0);
        }
      } else if (pos < 0) {
        for (int i=numled-1; i > numled + pos/4-1; i--) {
          leds.setPixel(i, B);
        }
        for (int i=numled + pos/4-1; i > 0; i--) {
          leds.setPixel(i, 0);
        }
      }
      break;

    case 3:
      pos = myEnc.read();
      if (pos > maxPos) {
        myEnc.write(maxPos);
      } else if (pos < 0) {
        myEnc.write(0);
      }
      for (int i=0; i<pos/4; i++) {
        leds.setPixel(i, 0);
      }
      leds.setPixel(pos/4, R);
      for (int i=pos/4+1; i<numled; i++) {
        leds.setPixel(i, 0);
      }
      break;

    case 4:
      pos = myEnc.read();
      if (pos > maxPos) {
        myEnc.write(maxPos);
      } else if (pos < 0) {
        myEnc.write(0);
      }
      for (int i=0; i < pos/4; i++) {
        float* RGB = hsv2rgb((i)/float(numled), 1, .25, col);
        leds.setPixel(i, int(RGB[0]*255), int(RGB[1]*255), int(RGB[2]*255));
      }
      for (int i=pos/4; i < numled; i++) {
        leds.setPixel(i, 0);
      }
    }
    leds.show();
}
