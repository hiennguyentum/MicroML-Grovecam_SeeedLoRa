#include <EloquentSVMSMO.h>
#include "Adafruit_TCS34725.h"

#define MAX_TRAINING_SAMPLES 20
#define FEATURES_DIM 3

using namespace Eloquent::ML;

int numSamples;
float X_train[MAX_TRAINING_SAMPLES][FEATURES_DIM];
int y_train[MAX_TRAINING_SAMPLES];
SVMSMO<FEATURES_DIM> classifier(linearKernel);

Adafruit_TCS34725 tcs = Adafruit_TCS34725(TCS34725_INTEGRATIONTIME_50MS, TCS34725_GAIN_4X);

void setup() {
    // Initizaling the board and color sensor
    digitalWrite(38, HIGH);
    Serial.begin(9600);
    while (!Serial) {};
    Serial.println("Color Sensor TCS34725 Connected!");

    if (tcs.begin()) 
    {
      Serial.println("Found sensor");
    } 
    else 
    {
      Serial.println("No TCS34725 found ... check your connections");
      while (1); // halt!
    }
    
    classifier.setC(5);
    classifier.setTol(1e-5);
    classifier.setMaxIter(10000);
}

void loop() {
    if (!Serial.available()) {
        delay(100);
        return;
    }

    String command = Serial.readStringUntil('\n');

    if (command == "help") {
        Serial.println("Available commands:");
        Serial.println("\tfit: train the classifier on a new set of samples");
        Serial.println("\tpredict: classify a new sample");
        Serial.println("\tinspect: print X_train and y_train");
    }
    else if (command == "fit") {
        Serial.println("How many samples will you record? ");
        numSamples = readSerialNumber();

        for (int i = 0; i < numSamples; i++) {
            Serial.print(i + 1);
            Serial.print("/");
            Serial.print(numSamples);
            Serial.println(" Which class does the sample belongs to, 1 or -1?");
            y_train[i] = readSerialNumber() > 0 ? 1 : -1;
            getFeatures(X_train[i]);
        }

        Serial.print("Start training... ");
        classifier.fit(X_train, y_train, numSamples);
        Serial.println("Done");
    }
    else if (command == "predict") {
        int label;
        float x[FEATURES_DIM];

        getFeatures(x);
        Serial.print("Predicted label is ");
        Serial.println(classifier.predict(X_train, x));
    }
    else if (command == "inspect") {
        for (int i = 0; i < numSamples; i++) {
            Serial.print("[");
            Serial.print(y_train[i]);
            Serial.print("] ");

            for (int j = 0; j < FEATURES_DIM; j++) {
                Serial.print(X_train[i][j]);
                Serial.print(", ");
            }

            Serial.println();
        }
    }
}

/**
 *
 * @return
 */
int readSerialNumber() {
    while (!Serial.available()) delay(1);

    return Serial.readStringUntil('\n').toInt();
}

/**
 * Get features for new sample
 * @param x
 */
void getFeatures(float x[FEATURES_DIM]) 
{
    readColorSensor(x);

    for (int i = 0; i < FEATURES_DIM; i++) {
        Serial.print(x[i]);
        Serial.print(", ");
    }

    Serial.println();
}

void readColorSensor(float x[2]) 
{
    // Define RGB (for CSV) float
    float red, green, blue;
//    tcs.setInterrupt(false);  // turn on LED
//    delay(60);  // takes 60ms to read
    tcs.getRGB(&red, &green, &blue);
//    tcs.setInterrupt(true);  // turn off LED

    x[0] = red;
    x[1] = green;
    x[2] = blue;
}
