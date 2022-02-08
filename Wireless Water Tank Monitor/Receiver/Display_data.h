/*
   Display Temperature and Water Level Header file

   Defines:
          Displays either the water level or the water temperture
          after checking the relevant flag
*/


void Display_data() {

 // DISPLAY DATA
 if (display_temp == 0) {
     displayNumber(1, level/100);          // Display WATER LEVEL
     displayNumber(2, ((level/10)%10));
     displayNumber(3, level%10);
     displayNumber(4, 0);
 } else {            // ******** FOR TESTING, replaced    temperature    with    TEST_MISSES ********
       displayNumber(1, TEST_MISSES/100);
       displayNumber(2, ((TEST_MISSES/10)%10));
       displayNumber(3, TEST_MISSES%10);
       displayNumber(4, 17);          // Display Celsius C sign
       display_temp = 0;
       
       Delay_ms(500);
       
       displayNumber(1, level/100);          // Display WATER LEVEL
       displayNumber(2, ((level/10)%10));
       displayNumber(3, level%10);
       displayNumber(4, 0);
   } // End of DISPLAY DATA
}

void clear_display() {           // This function does NOT clear the SINE and BAT warning leds
       displayNumber(1, 50);     // Clear first character
       displayNumber(2, 50);     // Clear the display
       displayNumber(3, 50);     // Clear the display
       displayNumber(4, 50);     // Clear the Celcius sign if it is displayed
       Delay_ms(100);
}