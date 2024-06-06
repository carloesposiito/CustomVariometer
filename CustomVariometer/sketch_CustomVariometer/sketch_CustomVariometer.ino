#pragma region "INCLUDES"

#include <Wire.h>
#include <SPI.h>
#include <Adafruit_BMP280.h>
#include "CuteBuzzerSounds.h"
#include <math.h>
#include <Adafruit_SSD1306.h>

#pragma endregion

/***************************************************************************
  By Carlo Esposito, 2024, Github: @carloesposiito
 ***************************************************************************/

/***************************************************************************
 *                        START EDITABLE PARAMETERS                           
 ***************************************************************************/

// Enable/disable display output
bool const USE_DISPLAY = true;

// Enable/disable low battery check
int const LOW_BATTERY_CHECK = false;

// REFERENCE PRESSURE
// A reference pressure is needed to calculate altitude, then there are 3 ways:
// 1. Using default pressure (1013.25 hPa) as reference;
// 2. Using relative pressure as reference, detected when variometer starts (set to true to enable);
bool const USE_RELATIVE_PRESSURE = false; 
// 3. Using custom pressure: in this case please insert sea pressure value according to your local forecast.
// A more accurate CUSTOM_PRESSURE_HPA value gives more accurate readings! (set to true to enable and adjust value as preferred);
bool const USE_CUSTOM_PRESSURE = false;
float const CUSTOM_PRESSURE_HPA = 1013.25;

// Speed variation in m/s while going up that triggers alarm.
float liftAlarm = 0.3;

// Speed variation in m/s while going down that triggers alarm.
float sinkAlarm = -2;

/***************************************************************************
 *                           END EDITABLE PARAMETERS                           
 ***************************************************************************/

#pragma region "Variables"

bool const DEBUG = true;

// BMP280 parameters for SPI on ESP32 Super Mini
#define BMP_SCK 4
#define BMP_MISO 5
#define BMP_MOSI 6
#define BMP_CS 7
Adafruit_BMP280 bmp(BMP_CS, BMP_MOSI, BMP_MISO,  BMP_SCK);

// OLED display parameters
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT);
bool displayWorking = false;
int const BOOT_LOGO_DURATION = 2500;

// Buzzer
int const BUZZER = 3;

// Sensor interval detection variables
int const MS_DETECTION_INTERVAL = 1000;
unsigned long currentMillis = 0;
unsigned long previousDetectionMillis = 0;

// Battery interval detection variabels
int const MS_BATTERY_CHECK_INTERVAL = 10000;
unsigned long previousBatteryMillis = 0;

// Altitude
float startingAltitude = 0;
float previousAltitude = 0;
float referencePressure = 0;

// Temperature
int currentTemperature = 0;

// Battery
const int BATTERY_PIN = 2;

#pragma region "Images"

