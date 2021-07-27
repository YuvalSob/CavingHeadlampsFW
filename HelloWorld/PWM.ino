#include <avr/io.h>
#include <util/delay.h>

/* This code is to understand PWM for LED driving.
 * Output pins are: 
 * Pin 5 - OC0A – Output Compare pin A
 * Pin 6 - OC0B – Output Compare pin B
 * 
 * The output compare pin value is set by the comperator when the counter value is equal to Output Compare register (OCR0A or OCR0B)
 * Foe driving the LEDs PWM (phase correct) mode is used.
 * This mode is selected by:
 * 1. WGM0[2:0] (Waveform Generation Mode) = 0b001
 * 2. COMOA[1:0] (Compare Match Output A) = 0b10 
 * 2. COMOB[1:0] (Compare Match Output B) = 0b10 
 * 
 * Using this mode will claer the output compare pin when the counter value is equal to output compare register while counting up 
 * and set the output compare pin when the counter value is equal to output compare register while counting down 
 *  
 *| 
 *|         /\            /\            /\            /\         counter = 255
 *|        /  \          /  \          /  \          /  \      
 *|  _____/____\________/____\________/____\________/____\_____  counter = OCR0
 *|      /'    '\      /'    '\      /'    '\      /'    '\  
 *|     / '    ' \    / '    ' \    / '    ' \    / '    '\   
 *|    /  '    '  \  /  '    '  \  /  '    '  \  /  '    ' \  
 *|   /   '    '   \/   '    '   \/   '    '   \/   '    '  \    counter = 0
 *|       '    '        '    '        '    '        '    '    
 *|   ====|    |========|    |========|    |========|    |====== OC0 = "1" 
 *|       |    |        |    |        |    |        |    |
 *|       |====|        |====|        |====|        |====|       OC0 = "0"
 *
  */
      
     
 

void reset_params(){
   // PWN configuration
   TCCR0A = 0b10100001; // reset 
   //TCCR0A |= ((0b10<<COM0A0)|(0b10<<COM0B0)|(0b01<<WGM00)); //  COM0A = 10, COM0B = 10, WGM = (0)01 -> TCCR0A = 0b10100001
   TCCR0B = 0b00000001; // Reset
   //TCCR0B |= ((0<<FOC0A)|(0<<FOC0B)|(0<<WGM02)|(0b001<<CS00)); // FOC0A = 0, FOC0B = 0, WGM = 0(01), CS0 = 0b001 -> TCCR0B = 0b00000001

   // Pin configuration
   DDRB |= (1 << DDB0);  // Set pin 5 (PB0) to output
   DDRB |= (1 << DDB1);  // Set pin 6 (PB1) to output
   
   DDRB &= ~(1 << DDB3);  // Set pin 2 (PB3) to input
   PORTB |= (1 << PORTB3); // Set pin 2 (PB3) to pull up

   // Turn off LEDs
   OCR0B = 0;                    
   OCR0A = 0; 
}  

int main(){
   reset_params();
   while (1){
      if ((PINB&8) == 0) { // if switch is pressed
         if (OCR0A == 0){
            OCR0A = 255; // reset to full if get to off
         }
         else {
            OCR0A -= 10; // decrease brightness 
         }
      }
      else {
         OCR0A = 0; // turn of if not pressed
      }
      delay(200);
   }  
}
