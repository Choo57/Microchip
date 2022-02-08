/*
   LCD Module Header file

   Defines:
          LCD connection pins
          LCD module initialization routine
*/

// LCD module connections
sbit D2A at SEG21COM0_bit;    // Varitronix's digit 2 on LCD Datasheet is actually my units digit and their digit 3 is my tens digit
sbit D2B at SEG22COM0_bit;
sbit D2C at SEG17COM0_bit;
sbit D2D at SEG18COM0_bit;
sbit D2E at SEG19COM0_bit;
sbit D2F at SEG5COM0_bit;
sbit D2G at SEG4COM0_bit;

sbit D3A at SEG7COM0_bit;     // Varitronix's digit 2 on LCD Datasheet is actually my units digit and their digit 3 is my tens digit
sbit D3B at SEG15COM0_bit;
sbit D3C at SEG20COM0_bit;
sbit D3D at SEG1COM0_bit;
sbit D3E at SEG2COM0_bit;
sbit D3F at SEG12COM0_bit;
sbit D3G at SEG13COM0_bit;

sbit DK at SEG3COM0_bit;        // First 1 character on LED controlling two K pins

sbit D1AFG at SEG23COM0_bit;    // All 3 controlled by 1 pin, will show the Celsius C sign

sbit DSINE at SEG14COM0_bit;    // SINE LED

sbit DBAT at SEG6COM0_bit;      // Low Bat (BAT) LED

// End LCD module connections

// Initialize the LCD module
void LCD_initialize (void) {
     LCDPS = 0b00000110;     // Prescaler 1:7 gives 32Hz frame rate with 32.768kHz crystal on TMR1
     
 //    LCDPS = 0b00000010;    // Prescaler 1:3 gives 32Hz frame rate with Fosc 4MHz (Table 10-4, p.151)
     
     LCDSE0 = 0b11111110;   // SEG1-7 (SEG0 set as I/O, rest SEGMENTS)
     LCDSE1 = 0b11110000;   // SEG8-15 (only SEG15 to SEG12 as SEGMENTS, rest I/O)
     LCDSE2 = 0b11111110;   // SEG16-23 (SEG16 set as I/O, rest SEGMENTS)

  //   LCDCON = 0b10010000;   // CS <2:3> 00 = FOSC/8192
     LCDCON = 0b10010100;   //  Enabled during sleep, T1OSC/32,
                            //  LMUX = 00 (static, using COM0 only), p.147

     LCDDATA0 = 0x00;       // All pixels off
     LCDDATA1 = 0x00;
     LCDDATA2 = 0x00;
     LCDDATA3 = 0x00;
     LCDDATA4 = 0x00;
     LCDDATA5 = 0x00;
     LCDDATA6 = 0x00;
     LCDDATA7 = 0x00;
     LCDDATA8 = 0x00;
     LCDDATA9 = 0x00;
     LCDDATA10 = 0x00;
     LCDDATA11 = 0x00;
}

