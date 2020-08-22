// Include the library
#include <JPEGDecoder.h>

// Define Slave Select pin
#define SD_CS  4

// Define root directory from SD card
File root;

void setup() 
{
  
  // Start the SD
  if(!SD.begin(SD_CS)) 
  {
    // If the SD can't be started, loop forever
    //Serial.println("SD failed or not present!");
    Serial.println("SD failed or not present!");
    while(1);
  }

  //Serial.println("SD recognized!");
  Serial.println("SD recognized!");

  // Open the root directory
  root = SD.open("/");

  // List all files in root directory on SD card
  printDirectory(root, 0);

  Serial.println("Now it is the decoding part");
  decoder();
}

// Function to start Serial of 
void serial_setup()
{
  // Set pin 13 to output, otherwise SPI might hang
  pinMode(13, OUTPUT);

  // Begin Serial port for communication with PC
  Serial.begin(9600);
  SerialUSB.begin(9600);
  Serial1.begin(9600);

  // Wait for Serial Monitor to connect. Needed for native USB port boards only:
  while (!Serial);
  while (!SerialUSB);
  while (!Serial1);
}

// Function to decode images in SD card
void decoder()
{
  Serial.println("Start of decoding process");
  
  // Open the root directory
  root = SD.open("/");

  // Wait for the PC to signal
  //while (!Serial.available());
  //while (!Serial1.available());
  //while (!SerialUSB.available());
  
  // Send all files on the SD card
  while(true) 
  {     
      // Open the next file
      File jpgFile = root.openNextFile();
  
      // We have sent all files
      if(!jpgFile) 
      {
        //no more files
        break;
      }

      Serial.println(jpgFile);

      // Decode the JPEG file
      JpegDec.decodeSdFile(jpgFile);
  
  
      // Create a buffer for the packet
      char dataBuff[240];
  
      // Fill the buffer with zeros
      initBuff(dataBuff);
  
      // Create a header packet with info about the image
      String header = "$ITHDR,";
      header += JpegDec.width;
      header += ",";
      header += JpegDec.height;
      header += ",";
      header += JpegDec.MCUSPerRow;
      header += ",";
      header += JpegDec.MCUSPerCol;
      header += ",";
      header += jpgFile.name();
      header += ",";
      header.toCharArray(dataBuff, 240);
  
      // Send the header packet
      for(int j=0; j<240; j++) 
      {
        Serial.write(dataBuff[j]);
      }
  
      // Pointer to the current pixel
      uint16_t *pImg;
  
      // Color of the current pixel
      uint16_t color;
  
      // Create a data packet with the actual pixel colors
      strcpy(dataBuff, "$ITDAT");
      uint8_t i = 6;
  
      // Repeat for all MCUs in the image
      while(JpegDec.read()) 
      {
        // Save pointer the current pixel
        pImg = JpegDec.pImage;
  
        // Get the coordinates of the MCU we are currently processing
        int mcuXCoord = JpegDec.MCUx;
        int mcuYCoord = JpegDec.MCUy;
  
        // Get the number of pixels in the current MCU
        uint32_t mcuPixels = JpegDec.MCUWidth * JpegDec.MCUHeight;
  
        // Repeat for all pixels in the current MCU
        while(mcuPixels--) 
        {
          // Read the color of the pixel as 16-bit integer
          color = *pImg++;
          
          // Split it into two 8-bit integers
          dataBuff[i] = color >> 8;
          dataBuff[i+1] = color;
          i += 2;
  
          // If the packet is full, send it
          if(i == 240) 
          {
            for(int j=0; j<240; j++) 
            {
              Serial.write(dataBuff[j]);
            }
            i = 6;
          }
  
          // If we reach the end of the image, send a packet
          if((mcuXCoord == JpegDec.MCUSPerRow - 1) && 
            (mcuYCoord == JpegDec.MCUSPerCol - 1) && 
            (mcuPixels == 1)) 
            {        
            // Send the pixel values
              for(int j=0; j<i; j++) 
              {
                Serial.write(dataBuff[j]);
              }
              
              // Fill the rest of the packet with zeros
              for(int k=i; k<240; k++) 
              {
                int zero = 0;
                Serial.write(zero);
              }
            }
        }
      }
  }
  Serial.println("\r\n Closed root directory");
  // Safely close the root directory
  root.close();
  
}

// Function to fill the packet buffer with zeros
void initBuff(char* buff) 
{
  for(int i = 0; i < 240; i++) {
    buff[i] = 0;
  }
}

// Function to read and list all datas in SD card
void printDirectory(File dir, int numTabs) 
{
  while (true) 
  {

    File entry =  dir.openNextFile();
    if (! entry) 
    {
      // no more files
      break;
    }
    for (uint8_t i = 0; i < numTabs; i++) 
    {
      Serial.print('\t');
    }
    Serial.print(entry.name());
    if (entry.isDirectory()) 
    {
      Serial.println("/");
      printDirectory(entry, numTabs + 1);
    } 
    else 
    {
      // files have sizes, directories do not
      Serial.print("\t\t");
      Serial.println(entry.size(), DEC);
    }
    entry.close();
  }
}

void loop() 
{
  // Nothing here
  // We don't need to send the same images over and over again
}
