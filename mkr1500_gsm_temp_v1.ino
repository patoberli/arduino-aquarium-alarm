
/* Adafruit FONA GSM 32u4 module Template, massively adjusted for Arduino MKR NB 1500

modified 20200925 by Patrick Oberli

Required parts:
- Arduino MKR NB 1500 incl. antenna
	- https://store.arduino.cc/arduino-mkr-nb-1500-1413
- 1500 mAh 3.7 V battery minimum
	- https://www.adafruit.com/product/2011 (this is the 2000 mAh model)
- micro SIM card with support for 4G (LTE), check your operators
- Adafruit SSD1306 SPI Display
- Breadboard with 40 or more rows
- at least 20 dupond jumper wires
- Piezo speaker (also known as buzzer)
- 100 Ohm resistor (for piezo)
- two > 20 kOhm resistors for USB power check
- DS18B20 temp sensor with DFR0055 Plugable Terminal adapter (jumper set to Pull Up)
	- https://www.aliexpress.com/item/DS18b20-Water-Temperature-Sensor-Stainless-Steel-Probe-Temperature-Sensor-for-Arduino-SR002/32538132994.html
	- this sensor is sadly NOT salt water resistant, it requires you to protect it from direct water contact or it will quickly corrode/rust. 
		I suggest using a tiny bit of aquarium safe silicone or synthetic resin. 
		The thicker the layer, the slower the registered temparature change, so make it thin!
	- Manual DFR0055 https://www.dfrobot.com/wiki/index.php/Terminal_sensor_adapter_V2_SKU:DFR0055
*/

// Install the libraries from the Arduino IDE Library Manager named "MKRWAN", "MKRNB" and "OneWire" (by Jim Studt, Tom Pollard, ...) 
#include <gfxfont.h>
#include <Adafruit_SPITFT_Macros.h>
#include <Adafruit_SPITFT.h>
#include <Wire.h>
//#include <USI_TWI_Master.h>
//#include <TinyWireM.h>
#include <MKRNB.h>

#include "arduino_secrets.h" 
// Please enter your sensitive data in the Secret tab or arduino_secrets.h
// If it doesn't exist, create a new file in the same directory named arduino_secrets.h
// PIN Number
const char PINNUMBER[] = SECRET_PINNUMBER;

// initialize the library instance
NB nbAccess;
NB_SMS sms;

// this is a large buffer for replies
char replybuffer[255];


/* Start SPI display */
//***************************************************************

// Install the library from Arduino IDE library manager called "OneWire" by Jim Studt, Tom Pollard, ....
#include <OneWire.h>

// SPI Display
#include <spi.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

Adafruit_SSD1306 display = Adafruit_SSD1306(128, 32, &Wire);
#define OLED_RESET -1 // Reset pin # (or -1 if sharing Arduino reset pin)

/* End SPI display*/


int DS18S20_Pin = 5; //DS18S20 Signal pin on digital 5, changed from 2

//Temperature chip i/o
OneWire ds(DS18S20_Pin);  



// Modify for your own liking, number in brackets is the maximum ammount of allowed characters
char mobilenumber[20] = "+00000000000";
char messagepower[141] = "Aquarium without power!";
char messagetemp[141] = "Temperature > 27 deg C";

int TEMPERATURESMSREQ = 0;
int TEMPERATURESMSSENT = 0;
int VOLTAGESMSREQ = 0;
int VOLTAGESMSSENT = 0;

// Audio Alarm, add a Piezo speaker to pin 10

void setup() {
	//SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
	display.begin(SSD1306_SWITCHCAPVCC, 0x3C); // Address 0x3C for 128x32
	Serial.println("OLED begun");

	// Show image buffer on the display hardware.
	// Since the buffer is intialized with an Adafruit splashscreen
	// internally, this will display the splashscreen.
	display.display();
	delay(1000);

	// Clear the buffer.
	display.clearDisplay();
	display.display();

	// text display tests
	display.setTextSize(1);
	display.setTextColor(SSD1306_WHITE);
	display.setCursor(0, 0);
	display.print("Temperature:");
	display.display(); // actually display all of the above

  // set LED_BUILTIN (red LED) as an output
  pinMode(LED_BUILTIN, OUTPUT);
 
  // connection state
  bool connected = false;

  // Start NB module
  // If your SIM has PIN, add it to the arduino_secrets.h file
  while (!connected) {
    if (nbAccess.begin(PINNUMBER) == NB_READY) {
      connected = true;
    } else {
      Serial.println("Not connected");
      delay(1000);
    }
  }

  Serial.println("NB initialized");

 
  delay(10000); // wait 10 seconds for everything to initialize
}


