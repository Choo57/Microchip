/*
 * Project name:
     Paintball Scenario Clock Bomb
 * Copyright:
     (c) Cagan Cerkez, 2013.
 * Description:
     PCB of a Scenario Clock Bomb for Paintball. Circuit includes a keypad for
     entering the security key to activate/deactive the timer, an LCD for display,
     a key lock for activating/deactivating the circuit, green & red LEDs and a
     speaker as an audible alarm
 * Test configuration:
     MCU:                      PIC16F887
     MCU oscillator:           Internal 4Mhz
     TMR1:                     32.768 kHz external LP oscillator
     LCD:                      HD44780, 2x16 blue character
     Keypad:                   3x4 keypad
     SW:                       mikroC PRO for PIC
 * NOTES:
     -
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

// Keypad module connections
char keypadPort at PORTD;
// End of Keypad module connections

// Tri-color LED Transistor connections
sbit RED at RE1_bit;
sbit GREEN at RE0_bit;
sbit YELLOW at RE2_bit;

sbit RED_Direction at TRISE1_bit;
sbit GREEN_Direction at TRISE0_bit;
sbit YELLOW_Direction at TRISE2_bit;
// End of LED connections

// Key switch connections
sbit arm_switch at RA1_bit;
sbit arm_switch_Direction at TRISA1_bit;
// End of Key switch connections

unsigned short int updated=1;
unsigned short int a_code[9]; //= {0,0,0,0,0,0,0,0,0};     // Activation code
unsigned short int hhmmss[6] = {0,0,0,0,0,0};     // Entered game time
unsigned short int pos=0, key=0, code_length = 0;
char enter_pressed = 0;

// TMR1 interrupt trigerred every second
void interrupt () {
    TMR1H = 0x80;                  // Set initial value for the timer TMR1
    TMR1L = 0x00;
    PIR1.TMR1IF = 0;               // Reset the TMR1IF flag bit
    updated = 1;                   // Refresh the LCD
}

void key_pressed_beep () {
 PORTA.F4 = 1;
 Delay_ms(40);
 PORTA.F4 = 0;
}

// Read Keypad values
unsigned short int kp_text (unsigned short int key_pressed) {
    key_pressed_beep();
    switch (key_pressed) {
      case  1: key_pressed = '1'; break;
      case  2: key_pressed = '2'; break;
      case  3: key_pressed = '3'; break;
      case  5: key_pressed = '4'; break;
      case  6: key_pressed = '5'; break;
      case  7: key_pressed = '6'; break;
      case  9: key_pressed = '7'; break;
      case 10: key_pressed = '8'; break;
      case 11: key_pressed = '9'; break;
      case 13: key_pressed = '*'; break;
      case 14: key_pressed = '0'; break;
      case 15: key_pressed = '#'; break;
      default: key_pressed = 0;
    }
    return key_pressed;
}

// Received Game Time and Activation Code
void initialize_device(void) {
unsigned short int i=0;

// * * * * * * Enter Game Time * * * * * *
  Lcd_Out(1,1,"ENTER GAME TIME:");
  Lcd_Out(2,1,"  :   (hh:mm)");

  pos = 1;
  for (i=0; i<4; i++) {
      Lcd_Chr(2,pos,'_');
      
      do {
       hhmmss[i] = Keypad_Key_Press();               // Store set game time
       Delay_ms(150);
       
       if (hhmmss[i]==15) {                          //  # function as enter
           hhmmss[i]=0;                              //    Disable it until 4 digits are entered for time
       }
       
       if (hhmmss[i]==13){                           //  * function as backspace (delete)
           key_pressed_beep();
           if (i>0) {                                // Do nothing if backspace is pressed when nothing is entered
                i--;
                if (pos==4) {                        // Skip LCD character position 3 (: colon)
                    Lcd_Chr(2,pos,' ');              // Clear the 4th position before going to 2nd position
                    pos = 2;
                } else {
                      if (pos != 3) {                // If LCD character position is at colon's position (position 3)
                          Lcd_Chr(2,pos,' ');
                      }
                      pos--;
                  }
                Lcd_Chr(2,pos,'_');
            }
           hhmmss[i]=0;
       }
       if (i==2 && hhmmss[2]>6 && hhmmss[2]!=14) {   // Do not allow second digit of min > 5
           hhmmss[i]=0;
       }
      } while (!hhmmss[i]);
      
      hhmmss[i] = kp_text(hhmmss[i]);
      Lcd_Chr(2, pos, hhmmss[i]);                 // Print key ASCII value on LCD
      pos++;
      if (pos == 3) {                             // Skip LCD position 3 for the colon ":"
          pos = 4;
      }
  
      if (i==3) {
          do {
           enter_pressed = Keypad_Key_Press();    // Wait for the # key to set the game time
           Delay_ms(150);
           if (enter_pressed==13) {               // If * is pressed, delete the last character and continue
               i--;
               pos--;
               Lcd_Chr(2,pos,'_');
               enter_pressed = 15;
           }
          } while (enter_pressed != 15);
          key_pressed_beep();
      }
  }

   hhmmss[4] = hhmmss[5] = '0';                  // Seconds start from :00

  Delay_ms(100);
  Lcd_Cmd(_LCD_CLEAR);                    // Clear display

  // * * * * * * Enter Activation Code * * * * * *
  Lcd_Out(1,1,"ENTER ACTIVATION");
  Lcd_Out(2,1,"CODE: ");

  enter_pressed = 0;
  pos = 7;
  for (i=0; i<9; i++) {       // Allow a maximum of 9 characters (fits in the second row)
      do {
       a_code[i] = Keypad_Key_Press();             // Store key code in a_code variable
       Delay_ms(150);
       
       if (a_code[i] == 13) {                      // If * (delete) is pressed
          key_pressed_beep();
          if (i>0) {                              //   If no characters are entered, do nothing
              pos--;
              i--;
              Lcd_Chr(2, pos, ' ');
              a_code[i] = 0;
          } else {
            a_code[i] = 0;
            }
       }
       
       if (a_code[i] == 15) {            // Check if "enter" # is pressed
           key_pressed_beep();
           if (i > 0) {                  //  If at least 1 character is entered, set the code
               enter_pressed = 1;
           } else {
             a_code[i] = 0;              // Do not allow the first character of the code to be #
             }
       }
      } while (!a_code[i]);
      
      if (enter_pressed == 1) {
          code_length = i;      // Store activation code length
          i=10;                   // Break the loop
      } else {
          a_code[i] = kp_text(a_code[i]);
          Lcd_Chr(2, pos, a_code[i]);                 // Print key ASCII value on LCD
          pos++;
        }

      if (i==8) {                                 // If all 9 characters are entered, wait for the enter or the delete keys
          do {
           enter_pressed = Keypad_Key_Press();    // Wait for the # key to set the activation code if 9 digits are entered
           Delay_ms(150);
           if (enter_pressed==13) {               // If * is pressed, delete the last character and continue
               i--;
               pos--;
               Lcd_Chr(2,pos,' ');
               enter_pressed = 15;
           }
           code_length = i+1;
          } while (enter_pressed != 15);
          key_pressed_beep();
      }

  }
  Delay_ms(100);
  Lcd_Cmd(_LCD_CLEAR);                        // Clear display
  Lcd_Out(1,1,"Arm the Device");

  while (arm_switch == 1) {                   // Wait until the arm switch is turned on
      Delay_ms(50);                          // Put some delay to prevent activation due to debouncing of switch contacts
  }

  Lcd_Cmd(_LCD_CLEAR);                        // Clear display

  Lcd_Out(2,1,"             (0)");            // Shows the number of attepmts
  Delay_ms(100);
}

// Alarm function, blink RED LED's and sound the alarm
void alarm (void) {
    char j = 0;
    YELLOW = 0;
    GREEN = 0;

    T1CON.TMR1ON = 0;              // Turn Timer1 off
    Lcd_Out(2,1,"    YOU LOSE");
    for (j=0; j<50; j++) {
        RED=1;                     // Beep very loud (9V to buzzer)
        PORTA.F3 = 1;
        Delay_ms(100);
        RED=0;
        PORTA.F3 = 0;
        Delay_ms(100);
    }
    while(1);                // Stay here forever until circuit is turned off and on again
}

// Bomb deactivated, light green LEDS and stop countdown
void win (void) {
    char j = 0;
    YELLOW = 0;
    RED = 0;
    
    T1CON.TMR1ON = 0;              // Turn Timer1 off
    Lcd_Out(2,1,"    YOU WIN");    // Left of the LCD space still shows attempts made
    for (j=0; j<50; j++) {
        GREEN=1;
        PORTA.F3 = 1;              // Beep very loud (9V to buzzer)
        Delay_ms(100);
        GREEN=0;
        PORTA.F3 = 0;
        Delay_ms(100);
    }
    
    while(1);                // Stay here forever until circuit is turned off and on again
}

void main() {
  unsigned short int k=0, i=0, attempts=0;
  signed int time_s = 1;  // Variable to follow when countdown reaches 0, initially set to a non-zero value
  unsigned short int deact[9]; //= {0,0,0,0,0,0,0,0,0};      // Entered deactivation code
  
  Keypad_Init();                 // Initialize Keypad

  ANSEL  = 0;                    // Configure AN pins as digital
  ANSELH = 0;
  C1ON_bit = 0;                  // Disable comparators
  C2ON_bit = 0;
  ADCON0.ADON = 0;               // ADC disbaled to prevent current consumption

  TRISA = 0x00;                  // All output
  TRISB = 0x00;                  // All output
  TRISC = 0x03;                  // Set LP crystal pins (RC1 input, RC0 output), rest output
  TRISE = 0x00;                  // All output
  
  PORTA = 0;
  PORTC = 0;
  RED=GREEN=YELLOW=0;
  
  arm_switch_Direction = 1;      // Arm switch port set as input

  PORTB.F2 = 0;                  // Make R/W pin on LCD ground through PIC
                                 //   to prevent the need to double layer PCB

  OSCCON = 0b01110001;           // Set internal oscillator to 8 Mhz (p. 64)
                                 //      page 187/261 of 'PIC Microcontrollers Book'

  PIR1.TMR1IF = 0;               // Reset the TMR1IF flag bit
  TMR1H = 0x80;                  // Set initial value for the timer TMR1
  TMR1L = 0x00;
  T1CON.TMR1CS = 1;              // Clock source is supplied externally
  T1CON.T1OSCEN = 1;             // Internal LP oscillator is on
  T1CON.T1CKPS1 = 0;
  T1CON.T1CKPS0 = 0;             // Prescaler rate 1:1
  T1CON.T1SYNC = 1;              // Do not synchronize external clock input
  PIE1.TMR1IE = 1;               // Enable interrupt on overflow
  INTCON = 0b11000000;           // Global Interrupts and Timer0 Overflow Interrupt enabled, rest disabled (p.33)
  Delay_ms(50);                  // Delay for TMR1 stabilization
  T1CON.TMR1ON = 0;              // Keep TMR1 off until initial values are set

  Lcd_Init();                    // Initialize LCD
  Lcd_Cmd(_LCD_CLEAR);           // Clear display
  Lcd_Cmd(_LCD_CURSOR_OFF);      // Cursor off
  
  Delay_ms(10);                  // Wait for the LCD backlight capacitor to charge fully
  initialize_device();           // Enter activation code and time

  T1CON.TMR1ON = 1;              // Turn Timer1 on

  Lcd_Chr(1, 7, ':');
  Lcd_Chr(1, 10, ':');
  
  enter_pressed = 0;

  while(1) {
      while (time_s > 0) {
          YELLOW=0;
          if (updated == 1) {
              
              // Decrease time by 1 second
              if (hhmmss[5] == 48) {
                 hhmmss[5] = 57;
                 if (hhmmss[4] == 48) {
                     hhmmss[4] = 53;
                         if (hhmmss[3] == 48) {
                             hhmmss[3] = 57;
                             if (hhmmss[2] == 48) {
                                 hhmmss[2] = 53;
                                 if (hhmmss[1] == 48) {
                                     hhmmss[1] = 57;
                                     if (hhmmss[0] == 48) {
                                         hhmmss[0] = 57;
                                     } else {
                                         hhmmss[0]--;
                                       }
                                 } else {
                                     hhmmss[1]--;
                                   }
                             } else {
                                 hhmmss[2]--;
                               }
                         } else {
                             hhmmss[3]--;
                           }
                 } else {
                     hhmmss[4]--;
                   }
             } else {
                  hhmmss[5]--;
               }
              
              Lcd_Chr(1, 5, hhmmss[0]);          // Print time in "    hh:mm:ss    " format
              Lcd_Chr(1, 6, hhmmss[1]);
              Lcd_Chr(1, 8, hhmmss[2]);
              Lcd_Chr(1, 9, hhmmss[3]);
              Lcd_Chr(1, 11, hhmmss[4]);
              Lcd_Chr(1, 12, hhmmss[5]);
              
              YELLOW=1;                         // Blink yellow LEDs once every second, it is turned off at the beginning of the loop
              key_pressed_beep();               // Beep (quiter) every second

              updated = 0;
              
              time_s = 0;
              for (i=0; i<6; i++) {
                  time_s = time_s + (hhmmss[i]-48);
              }
          }
          
          key = Keypad_Key_Press();     // Using the Key_press function, which is not a blocking call like the Key_Click function
          if (key != 0) {
              Delay_ms(150);            // A little delay to prevent multiple key entries with single press
              if (key == 15) {          // If "enter" key # is pressed
                  key_pressed_beep();
                  enter_pressed = 1;
              } else if (key == 13) {   // If "delete key * is pressed
                    key_pressed_beep();
                    Lcd_Chr(2, k, ' ');
                    if (k > 0) {
                        k--;
                    }
                } else if (k<9) {      // Allow only 9 keys
                      key = kp_text(key);
                      deact[k] = key;
                      k++;
                      Lcd_Chr(2, k, key);
                      key = 0;
                  }
          }

          
          if (arm_switch == 1 && T1CON.TMR1ON == 1) {
              alarm();          // If switch is turned without activation code during countdown, sound the alarm
          }                     //   If swtich is turned of after WIN state (timer1 off), this will not be triggered
          
          if (enter_pressed == 1) {
              if (k != code_length) {          // First check if both codes have the same length
                  attempts++;
                  Lcd_Chr(2,15,attempts+48);
                  if (attempts > 2) {    // 3 attempts to deactivate
                      alarm();
                  }
              } else {
                    for(k=0; k<code_length; k++) {
                        if (deact[k] != a_code[k]) {
                            attempts++;
                            Lcd_Chr(2,15,attempts+48);
                            k = 50;                 // Increase attempts, set k to 50 to exit the loop and increase attempts by 1
                            if (attempts > 2) {     // 3 attempts to deactivate
                               alarm();
                            }
                        }
                    }
                }
              if (k==code_length) {
                  win();
              }

              Lcd_Out(2,1,"         ");
              k=0;
              enter_pressed = 0;
          }
      }
      
      alarm();       // If time expires, sound alarm
  }
}