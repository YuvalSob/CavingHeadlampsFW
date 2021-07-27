/* A very simnple program to understan momentary switch use
 * Press - LED 5 is on, LED 6 is off
 * Release - LED 6 is on, LED 5 is off 
 *
 *                      ---
 *                    -|   |- VCC
 *   Momentary Switch -|   |- 
 *                    -|   |- LED6
 *               GND  -|   |- LED5
 *                      --- 
 */
 
#include <avr/io.h>
#include <util/delay.h>

int main() {
  // Pins configuration
  // Set pin 5 to output
  DDRB |= (1 << DDB0);
  
  // Set pin 6 to output
  DDRB |= (1 << DDB1);

  // Set pin 2 (momentary switch) to pull up (it is input by reset value)
  PORTB |= (1 << PB3);
  
  while(1) {                  //Endless loop

     if ((PINB&8) == 0) {    // When the button is pressed (PB3 pulled down)
       PORTB |= (1 << PB0);   // turn on pin5               
       PORTB &= ~(1 << PB1);  // turn off pin6               
     }
     else 
     {
       PORTB |= (1 << PB1);   // turn on pin6               
       PORTB &= ~(1 << PB0);  // turn off pin5   
     }

    delay(25); //wait a bit before checking again
    
  } // Endless loop
} // main
