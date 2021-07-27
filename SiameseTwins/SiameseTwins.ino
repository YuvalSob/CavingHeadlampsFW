;/*
 * Project:Or SiameseTwins V1.0 firmware
 * *****************************************************************************
 * TODO: known bug no sleep mode - need to take out bettaries while not in use * 
 * *****************************************************************************
 * 2 channels (Spot & Flood) Caving light firmware.
 * 
 * This light uses a modified Nanjg 105C driver whith 2 PWM channels
 * 
 * The firmware is based on:
 * 1. MiniMo firmware by DrJones
 * 2. BLF A6 firmware by ToyKeeper
 * 3. Battcheck firmware by ToyKeeper
 * (Thank You guys)
 *
 * Copyright (C) 2020 Yuval Soboliev
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  
 * See the GNU General Public License for more details.
 *
 *******************************************************
 *******************************************************
 **** If you build any caving ligt based on this FW ****
 **** Please Share you project at the BLF website   ****
 *******************************************************
 *******************************************************
 * 
 * Modified Nanjg 105C driver diagram:
 *                     ---
 *                   -|   |- VCC
 *   Momentary Switch-|   |- Voltage ADC
 *        Colored LED-|   |- PWM Spot
 *               GND -|   |- PWM Food
 *                     ---
 *
 *
 * VOLTAGE
 *      Before using this FW it is reccomanded to burn my battarey clibration FW to your light in order to 
 *      get the voltage levels reading and change the code acordingly 
 *      (https://github.com/YuvalSob/CavingHeadlampsFW/blob/main/HelloWorld/ADC.ino)
 */

#include <avr/io.h>
#include <util/delay.h>

// For battery check
#define ADC_CHANNEL 0x01    // MUX 01 corresponds with PB2
#define ADC_DIDR    ADC1D   // Digital input disable bit corresponding with PB2
#define ADC_PRSCL   0x06    // clk/64

// Define valuse for Battery Check
// These valued measured using TotKeeper ADC.ino program
#define V3 170 // ADC reading for V=3.9v 
#define V2 160 // ADC reading for V=3.7v
#define V1 155 // ADC reading for V=3.6v
#define V0 146 // ADC reading for V=3.4v

// Define values for LED output: 0-OFF , 255-brightest 
#define SPOT_H 255 // ~1000Lm
#define SPOT_L 30  // ~120LM

#define FLOOD_HH 255 // ~1000Lm
#define FLOOD_H 100  // ~400Lm
#define FLOOD_M 50   // ~200Lm
#define FLOOD_L 15   // ~60Lm

#define MOON 8      // ~30Lm
#define OFF 0

// Define numbers (index) for differant modes
#define OFF_MODE 0

#define FLOOD_MODE_1 1
#define FLOOD_MODE_2 2
#define FLOOD_MODE_3 3
#define FLOOD_MODE_4 6

#define SPOT_MODE_1 4
#define SPOT_MODE_2 5

#define MOON_MODE 7

#define short_press_max_time 16
#define medium_press_max_time 40

// Functin that check battery level and return number between 4 to 0
uint8_t get_batt_lvl() {
  uint16_t voltage = 0;
  uint8_t i;
    
  // ADC on
  ADMUX  = (1 << REFS0) | (1 << ADLAR) | ADC_CHANNEL; // 1.1v reference, left-adjust, ADC1/PB2
  DIDR0 |= (1 << ADC_DIDR);                           // disable digital input on ADC pin to reduce power consumption
  ADCSRA = (1 << ADEN ) | (1 << ADSC ) | ADC_PRSCL;   // enable, start, prescale

  ACSR   |=  (1<<7); //AC off
            
  for (i=0; i<8; i++) {
    // Start conversion
    ADCSRA |= (1 << ADSC);
    // Wait for completion
    while (ADCSRA & (1 << ADSC));
    // See if voltage is lower than what we were looking for
    voltage += ADCH;
    _delay_ms(50);
  }

  // ADC off 
  ADCSRA &= ~(1<<7); //ADC off
  voltage = voltage >> 3;

  if (voltage > V3) return 4; // Almost full
  if (voltage > V2) return 3; // Above 75%
  if (voltage > V1) return 2; // Above 50% 
  if (voltage > V0) return 1; // Above 25%
  return 0; // Almost empty 
}

