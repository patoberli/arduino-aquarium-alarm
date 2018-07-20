
/* Adafruit FONA GSM 32u4 module Template

modified 20180718 by Patrick Oberli

Required parts:
- Adafruit FONA 32u4
	- https://www.adafruit.com/product/3027
- Adafruit GSM Antenna
	- https://www.adafruit.com/product/1991
	- In my case this antenna tends to create a crash of the Arduino because of the short cable, see the FAQ of the 32u4
	- There is a variant with longer cable, I haven't tested it: https://www.adafruit.com/product/3237
- 12-pin and 16-pin feather stacking headers
	- https://www.adafruit.com/product/2830
- 500 mAh 3.7 V battery, better at least 850 mAh (to high capacity will cause long delays in the power alarm!)
	- https://www.adafruit.com/product/258 (this is the 1200 mAh model)
- micro SIM card with support for 2G (GSM), check your operators, some are starting to turn of GSM
- Breadboard with 40 or more rows
- at least 20 dupond jumper wires
- 10k resistor potentiometer for the LCD
	- https://www.arduino.cc/en/Tutorial/LiquidCrystalDisplay (PINs adjusted in this sketch, see below)
- 1602A Char LCD / 3.3V / Standard ST7066/HD44780 (must be a special 3.3V! one)
	- https://www.aliexpress.com/item/3-3V-LCD1602-LCD-monitor-1602-Blue-Screen-White-Code-Blacklight-16x2-Character-LCD-Display-Module/32453954557.html
- Piezo speaker (also known as buzzer)
- 100 Ohm resistor (for piezo)
- 220 Ohm resistor (for LCD)
- DS18B20 temp sensor with DFR0055 Plugable Terminal adapter (jumper set to Pull Up)
	- https://www.aliexpress.com/item/DS18b20-Water-Temperature-Sensor-Stainless-Steel-Probe-Temperature-Sensor-for-Arduino-SR002/32538132994.html
	- this sensor is sadly NOT salt water resistant, it requires you to protect it from direct water contact or it will quickly corrode/rust. 
		I suggest using a tiny bit of aquarium safe silicone or synthetic resin. 
		The thicker the layer, the slower the registered temparature change, so make it thin!
	- Manual DFR0055 https://www.dfrobot.com/wiki/index.php/Terminal_sensor_adapter_V2_SKU:DFR0055
*/

// Install the libraries from the Arduino IDE Library Manager named "MKRWAN", "Adafruit FONA Library" and "OneWire" (by Jim Studt, Tom Pollard, ...) 
#include <Wire.h>
#include <USI_TWI_Master.h>
#include <TinyWireM.h>
#include "Adafruit_FONA.h"

#define FONA_RX  9
#define FONA_TX  8
#define FONA_RST 4
#define FONA_RI  7

// this is a large buffer for replies
char replybuffer[255];

// We default to using software serial. If you want to use hardware serial
// (because softserial isnt supported) comment out the following three lines
// and uncomment the HardwareSerial line
#include <SoftwareSerial.h>
SoftwareSerial fonaSS = SoftwareSerial(FONA_TX, FONA_RX);
SoftwareSerial *fonaSerial = &fonaSS;

// Hardware serial is also possible!
//  HardwareSerial *fonaSerial = &Serial1;

// Use this for FONA 800 and 808s
Adafruit_FONA fona = Adafruit_FONA(FONA_RST);
// Use this one for FONA 3G
//Adafruit_FONA_3G fona = Adafruit_FONA_3G(FONA_RST);

uint8_t readline(char *buff, uint8_t maxbuff, uint16_t timeout = 0);

uint8_t type;

/******** End Adafruit FONA template ********/

/***************************************************************
Insturciton:
Connection:
1.Plugable Terminal Sensor Adapter & Waterproof DS18B20 Digital Temperature Sensor
A   ---->     Blue(DATA SIGNAL)
B   ---->     RED   (VIN)
C   ---->     Black (GND)

2.Waterproof DS18B20 Digital Temperature Sensor & Arduino board
1(A)   ---->     Digital Pin5 (changed from 2)
2(B)   ---->     5V/3.3V
3(C)   ---->     GND

Setting for the Pull-up Register/Pull-down Register Selection Jumpers
When connect DS18B20 with the adapter,please choose to use the
Pull-up Register Jumper
***************************************************************/

// Install the library from Arduino IDE library manager called "OneWire" by Jim Studt, Tom Pollard, ....
#include <OneWire.h>

int DS18S20_Pin = 5; //DS18S20 Signal pin on digital 5, changed from 2

//Temperature chip i/o
OneWire ds(DS18S20_Pin);  

