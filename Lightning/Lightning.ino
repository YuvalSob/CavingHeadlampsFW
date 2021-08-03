/*
 * Extraction of the lightning mode from Anduril by ToyKeeper
 * Can be used under the same condition as the original Anduril Code:

 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

*/

#include <avr/io.h>
#include <util/delay.h>

#define MAX_LEVEL 255
volatile uint8_t pseudo_rand_seed = 0;

uint8_t pseudo_rand() {
    static uint16_t offset = 1024;
    // loop from 1024 to 4095
    offset = ((offset + 1) & 0x0fff) | 0x0400;
    pseudo_rand_seed += 0b01010101;  // 85
    return pgm_read_byte(offset) + pseudo_rand_seed;
}

void lightning_storm_iter() {
    // one iteration of main loop()
    int16_t brightness;
    uint16_t rand_time;

    // turn the emitter on at a random level,
    // for a random amount of time between 1ms and 32ms
    //rand_time = 1 << (pseudo_rand() % 7);
    rand_time = pseudo_rand() & 63;
    brightness = 1 << (pseudo_rand() % 7);  // 1, 2, 4, 8, 16, 32, 64
    brightness += 1 << (pseudo_rand() % 5);  // 2 to 80 now
    brightness += pseudo_rand() % brightness;  // 2 to 159 now (w/ low bias)
    if (brightness > MAX_LEVEL) brightness = MAX_LEVEL;
    OCR0B = brightness; // set_level(brightness);
     delay(rand_time); // nice_delay_ms(rand_time);

    // decrease the brightness somewhat more gradually, like lightning
    uint8_t stepdown = brightness >> 3;
    if (stepdown < 1) stepdown = 1;
    while(brightness > 1) {
         delay(rand_time); // nice_delay_ms(rand_time);
        brightness -= stepdown;
        if (brightness < 0) brightness = 0;
        OCR0B = brightness;  //set_level(brightness);
        /*
           if ((brightness < MAX_LEVEL/2) && (! (pseudo_rand() & 15))) {
           brightness <<= 1;
           set_level(brightness);
           }
           */
        if (! (pseudo_rand() & 3)) {
             delay(rand_time); // nice_delay_ms(rand_time);
            OCR0B = (brightness >> 1); //set_level(brightness>>1);
        }
    }

    // turn the emitter off,
    // for a random amount of time between 1ms and 8192ms
    // (with a low bias)
    rand_time = 1 << (pseudo_rand() % 13);
    rand_time += pseudo_rand() % rand_time;
    OCR0B = 0; //set_level(0);
    delay(rand_time); // nice_delay_ms(rand_time);  // no return check necessary on final delay
}

int main()
{
   DDRB = 0; // Reset all pins to input
   DDRB |= (1 << DDB1); // Set pin 6 (PB1) to output (1)
   DDRB |= (1 << DDB0); // Set pin 5 (PB0) to output (1)

   // PWN configuration
   TCCR0A = 0b10100001;
   TCCR0B = 0b00000001;
   
  while (1)
  {
     lightning_storm_iter();
  }
}
