/*
 * Project name:
     Automatic Flower Pot watering
* Copyright:
     Cagan Cerkez, 2014
  * Description:
     PIC12F1501 will run a small submersible pump, once every 24 hours. User will
     press the button on the board, TMR1 will measure the time the button is pressed for and
     right after the button is released, the LED will be lit for that long to confirm with 
     the user the time the button was pressed for. MCU will then operate the pump for that
     duration every 24 hours. The LED on the board will blink periodically to inform the 
     user that the device is working fine and to indicate that the batteries have not died.
     If the user presses the button again, the 24-hour period will be restarted
     and the watering period will be updated.
     (To be included in the next version: WDT will be used to prevent any unexpected 
     freezes/failures as much as possible.)
     
     WDT and the uncalibrated 31 kHz internal oscillator will be used to minimize the
     required external components (e.g. external 32.768kHz crystal) as an exact 24-hour 
     period is not required. According to PIC12F1501 datasheet, page 264-265, LFINTOSC
     accuracy is between 35kHz-27kHz (+-13%, +-3 hour difference from expected 24-hour period) 
     and HFINTOSC is +-2% accurate (+-0.5 hour difference from expected 24-hour period).
     
 * Configuration:
     MCU:                      PIC12F1501
     Oscillator:               31 KHz internal uncalibrated
     SW:                       mikroC PRO for PIC
*/

sbit pump at RA1_bit;               // Port that controls the submersible pump transistor (2N2222)
sbit wbutton at RA5_bit;            // Port that is connected to the button
sbit LED at RA2_bit;                // Port connected to LED indicator

unsigned short int button_pressed = 0;
unsigned int tmr1_overflow = 0;
unsigned short int LED_flag = 0;

unsigned long i = 0;
unsigned long pumping_time = 0;

unsigned int one_day = 5400;   // Water every 24 hours: TMR1 overflows every 16 seconds so 24 hours will be 5400 overflows

// Interrupt service routine
void interrupt() {          // Peripheral interrupt-on-change
 if (IOCAF.IOCAF5) {        // Interrupt-on-Change enabled on the pin for a negative going edge.
    button_pressed = 1;
    IOCAF.IOCAF5 = 0;        // Clear the interrupt flag
    INTCON.IOCIE = 0;        // Disable interrupt-on-change to prevent multiple interrupts because of switch bouncing
    IOCAN = 0b00000000;      // IOC on negative edge is disabled
    IOCAP = 0b00000000;
}

 if (PIR1.TMR1IF) {    // If interrupt is due to Timer1 overflow, calculate the time the button was pressed for
    tmr1_overflow++;
    PIR1.TMR1IF = 0;        // Clear int flag bit
    TMR1H = 0x0D;           // TRM1 will start from 3535 so each interrupt will be after 16.0 seconds
    TMR1L = 0xCF;           // the prescaler counter is cleared upon a write to TMR1H or TMR1L so setting the prescaler to 1:8 again
    T1CON = 0b11110101;     // Timer1 clock source is LFINTOSC, 1:8 Prescale value, Do not synchronize asynchronous clock input, TMR1 ON
 }
}

