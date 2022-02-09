/*
 * Project name:
     Capacitance Meter
 * Copyright:
     Cagan Cerkez, 2012
 * Configuration:
     MCU:             PIC16F887
     Oscillator:      8.000 MHz (using internal oscillator)
     SW:              mikroC PRO for PIC
 * Connections:
     LCD:                    RD3-RD7 & RC5-RC7 (RC6 to GND for easy PCB layout)
     RB3:                    to VDD (Low-voltage ICSP)
*/

// LCD module connections
sbit LCD_RS at RD3_bit;
sbit LCD_EN at RC7_bit;
sbit LCD_D4 at RD4_bit;
sbit LCD_D5 at RD5_bit;
sbit LCD_D6 at RD6_bit;
sbit LCD_D7 at RD7_bit;

sbit LCD_RS_Direction at TRISD3_bit;
sbit LCD_EN_Direction at TRISC7_bit;
sbit LCD_D4_Direction at TRISD4_bit;
sbit LCD_D5_Direction at TRISD5_bit;
sbit LCD_D6_Direction at TRISD6_bit;
sbit LCD_D7_Direction at TRISD7_bit;
// End LCD module connections

#define Vapplied PORTE.F0
#define TEST PORTA.F0
unsigned long tmr1_counter = 0;
unsigned long capacitance = 0;
unsigned long max = 0;
unsigned int overflow = 0;
char temporary[11];
char temporary2[11];
char Cap_string[11];

const array_size = 20;           // Store 20 readings and display the most frequent
unsigned int Cap_array[array_size];      //    to minimize fluctuating cap. readings
char Cap_checked[array_size];
char Cap_frequency[array_size];

void interrupt(){
    if(PIR1.TMR1IF){
        overflow++;              // TMR1 exceeded 65,535. Overflow occured
        PIR1.TMR1IF = 0;         // Clear int flag bit
        TMR1H = 0x00;
        TMR1L = 0x00;
     }
}

void main(){
    char i, j, c, a, b;
    
    ANSEL = 0x0A;          // RA1 & RA3 analog (input for comp), rest digital
    ANSELH = 0x00;
    
    TRISA = 0x0B;          // RA0, RA1 & RA3 input, rest output
    TRISB = 0x00;          // Set direction to be output
    TRISC = 0x00;          // Set direction to be output
    TRISD = 0x00;          // Set direction to be output
    TRISE = 0x00;          // Set direction to be output

    PORTC.F6 = 0;          // Set to GND for easy PCB layout
    PORTD = 0x00;
    PORTA = 0x00;
    PORTE = 0x00;
    
    //OSCCON = 0x7E;       // 8Mhz, RC internal clock   (I changed this line)
    OSCCON = 0x71;         // Set internal oscillator to 8Mhz (p. 64)
                           //      page 187/261 of 'PIC Microcontrollers Book'
    //OSCCON = 0x61;         // Set internal oscillator to 4Mhz (p. 64)
    
    // Setup TMR1
    PIR1.TMR1IF = 0;       // Reset the TMR1IF flag bit
    T1CON.TMR1CS = 0;      // Timer1 counts pulses from internal oscillator
    T1CON.T1CKPS1 = 0;     // Assigned prescaler rate is 1:1
    T1CON.T1CKPS0 = 0;
    PIE1.TMR1IE = 1;       // Enable interrupt on overflow
    INTCON.GIE = 1;        // Enable global interrupt
    INTCON.PEIE = 1;       // Enable peripheral interrupt

    //Comparator setup
    CM1CON0.C1ON = 1;      // Enable Comparator 1 (p.90)
    CM1CON0.C1OE = 1;      // Enable Comparator 1 Output (p.90)
    CM1CON0.C1R = 0;       // C1Vin+ connects to RA3 (p.90)
    
    CM1CON0.C1CH0 = 1;      // RA1 (C12IN1-) is selected as VIN-
    CM1CON0.C1CH1 = 0;      // (p.90)
    
    // RA1 = Vin-   RA3 = Vin+ 
    CM1CON0.C1POL = 1;      // C1 output is 1 when RA1 > RA3 (Vin- > Vin+)

    Lcd_Init();                             // Initialize LCD
    Lcd_Cmd(_LCD_CLEAR);                    // Clear display
    Lcd_Cmd(_LCD_CURSOR_OFF);               // Cursor off
    Lcd_Out(1, 1, "Capacitance");
    Lcd_Out(2, 1, "Meter");
    Delay_ms(150);
    Lcd_Cmd(_LCD_CLEAR);
    
    while(1){
        for (c=0; c<array_size; c++) {
            tmr1_counter = 0;     // Reset counters
            overflow = 0;

            TMR1H = 0x00;          // Set initial value for the timer TMR1
            TMR1L = 0x00;
            Vapplied = 1;          // Apply voltage
            T1CON.TMR1ON = 1;      // Start timer1

            while(CM1CON0.C1OUT == 0) {     // Stay here until (1/2)Vs is reached
            }

            T1CON.TMR1ON = 0;               // Stop timer1
            tmr1_counter =(overflow * 65536) + (TMR1H<<8) + TMR1L;
            Cap_array[c] = tmr1_counter;
        
            Vapplied = 0;                   // Stop the voltage applied
            Delay_ms(1000);                  // Discharge the capacitor
        }

        for (c=0; c<array_size; c++){
            Cap_checked[c] = 1;   // All array values initially unchecked (1)
            Cap_frequency[c] = 0;           // All frequencies reset to 0
        }
        
        // Find the most frequent reading
        for (a=0; a<array_size; a++){
            if(Cap_checked[a] == 1) {                  //  Check if that array element is counted before
                for (b=a; b<array_size; b++){
                    if (Cap_array[a] == Cap_array[b]){
                        Cap_frequency[a]++;
                        Cap_checked[b] = 0;
                    }
                }
            }
        }
        
        a = 0;                                      // Count the most frequently occured value
        for (b=1; b<array_size; b++){
            if (Cap_frequency[a] < Cap_frequency[b]) {
                a = b;
              }
        }

        tmr1_counter = Cap_array[a];
        capacitance = tmr1_counter * 721;
        
        a = 0;                                      // Count the maximum value
        for (b=1; b<array_size; b++){
            if (Cap_array[a] < Cap_array[b]) {
                a = b;
              }
        }
        max = Cap_array[a];
        
        LongWordToStr(tmr1_counter, temporary);     // Convert int to string
        j=0;                                        //   remove spaces
        for(i=0; i<11; i++){
            if(temporary[i]!= ' ') {
               Cap_string[j] = temporary[i];
               j++;
            }
        }

        LongWordToStr(max, temporary2);     // Convert int to string
        j=0;                                    //   remove spaces
        for(i=0; i<11; i++){
            if(temporary[i]!= ' ') {
               Cap_string[j] = temporary[i];
               j++;
            }
        }
        Lcd_Cmd(_LCD_CLEAR);
        Lcd_Out(1, 1, temporary);
        Lcd_Out(2, 1, temporary2);

        
    }
}