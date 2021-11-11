/*   Project:Or DualDriver V1.0 
 *    
 *   FW for a dual driver light  
 *   
 *  2 drivers, 1 for each led 
 * 
 *  FUSE:
 *  ----
 *  BOD - Disabled (to save power)
 *  Clock - 4.8MHz (delay (1,000) = 1 sec) 
 *  
 *  Features:
 *  ---------
 *  1. 4 modes with stepped ramping 
 *  2. Step down from Turbo mode 
 *  3. Low power consumption while off
 *  4. Low voltage indication 
 *  5. Very low voltage protection
 *  
 *  UI:
 *  ---
 *  Short press - turn on/off 
 *  Long press - ramps up from low
 *  
 *                     ---
 *                   -|   |- VCC
 *   Momentary Switch-|   |- Voltage ADC
 *                   -|   |- PWM LED
 *               GND -|   |- 
 *                     ---
 *                     
 *  Based on STAR_mom_1.0 (STAR firmware) from JonnyC (Thanks Junny)
 *  
 *  Copyright (C) 2021 Yuval Soboliev
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  
 *  See the GNU General Public License for more details.
 *
 *  ***********************************************************
 *  ***********************************************************
 *  ****                                                   ****
 *  ****   If you build any caving light based on this FW  ****
 *  ****   Please Share you project on the BLF website     ****
 *  ****                                                   ****
 *  ***********************************************************
 *  ***********************************************************
 *  
 *  Before using this FW it is recommended to burn my battery calibrating FW to your light in order to 
 *  get the voltage levels reading and change the code accordingly
 *  
 */

 
#include <avr/io.h>
#include <util/delay.h>

#define TURBO 255

// #define RAMP_DOWN // comment/uncomment 

#ifndef RAMP_DOWN
  // Define PWM level for LED (default is rampup)
  #define M0 0   // Off
  #define M1 10  // Low ~40Lm
  #define M2 50  // Medium ~200Lm
  #define M3 100 // High ~400Lm
  #define M4 TURBO // Turbo ~1000Lm
#else
  // Define PWM level for LED for ramp down (need #define RAMP_DOWN)
  #define M0 0   // Off
  #define M4 10  // Low ~40Lm
  #define M3 50  // Medium ~200Lm
  #define M2 100 // High ~400Lm
  #define M1 TURBO // Turbo ~1000Lm
#endif


// Define timing
#define SEC 100 // each main loop cycle has delay(10) so hundred cycles are 100*10=1000 delay = 1 sec

#define LOW_BATT_LVL 98 // This value is measured using another program (https://github.com/YuvalSob/CavingHeadlampsFW/blob/main/HelloWorld/ADC.ino)
#define CUT_OFF_LVL 80 // This value is measured using another program  (https://github.com/YuvalSob/CavingHeadlampsFW/blob/main/HelloWorld/ADC.ino) 

//**********************************************
//                 mblink()
//**********************************************
// Function to blink the LED (used for low battery indication  
void mblink (int times, long on_time, long off_time){
  int curr_pwm = OCR0B; // saving the current brightness  

  for (int i = 0; i < times; i++) {
    OCR0B = curr_pwm; // Turn LED on
    delay(on_time);   // Wait on time
    OCR0B = M0;       // Turn LED off
    delay(off_time);  // Wait off time
  }
  OCR0B = curr_pwm; // restore brightness  
}

//**********************************************
//            check_batt()
//**********************************************
// Blinks if battery level is low
void check_batt() {
  long voltage = 0;
  int i;

  // Turn on ADC
  PRR &= ~(1<<PRADC);
  
  // Setting internal 1.1v as referance voltage to ADC
  // ADMUX – ADC Multiplexer Selection Register
  DIDR0 |= (1 << ADC1D);
  ADMUX  = (1 << REFS0) | (1 << ADLAR) | 0x01; // 1.1v reference, left-adjust, ADC1/PB2
  ADCSRA = (1 << ADEN ) | (1 << ADSC ) | 0x06; // enable, start, prescale

  ACSR   |=  (1<<7); //AC off

  // Averaging 8 conversions          
  for (i=0; i<8; i++) {
    ADCSRA |= (1 << ADSC); // Start conversion
    while (ADCSRA & (1 << ADSC)); // Wait for completion

    delay(50);
    voltage += ADCH; // Add conversion to sum
  }

  ADCSRA &= ~(1<<7); //ADC off
  voltage = voltage >> 3; // divide sum by 8

  // blink if low voltage 
  if (voltage < LOW_BATT_LVL) mblink(4, 150, 150);
  if (voltage < CUT_OFF_LVL) OCR0B = M0; // Turn off for very low voltage
}

//**********************************************
//            configure_active_mode()
//**********************************************
// Pin configuration etc.
void configure_active_mode(){
   DDRB = 0; // Reset all pins to input
   DDRB |= (1 << DDB1); // Set pin 6 (PB1) to output (1)
   DDRB |= (1 << DDB0); // Set pin 5 (PB0) to output (1)
   DDRB &= ~(0<<DDB3);  // Set pin 2 (PB3) to input  (0)
   PORTB |= (1<<PORTB3);  // Set pullup to pin 2 (PB3)(1)  

   // turn on timer
   PRR &= ~(1<<PRTIM0);

   // PWN configuration
   TCCR0A = 0b10100001;
   TCCR0B = 0b00000001;
}