/*
LiquidCrystal Library - Hello World

The circuit:
* LCD RS pin to digital pin 12
* LCD Enable pin to digital pin 11
* LCD D4 pin to digital pin 1
* LCD D5 pin to digital pin 0
* LCD D6 pin to digital pin 3
* LCD D7 pin to digital pin 2
* LCD R/W pin to ground
* LCD VSS pin to ground
* LCD VCC pin to 3.3V
* 10K resistor:
* ends to +3.3V and ground
* wiper to LCD VO pin (pin 3)

Library originally added 18 Apr 2008
by David A. Mellis
library modified 5 Jul 2009
by Limor Fried (http://www.ladyada.net)
example added 9 Jul 2009
by Tom Igoe
modified 22 Nov 2010
by Tom Igoe
modified 7 Nov 2016
by Arturo Guadalupi
modified 18. July 2018
by Patrick Oberli

This example code is in the public domain.

http://www.arduino.cc/en/Tutorial/LiquidCrystalHelloWorld

*/

// include the library code:
#include <LiquidCrystal.h>

// initialize the library by associating any needed LCD interface pin
// with the arduino pin number it is connected to
const int rs = 12, en = 11, d4 = 1, d5 = 0, d6 = 3, d7 = 2; // d4 changed from 5 to 1, d5 changed from 4 to 0
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);


// Modify for your own liking, number in brackets is the maximum ammount of allowed characters
char mobilenumber[20] = "+000000000";
char messagepower[141] = "Aquarium without power!";
char messagetemp[141] = "Temperature > 27 deg C";

int TEMPERATURESMSREQ = 0;
int TEMPERATURESMSSENT = 0;
int VOLTAGESMSREQ = 0;
int VOLTAGESMSSENT = 0;

// Audio Alarm, add a Piezo speaker to pin 10

void setup() {
  // set PIN 13 (red LED) as an output
  pinMode(13, OUTPUT);
 
  // set up the LCD's number of columns and rows:
  lcd.begin(16, 2);
  // Print a message to the LCD.
  lcd.print("Temperature:");

  /* Start initialize the FONA module */

  // disable wait on Serial connection
  //while (!Serial);

  Serial.begin(115200);
  Serial.println(F("FONA basic test"));
  Serial.println(F("Initializing....(May take 3 seconds)"));

  fonaSerial->begin(4800);
  if (! fona.begin(*fonaSerial)) {
    Serial.println(F("Couldn't find FONA"));
    while (1);
  }
  type = fona.type();
  Serial.println(F("FONA is OK"));
  Serial.print(F("Found "));
  switch (type) {
    case FONA800L:
      Serial.println(F("FONA 800L")); break;
    case FONA800H:
      Serial.println(F("FONA 800H")); break;
    case FONA808_V1:
      Serial.println(F("FONA 808 (v1)")); break;
    case FONA808_V2:
      Serial.println(F("FONA 808 (v2)")); break;
    case FONA3G_A:
      Serial.println(F("FONA 3G (American)")); break;
    case FONA3G_E:
      Serial.println(F("FONA 3G (European)")); break;
    default:
      Serial.println(F("???")); break;
  }

  // Print module IMEI number.
  char imei[16] = {0}; // MUST use a 16 character buffer for IMEI!
  uint8_t imeiLen = fona.getIMEI(imei);
  if (imeiLen > 0) {
    Serial.print("Module IMEI: "); Serial.println(imei);
  }
  /* End initialize FONA module */
  delay(3000);
}


