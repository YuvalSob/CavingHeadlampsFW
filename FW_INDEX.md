# FW Index
## [ClickPress](https://github.com/YuvalSob/CavingHeadlampsFW/tree/main/ClickPress)

FW for an Headlamp with single driver and 2 PWM channels (Flood + Spot) 

Driver type: Nanjg 105c (Attiny13A + 8*AMC7135) + 8*AMC7135 slave board

Switch type: Single momentary switch connected to Star4/Pin2/PB3 (need to cut outer ring)

###### Features: ######
1. 2 separate PWM channels to control 2 LEDs
2. Step down from Turbo mode after 50 sec 
3. Battery check mode (using blinks)
4. Low voltage indication (using blinks)
5. Very low voltage protection (turned off but can be turen on again)
6. Lock off mode to prevent accidental turn on
7. Auto lock after 1 min in off
8. Infinat stapped ramp down
9. Low power consumtion while stand by


## [DualDriver](https://github.com/YuvalSob/CavingHeadlampsFW/tree/main/DualDriver)

FW for an Headlamp with 2 drivers and 2 momentary switches 

Driver type: 2 * Nanjg105c (Attiny13A + 8*AMC7135)

Switch type: 2 momentary switches (Scurion type knob) connected to Star2/Pin2/PB3 (need to cut outer ring)

###### Features: ######
1. 4 modes with stepped ramping (step up or down uding define)
2. Step down from Turbo mode after 30 sec 
3. Low power consumption (<4uA) while off
4. Low voltage indication 
5. Very low voltage protection


## [SiameseTwins](https://github.com/YuvalSob/CavingHeadlampsFW/tree/main/SiameseTwins)

FW for an Headlamp with single driver and 2 PWM channels (Flood + Spot) 

Driver type: Nanjg 105c (Attiny13A + 8*AMC7135) + 8*AMC7135 slave board

Switch type: Single momentary switch connected to Star4/Pin2/PB3 (need to cut outer ring)

###### Features: ######
1. 2 separate PWM channels to control 2 LEDs
2. Step down from Turbo mode after 30 sec 
3. Battery check mode (using blinks)
4. Low voltage indication usin colores LED connected to Star3/Pin3/PB4 (need to cut outer ring)
5. Very low voltage protection (turned off but can be turen on again)
6. Lock off mode to prevent accidental turn on
7. Auto lock after 5 min in off
8. Very low moon mode for emergency 


## [HelloWorld](https://github.com/YuvalSob/CavingHeadlampsFW/tree/main/HelloWorld)
5 Basic programs to help beginers to understand and write by their own caving lights FW.
Each program contains 1 feature of a light FW:
 
***[Blink](https://github.com/YuvalSob/CavingHeadlampsFW/blob/main/HelloWorld/Blink.ino)***

A very basic FW, only blinks 2 LED on pin 5 and 6 to see everting is connected correctly

***[MicroMo](https://github.com/YuvalSob/CavingHeadlampsFW/blob/main/HelloWorld/MicroMo.ino)***

A very basic FW, Turn on/off LEDs according to switch

***[ADC](https://github.com/YuvalSob/CavingHeadlampsFW/blob/main/HelloWorld/ADC.ino)
(Analog Digital Converter) - AKA battery check***

A basic FW to understand voltage measurment

This FW is also used to calibrate ACD values for all other FWs' battary check function

***[PWM](https://github.com/YuvalSob/CavingHeadlampsFW/blob/main/HelloWorld/PWM.ino)
(Pulse Width Modulation) - AKA LED dimming***

A basic FW to understand LED dimming

***[Sleep](https://github.com/YuvalSob/CavingHeadlampsFW/blob/main/HelloWorld/Sleep.ino)***

A basic FW to understand entering sleep mode to save power while light is off

## [Lightning](https://github.com/YuvalSob/CavingHeadlampsFW/tree/main/Lightning)
Extraction of the lightning mode out of Anduril by Toykeeper