void loop() {

  // read and print the temperature to console
  float temperature = getTemp();
  Serial.print("Temperature: "); Serial.println(temperature);

  delay(100); //just here to slow down the output so it is easier to read

// set the cursor to column 0, line 10
// (note: line 10 is the elevnenth row, since counting begins with 0):
  display.clearDisplay();
  display.setCursor(0, 0);
  display.print("Temperature:");
  display.setCursor(0, 10);

  // print the temperature on the LCD:
  display.print(temperature);
  display.display();
  if (temperature >= 30) //measure if temperate is >= 27°C and produce an audio alarm if to high
  {
	  tone(10, 5000, 1000); // Audio alarm, add piezo to Pin 10, frequency 5000 Hz, 1 second beep duration
	  TEMPERATURESMSREQ = 1;
	  Serial.print("Temperature to high!: "); Serial.println(temperature);
  }
  else if (temperature <= 25.8 && temperature > 23.5) // reactivate the alarm, turn of buzzer
  {
	  noTone(10);
	  TEMPERATURESMSREQ = 0;
	  TEMPERATURESMSSENT = 0;
  }
  else {
  // do nothing, temperature is not to warm
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

  // read voltage on USB pin, to check of power is still supplied.
  // read the input on analog pin 0:
  int sensorValue = analogRead(A0);
  // Convert the analog reading (which goes from 0 - 1023) to a voltage (0 - 3.3V):
  float voltage = sensorValue * (3.3 / 1023.0);
  // print out the value you read:
  Serial.print("Measured power on USB: ");  Serial.println(voltage);

  // DEBUG stuff
  //display.clearDisplay();
  display.setCursor(80, 0);
  display.print("Voltage:");
  // set the cursor to column 90, line 10
  display.setCursor(90, 10);
  // print the temperature on the LCD, use this for debugging the voltage:
  display.print(voltage);
  display.display();

  if (voltage >= 2.40) // power ok, the 2.40 might need some fine tuning, depending on resistors and cable lengths
  {
	  digitalWrite(LED_BUILTIN, LOW);
	  VOLTAGESMSREQ = 0;
	  VOLTAGESMSSENT = 0;
	  noTone(10);
	  delay(5000);

  }
  else // no power from USB
  {
	  digitalWrite(LED_BUILTIN, HIGH);
	  VOLTAGESMSREQ = 1; // comment this while testing, to disable sending SMS
	  tone(10, 5000, 1000); // Audio alarm, add piezo to Pin 10, frequency 5000 Hz, 1 second beep duration
	  delay(4000);
  }

  // flush input
  flushSerial();

}

void sendSMSpower() {
	// send an SMS!
	flushSerial();
	Serial.println(mobilenumber);
	Serial.println(messagepower);
//	if (!fona.sendSMS(mobilenumber, messagepower)) {
//		Serial.println(F("Failed"));
//	}
//	else {
//		Serial.println(F("Sent!"));
//	}
  sms.beginSMS(mobilenumber);
  sms.print(messagepower);
  sms.endSMS();
  Serial.println("\nCOMPLETE!\n");
}

void sendSMStemp() {
	// send an SMS!
	flushSerial();
	Serial.println(mobilenumber);
	Serial.println(messagetemp);
//	if (!fona.sendSMS(mobilenumber, messagetemp)) {
//		Serial.println(F("Failed"));
//	}
//	else {
//		Serial.println(F("Sent!"));
//	}
  sms.beginSMS(mobilenumber);
  sms.print(messagetemp);
  sms.endSMS();
  Serial.println("\nCOMPLETE!\n");
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
