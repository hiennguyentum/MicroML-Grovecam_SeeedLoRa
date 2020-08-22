#include <Wire.h>
#include "Adafruit_TCS34725.h"

//// Pick analog outputs, for the UNO these three work well
//// use ~560  ohm resistor between Red & Blue, ~1K for green (its brighter)
//#define redpin 3
//#define greenpin 5
//#define bluepin 6
//// for a common anode LED, connect the common pin to +5V
//// for common cathode, connect the common to ground
//
//// set to false if using a common cathode LED
//#define commonAnode true
//
//// our RGB -> eye-recognized gamma color
//byte gammatable[256];

Adafruit_TCS34725 tcs = Adafruit_TCS34725(TCS34725_INTEGRATIONTIME_50MS, TCS34725_GAIN_4X);

void setup() 
{
  // Initizaling the board and color sensor
  digitalWrite(38, HIGH);
  Serial.begin(9600);
  while (!Serial) {}; // wait for serial port to connect. Needed for native USB port only
  Serial.println("Color Sensor TCS34725 Connected!");

  if (tcs.begin()) {
    //Serial.println("Found sensor");
  } else {
    Serial.println("No TCS34725 found ... check your connections");
    while (1); // halt!
  }

  // Print the header
  Serial.println("Red,Green,Blue");
  
}

void loop() 
{
  readColorSensorData();
}

void readColorSensorData()
{
  // Define RGB and Sum (for CSV) float
  float red, green, blue;
//  float sum;

  // Read the color data
  tcs.setInterrupt(false);  // turn on LED
  delay(60);  // takes 60ms to read
  tcs.getRGB(&red, &green, &blue);
//  sum = 1000;
  tcs.setInterrupt(true);  // turn off LED

  // For CSV format
//  float redRatio = red / sum;
//  float greenRatio = green / sum;
//  float blueRatio = blue / sum;

  // Print the data in CSV format
  Serial.print(red, 3);
  Serial.print(',');
  Serial.print(green, 3);
  Serial.print(',');
  Serial.print(blue, 3);
  Serial.println();
}
