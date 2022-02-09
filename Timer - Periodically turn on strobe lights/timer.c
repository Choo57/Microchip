/*
 * Project name:
     Timer Relay with LCD Display
 * Copyright:
     (c) Cagan Cerkez, 2011.
 * Description:
     Timer with an HD44780 LCD display that operates a relay continuously
      throughout the day (until stopped) for a set period. The period can be
      changed by stopping the counter and using the UP or DOWN buttons.
 * Test configuration:
     MCU:                      PIC16F887
     Oscillator:               4.0000 MHz
     LCD:                      HD44780, 2x16 blue character
     SW:                       mikroC PRO for PIC
 * NOTES:
     - Start/Stop button connected to RB0
     - UP button connected to RD3
     - DOWN button connected to RD2
     - Relay connected to RD1
     - LCD connected to RB2-RB7
*/

// LCD module connections
sbit LCD_RS at RB1_bit;
sbit LCD_EN at RB3_bit;
sbit LCD_D4 at RB4_bit;
sbit LCD_D5 at RB5_bit;
sbit LCD_D6 at RB6_bit;
sbit LCD_D7 at RB7_bit;

sbit LCD_RS_Direction at TRISB1_bit;
sbit LCD_EN_Direction at TRISB3_bit;
sbit LCD_D4_Direction at TRISB4_bit;
sbit LCD_D5_Direction at TRISB5_bit;
sbit LCD_D6_Direction at TRISB6_bit;
sbit LCD_D7_Direction at TRISB7_bit;
//End LCD module connections

unsigned char start_countdown = 0;     // Count-down if 1, stop and set period if 0
unsigned char light_counter = 7;  // Seconds the light will stay on
signed int i=0;   // Signed so the counter counts until 0
unsigned char j=0, k=0;
long time, def_val=30;
const unsigned char array_size=12;
char time_txt[array_size], time_temp[array_size];


// If start/stop button is pressed
void interrupt () {
     if (start_countdown == 1) {             // Toggle the flag value
         start_countdown = 0;
     }
     else {                            // Toggle the counter value
         start_countdown = 1;
     }
     INTCON.INTF = 0;                  // Clear the interrupt flag
}

void blink_light() {                   // Strobe light flashes itself, period
    for(i=light_counter ; i >= 0; i--) {  //   setting is on the light
        if (start_countdown == 0) break;    // Break from the loop if stop button
        Lcd_Out(2,1," Change!  (");    //   is pressed while the light blinks
        Lcd_Chr_Cp(i+48);
        Lcd_Chr_Cp(')');
        PORTD.F1 = 1;
        Delay_ms(1000);
    }
    PORTD.F1 = 0;
    Lcd_Out(2,1," Time:          ");        // Clear second line
}

void copy_nonblank () {    // Copy non-blank values to the time array for
      j=0;                 //     correct display on the LCD
      for(k=0; k<=array_size-1; k++) {
         if(time_temp[k] != ' ' && time_temp[k] != '\0') {
             time_txt[j] = time_temp[k];
             j++;
         }
      }
      for(; j<array_size-2; j++) {              // Blank out the empty positions
          time_txt[j] = ' ';                    // for correct display on LCD
      }
      time_txt[array_size-1] = '\0';
}

void main() {

  ANSEL  = 0;            // Configure AN pins as digital
  ANSELH = 0;
  C1ON_bit = 0;          // Disable comparators
  C2ON_bit = 0;

  TRISA = 0x00;          // Set direction to be output
  TRISB = 0x01;          // RB0 (INT0) input
  TRISC = 0x00;          // Set direction to be output
  TRISD = 0x0C;          // RD2 & RD3 (buttons) input, rest output

  PORTD.F1 = 0;          // Relay port is 0
  PORTB.F2 = 0;          // Make R/W pin on LCD ground through PIC
                         //   to prevent the need to double layer PCB

  INTCON = 0x90;           // Interrupt flags:
                           //Enable Global Interrupt
                           //Enable External Interrupt
  WPUB = 0x01;             // Weak Pull-Up on pin B0

  Lcd_Init();                             // Initialize LCD
  Lcd_Cmd(_LCD_CLEAR);                    // Clear display
  Lcd_Cmd(_LCD_CURSOR_OFF);               // Cursor off
  Lcd_Out(1,1," Vivafit Cyprus");
  Lcd_Out(2,1," Love living fit");
  Delay_ms(4000);
  Lcd_Out(2,1," Time:          ");        // Clear second line

  while(1) {
      time = def_val;
      LongToStr(time, time_temp);
      copy_nonblank();
      Lcd_Out(2,8,time_txt);           // Write on LCD

      if(start_countdown == 1) {                   // Start button is pressed
         time = def_val;
         for(time; time >= 0; time--) {
             if(start_countdown == 0) break;
             LongToStr(time, time_temp);
             copy_nonblank();
             Lcd_Out(2,8,time_txt);           // Write on LCD
             Delay_ms(1000);                  // Count down every 1 second
         }
         if(start_countdown == 1) {
             Blink_light();
             //start_countdown = 0; Commented out to have a continuous loop
         }
      }

      if (start_countdown == 0) {
         if (PORTD.F3 == 0) {          // If UP button is pressed
             Delay_ms(100);            //  increment timer
             if (PORTD.F3 == 0) {      // Prevent debouncing
                 time++;
                 LongToStr(time, time_temp);
                 copy_nonblank();
                 Lcd_Out(2,8,time_txt);
                 def_val = time;       // Set the default value to current
             }
         }
         if (PORTD.F2 == 0  && time > 0) {     // If DOWN button is pressed
             Delay_ms(100);                    //  decrement timer
             if (PORTD.F2 == 0) {              // Time setting always > 0
                 time--;
                 LongToStr(time, time_temp);
                 copy_nonblank();
                 Lcd_Out(2,8,time_txt);
                 def_val = time;            // Set the default value to current
             }
         }
      }
  }

}