// Function to Display Digits on the 2.5 digits of the 3.5 digit LCD
void displayNumber(unsigned char Position, unsigned char Number){
    switch(Position){
        case 0:     // Position 0 for the 'SINE' (Signal level) and 'BAT' (Low Battery level indicator) segments
            switch(Number){                 // 1 to light SINE segment
                case 1:
                        DSINE = 1;
                        break;
                case 2:                    // 2 to turn off SINE segment
                        DSINE = 0;
                        break;
                case 3:                    // 3 to light low bat (BAT) segment
                        DBAT = 1;
                        break;
                case 4:                    // 4 to turn off low bat (BAT) segment
                        DBAT = 0;
                        break;
                default:
                        DSINE = 0;            //If cases not matched, both digits will be cleared
                        DBAT= 0;
                }
            break;
        case 1:
            switch(Number){
                case 1:
                        DK = 1;
                        break;
                default:
                        DK = 0;
                }
            break;
        case 3:      // Varitronix's digit 2 on LCD Datasheet is actually my units digit and their digit 3 is my tens digit
            switch(Number){                              //  if following case not matched
                case 1:
                        D2B = D2C = 1;
                        D2A = D2D = D2E = D2F = D2G = 0;
                        break;
                case 2:
                        D2A = D2B = D2G = D2E = D2D = 1;
                        D2C = D2F = 0;
                        break;
                case 3:
                        D2A = D2B = D2G = D2C = D2D = 1;
                        D2E = D2F = 0;
                        break;
                case 4:
                        D2F = D2G = D2B = D2C = 1;
                        D2A = D2D = D2E = 0;
                        break;
                case 5:
                        D2A = D2F = D2G = D2C = D2D = 1;
                        D2B = D2E = 0;
                        break;
                case 6:
                        D2A = D2F = D2E = D2D = D2C = D2G = 1;
                        D2B = 0;
                        break;
                case 7:
                        D2A = D2B = D2C = 1;
                        D2D = D2E = D2F = D2G = 0;
                        break;
                case 8:
                        D2A = D2B = D2C = D2D = D2E = D2F = D2G = 1;
                        break;
                case 9:
                        D2A = D2B = D2C = D2D = D2F = D2G = 1;
                        D2E = 0;
                        break;
                case 10:       // Character A
                        D2A = D2B = D2C = D2E = D2F = D2G = 1;
                        D2D = 0;
                        break;
                case 11:       // Character b
                        D2C = D2D = D2E = D2F = D2G = 1;
                        D2A = D2B = 0;
                        break;
                case 12:      // Character C
                        D2A = D2D = D2E = D2F = 1;
                        D2B = D2C = D2G = 0;
                        break;
                case 13:      // Character d
                        D2B = D2C = D2D = D2E = D2G = 1;
                        D2F = 0;
                        break;
                case 14:      // Character E
                        D2A = D2D = D2E = D2F = D2G = 1;
                        D2B = D2C = 0;
                        break;
                case 15:      // Character F
                        D2A = D2E = D2F = D2G = 1;
                        D2B = D2C = D2D = 0;
                        break;
                case 16:      // Display dash " - "
                        D2G = 1;
                        D2A = D2B = D2C = D2D = D2E = D2F = 0;
                        break;
                case 0:
                        D2A = D2B = D2C = D2D = D2E = D2F = 1;
                        D2G = 0;
                        break;
                default:
                        D2A = D2B = D2C = D2D = D2E = D2F = D2G = 0;   //If cases not matched, digit will be cleared
                        break;
                }
            break;
        case 2:         // Varitronix's digit 2 on LCD Datasheet is actually my units digit and their digit 3 is my tens digit
            switch(Number){                              //  if following case not matched
                case 1:
                        D3B = D3C = 1;
                        D3A = D3D = D3E = D3F = D3G = 0;
                        break;
                case 2:
                        D3A = D3B = D3G = D3E = D3D = 1;
                        D3C = D3F = 0;
                        break;
                case 3:
                        D3A = D3B = D3G = D3C = D3D = 1;
                        D3E = D3F = 0;
                        break;
                case 4:
                        D3F = D3G = D3B = D3C = 1;
                        D3A = D3D = D3E = 0;
                        break;
                case 5:
                        D3A = D3F = D3G = D3C = D3D = 1;
                        D3B = D3E = 0;
                        break;
                case 6:
                        D3A = D3F = D3E = D3D = D3C = D3G = 1;
                        D3B = 0;
                        break;
                case 7:
                        D3A = D3B = D3C = 1;
                        D3D = D3E = D3F = D3G = 0;
                        break;
                case 8:
                        D3A = D3B = D3C = D3D = D3E = D3F = D3G = 1;
                        break;
                case 9:
                        D3A = D3B = D3C = D3D = D3F = D3G = 1;
                        D3E = 0;
                        break;
                case 10:
                        D3A = D3B = D3C = D3E = D3F = D3G = 1;
                        D3D = 0;
                        break;
                case 11:
                        D3C = D3D = D3E = D3F = D3G = 1;
                        D3A = D3B = 0;
                        break;
                case 12:
                        D3A = D3D = D3E = D3F = 1;
                        D3B = D3C = D3G = 0;
                        break;
                case 13:
                        D3B = D3C = D3D = D3E = D3G = 1;
                        D3F = 0;
                        break;
                case 14:
                        D3A = D3D = D3E = D3F = D3G = 1;
                        D3B = D3C = 0;
                        break;
                case 15:
                        D3A = D3E = D3F = D3G = 1;
                        D3B = D3C = D3D = 0;
                        break;
                case 16:      // Display dash " - "
                        D3G =1;
                        D3A = D3B = D3C = D3D = D3E = D3F = 0;
                        break;
                case 0:
                        D3A = D3B = D3C = D3D = D3E = D3F = 1;
                        D3G = 0;
                        break;
                default:
                        D3A = D3B = D3C = D3D = D3E = D3F = D3G = 0; //If cases not matched, digit will be cleared
                }
            break;
        case 4:
            switch(Number){                              //  if following case not matched
                case 17:       // Display the Celsius superseeded C sign
                        D1AFG = 1;
                        break;
                default:
                        D1AFG = 0;  //If cases not matched, digit will be cleared
                        break;
            }
        }
}