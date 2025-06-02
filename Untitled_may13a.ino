#include "arduino_secrets.h"
/*
  File: fargsensor_med_array.ino
  Author: William Kowalski Merrick
  Date: 2025-05-13
  Description:
    This program moves candies to a color sensor using servos. The detected color is then used to light up a NeoPixel ring with the corresponding RGB color.
    Based on the color, the candy is moved to a specific location using another servo.
    Now integrated with Arduino IoT Cloud to count red, green, and blue candies.
*/

#include <Servo.h>
#include <Adafruit_NeoPixel.h>
#ifdef __AVR__
  #include <avr/power.h>
#endif
#include "thingProperties.h"

#define PIN 11
Adafruit_NeoPixel strip = Adafruit_NeoPixel(24, PIN, NEO_GRB + NEO_KHZ800);

#define S0 4
#define S1 5
#define S2 6
#define S3 7
#define sensorOut 8

int frequency = 0;
String colorBuffer[10];
int bufferIndex = 0;
String prevColor = "";

Servo myservo1;
Servo myservo2;

void setup() {
  pinMode(S0, OUTPUT);
  pinMode(S1, OUTPUT);
  pinMode(S2, OUTPUT);
  pinMode(S3, OUTPUT);
  pinMode(sensorOut, INPUT);
  myservo1.attach(9);
  myservo2.attach(10);

  digitalWrite(S0, HIGH);
  digitalWrite(S1, LOW);

  #if defined (__AVR_ATtiny85__)
    if (F_CPU == 16000000) clock_prescale_set(clock_div_1);
  #endif

  strip.begin();
  strip.setBrightness(50);
  strip.show();

  Serial.begin(9600);
  delay(1500);

  initProperties();
  ArduinoCloud.begin(ArduinoIoTPreferredConnection);
  setDebugMessageLevel(2);
  ArduinoCloud.printDebugInfo();
}

String detectColor() {
  digitalWrite(S2, LOW);
  digitalWrite(S3, LOW);
  int redVal = pulseIn(sensorOut, LOW);

  digitalWrite(S2, HIGH);
  digitalWrite(S3, HIGH);
  int greenVal = pulseIn(sensorOut, LOW);

  digitalWrite(S2, LOW);
  digitalWrite(S3, HIGH);
  int blueVal = pulseIn(sensorOut, LOW);

  if (redVal < greenVal && redVal < blueVal) return "Red";
  if (greenVal < redVal && greenVal < blueVal) return "Green";
  if (blueVal < redVal && blueVal < greenVal) return "Blue";
  return "Unknown";
}

void updateColorBuffer(String newColor) {
  for (int i = 9; i > 0; i--) {
    colorBuffer[i] = colorBuffer[i - 1];
  }
  colorBuffer[0] = newColor;
}

void printColorBuffer() {
  Serial.print("Last 10 colors: ");
  for (int i = 0; i < 10; i++) {
    Serial.print(colorBuffer[i]);
  }
  Serial.println();
}

void colorWipe(uint32_t c, uint8_t wait) {
  for(uint16_t i=0; i<strip.numPixels(); i++) {
    strip.setPixelColor(i, c);
    strip.show();
    delay(wait);
  }
}

void vilkenfarg() {
  String detectedColor = detectColor();
  Serial.println("Detected color: " + detectedColor + " " + String(red)+ " " + String(green)+ " " + String(blue));
  if (detectedColor != prevColor) {
    if (detectedColor == "Green") {
      green++;
      colorWipe(strip.Color(0, 255, 0), 50);
      myservo1.write(40);
    } else if (detectedColor == "Blue") {
      blue++;
      colorWipe(strip.Color(0, 0, 255), 50);
      myservo1.write(100);
    } else if (detectedColor == "Red") {
      red++;
      colorWipe(strip.Color(255, 0, 0), 50);
      myservo1.write(180);
    }

    updateColorBuffer(detectedColor);
    printColorBuffer();
    prevColor = detectedColor;
  }

  delay(1000);
}

void loop() {
  ArduinoCloud.update();
  myservo2.write(0);
  delay(1000);
  myservo2.write(60);
  delay(3000);
  vilkenfarg();
  myservo2.write(90);
  delay(500);
}

void servo1() {}