const unsigned char BOOT_LOGO [] PROGMEM = {
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x7f, 0xfe, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0f, 0xff, 0xff, 0xf0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0xff, 0xff, 0xff, 0xff, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0xff, 0xff, 0xff, 0xff, 0xc0, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0xff, 0xff, 0xff, 0xff, 0xc0, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0xff, 0xff, 0xff, 0xff, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0xff, 0xff, 0xff, 0xff, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x7f, 0xff, 0xff, 0xfe, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x7f, 0xff, 0xff, 0xfe, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x3f, 0x80, 0x01, 0xfc, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x08, 0x00, 0x00, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x20, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xc0, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x40, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x63, 0xc6, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x63, 0xc6, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x73, 0xce, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x33, 0xcc, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x39, 0x9c, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x3f, 0xfc, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1f, 0xf8, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x07, 0xe0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x07, 0xe0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x07, 0xe0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x07, 0xe0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x07, 0xe0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x07, 0xe0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0f, 0xf0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0e, 0xf0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0e, 0x70, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0c, 0x70, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0c, 0x70, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0c, 0x70, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0c, 0x70, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0c, 0x30, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x0f, 0xa4, 0xc4, 0x02, 0x00, 0x1e, 0x00, 0x00, 0x40, 0x00, 0x00, 0x00, 0x16, 0xc0, 0x00, 
	0x00, 0x10, 0x04, 0x84, 0x06, 0x00, 0x61, 0x00, 0x00, 0x40, 0x00, 0x00, 0x00, 0x00, 0x80, 0x00, 
	0x00, 0x20, 0x2e, 0x85, 0x17, 0x90, 0x5e, 0x9c, 0xf7, 0x4e, 0x3b, 0xbe, 0x73, 0xb5, 0xee, 0x00, 
	0x00, 0x20, 0x6c, 0xfd, 0x16, 0xd0, 0x96, 0xb1, 0x36, 0x5b, 0x4a, 0x32, 0xda, 0x24, 0x92, 0x00, 
	0x00, 0x27, 0x49, 0x8d, 0x34, 0xc0, 0xa4, 0xa3, 0x24, 0xd1, 0xfb, 0x22, 0x8a, 0x24, 0x92, 0x00, 
	0x00, 0x21, 0x49, 0x09, 0x24, 0xc0, 0xa5, 0xa3, 0x2c, 0xb3, 0xc1, 0xa3, 0x99, 0x2d, 0xb2, 0x00, 
	0x00, 0x33, 0x49, 0x0b, 0x6c, 0xa0, 0xbf, 0x23, 0x68, 0x92, 0xc0, 0xa4, 0x91, 0xe9, 0x32, 0x00, 
	0x00, 0x1e, 0x6d, 0x09, 0xef, 0x20, 0x40, 0x39, 0xe8, 0xdc, 0x77, 0xfc, 0xe7, 0x3d, 0xdc, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x3e, 0x00, 0x00, 0x00, 0x00, 0x60, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

const unsigned char MAIN_SCREEN [] PROGMEM = {
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x07, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xe0, 0x00, 
	0x00, 0x38, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1c, 0x00, 
	0x00, 0xc0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x00, 
	0x01, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x80, 
	0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x40, 
	0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x20, 
	0x0c, 0x00, 0x00, 0x00, 0xe0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x30, 
	0x08, 0x00, 0x00, 0x01, 0xf0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x10, 
	0x10, 0x00, 0x00, 0x03, 0xf8, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x08, 
	0x10, 0x00, 0x00, 0x07, 0xf8, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x08, 
	0x10, 0x00, 0x00, 0x07, 0xf8, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x08, 
	0x20, 0x00, 0x00, 0x00, 0xe0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x04, 
	0x20, 0x00, 0x00, 0x00, 0xe0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x04, 
	0x20, 0x00, 0x00, 0x00, 0xe0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x04, 
	0x20, 0x00, 0x00, 0x00, 0xe0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x04, 
	0x20, 0x00, 0x00, 0x00, 0xe0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x04, 
	0x20, 0x00, 0x00, 0x00, 0xe0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x04, 
	0x20, 0x00, 0x1c, 0x00, 0xe0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x04, 
	0x20, 0x00, 0x3e, 0x00, 0xe0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x04, 
	0x20, 0x00, 0x3e, 0x00, 0xe0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x04, 
	0x20, 0x00, 0x7f, 0x00, 0xe0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x04, 
	0x20, 0x00, 0xff, 0x80, 0xe0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x04, 
	0x20, 0x01, 0xff, 0x80, 0xe0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x04, 
	0x20, 0x01, 0xff, 0xc0, 0xe0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x04, 
	0x20, 0x03, 0xff, 0xe7, 0xf8, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x04, 
	0x10, 0x07, 0xff, 0xf7, 0xf8, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x08, 
	0x10, 0x0f, 0xff, 0xf3, 0xf8, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x08, 
	0x10, 0x0f, 0xff, 0xf9, 0xf0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x08, 
	0x08, 0x1f, 0xff, 0xfc, 0xe0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x10, 
	0x0c, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x30, 
	0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x20, 
	0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x40, 
	0x01, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x80, 
	0x00, 0xc0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x00, 
	0x00, 0x38, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1c, 0x00, 
	0x00, 0x07, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xe0, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x80, 0x01, 0xff, 0xff, 0xff, 0xff, 0x00, 
	0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x60, 0x06, 0x00, 0x00, 0x00, 0x00, 0xc0, 
	0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x10, 0x08, 0x00, 0x00, 0x00, 0x00, 0x20, 
	0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x08, 0x10, 0x00, 0x00, 0x00, 0x00, 0x10, 
	0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x04, 0x20, 0x1e, 0x38, 0x00, 0x00, 0x08, 
	0x10, 0x00, 0xce, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x04, 0x20, 0x32, 0x00, 0x00, 0x00, 0x08, 
	0x20, 0x01, 0xef, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x40, 0x33, 0x00, 0x00, 0x00, 0x04, 
	0x20, 0x03, 0x8f, 0xc0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x40, 0x33, 0x38, 0x00, 0x00, 0x04, 
	0x20, 0x0c, 0x0c, 0xe0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x40, 0x33, 0x00, 0x00, 0x00, 0x04, 
	0x20, 0x08, 0x0c, 0xf0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x40, 0x33, 0x00, 0x00, 0x00, 0x04, 
	0x20, 0x10, 0x18, 0x78, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x40, 0x33, 0x00, 0x00, 0x00, 0x04, 
	0x20, 0x00, 0x18, 0x78, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x40, 0x61, 0x80, 0x00, 0x00, 0x04, 
	0x20, 0x00, 0x38, 0x38, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x40, 0x40, 0x80, 0x00, 0x00, 0x04, 
	0x20, 0x20, 0x38, 0x3c, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x40, 0x40, 0x80, 0x00, 0x00, 0x04, 
	0x20, 0x20, 0x38, 0x3c, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x40, 0x40, 0x80, 0x00, 0x00, 0x04, 
	0x20, 0x20, 0x30, 0x3c, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x40, 0x40, 0x80, 0x00, 0x00, 0x04, 
	0x10, 0x00, 0x00, 0x1c, 0x00, 0x00, 0x00, 0x00, 0x00, 0x04, 0x20, 0x61, 0x80, 0x00, 0x00, 0x08, 
	0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x04, 0x20, 0x3f, 0x00, 0x00, 0x00, 0x08, 
	0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x08, 0x10, 0x00, 0x00, 0x00, 0x00, 0x10, 
	0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x10, 0x08, 0x00, 0x00, 0x00, 0x00, 0x20, 
	0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x60, 0x06, 0x00, 0x00, 0x00, 0x00, 0xc0, 
	0x00, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x80, 0x01, 0xff, 0xff, 0xff, 0xff, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

#pragma endregion

#pragma endregion

#pragma region "Functions"

#pragma region "BMP280"

void InitSensor()
{
  if (bmp.begin())
  {
    // Load default settings from datasheet
    bmp.setSampling(Adafruit_BMP280::MODE_NORMAL,     // Operating Mode
                    Adafruit_BMP280::SAMPLING_X2,     // Temperature oversampling
                    Adafruit_BMP280::SAMPLING_X16,    // Pressure oversampling
                    Adafruit_BMP280::FILTER_X16,      // Filtering
                    Adafruit_BMP280::STANDBY_MS_500); // Standby time

    // Reference pressure
    String choice;
    if (USE_RELATIVE_PRESSURE && !USE_CUSTOM_PRESSURE)
    {
      referencePressure = (bmp.readPressure() / 100);
      choice = "relative";
    }
    else if (!USE_RELATIVE_PRESSURE && USE_CUSTOM_PRESSURE)
    {
      referencePressure = CUSTOM_PRESSURE_HPA;
      choice = "custom";
    }
    else
    {
      referencePressure = 1013.25;
      choice = "default";
    }

    startingAltitude = bmp.readAltitude(referencePressure);
    previousAltitude = startingAltitude;

    // Connected
    if (DEBUG)
    {
      Serial.println("Sensor initialized!");
      Serial.println("Using " + choice + " reference pressure: " + (String)referencePressure + " hPa");
      Serial.println("");
    }
    cute.play(S_CONNECTION);
  }
  else
  {
    // Disconnected then stop program
    if (DEBUG) Serial.print("Sensor not initialized!");
    cute.play(S_DISCONNECTION);
    while (true) { }
  }
}

void ReadSensor()
{
  // Control flow through millis function
  // Returns milliseconds passed from controller startup
  // In this way it's possible to perform actions every certain interval of time
  currentMillis = millis();
   
  if (currentMillis - previousDetectionMillis >= MS_DETECTION_INTERVAL)
  {
    previousDetectionMillis = currentMillis;

    // Calculate altitude data
    float currentAltitude = bmp.readAltitude(referencePressure);
    float relativeAltitude = currentAltitude - startingAltitude;
    float altitudeVariation_meters = currentAltitude - previousAltitude;
    float altitudeVariation_speed = altitudeVariation_meters  * (1000 / MS_DETECTION_INTERVAL);
    previousAltitude = currentAltitude;

    // Read temperature
    currentTemperature = (int)bmp.readTemperature();

    // Print to console if debug mode
    if (DEBUG)
    {
      Serial.println("Pressure: " + (String)(bmp.readPressure() / 100) + " hPa");
      Serial.println("Altitude: " + (String)currentAltitude + " m [Δh speed: " + (String)altitudeVariation_speed + " m/s]");
      Serial.println("Temperature: " + (String)currentTemperature + " °C");
      Serial.println("");
    }

    DisplayData((String)currentAltitude, (String)relativeAltitude, (String)altitudeVariation_speed, (String)currentTemperature);
    
    // Lift beep beep
    if (altitudeVariation_speed >= liftAlarm) cute.play(S_MODE1);

    // Sink beep
    if (altitudeVariation_speed <= sinkAlarm) cute.play(S_FART1);    
  }
}

#pragma endregion

#pragma region "DISPLAY"

void InitDisplay()
{
  if (display.begin(SSD1306_SWITCHCAPVCC, 0x3C))
  {
    delay(250);
    display.clearDisplay();
    
    // Draw boot logo
    display.drawBitmap(0, 0, BOOT_LOGO, SCREEN_WIDTH, SCREEN_HEIGHT, WHITE);
    display.display();

    displayWorking = true;
    Serial.println("Display initialized!");
  }
  else
  {
    displayWorking = false;
    Serial.println("Display not initialized!");
  } 
}

void DisplayData(String absAlt, String relAlt, String variationSpeed, String temp) 
{
  if (displayWorking)
  {
    WriteAltitude(absAlt, relAlt);
    WriteTemp(temp);
    WriteSpeed(variationSpeed);
  }
}

void WriteAltitude(String absAlt, String relAlt)
{
  // Clean altitude zone
  display.fillRect(44, 10, 80, 27, SSD1306_BLACK);
  display.display();

  // Write absolute altitude
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(44, 10);
  display.println("ABS: " + absAlt);
  display.display(); 

  // Write relative altitude
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(44, 23);
  display.println("REL: " + relAlt);
  display.display(); 
}

void WriteTemp(String temp)
{
  // Clean temp zone
  display.fillRect(104, 48, 10, 5, SSD1306_BLACK);
  display.display();

  // Write temp
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(104, 48);
  display.println(temp);
  display.display(); 
}

void WriteSpeed(String varSpeed)
{
  // Clean speed zone
  display.fillRect(36, 48, 40, 10, SSD1306_BLACK);
  display.display();

  // Write speed
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(36, 48);
  display.println(varSpeed);
  display.display(); 
}

#pragma endregion

#pragma region "BATTERY"

void CheckBatteryVoltage()
{
  if (LOW_BATTERY_CHECK)
  {
    if (currentMillis - previousBatteryMillis >= MS_BATTERY_CHECK_INTERVAL)
    {
      previousBatteryMillis = currentMillis;

      // Check lipo/lition battery voltage
      float batteryVoltageSensor = (6.60 * analogRead(BATTERY_PIN)) / 1023.00;
        
      if (batteryVoltageSensor <= 3.4)
      {
        cute.play(S_DISCONNECTION);
        delay(300);
        cute.play(S_DISCONNECTION);
        delay(300);
        cute.play(S_DISCONNECTION);
        delay(300);
      }
        
      if (DEBUG)
      {
        Serial.println("Low battery: " + (String)batteryVoltageSensor + "V");
        Serial.println("");
      } 
    }
  }
}

#pragma endregion

#pragma endregion

void setup()
{
  if (DEBUG)
  {
    Serial.begin(9600);
    Serial.println("Variometer starting... ");
  }

  // Init sound library
  cute.init(BUZZER);

  // Initialize display
  if (USE_DISPLAY)
  {
    InitDisplay();
  } 

  // Initialize BMP280 sensor
  InitSensor(); 

  if (displayWorking)
  {
    delay(BOOT_LOGO_DURATION);
    display.clearDisplay();
    
    // Draw main screen page
    display.drawBitmap(0, 0, MAIN_SCREEN, SCREEN_WIDTH, SCREEN_HEIGHT, WHITE);
    display.display();
  }
}

void loop()
{
  ReadSensor();  
  CheckBatteryVoltage();
}
