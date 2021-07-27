/* A very basic program to blink 3 LEDS 

 *                      ---
 *                    -|   |- VCC
 *                    -|   |- 
 *                    -|   |- LED6 -> resistor -> GND
 *               GND  -|   |- LED5 -> resistor -> GND
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

    PORTB |= (1 << PB0);   // turn on pin5               
    PORTB &= ~(1 << PB1);  // turn off pin6               
    delay(250); //wait a bit before checking again

    PORTB |= (1 << PB1);   // turn on pin6               
    PORTB &= ~(1 << PB0);  // turn off pin5   
    delay(250); //wait a bit before checking again
    
  } // Endless loop
} // main
