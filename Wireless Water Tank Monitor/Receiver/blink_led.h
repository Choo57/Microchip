/*
   LED Header file

   Defines:
          IC connection pins
          Function to blink led
*/


void blink_led(){
        PORTD.F1 = 1;
        Delay_ms(60);
        PORTD.F1 = 0;
        Delay_ms(60);
}