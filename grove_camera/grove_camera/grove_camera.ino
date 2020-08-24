//  File grove_camera.ino
//  25/7/2020 HN
//  Demo code for using Seeeduino LoRaWAN board to cature jpg format
//  picture from Seeed Grove Serial Camera and save it into SD card on SD Card Shield v4 by pushing the
//  Grove Button to take the a picture
//  Inspired from SerialCamera_DemoCode_CJ-OV528.ino by 
//  8/8/2013 Jack Shao
//  For more details about the product please check http://www.seeedstudio.com/depot/

#include <SPI.h>
#include <SD.h>
#include <arduino.h>

// Camera Config
#define PIC_PKT_LEN    128                  //data length of each read, dont set this too big because ram is limited
#define CAM_ADDR       0
#define CAM_SERIAL     Serial1

// JPEG Resolution => define int jpeg_resolution from camInitialize()
#define JR_160x128   3
#define JR_320x240   5
#define JR_640x480   7

// For Camera and Button
File myFile;                              // define SD card

int picNameNum = 0;                       // picture name
unsigned long picTotalLen = 0;            // picture size
const byte cameraAddr = (CAM_ADDR << 5);  // addr
const int buttonPin = A5;                 // the number of the pushbutton pin

/* 
 *  For MicroML - Feature extraction and image downsizing
 */

// Resizing images and features extraction
#define FRAME_SIZE FRAMESIZE_QQVGA
#define WIDTH 160
#define HEIGHT 120
#define BLOCK_SIZE 5
#define W (WIDTH / BLOCK_SIZE)
#define H (HEIGHT / BLOCK_SIZE)
#define THRESHOLD 127

//double features[H*W] = { 0 };

//bool camInitialize();
//bool capture_still();
//void print_features();
//void classify();

/*********************************************************************/

void setup()
{
    Serial.begin(9600);
    CAM_SERIAL.begin(9600);       //cant be faster than 9600, maybe difference with diff board.
    
    while (!Serial) 
    {
    ; // wait for serial port to connect. Needed for native USB port only
    }
  
    pinMode(buttonPin, INPUT);    // initialize the pushbutton pin as an input
    Serial.print("\r\n Initializing SD card ....");
    pinMode(4,OUTPUT);          // CS pin of SD Card Shield

    if (!SD.begin(4)) 
    {
        Serial.println("\r\n SD card initializing failed !! ");
        return;
    }
    
    Serial.println("\r\n SD card initializing done !!! ");
    camInitialize();
}

/*********************************************************************/

void loop()
{
    int n = 0;
    while(1)
    {
        Serial.println("\r\n Press the button to take a picture");
        while (digitalRead(buttonPin) == LOW);      //wait for buttonPin status to HIGH
        
        if(digitalRead(buttonPin) == HIGH){
            delay(20);                               //Debounce
            if (digitalRead(buttonPin) == HIGH)
            {
                Serial.print("\r\n Begin to take picture");
                delay(200);
                if (n == 0); 
                //Capture();
                resPreview();
                Serial.println("\r\n Saving picture....");
                GetData();       
            }
            Serial.print("\r\n Successfully saved picture on SD card !!! Picture number: ");
            Serial.println(n);
            n++ ;
        }
    }

//    if (!capture_still()) 
//    {
//        Serial.println("Failed capture");
//        delay(2000);
//
//        return;
//    }
//
//    // during training
//    print_features();
//    // durgin inference
//    // classify();
//    delay(3000);
}

/*********************************************************************/

void clearRxBuf()
{
    while (CAM_SERIAL.available())
    {
        CAM_SERIAL.read();
    }
}

/*********************************************************************/

void sendCmd(char cmd[], int cmd_len)
{
    for (char i = 0; i < cmd_len; i++) CAM_SERIAL.print(cmd[i]);
}

/*********************************************************************/

int readBytes(char *dest, int len, unsigned int timeout)
{
  int read_len = 0;
  unsigned long t = millis();
  while (read_len < len)
  {
    while (CAM_SERIAL.available() < 1)
    {
      if ((millis() - t) > timeout)
      {
        return read_len;
      }
    }
    *(dest + read_len) = CAM_SERIAL.read();
    //Serial.write(*(dest+read_len));
    read_len++;
  }
  return read_len;
}

