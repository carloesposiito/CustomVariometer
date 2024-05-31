# CustomVariometer

[By Carlo Esposito, 2024, Github: @carloesposiito]


The CustomVariometer project allows you to use a microcontroller (Arduino, ESP32, ...) as a variometer, which is a device that measures altitude variation and provides real-time information on the rate of ascent and descent.


The variometer, when properly configured, will sound when a certain rate of ascent or descent is exceeded, emitting different sounds.<br/>
Additionally, it will display the absolute altitude and the altitude relative to the starting point, the rate of altitude change in m/s, and the recorded temperature.<br/>
It is possible to use the program even without a display: the fundamental components are the microcontroller, BMP280 sensor, and passive buzzer.


For all possible configurations, refer to the Code section below.<br/>


![](https://github.com/carloesposiito/CustomVariometer/blob/main/CustomVariometer/photos/mainScreen_CustomVariometer.png)


# Required Hardware

- 1x ESP32 Super Mini;<br/>
- 1x Passive Buzzer;<br/>
- 1x BMP280 Sensor;<br/>
- 1x OLED Display 0.96" 128x64;


In this project is used a ESP32 Super Mini, but other microcontrollers can also be used.<br/>
Additionally, there are two types of BMP280 sensors: in this case, the 6-pin model was chosen, but the 4-pin model is also compatible with the project (with appropriate modifications).


# Required Software

The libraries needed to control the sensor, display, and buzzer are:<br/>
- Adafruit_BMP280;<br/>
- Adafruit_SSD1306;<br/>
- Adafruit_BusIO;<br/>
- Adafruit_GFX_Library;<br/>
- Adafruit_Unified_Sensor;<br/>
CuteBuzzerSounds;


They can be manually installed via the Arduino IDE, or automatically extracted into the `C:\\Users\\USER\\Documents\\Arduino\\libraries` folder by running the `CustomVariometer/libraries/libraries_CustomVariometer.exe` file.<br/>
In the latter case, make sure there are no existing folders with the same names as those to be extracted to avoid conflicts (though this is unlikely).


# Connection Diagram

The connection diagram is as follows:


![](https://github.com/carloesposiito/CustomVariometer/blob/main/CustomVariometer/scheme/scheme_CustomVariometer.jpg)


The sensor module is connected via SPI, while the display is connected via I2C.


# Code

Some functionalities of the program can be configured through the **EDITABLE PARAMETERS** section:


- **DEBUG**: if _true_ allows data to be displayed in the serial monitor.


- **MS_DETECTION_INTERVAL**: indicates the sensor detection interval in milliseconds.


- **USE_DISPLAY**: if _true_ enables the data display mode on the connected OLED display.


- **BATTERY CHECK**: if _true_ allows monitoring the battery charge connected to the pin indicated in the **BATTERY_PIN** variable. This is not included in the connection diagram.


- **USE_SEA_PRESSURE**: if _true_, the BMP280 sensor is calibrated using the pressure indicated in the **SEA_PRESSURE_HPA** variable as a reference; otherwise, the reference is the pressure detected by the sensor at startup.<br/>
Note: if _true_, the more accurate the value (unit of measure hPa), the better the detection. Check the value in your location as it may change daily!


- **SENSIBILTY**: indicates the position of the chosen value within the **sensibilities** array (numbering starts from zero).<br/>
Represents the ascent rate (m/s) that must be exceeded to trigger the alarm.


- **SINK_ALARM**: indicates the position of the chosen value within the **sinkAlarms** array (numbering starts from zero).<br/>
Represents the negative descent rate (m/s) that must be exceeded to trigger the alarm.
