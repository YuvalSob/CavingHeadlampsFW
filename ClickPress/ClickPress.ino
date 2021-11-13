/*   Project:Or ClickPress V1.0 
 *    
 *   FW for a single driver light  
 *   
 *  1 driver 2 LEDs 
 * 
 *  FUSE:
 *  ----
 *  BOD - Disabled (to save power)
 *  Clock - 4.8MHz (delay (1,000) = 1 sec) 
 *  
 *  Features:
 *  ---------
 *  1. 3 basic flood modes 
 *  2. 2 Turbo modes (one for each beam) 
 *  3. Special ramping mode
 *  4. Low power consumption while off
 *  5. Battery check mode
 *  6. lock off
 *  7. Low voltage indication 
 *  8. Very low voltage protection
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
 *               GND -|   |- PWM LED
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

#define M_TURBO 255  // ~800Lm
#define HOT   180  // ~600/700Lm
#define M_HIGH  130  // ~400Lm
#define M_MED   50   // ~160Lm
#define M_LOW   10   // ~30Lm
#define M_OFF 0

// Define valuse for Battery Check
// These values were measured using my battery calibrating FW
#define V3 170 // ADC reading for V=3.9v 
#define V2 160 // ADC reading for V=3.7v
#define V1 155 // ADC reading for V=3.6v
#define V0 146 // ADC reading for V=3.4v



// Define timing
#define SEC 100 // each main loop cycle has delay(10) so hundred cycles are 100*10=1000 delay = 1 sec


#define M_LOW_BATT_LVL 98 // These values were measured using my battery calibrating FW
#define CUT_M_OFF_LVL 80 // These values were measured using my battery calibrating FW

//**********************************************
//            mblink()
//**********************************************
// Function to blink the LED (used for low battery indication  
void mblink (byte times, byte on_time, byte off_time){
  byte curr_pwm_a = OCR0A; // saving the current brightness  
  byte lvl = OCR0A == M_OFF ? M_MED : OCR0A; // brightness is current unless it is off  
   
    OCR0A = M_OFF; // Turn LED off
    OCR0B = M_OFF; // Turn LED off
    
  for (byte i = 0; i < times; i++) {
    OCR0A = lvl; // Turn LED on
    delay(on_time);   // Wait on time
    OCR0A = M_OFF;       // Turn LED off
    delay(off_time);  // Wait off time
  }
  OCR0A = curr_pwm_a; // restore brightness  
}


//**********************************************
//            meas_batt()
//**********************************************
// measure battery voltage 
long meas_batt() {
  long tmp_voltage = 0;
  byte i;

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
    tmp_voltage += ADCH; // Add conversion to sum
  }

  ADCSRA &= ~(1<<7); //ADC off
  tmp_voltage = tmp_voltage >> 3; // divide sum by 8

  return(tmp_voltage);
}


//**********************************************
//            check_batt()
//**********************************************
// Blinks if battery level is low
// Value for low level can be measured using my bat_voltage program 
void check_batt() {
  long voltage = meas_batt();

  // blink if low voltage 
  if (voltage < M_LOW_BATT_LVL) mblink(4, 150, 150);
  if (voltage < CUT_M_OFF_LVL) {
    OCR0A = M_OFF; // Turn off for very low voltage
    OCR0B = M_OFF; // Turn off for very low voltage
  }
}

//**********************************************
//            batt_lvl()
//**********************************************
// Blinks according to battery voltage
// Value for levels can be measured using my bat_voltage program 
byte batt_lvl() {
  long voltage = meas_batt();


  if (voltage > V3) return(4); // Almost full
  if (voltage > V2) return(3); // Above 75%
  if (voltage > V1) return(2); // Above 50% 
  if (voltage > V0) return(1); // Above 25%
  return(0); // Almost empty 
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

  
  
  //OCR0A = M_LOW; // Turn on 1st level
}

//**********************************************
//                msleep()
//**********************************************
// Puts MCU in low power mode 
void msleep()
{
  // Turn off LED
  OCR0A = M_OFF;
  OCR0B = M_OFF;
       
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
  byte press_len = 0;
  boolean short_press = true; 
  boolean double_click = true;
  boolean triple_click = false;
  boolean first_turbo = true;
  boolean turn_off = false;
  boolean lock = false;
  boolean just_after_press = false;

  byte cycles_counter = 0; //counts cycles
  byte sec_counter = 0; // counts sec
  byte min_counter = 0; // counts min
  int turbo_counter = 0; // counts turbo time
  int lock_counter = 0; // counts time before lock

  // Set pins 
  configure_active_mode();

  // After battery insert go to sleep to save power 
  lock = true;
  msleep();

  // Main flow running forever 
  while(1) {

    // Reset counters
    double_click = !just_after_press;
    just_after_press = false;
    
    cycles_counter = 0; //counts cycles
    sec_counter = 0; // counts sec
    min_counter = 0; // counts min
    turbo_counter = 0; // counts turbo time

    /* Wait for button press (PB2 pulled down) */
    while((PINB&8) != 0) 
    {
      delay(10); // Wait some 

      // Count cycles
      cycles_counter++;
      // If long time since last click it is not double/triple click
      if (double_click == true && cycles_counter == 35) double_click = false;
      if (triple_click == true && cycles_counter == 35) triple_click = false;

      // Count sec
      if (cycles_counter == SEC) 
      {
        sec_counter++;
        if (first_turbo == false && sec_counter == 3) first_turbo = true;
        cycles_counter = 0;
      }

      // Count min
      if (sec_counter == 60) 
      {
        // Sleep after 1 min in lock with no unlock try 
        if (lock) {
          mblink(2, 150, 150);
          delay(10);
          msleep();
          // ********* Continue here after wake up ********
        }
        min_counter++;
        sec_counter = 0;
      }

      // Count 5 min
      if (min_counter == 5){
        min_counter = 0;
        check_batt();
      }

      // If turbo
      if (OCR0A > HOT || OCR0B > HOT){
        turbo_counter++;
        if (turbo_counter == 50*SEC)
        {
          OCR0A = M_MED; // Step down from Turbo
          OCR0B = M_OFF; // Step down from Turbo
        }
      }

      // If off
      // After 1 min off -> lock
      if (OCR0A == M_OFF && OCR0B == M_OFF){
        lock_counter++;
        if (lock_counter == 10*SEC) { // TODO 60
          lock = true;
          mblink(1, 150, 150);
          delay(10);
          msleep();
          // ********* Continue here after wake up ********
        } 
      }      
    } // Wait for button press (PB2 pulled down)


    /* Wait for button release (PB2 pulled up) */ 
    press_len = 0; // reset 
    short_press = true; // reset 
    
    while((PINB&8) == 0) 
    {
      delay(10);
      press_len++;
      // Press (not click)
      if (press_len == 50) {
        short_press = false;
        just_after_press = true;
        // Just press (without click before)
        if (double_click == false) {
          turn_off = true;
          OCR0A = M_OFF; // turn off on long press 
          OCR0B = M_OFF; // turn off on long press 
        }
        // Press after click
        else{
          if (lock == false)
          {
            turbo_counter = 0; // reset at start of turbo
            // Spot turbo
            if (first_turbo){
              first_turbo = false;
              OCR0A = M_OFF;
              OCR0B = M_TURBO;
              while((PINB&8) == 0) // // Wait for button release (PB2 pulled up)
              {
                delay(150);
                OCR0B -= 10;
              }
            }
            // Flood turbo
            else{
              first_turbo = true;
              OCR0B = M_OFF;
              OCR0A = M_TURBO;
              while((PINB&8) == 0) // // Wait for button release (PB2 pulled up)
              {
                delay(150);
                OCR0A -= 10;
              }
            }
          }
        }
      }
    }// wait for release

    /* After release */ 
    // Long press
    if (short_press == false) {
      if (turn_off) {
        turn_off = false;
        lock_counter = 0;
        delay(10);
      }
    }
    
    // click
    else {
      // Triple click
      if (triple_click) {
        mblink (10, 50,150);
        triple_click = false;
        lock = false;
        OCR0A = M_LOW;
      }
      else {
        // Double click
        if (double_click) {
          double_click = false;
          triple_click = true;
          if (lock == false) {
            mblink (batt_lvl(), 100, 200);      
          }
        }
        else{
          // Single click
          if (lock == false) {
            switch (OCR0A) {
              case M_LOW : OCR0A = M_MED; break;
              case M_MED : OCR0A = M_HIGH; break;
              case M_HIGH : OCR0A = M_LOW; break;
              default:  {
                OCR0A = M_MED;
                OCR0B = M_OFF;
              }
            }
          }
        }
      }
    }      
  } // forever
} // main