void loop() {
  // put your main code here, to run repeatedly:
/*
  // read the RSSI
  uint8_t n = fona.getRSSI();
  int8_t r;
  // print the RSSI to serial
  Serial.print(F("RSSI = ")); Serial.print(n); Serial.print(": ");
  if (n == 0) r = -115;
  if (n == 1) r = -111;
  if (n == 31) r = -52;
  if ((n >= 2) && (n <= 30)) {
    r = map(n, 2, 30, -110, -54);
  }
  Serial.print(r); Serial.println(F(" dBm"));
  delay(1000);
*/
  uint16_t vbat;
  Serial.print(F("Voltage Battery - Debug = ")); Serial.println(vbat);

  fona.getBattVoltage(&vbat);

 /***************************************************************
 Adjust the vbat number according to your battery. Use the Adafruit FONAtest sketch 
 download here: https://learn.adafruit.com/adafruit-feather-32u4-fona/fona-test
 charge your battery to 100% and use the [b] function to check the maximum voltage of your battery. 
 Reduce the vbat number by around 100-200, this will cause a power alarm after around 10-30 minutes, 
 depending on the attached hardware. Requires some finetuning per battery.
 The percentage didn't work well in my case, as the battery % number sometimes dropped to 90%  before
 being charged by the FONA 32u4 board. Piezo will add another 1 second wait time, lower down in the code.
 ***************************************************************/

  if (vbat >= 4050) { // check every 5 seconds if the power is still ok, or the unit is running from the battery power
    Serial.println("Power OK");
    Serial.print(F("Voltage Battery = ")); Serial.print(vbat); Serial.println(F(" mV"));
    digitalWrite(13, LOW);
	VOLTAGESMSREQ = 0;
	VOLTAGESMSSENT = 0;
    delay(5000);
  } else {
    Serial.print("Power message: "); Serial.println(messagepower);
    Serial.print(F("Battery low = ")); Serial.print(vbat); Serial.println(F(" mV!"));
    digitalWrite(13, HIGH);
	VOLTAGESMSREQ = 1;
    delay(5000);
  }



//Serial.print("SMS Number: "); Serial.println(mobilenumber);
//Serial.print("No Power message: "); Serial.println(messagepower);
//Serial.print("Temp message: "); Serial.println(messagetemp);



  while (fona.available()) {
    Serial.write(fona.read());
  }

  float temperature = getTemp();
  Serial.println(temperature);

  delay(100); //just here to slow down the output so it is easier to read

// set the cursor to column 0, line 1
// (note: line 1 is the second row, since counting begins with 0):
  lcd.setCursor(0, 1);

  // print the temperature on the LCD:
  lcd.print(temperature);
  if (temperature >= 27) //measure if temperate is >= 27°C and produce an audio alarm if to high
  {
	  tone(10, 5000, 1000); // Audio alarm, add piezo to Pin 10, frequency 5000 Hz, 1 second beep duration
	  TEMPERATURESMSREQ = 1;
	  Serial.print("Temperature to high!: "); Serial.println(temperature);
  }
  else
  {
	  noTone(10);
	  TEMPERATURESMSREQ = 0;
	  TEMPERATURESMSSENT = 0;
  }
  
  while (VOLTAGESMSREQ == 1 && VOLTAGESMSSENT == 0)
  {
	  sendSMSpower();
	  VOLTAGESMSSENT = 1;
  }

  while (TEMPERATURESMSREQ == 1 && TEMPERATURESMSSENT == 0)
  {
	  sendSMStemp();
	  TEMPERATURESMSSENT = 1;
  }

  // flush input
  flushSerial();

}

void sendSMSpower() {
	// send an SMS!
	//char sendto[21], message[141]; // old code
	flushSerial();
	//Serial.print(F("Send to #"));  // old code
	//readline(sendto, 20); // old code
	Serial.println(mobilenumber);
	//Serial.print(F("Type out one-line message (140 char): ")); // old code
	//readline(message, 140); // old code
	Serial.println(messagepower);
	if (!fona.sendSMS(mobilenumber, messagepower)) {
		Serial.println(F("Failed"));
	}
	else {
		Serial.println(F("Sent!"));
	}
}

void sendSMStemp() {
	// send an SMS!
	//char sendto[21], message[141]; // old code
	flushSerial();
	//Serial.print(F("Send to #")); // old code
	//readline(sendto, 20); // old code
	Serial.println(mobilenumber);
	//Serial.print(F("Type out one-line message (140 char): ")); // old code
	//readline(message, 140); // old code
	Serial.println(messagetemp);
	if (!fona.sendSMS(mobilenumber, messagetemp)) {
		Serial.println(F("Failed"));
	}
	else {
		Serial.println(F("Sent!"));
	}
}

void flushSerial() {
  while (Serial.available())
    Serial.read();
}

float getTemp() {
	//returns the temperature from one DS18S20 in DEG Celsius

	byte data[12];
	byte addr[8];

	if (!ds.search(addr)) {
		//no more sensors on chain, reset search
		Serial.println("no more sensors on chain, reset search!");
		ds.reset_search();
		return -1000;
	}

	if (OneWire::crc8(addr, 7) != addr[7]) {
		Serial.println("CRC is not valid!");
		return -1000;
	}

	if (addr[0] != 0x10 && addr[0] != 0x28) {
		Serial.print("Device is not recognized");
		return -1000;
	}

	ds.reset();
	ds.select(addr);
	ds.write(0x44, 1); // start conversion, with parasite power on at the end

	byte present = ds.reset();
	ds.select(addr);
	ds.write(0xBE); // Read Scratchpad


	for (int i = 0; i < 9; i++) { // we need 9 bytes
		data[i] = ds.read();
	}

	ds.reset_search();

	byte MSB = data[1];
	byte LSB = data[0];

	float tempRead = ((MSB << 8) | LSB); //using two's compliment
	float TemperatureSum = tempRead / 16;

	return TemperatureSum;

}