/*********************************************************************/

void preCapture(int jpeg_resolution) // This fucntion is to define the image resolution that you want before capurting
{   
    // Set Image Resolution command
    char cmd[] = { 0xaa, 0x01 | cameraAddr, 0x00, 0x00, 0x03, jpeg_resolution };
    unsigned char resp[6];

    Serial.setTimeout(100);
    
    while (1)
    {
        clearRxBuf();
        sendCmd(cmd, 6);
        if (readBytes((char *)resp, 6, 100) != 6) continue; 
        if (resp[0] == 0xaa && resp[1] == (0x0e | cameraAddr) && resp[2] == 0x01 && resp[4] == 0 && resp[5] == 0) break;
    }
}

/*********************************************************************/

void camInitialize()
{
    // SYNC command to make connection between camera and Seeeduino
    char cmd[] = {0xaa,0x0d|cameraAddr,0x00,0x00,0x00,0x00} ;
    unsigned char resp[6];

    Serial.print("\r\n Initializing Camera ...");
    
    Serial.setTimeout(100);
    
    while (1)
    {
        //clearRxBuf();
        sendCmd(cmd,6);
        if (readBytes((char *)resp, 6, 300) != 6) // at least 300ms delay
        {
            Serial.print(".");
            continue;
        }
        if (resp[0] == 0xaa && resp[1] == (0x0e | cameraAddr) && resp[2] == 0x0d && resp[4] == 0 && resp[5] == 0)
        {
            if (readBytes((char *)resp, 6, 300) != 6) continue; // // at least 300ms delay
            if (resp[0] == 0xaa && resp[1] == (0x0d | cameraAddr) && resp[2] == 0 && resp[3] == 0 && resp[4] == 0 && resp[5] == 0) break;
        }
    }
    
    cmd[1] = 0x0e | cameraAddr;
    cmd[2] = 0x0d;
    sendCmd(cmd, 6);

    // Define the image size using the above definitions
    preCapture(JR_160x128);

    // Set Image Package Type Size command
    char cmd2[] = { 0xaa, 0x06 | cameraAddr, 0x08, PIC_PKT_LEN & 0xff, (PIC_PKT_LEN>>8) & 0xff ,0};
    
    Serial.setTimeout(100);
    
    while (1)
    {
        clearRxBuf();
        sendCmd(cmd2, 6);
        if (readBytes((char *)resp, 6, 100) != 6) continue;
        if (resp[0] == 0xaa && resp[1] == (0x0e | cameraAddr) && resp[2] == 0x06 && resp[4] == 0 && resp[5] == 0) break;
    }
    
    Serial.println("\r\n Camera initialization done !! ");
}

/*********************************************************************/

void Capture()
{   
    // Get Picture command, 1 = snapshot, 2 = preview picture, 3 = JPEG Preview Picture
    char cmd[] = { 0xaa, 0x04 | cameraAddr, 0x01, 0x00, 0x00, 0x00};
    unsigned char resp[6];
    
    while (1)
    {
        clearRxBuf();
        sendCmd(cmd, 6);
        if (readBytes((char *)resp, 6, 100) != 6) continue;
        if (resp[0] == 0xaa && resp[1] == (0x0e | cameraAddr) && resp[2] == 0x04 && resp[4] == 0 && resp[5] == 0)
        {
          
            Serial.setTimeout(100);
            
            if (readBytes((char *)resp, 6, 300) != 6) // at least 300ms delay
            {
                continue;
            }
            if (resp[0] == 0xaa && resp[1] == (0x0a | cameraAddr) && resp[2] == 0x01)
            {
                picTotalLen = (resp[2]) | (resp[4] << 8) | (resp[5] << 16);
                Serial.print("\r\n Size of the image: ");
                Serial.print(picTotalLen);
                Serial.println(" bytes");
                break;
            }
        }
    }
}

/*********************************************************************/