void main() {
    ANSELA = 0b00000000;    // All digital
    TRISA = 0b00101000;     // RA5 (button) input, rest output

    pump = 0;
    LED = 0;
    PORTA = 0x00;           // Set all ports connected to 0 initially

    OSCCON = 0b00000010;    // Use internal oscillator at 31kHz (LFINTOSC)
    
    // Setup Interrupt-on-change
    INTCON.GIE = 1;         // Enable global interrupt
    INTCON.PEIE = 1;        // Enable peripheral interrupt
    INTCON.IOCIE = 1;       // Enable interrupt-on-change
    IOCAN = 0b00100000;     // IOC on negative edge for the button pin RA5
                            //   RA5 is normally high. When button is pressed it goes low
                            //   Interrupt-on-Change enabled on the pin for a negative going edge.
    IOCAP = 0b00000000;     // IOC on positive edge is disabled (both negative and positive edge can operate simultaneously, independently
    IOCAF = 0x00;           // Clear all IOC flags

    // Timer1 setup
    T1GCON.TMR1GE = 0;      // Timer1 counts regardless of Timer1 gate function
    TMR1H = 0x0D;
    TMR1L = 0xCF;           // Setting prescaler after clearing TMR1L & TMR1H
    T1CON = 0b11110100;     // Timer1 clock source is LFINTOSC, 1:8 Prescale value, Do not synchronize asynchronous clock input, TMR1 OFF
    PIE1.TMR1IE   = 1;      // Timer1 interrupt disabled
    PIR1.TMR1IF   = 0;      // Timer1 interrupt Flag cleared

    // Disable unused Peripherals
    //VREGCON.VREGPM = 1;     // Low-power sleep mode is enabled
    CM1CON0.C1ON = 0;       // Comparator 1 off
    WPUA = 0x00;            // Weak pull-ups are disabled
    WDTCON.SWDTEN = 0;      // WDT off
    ADCON0.ADON = 0;        // ADC off

    while (1) {
        // CHECK IF BUTTON IS PRESSED

        if ((button_pressed == 1) && (wbutton == 0)) {
            tmr1_overflow = 0;
            TMR1H = 0x0D;           //
            TMR1L = 0xCF;           // the prescaler counter is cleared upon a write to TMR1H or TMR1L so setting the prescaler to 1:8 again
            T1CON = 0b11110101;     // Timer1 clock source is LFINTOSC, 1:8 Prescale value, Do not synchronize asynchronous clock input, TMR1 ON

            while (wbutton == 0) {    // Pump water as long as the button is pushed down
                pump = 1;
                Delay_ms(350);         // Delay to compensate for switch contact bouncing
            }
            
            pump = 0;               // Pump off
            T1CON.TMR1ON = 0;       // Timer1 OFF
            LED = 0;                // LED off
            button_pressed = 0;
            pumping_time = 0;
            
            // CALCULATE PUMPING TIME
            pumping_time = (TMR1H << 8) + TMR1L;          // Read TRM1 value
            pumping_time = pumping_time - 3535;                // Extract the initial TRM1H & TMR1L values
            pumping_time = pumping_time * 8;                   // 1:8 prescaler  is used
            pumping_time = pumping_time / 50;                  // Divide by 50 instead of 100 (pumping loop uses 500ms Delays)
            pumping_time = pumping_time / 31;                  // 31 kHz internal oscillator LFINTOSC is used for Timer1
            pumping_time = pumping_time + (tmr1_overflow*160);    // Each Timer1 overflow (STARTING FROM 3535) takes 16.0 seconds
            if ((pumping_time % 10) < 5) {                     // pumping_time is x10 SECONDS (e.g. pumping_time=186 for 9.3 seconds)
                pumping_time = pumping_time/10;        // Time the button is pressed for in SECONDS x2
            } else {
                  pumping_time = pumping_time/10 + 1;   // Time the button is pressed for in SECONDS x2
              }

            for (i=0; i<pumping_time; i++) {    // Operate the pump for the pumping duration set
                LED = 1;
                Delay_ms(500);
            }
            LED = 0;
            
            // TURN TIMER1 ON TO START COUNTING FOR 24 HOURS
            Delay_ms(100);
            INTCON.IOCIE = 1;       // Enable interrupt-on-change
            IOCAN = 0b00100000;     // IOC on negative edge for the button pin RA
            IOCAF = 0x00;           // Clear all IOC flags
            
            TMR1H = 0x0D;           // TRM1 will start from 3535 so each interrupt will be after 16.0 seconds
            TMR1L = 0xCF;
            T1CON = 0b11110101;     // Timer1 clock source is LFINTOSC, 1:8 Prescale value, Do not synchronize asynchronous clock input, TMR1 ON
            tmr1_overflow = 0;
        }

        // LIGHT LED PERIODICALLY
        if ((tmr1_overflow % 2) == 0) {  // Light LED every 32 seconds (once every 2 TMR1 overflows which happen every 16s)
            if (LED_flag == 1) {
                LED = 1;
                Delay_ms(60);
                LED = 0;
                LED_flag = 0;             // Do not light LED until the next 4th TMR1 overflow
            }
        } else {
              LED_flag = 1;               // Clear the LED flag so it will be lit next time and wont stay on during the whole tmr_overflow%4 == 0 period (16 seconds)
          }

        // 24 HOURS REACHED, PUMP WATER
        if (tmr1_overflow >= one_day) {      // If TMR1 overflow reaches the 24-hour limit
            TMR1H = 0x0D;           // TRM1 will start from 3535 so each interrupt will be after 16.0 seconds
            TMR1L = 0xCF;           // the prescaler counter is cleared upon a write to TMR1H or TMR1L so setting the prescaler to 1:8 again
            T1CON = 0b11110101;     // Timer1 clock source is LFINTOSC, 1:8 Prescale value, Do not synchronize asynchronous clock input, TMR1 ON
            tmr1_overflow = 0;

            for (i=0; i<pumping_time; i++) {    // Operate the pump for the pumping duration set
                pump = 1;
                Delay_ms(500);
            }
            pump = 0;
        }

    }
}