# Arduino aquarium monitor with alarm
## Arduino based aquarium alarm which checks temperature and supply voltage and alarms by SMS.

It includes a small piezo buzzer which makes alarm beeps if the temperature to high. A display that shows the current temperature and a backup battery. 

Please read the text inside the .ino sketch, there are some fields you have to modify (most importantly your mobile number).

## The design 
Please note, the temp sensor is the wrong model on this image. It also doesn't need to be attached to the breadboard, this is just for illustration!

![alt text](https://github.com/patoberli/arduino-aquarium-alarm/blob/2.0/fritzing_sketch_2.0_bb.png "Fritzing Design")

## Required parts:
- Adafruit FONA 32u4
  - https://www.adafruit.com/product/3027
- Adafruit GSM Antenna
  - https://www.adafruit.com/product/1991
  - In my case this antenna tends to create a crash of the Arduino because of the short cable, see the FAQ of the 32u4
  - There is a variant with longer cable, I haven't tested it: https://www.adafruit.com/product/3237
  - I solved it by putting a small ferrit core around the antenna cable
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
  - this sensor is sadly NOT salt water resistant, it requires you to protect it from direct water contact or it will quickly   corrode/rust. 
    I suggest using a tiny bit of aquarium safe silicone or synthetic resin. 
    The thicker the layer, the slower the registered temparature change, so make it thin!
  - Manual DFR0055 https://www.dfrobot.com/wiki/index.php/Terminal_sensor_adapter_V2_SKU:DFR0055
