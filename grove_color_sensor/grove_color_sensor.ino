#include <Wire.h>
#include <TensorFlowLite.h> // TensorflowLite library

#include "tensorflow/lite/experimental/micro/kernels/all_ops_resolver.h"
#include "tensorflow/lite/experimental/micro/micro_error_reporter.h"
#include "tensorflow/lite/experimental/micro/micro_interpreter.h"
#include "tensorflow/lite/schema/schema_generated.h"
#include "tensorflow/lite/version.h"

#include "model.h" // Train model
#include "Adafruit_TCS34725.h" // Grove Color Sensor library
Adafruit_TCS34725 tcs = Adafruit_TCS34725(TCS34725_INTEGRATIONTIME_50MS, TCS34725_GAIN_4X);

// global variables used for TensorFlow Lite (Micro)
tflite::MicroErrorReporter tflErrorReporter;

// pull in all the TFLM ops, you can remove this line and
// only pull in the TFLM ops you need, if would like to reduce
// the compiled size of the sketch.
tflite::ops::micro::AllOpsResolver tflOpsResolver;

const tflite::Model* tflModel = nullptr;
tflite::MicroInterpreter* tflInterpreter = nullptr;
TfLiteTensor* tflInputTensor = nullptr;
TfLiteTensor* tflOutputTensor = nullptr;

// Create a static memory buffer for TFLM, the size may need to
// be adjusted based on the model you are using
constexpr int tensorArenaSize = 8 * 1024;
byte tensorArena[tensorArenaSize];

// array to map gesture index to a name
const char* CLASSES[] = {
  "Empty", 
  "Lemon", 
  "Lime", 
  "Tomato"
};

#define NUM_CLASSES (sizeof(CLASSES) / sizeof(CLASSES[0]))

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

  // get the TFL representation of the model byte array
  tflModel = tflite::GetModel(model);
  if (tflModel->version() != TFLITE_SCHEMA_VERSION) {
    Serial.println("Model schema mismatch!");
    while (1);
  }

  // Create an interpreter to run the model
  tflInterpreter = new tflite::MicroInterpreter(tflModel, tflOpsResolver, tensorArena, tensorArenaSize, &tflErrorReporter);

  // Allocate memory for the model's input and output tensors
  tflInterpreter->AllocateTensors();

  // Get pointers for the model's input and output tensors
  tflInputTensor = tflInterpreter->input(0);
  tflOutputTensor = tflInterpreter->output(0);

  Serial.println("Type 'help' in the command box to see the options");
  Serial.println();
}

void loop() 
{
  if (!Serial.available()) 
  {
    delay(100);
    return;
  }

  String command = Serial.readStringUntil('\n');

    if (command == "help") 
    {
        Serial.println("Available commands:");
        Serial.println("\tcapture: start capturing color sensor data");
        Serial.println("\tpredict: classify a new sample");
        Serial.println();
    }
    else if (command == "capture") 
    {
        int numSamples;
        Serial.println("How many samples would you like to record? ");
        numSamples = readSerialNumber();

        Serial.print("Starting to capture ");
        Serial.print(numSamples);
        Serial.println(" samples .... ");
        Serial.println("*2 seconds delay is applied between each record for rotating the object* ");
        Serial.println();
        
        // Print the header
        Serial.println("Red,Green,Blue");        
        for (int i = 0; i < numSamples; i++) 
        {
            float r, g, b;
            readColorSensorData(&r, &g, &b);

            // Print the data in CSV format
            Serial.print(r, 3);
            Serial.print(',');
            Serial.print(g, 3);
            Serial.print(',');
            Serial.print(b, 3);
            Serial.println();
            
            delay(1000); // Give users 2 seconds to rotate object's surface
        }
        Serial.println("Done");
        Serial.println();
    }
    else if (command == "predict") 
    {
        float r, g, b;
        readColorSensorData(&r, &g, &b);

        // input sensor data to tensorflow
        tflInputTensor->data.f[0] = r;
        tflInputTensor->data.f[1] = g;
        tflInputTensor->data.f[2] = b;

        // run inferencing
        TfLiteStatus invokeStatus = tflInterpreter->Invoke();
        
        if (invokeStatus != kTfLiteOk) 
        {
          Serial.println("Invoke failed!");
          while (1);
          return;
        }

        // output results
        for (int i = 0; i < NUM_CLASSES; i++) 
        {
          Serial.print(CLASSES[i]);
          Serial.print(" ");
          Serial.print(int(tflOutputTensor->data.f[i] * 100));
          Serial.print("%\n");
        }
        Serial.println();
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
 * Read color sensor data as RGB
 * @param x
 */
 
void readColorSensorData(float *r, float *g, float *b)
{
  // Define RGB and Sum (for CSV) float
  float red, green, blue, sum;

  // Read the color data
  tcs.setInterrupt(false);  // turn on LED
  delay(60);  // takes 60ms to read
  tcs.getRGB(&red, &green, &blue);
  sum = red + green + blue;
  tcs.setInterrupt(true);  // turn off LED

  *r = red / sum;
  *g = green / sum;
  *b = blue / sum;

  return;
}
