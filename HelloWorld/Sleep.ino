/* A FW to understand sleep function to save power while off  
 * Waks up ONLY on pin2 pull down
 *
 *                     ---
 *                   -|   |- VCC
 *   Momentary Switch-|   |- Voltage ADC
 *        Colored LED-|   |- LED
 *               GND -|   |- LED
 *                     ---
 *
 * 1. Blinks Alternately befor sleep
 * 2. Wait for pin2 interrupt
 * 3. blinks fast (ISR)
 * 4. blinks slow after wakup
*/

#include <avr/io.h>
#include <util/delay.h>



void set_led(byte led, boolean mode){
   if (mode){
      switch (led) {
         case 5: PORTB |= (1<<PORTB0); break;
         case 6: PORTB |= (1<<PORTB1); break;  
      }
   }
   else {
      switch (led) {
         case 5: PORTB &= ~(1<<PORTB0); break;
         case 6: PORTB &= ~(1<<PORTB1); break;          
      }
   }
}

void mblink (int times, boolean led5a, boolean led6a, long delaya, boolean led5b, boolean led6b, long delayb){
   for (int i = 0; i < times; i++) {
      set_led(5, led5a); 
      set_led(6, led6a); 
      delay(delaya);   
      set_led(5, led5b); 
      set_led(6, led6b); 
      delay(delayb);   
   }
   set_led(5, false); 
   set_led(6, false); 
}

// Interrupt service routine
// AKA what to do when interrupt happenes 
ISR (PCINT0_vect){
   // Disable sleep - SE(Sleep Enable) = 0
   MCUCR &= ~(1<<SE);
   mblink (10, true, true, 200, false, false, 50);
}


void wdt_off(){
   cli(); // Global interrupts disable
   __asm__ __volatile__ ("wdr");  // watchdog_reset

   /* Clear WDRF in MCUSR */
   MCUSR &= ~(1<<WDRF);
   /* Write logical one to WDCE and WDE */
   /* Keep old prescaler setting to prevent unintentional time-out */
   WDTCR |= (1<<WDCE) | (1<<WDE);
   /* Turn off WDT */
   WDTCR = 0x00;
}

// Will only wakup on pull down of pin 2
// Dont forger to write ISR (Interrupt service routine)
void configure_interrupt() {
   DDRB &= ~(0<<DDB3);  // Set pin 2 (PB3) to input  (0)
   PORTB |= (1<<PORTB3);  // Set pullup to pin 2 (PB3)(1)  

  
   WDTCR &= ~(1<<WDTIE); // Disable WDT interrupt
   GIMSK &= ~(1<<INT0); // Disable INT0 interrupt
   GIMSK |= (1<<PCIE); // Enable Pin change interrupt
   PCMSK = 0; // Mask all interrupts
   PCMSK |= (1<<PCINT3);   // Unmask Pin change interrupt to pin 2 
   sei(); //enabling global interrupt
}

// Put AVR in power down sleep mode
void sleep(){
   // Set all pins to output and drive "0"
   DDRB = 0b11111111;  // all output
   PORTB = 0b00000000;  // drive "0"  
  
   // Disable analog comperator to save power
   ACSR |= (1<<ACD); // Analog Comparator Disable

   // Disable ADC to save power
   ADCSRA &= ~(1<<ADEN);

   // Shut down ADC (The ADC must be disabled before shut down)
   PRR |= (1<<PRADC);

   // Stop Timer/Counter
   // Clock select[2:0] = 000 - no clock
   TCCR0B &= 0b11111000;
   
   // Shut down Timer/Counter0 module
   PRR |= (1<<PRTIM0);

   // Turn off watch dog timer to save power 
   wdt_off();
   
   // Befor sleeping configure interrupt
   configure_interrupt();

   // SM[1:0](Sleep Mode) = 10 - power down 
   MCUCR &= ~(0<<SM0);
   MCUCR |= (1<<SM1);

   // SE(Sleep Enable) = 1
   MCUCR |= (1<<SE);
     
   // Sleep
   __asm__ __volatile__ ("sleep");  
}

void configure_pins(){
   DDRB = 0; // Reset all pins to input
   DDRB |= (1 << DDB1); // Set pin 6 (PB1) to output (1)
   DDRB |= (1 << DDB0); // Set pin 5 (PB0) to output (1)
   DDRB &= ~(0<<DDB3);  // Set pin 2 (PB3) to input  (0)
   PORTB |= (1<<PORTB3);  // Set pullup to pin 2 (PB3)(1)  
}

int main() {

  configure_pins();
  
   while(1) {
      mblink (5, true, false, 500, false, true, 500); // blink LEDs Alternately befor sleep 
      sleep();                                        // Go to sleep

      // ========================  Wait for switch press ============================================  

      // ISR also have internal blinks
      mblink (5, true, true, 500, false, false, 500); // blink LED together
  }
} // main