// Function to blink LED (used for battery level indiction anb botton press feedback)
// The input for fonction is the number of blinkes
void blink(uint8_t i) {
  OCR0B = 0; // Turn main LEDs 
  OCR0A = 0; // Turn main LEDs 
  PORTB &= ~(1 << PB4); // Turn the LED off 

  while (i>0) {
    _delay_ms(800);
    PORTB |= (1 << PB4);  // Turn the LED on
    _delay_ms(200);
    PORTB &= ~(1 << PB4); // Turn the LED off 
    i--;
  }
  _delay_ms(800);
}

int main() {
  // Pins configuration
  // Set pin 5 (Flood) to output
  DDRB |= (1 << DDB0);
  
  // Set pin 6 (Spot) to output
  DDRB |= (1 << DDB1);

  // Set pin 3 (status LED) to outputDDRB |= (1 << PB4);

  // Set pin 2 (momentary switch) to pull up (it is input by reset value)
  PORTB |= (1 << PB3);
  
  // PWN configuration
  TCCR0A=0b10100001; 
  TCCR0B=0b00000001;  //PWM setup, 9kHz

  // Variables
  uint8_t press_len=0;   // Used to measur press length (short/medium/long)
  uint8_t press_flag=0;  // Indicates that button was pressed
  uint8_t lock=0;        // Indicates that lamp is in "lock mode"
  uint8_t press_num=0;   // Count clicks in order to unlock the lamp (need 4 clicks to unlock)
  uint8_t time_from_last_press=0;   // Measur the time from last click in order to unlock the lamp (if time is too long stay in lock)
  uint8_t special_mode = 0;   // Flag for while in soecial ,odes

  uint16_t cycle_cnt = 0; // Counts cycles of main loop in order to calculate time
  uint8_t ten_sec_cnt = 0; // Counts time of 10 seconds
  
  uint8_t batt_lvl = get_batt_lvl(); // Stores the level of the battery
  uint8_t warn_num[]={255,3,0,0,0};  // Number of times to warn for each battery level (currently only warns for low and cutoff)

  // set LED levels for each mode
  uint8_t spot_output[]= {OFF, OFF,     OFF,     OFF,     SPOT_L, SPOT_H, OFF     , OFF}; 
  uint8_t flood_output[]={OFF, FLOOD_L, FLOOD_M, FLOOD_H, OFF,    OFF,    FLOOD_HH, MOON}; 
  uint8_t mode=OFF_MODE; // Used to set lamp output

  
  while(1) {                             //Endless loop
    // Locked Mode
    //////////////////////
    if (lock == 1) {
      // Count time since last press
      if (press_num>0) {
        time_from_last_press++;                // More 25 ms passed since last press
        if (time_from_last_press==32) {
        // If too long time has passed reset count 
          press_num=0;
          // blink to indicate timeout
          blink(1);
        }
      }
      if ((PINB&8)==0) {                   // When the button is pressed (PB3 pulled down)
        press_flag=1;                      // Remember that the button was pressed, see below      
        press_len++;                       // Length of button press        
      }
      else {                               // Button not pressed
        if (press_flag) {                  // But it was pressed, so it has just been released!
          if (press_len<16) {
            press_num++;                   // Count the number of short clicks
            if (press_num==4){             // If 4 fast cliks - release the locking 
              lock=0;
              press_num=0;
              mode=FLOOD_MODE_2;           // Turn on light
            }
          }
          // Reset all counters and flags
          press_flag=0;                      
          press_len=0;                       
          time_from_last_press=0;
        }
      }
    } // If lock

    // Operational  mode
    /////////////////////////
    else {
      if ((PINB&8) == 0) {                   // When the button is pressed (PB3 pulled down)
        press_flag = 1;                      // Remember that the button was pressed, see below      
        press_len++;                         // Press_len length of button press
        cycle_cnt = 0;                       // Any boutton press reset timers
        ten_sec_cnt = 0;                     // Any button press reset timers
      }
      else {                               // Button not pressed
        if (press_flag) {                  // But it was pressed, so it has just been released!
          if (press_len < short_press_max_time) {                  
            // Short press - cycle between flood modes - always start at medium flood
            special_mode = 0;
            switch(mode) {
              case FLOOD_MODE_2 : mode=FLOOD_MODE_3; break; 
              case FLOOD_MODE_3 : mode=FLOOD_MODE_1; break; 

              default : mode=FLOOD_MODE_2;
            }
          } // if short press
          else {
            if (press_len < medium_press_max_time) {
              // Medium Press
              // if off check battery
              if (mode == OFF_MODE) {               
                // Battery check
                blink(get_batt_lvl());
                mode = MOON_MODE;
              }
              // if not off, cycle between spot modes - always start at high spot
              else {
                if (special_mode == 0) {
                  mode = SPOT_MODE_2;
                  special_mode = 1; 
                }
                else { 
                  switch(mode) {
                    case SPOT_MODE_2 : mode = SPOT_MODE_1; break; 
                    case SPOT_MODE_1 : mode = FLOOD_MODE_4; break; 
                    default : mode = SPOT_MODE_2;
                  } //case
                } // if special mode 
              }
            } // If medium press
            else {
              // Long Press
              special_mode = 0;
              if (mode == OFF_MODE) { // Lock the lamp if already in off mode
                PORTB |= (1 << PB4); // Turn the LED on
                lock = 1; 
                _delay_ms(2000);
                PORTB &= ~(1 << PB4); // Turn the LED off 
              }
              // Turn off all LEDs
              mode = OFF_MODE;
              // Reset warning numbers on turn off 
              warn_num[0]=255;
              warn_num[1]=3;
            }
          }
          // Reset all flags and timers
          press_flag=0;                      
          press_len=0;                          
        }
      }    
    }

    // Set output of LEDs according to the mode
    OCR0B = spot_output[mode];                    
    OCR0A = flood_output[mode]; 
                        
    _delay_ms(25);                         //wait a bit before checking again, important for press_lening

    // Before start the loop again measure time and act according to time passed
    cycle_cnt++;
    if (cycle_cnt == 1000) { // every 1000 iterations is about 10 sec 
      cycle_cnt = 0;
      ten_sec_cnt++;
      
      // Timer to step down from high mode after 30 sec
      if (ten_sec_cnt == 3) { // Any press zeros the counter so when setting mode to high counter is 0 
        if (mode == SPOT_MODE_2 || mode == FLOOD_MODE_4) {
          mode = FLOOD_MODE_2;
        }
      }

      // Timer of 5 minutes
      if (ten_sec_cnt == 8) {       // 30 * 10 sec = 5 min // TODO:
        ten_sec_cnt = 0;
      
        // automatically lock lamp after 5 min in off mode  
        if (mode == OFF_MODE) { 
          if (lock == 0) {
            PORTB |= (1 << PB4); // Turn the LED off 
            lock = 1;   
            _delay_ms(2000);
            PORTB &= ~(1 << PB4); // Turn the LED off 
          } 
        }
        else {
          // Every 5 mimiutes check battery 
          batt_lvl = get_batt_lvl();
          if (warn_num[batt_lvl] > 0) { // Only warn warn_num (=3 for level 0 and level 1) times for each level
            warn_num[batt_lvl]--; 
            blink(4-batt_lvl);
          }
          // Below cut off - turn light off (can turned on again)
          if (batt_lvl == 0) { 
            mode = OFF_MODE;
          }
        }
      } // 5 min timer
    } // 10 sec timer
  } // Endless loop
} // main