void resPreview()
{
    // Get Picture resolution info command, 1 = snapshot, 2 = preview picture, 3 = JPEG Preview Picture
    char cmd[] = { 0xaa, 0x04 | cameraAddr, 0x02, 0x00, 0x00, 0x00};
    unsigned char resp[6];
    unsigned long dimension = 0;
    
    while (1)
    {
        clearRxBuf();
        sendCmd(cmd, 6);
        if (readBytes((char *)resp, 6, 100) != 6) continue;
        if (resp[0] == 0xaa && resp[1] == (0x0e | cameraAddr) && resp[2] == 0x04 && resp[4] == 0 && resp[5] == 0)
        {
          
            Serial.setTimeout(100);
            
            if (readBytes((char *)resp, 6, 300) != 6) // at least 300ms delay
            {
                continue;
            }
            if (resp[0] == 0xaa && resp[1] == (0x0a | cameraAddr) && resp[2] == 0x02)
            {
                dimension = (resp[3]) | (resp[4] << 8) | (resp[5] << 16);
                Serial.print("\r\n Dimension of the image: ");
                Serial.print(dimension);
                //Serial.println(" bytes");
                break;
            }
        }
    }
}
/*********************************************************************/

/**
 * Capture image and do down-sampling
 */



//bool capture_still() {
//    camera_fb_t *frame = esp_camera_fb_get();
//
//    if (!frame)
//        return false;
//
//    // reset all features
//    for (size_t i = 0; i < H * W; i++)
//        features[i] = 0;
//
//    // for each pixel, compute the position in the downsampled image
//    for (size_t i = 0; i < frame->len; i++) {
//        const uint16_t x = i % WIDTH;
//        const uint16_t y = floor(i / WIDTH);
//        const uint8_t block_x = floor(x / BLOCK_SIZE);
//        const uint8_t block_y = floor(y / BLOCK_SIZE);
//        const uint16_t j = block_y * W + block_x;
//
//        features[j] += frame->buf[i];
//    }
//
//    // apply threshold
//    for (size_t i = 0; i < H * W; i++) {
//        features[i] = (features[i] / (BLOCK_SIZE * BLOCK_SIZE) > THRESHOLD) ? 1 : 0;
//    }
//
//    return true;
//}

/*********************************************************************/

void GetData()
{
    unsigned int pktCnt = (picTotalLen) / (PIC_PKT_LEN - 6);      // picture count
    if ((picTotalLen % (PIC_PKT_LEN-6)) != 0) pktCnt += 1;
    
    char cmd[] = { 0xaa, 0x0e | cameraAddr, 0x00, 0x00, 0x00, 0x00 };
    unsigned char pkt[PIC_PKT_LEN];

    char picName[] = "IMG00.jpg"; // "IMAGE00.jpg";
    picName[3] = picNameNum/10 + '0';
    picName[4] = picNameNum%10 + '1';

    if (SD.exists(picName))
    {
        SD.remove(picName);
    }

    myFile = SD.open(picName, FILE_WRITE);
    if(!myFile){
        Serial.print("\nmyFile open fail...");
    }
    else{
      
        Serial.setTimeout(100);
        
        for (unsigned int i = 0; i < pktCnt; i++)
        {
            cmd[4] = i & 0xff;
            cmd[5] = (i >> 8) & 0xff;

            int retry_cnt = 0;
            retry:
            delay(10);
            clearRxBuf();
            sendCmd(cmd, 6);
            uint16_t cnt = readBytes((char *)pkt, PIC_PKT_LEN, 200); // at least 200ms delay

            unsigned char sum = 0;
            for (int y = 0; y < cnt - 2; y++)
            {
                sum += pkt[y];
            }

            Serial.print("\r\n Sum: ");
            Serial.print(sum);
            
            if (sum != pkt[cnt-2])
            {
                if (++retry_cnt < 100) goto retry;
                else break;
            }
            
            myFile.write((const uint8_t *)&pkt[4], cnt-6);
            //if (cnt != PIC_PKT_LEN) break;
        }
        cmd[4] = 0xf0;
        cmd[5] = 0xf0;
        sendCmd(cmd, 6);

    }
    myFile.close();
    picNameNum ++;
}

/*********************************************************************/

//void print_features() {
//    for (size_t i = 0; i < H * W; i++) {
//        Serial.print(features[i]);
//
//        // don't print "," after the last element
//        if (i != H * W - 1)
//            Serial.print(',');
//    }
//
//    Serial.println();
//}

/*********************************************************************/

//void classify() {
//    Serial.print("Number: ");
//    Serial.println(classIdxToName(predict(features)));
//}

/*********************************************************************/
