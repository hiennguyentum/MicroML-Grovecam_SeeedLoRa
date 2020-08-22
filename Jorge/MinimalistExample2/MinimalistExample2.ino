/* The code is only recording data if a movement is detacted
 * So you can coppy and paste direct from the serial monitor into the txt.-file
 * just add the header 'aX,aY,aZ,gX,gY,gZ' to the txt.-file and convert it into a csv.-file
 *
 * The calibration was ajusted as well, now the code uses 100 data points instead of only 10 for calibration
 *
 * David Zeller, 03.07.2020
 */



#include "SparkFunLSM6DS3.h"
#include "Wire.h"
#include "SPI.h"
//Max acceleration
#define TRUNCATE_AT 30
//Dimensions
#define NUM_AXES 3
#define NUM_VALS 6
// Acceleration Threshold
#define KICK_DEF 20
//Samples per motion detected
#define NUM_SAMPLES 40
//Interval to wait after end of sampling
#define INTERVAL 50
double baseline[NUM_VALS];
double features[NUM_SAMPLES * NUM_VALS];

LSM6DS3 myIMU(I2C_MODE, 0x6A); //Default constructor is I2C, addr 0x6B

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  delay(1000); //relax...
  Serial.println("Processor came out of reset.\n");

  //Call .begin() to configure the IMU
  myIMU.begin();
  //Call to calibration routine
  calibrate();


}


void loop()
{
  //Declare variables
  float ax, ay, az;
  float gx, gy, gz;
  //Obtain parameters
  //Acceleration
  ax = 9.81 * myIMU.readFloatAccelX();
  ay = 9.81 * myIMU.readFloatAccelY();
  az = 9.81 * myIMU.readFloatAccelZ();
  //Gyroscope
  gx = myIMU.readFloatGyroX();
  gy = myIMU.readFloatGyroY();
  gz = myIMU.readFloatGyroZ();

  //Constrain variables
  //constrain up to +-20 and substract baseline values from calibration
  ax = constrain(ax - baseline[0], -TRUNCATE_AT, TRUNCATE_AT);
  ay = constrain(ay - baseline[1], -TRUNCATE_AT, TRUNCATE_AT);
  az = constrain(az - baseline[2], -TRUNCATE_AT, TRUNCATE_AT);

  //Print parameters

/*
  Serial.print(ax);
  Serial.print('\t');
  Serial.print(ay);
  Serial.print('\t');
  Serial.print(az);
  Serial.print('\t');

  Serial.print(gx);
  Serial.print('\t');
  Serial.print(gy);
  Serial.print('\t');
  Serial.println(gz);
 */

  //Call to kick
  if (!kick(ax, ay, az)) {
    delay(10);
    return;
  }
  //If motion is detected
  //Call record routine
  recordIMU();
  //Print sample
  printFeatures();
  delay(700);

}
//Calibration routine
void calibrate() {
  float sumax = 0;
  float sumay = 0;
  float sumaz = 0;
  // Record the first 10 readings as baseline
  for (int i = 0; i <= 100; i++) {
    sumax = sumax + 9.81 * myIMU.readFloatAccelX();
    sumay = sumay + 9.81 * myIMU.readFloatAccelY();
    sumaz = sumaz + 9.81 * myIMU.readFloatAccelZ();
    delay(10);
  }


  baseline[0] = sumax/100;
  baseline[1] = sumay/100;
  baseline[2] = sumaz/100;

}

//If the absolute value of the addition of linear accelerations surpasses the threshold, activates the function
bool kick(float ax, float ay, float az) {
  return (abs(ax) + abs(ay) + abs(az)) > KICK_DEF;
}

//Routine to record sample as  one vector instead of array (Dimensions = 1x NUM_AXES*NUM_SAMPLES)
void recordIMU() {
  float ax, ay, az, gx, gy, gz;
  //Serial.print("aX,aY,aZ,gX,gY,gZ\n");

  for (int i = 0; i < NUM_SAMPLES; i++) {
    ax = 9.81 * myIMU.readFloatAccelX();
    ay = 9.81 * myIMU.readFloatAccelY();
    az = 9.81 * myIMU.readFloatAccelZ();

    gx = myIMU.readFloatGyroX();
    gy = myIMU.readFloatGyroY();
    gz = myIMU.readFloatGyroZ();

    ax = constrain(ax - baseline[0], -TRUNCATE_AT, TRUNCATE_AT);
    ay = constrain(ay - baseline[1], -TRUNCATE_AT, TRUNCATE_AT);
    az = constrain(az - baseline[2], -TRUNCATE_AT, TRUNCATE_AT);
//Record x, y and z components  in groups of 3
    features[i * NUM_VALS + 0] = ax;
    features[i * NUM_VALS + 1] = ay;
    features[i * NUM_VALS + 2] = az;
    features[i * NUM_VALS + 3] = gx;
    features[i * NUM_VALS + 4] = gy;
    features[i * NUM_VALS + 5] = gz;


    delay(INTERVAL);
  }
}
//Print the sample

void printFeatures() {

  const uint16_t numFeatures = sizeof(features) / sizeof(double);
  int counter = 0;

    for (int i = 0; i < numFeatures; i++) {
      counter++;
      Serial.print(features[i]);
      Serial.print(",");
      //Serial.print(i == numFeatures - 1 ? 'n' : ',');

   if (counter%6 == 0){

   //Serial.println();

   }
  }

  Serial.print(" \n");

}
