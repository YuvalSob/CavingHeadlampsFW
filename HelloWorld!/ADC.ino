// This firmware is based on Battcheck firmware by ToyKeeper (thanks ToyKeeper)
// http://bazaar.launchpad.net/~toykeeper/flashlight-firmware/trunk/view/head:/ToyKeeper/battcheck/battcheck.c

/* A program to understand ADC (Analog to Digital Converter) - AKA battarey measuring 
 *  
 *  ADC is taking an analog value and convert it into a digital number 
 *  In this case the ADC converts the voltage in pin 7 (ADC1) to 10 bits number (0-2^10) 
 *  
 *  Pins: 
 *  pin 2 - momentary switch to Vss (input, pullup)
 *  pin 5 - LED (output)
 *  pin 6 - LED (output)
 *  pin 7 - ACD input (voltage divider)    Vcc -> R1 (19.1 Kohm) -> ADC1 -> R2 (4.7Kohm) -> Vss
 *  
 *                     ---
 *                   -|   |- VCC
 *   Momentary Switch-|   |- Voltage ADC
 *        Colored LED-|   |- LED
 *               GND -|   |- LED
 *                     ---
 *  
 *  In order to measur the battarey voltage, we are using the ADC output  
 *  The ADC output is representing the digital value of the voltage in pin 7, relative to the referance internal voltage of 1.1v
 *  We don't raelly care hoe the ADC output represet the voltage, we only intrested what is the ADC output for some pre defined input voltage levels.
 *  
 *  In order to get an accurate mesure we will avrage 8 ADC samples.  
 *  To prefore an ADC sample we have to write 1 to ADSC (ADC Start Conversion) bit
 *  When the conversion is complete, this bit returns to zero
 *  
 *  Since we don't need a very accurate read we will only use the 8 most segnificant bits
 *  
 *  On each press of the button (pin 2 to Vss) we will take 8 ADC conversion and avrage the results 
 *  the result will be presented by the LEDs:
 *  a. 2 fast blinks on pin 6 (PB1) 
 *  b. pin 5 (PB0) blinks as the hundrede digit
 *  c. 2 fast blinks on pin 6 (PB1) 
 *  d. pin 5 (PB0) blinks as the tenths digit
 *  e. 2 fast blinks on pin 6 (PB1) 
 *  f. pin 5 (PB0) blinks as the units digit
 *  
 *  Example: 106 = PB1*2 -> PB0*1 -> PB1*2 -> no blinks -> PB1*2 -> PB0*6
 */
#include <avr/io.h>
#include <util/delay.h>

#define ADC_CHANNEL 0x01    // MUX 01 corresponds with PB2
#define ADC_DIDR    ADC1D   // Digital input disable bit corresponding with PB2
#define ADC_PRSCL   0x06    // clk/64

//**********************************************
//            get_batt_lvl()
//**********************************************
// Represents the ADC conversion of the battarey voltage using LEDs blinks 
uint8_t get_batt_lvl() {
  uint16_t voltage = 0;
  uint8_t i;


  DIDR0 |= (1 << ADC1D);
  ADMUX  = (1 << REFS0) | (1 << ADLAR) | ADC_CHANNEL; // 1.1v reference, left-adjust, ADC1/PB2
  ADCSRA = (1 << ADEN ) | (1 << ADSC ) | ADC_PRSCL;   // enable, start, prescale

  ACSR   |=  (1<<7); //AC off
            
  for (i=0; i<8; i++) {
    // Start conversion
    ADCSRA |= (1 << ADSC);
    // Wait for completion
    while (ADCSRA & (1 << ADSC));
    _delay_ms(50);
    voltage += ADCH;
  }


  // ADC off 
  ADCSRA &= ~(1<<7); //ADC off
  voltage = voltage >> 3;

   mblink(voltage/100);
   delay(1500);
   mblink((voltage%100)/10);
   delay(1500);
   mblink(voltage%10);
}

//**********************************************
//            configure_pins
//**********************************************
// Function to configure pins directions
void configure_pins(){
   DDRB |= (1 << DDB1); // Set pin 6 (PB1) to output
   DDRB |= (1 << DDB0); // Set pin 5 (PB0) to output)
   DDRB &= ~(0<<DDB3);  // Set pin 2 (PB3) to input 
   PORTB |= (1<<PORTB3);  // Set pullup to pin 2 (PB3) 
}

//**********************************************
//                 mblink
//**********************************************
// Function to slow blink LED connected to pin 5
// Input is number of blinks
void mblink (int num) {
   uint8_t i;
   fblink(2);

   for (i=0; i<num; i++) {
      delay(500);
      PORTB |= (1<<PORTB0); // Turn on LED
      delay(500);
      PORTB &= ~(1<<PORTB0); // Turn off LED
      }
   delay(100);
}

//**********************************************
//                 fblink
//**********************************************
// Function to fast blink LED connected to pin 6
// Input is number of blinks
void fblink (int num) {
   uint8_t i;
   for (i=0; i<num; i++) {
      delay(20);
      PORTB |= (1<<PORTB1); // Turn on LED on pin 6 
      delay(50);
      PORTB &= ~(1<<PORTB1); // Turn off LED on pin 6
   }
   delay(100);
}


//**********************************************
//                 Main
//**********************************************
int main() {
   configure_pins(); // Initial configuration

   while(1) {
      if ((PINB&8) == 0) get_batt_lvl(); // If pin 2 is pulled down -> measure voltage
      delay(10);
  } // endless loop
} // main
