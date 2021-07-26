# FW Index
## [SiameseTwins](https://github.com/YuvalSob/CavingHeadlampsFW/tree/main/SiameseTwins)

FW for an Headlamp with single driver and 2 PWM channels (Flood + Spot) 

Driver type: Nanjg 105c (Attiny13A + 8*AMC7135)

Switch type: Single momentary switch connected to Star4/Pin2/PB3 (need to cut outer ring)

###### Features: ######
1. 2 separate PWM channels to control 2 LEDs
2. Step down from Turbo mode after 30 sec 
3. Battery check mode (using blinks)
4. Low voltage indication usin colores LED connected to Star3/Pin3/PB4 (need to cut outer ring)
5. Very low voltage protection (turned off but can be turen on again)
6. Lock off mode to prevent accidental turn on
7. Auto lock after 5 min in of
8. Very low moon mode for emergency 





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
