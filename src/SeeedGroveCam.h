/*
  SeeedGroveCam.h - Library for Seeed Grove Camera .
  Created by HN, July 25, 2020.
  Released into the public domain.
*/

#pragma once

#ifndef SeeedGroveCam_h
#define SeeedGroveCam_h

#include "Arduino.h"
#include <SPI.h>

#define PIC_PKT_LEN		128			/*!< Data length of each read, dont set this too big because ram is limited *///
#define CAM_ADDR		0x00		/*!< To start the camera */
#define CAM_SERIAL		Serial1		/*!< Serial port for Grove Camera */

#define JR_160x128		0x03		/*!< JPEG Resolution of 160x128 */
#define JR_320x240		0x05		/*!< JPEG Resolution of 320x240 */
#define JR_640x480		0x07		/*!< JPEG Resolution of 640x480 */

class SeeedGroveCam
{
public:
	SeeedGroveCam();
	void clearRxBuf();
	void sendCmd(char cmd[], int cmd_len);
	void preCapture(int jpeg_resolution);


private:
	int readBytes(char *dest, int len, unsigned timeout);
	int _pin;
};

#endif