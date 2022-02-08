/*
   LED Header file

   Defines:
          IC connection pins
          Function to blink led
*/


void blink_led() {
    PORTA.F7 = 1;
    Delay_ms(40);
    PORTA.F7 = 0;
    Delay_ms(40);
}