//**********************************************
//       Interrupt service routine
//**********************************************
// AKA what to do when interrupt happens  
ISR (PCINT0_vect){
  MCUCR &= ~(1<<SE); // Disable sleep
  PCMSK = 0; // Mask all interrupts
  cli(); //disabling global interrupt
  configure_active_mode();
  
  while((PINB&8) == 0) // Wait button release  
  {
    delay(10);
  } 
  OCR0B = M1; // Turn on 1st level
}

//**********************************************
//                msleep()
//**********************************************
// Puts MCU in low power mode 
void msleep()
{
  // Turn off LED
  OCR0B = M0;
       
  // Pre sleep (make sure to wake up ONLY at PB2 pull down)
  WDTCR &= ~(1<<WDTIE); // Disable WDT interrupt
  GIMSK &= ~(1<<INT0); // Disable INT0 interrupt
  GIMSK |= (1<<PCIE); // Enable Pin change interrupt
  PCMSK = 0; // Mask all interrupts

  // Power saving 
  // Set all pins to output and drive "0"
  DDRB = 0b11111111;  // all output
  PORTB = 0b00000000;  // drive "0"  
  
  // Disable analog comparator to save power
  ACSR |= (1<<ACD); // Analog Comparator Disable

  // Disable ADC to save power
  ADCSRA &= ~(1<<ADEN);

  // Shut down ADC (The ADC must be disabled before shut down)
  PRR |= (1<<PRADC);

  // Stop Timer/Counter to save power
  // Clock select[2:0] = 000 - no clock
  TCCR0B &= 0b11111000;
   
  // Shut down Timer/Counter0 module to save power
  PRR |= (1<<PRTIM0);

  // Turn off watch dog timer to save power 
  cli(); // Global interrupts disable
  __asm__ __volatile__ ("wdr");  // watchdog_reset

  //Clear WDRF in MCUSR
  MCUSR &= ~(1<<WDRF);
  
  // Write logical one to WDCE and WDE
  // Keep old prescaler setting to prevent unintentional time-out
  WDTCR |= (1<<WDCE) | (1<<WDE);
  // Turn off WDT
  WDTCR = 0x00;

  DDRB &= ~(0<<DDB3);  // Set pin 2 (PB3) to input  (0)
  PORTB |= (1<<PORTB3);  // Set pullup to pin 2 (PB3)(1)       
  PCMSK |= (1<<PCINT3);   // Unmask Pin change interrupt to pin 2 
  sei(); // Enable global interrupt    
  MCUCR &= ~(0<<SM0); MCUCR |= (1<<SM1); // Sleep mode = power down
  MCUCR |= (1<<SE); // Enable sleep
  
  // Sleep
  __asm__ __volatile__ ("sleep");  
}

//**********************************************
//                main()
//**********************************************
int main() {
  int press_len = 0;
  boolean short_press = true; 
  int cycles_counter = 0; //counts cycles
  int sec_counter = 0; // counts sec
  int min_counter = 0; // counts min
  int turbo_counter = 0; // counts turbo time

  // Set pins 
  configure_active_mode();

  // After battery insert go to sleep to save power 
  msleep();

  // Main flow running forever 
  while(1) {

    // Reset counters
    cycles_counter = 0; //counts cycles
    sec_counter = 0; // counts sec
    min_counter = 0; // counts min
    turbo_counter = 0; // counts min

    // Wait for button press (PB2 pulled down)
    while((PINB&8) != 0) 
    {
      delay(10); // Wait some 
      cycles_counter++;

      // Count cycles
      if (cycles_counter == SEC) 
      {
        sec_counter++;
        cycles_counter = 0;
      }

      // Count sec
      if (sec_counter == 60) 
      {
        min_counter++;
        sec_counter = 0;
      }

      // Count min
      if (min_counter == 5){
        min_counter = 0;
        check_batt();
      }

      // If turbo
      if (OCR0B == TURBO){
        turbo_counter++;
        if (turbo_counter == 50*SEC)
        {
          turbo_counter = 0;
          OCR0B = M2; // Step down from Turbo
        }
      }
    } // Wait for button press (PB2 pulled down)

    press_len = 0; // reset 
    short_press = true;
    OCR0B = M1; // Start on 1st mode 

    // Ramp up 
    while((PINB&8) == 0) // // Wait for button release (PB2 pulled up) 
    {
      delay(10);
      press_len++;
      switch (press_len) {
        case 50 :  OCR0B = M2; short_press = false; break;
        case 100 : OCR0B = M3; break;
        case 150 : OCR0B = M4; break;
        case 200 : OCR0B = M1; press_len = 0; break;
      }
    }

    // Turn off and save power        
    if (short_press) {
      OCR0B = M0;
      delay(10);
      msleep();
      // ********* Continu here after wake up ********
    }
  } // forever
} // main
