/*
 * Project name:
     Wireless Water Level and Hot Water Temperature Indicator
* Copyright:
     Cagan Cerkez, August 2014
  * Description:
     Water level is measured using a self-made 10-level reed switch float level probe.
     Reed switches are sealed inside a plastic pipe, so no contact with water. 
     A magnet, also sealed inside a float around the vertically placed reed switch pipe 
     turns on reed switches, whcih are read to find the water level.
     Hot water temperature is measured using an MCP9700A temperature sensor. Values
     are transmitted via a Microchip MRF89XAM8A Transciever. Device is battery
     powered. Schematic is in the project folder. Timer1 and 32.768kHz crystal is
     used during sleep for improved/consistent sleep periods so the RX unit can
     synchronize reception more successfully.
 * Configuration:
     MCU:             PIC16F883
     Oscillator:      8.000 MHz (using internal oscillator)
     SW:              mikroC PRO for PIC
 * Connections:
     Battery:                x1 D battery
     Level probe:            AN11 & AN13
     MCP9700A:               Vs (RB1), Vout (RB0, AN12)
     MRF89XAM8A:             CSCON (RC6), CSDATA (RC2), SPI (RC3 RC4 RC5)
     Low-voltage ICSP:       RB6 and RB7 reserved for PGD and PGC
 * NOTES:
     - Send 203 at the 1st, 2nd, 9th and 10th packets to indicate continuous transmit mode during the initial power on
     - RB3 should be grounded during low voltage ICSP
*/

// Static constant for the initial continuous transmit mode, will continue for 10 minutes, PIC will transmit every ~4 seconds
//  so the counter will need to count till 150 (5 min = 600 seconds / 4 = 150)
static const TX_mode_counter = 5;
static const TMR1_sleep = 19;     // TRM1 overflows every 16s, TRM1_sleep = 18 is about 04:48, plus 1/2 TMR1_sleep will be assigned to TRM1H:L so total sleep_time -> 04:56

static const SYNC_word1 = 0xCC;               // Network address btye 1 (between 0-255)
static const SYNC_word2 = 0xCC;               // Network address btye 2 (between 0-255)
static const SYNC_word3 = 0xCC;               // Network address btye 3 (between 0-255)
static const SYNC_word4 = 0xCC;               // Network address btye 4 (between 0-255)
static const unique_node_adrs = 10;        // Node address (between 0-255)
static const payload_lngth = 0b00000110;   // Payload length set to 6 (1 node address byte, level byte, temp byte, battery chck byte, transmit # byte, TX mode byte. Bit 7 = 0 disables Manchester encoding


                                     // ADC reference voltage is 3.3V (3300 mv / 1024 steps = 3.22)
static const lowbat_threshold = 310; // ADC results: Battery voltage 1.6V (497), 1.5V(465),1.0V(310), 0.9V(279), 0.8V(248), 0.75V(232), 0.7V(216), 0.65V(201), 0.6V(186)

unsigned short TMR1_counter = 0;    // Counter for TIMER1

#include "built_in.h"
#include "Registers.h"
#include "blink_led.h"
#include "TRX_config.h"
#include "water_level.h"
#include "temperature.h"
#include "battery.h"
#include "transmit_data.h"

// Interrupt service routine
void interrupt() {
 if (TMR1IF_bit) {     // Check if interrupt is due to TMR1 overflow
     TMR1_counter++;   // Counter incremented on every TRM1 overflow
     if (TMR1_counter == (TMR1_sleep - 1)) {
         TMR1H = 0b10000000;       // Move TMR1H:L to the mid value so the next overflow will be 8s later (instead of 16s)
         TMR1L = 0b00000000;
         T1CON.T1CKPS0 = 1;     // TMR1 prescaler set to 1:8, TRM1 will interrupt every 16 seconds with 32.768 kHz crystal
         T1CON.T1CKPS1 = 1;
     }
     TMR1IF_bit = 0;   // Clear TMR1IF
   //TMR1H = 0x00;     // Refer to AN580 (Using Timer1 in Asynchronous Clock Mode) for TMR1 & Sleep mode
   //TMR1L = 0x00;     // !!!! LOADING TRM1H & TMR1L with anything clears Timer1 prescaler to 1!!!!!!! Be careful if prescaler is set differently.
 }
    
}

void main(){
    ANSEL = 0b00000010;    // RA1 (AN1) (battery voltage) analog, rest digital
    ANSELH = 0b00110000;   // RB0 (AN12) (temp sensor), RB5 (AN13)(reed level sensor) analog, rest digital

    TRISA = 0b00000010;    // RA1 (AN1) (battery voltage) input, rest output
    TRISB = 0b00100001;    // RB5 (reed level sensor) & RB0 input (temp sensor), rest output
    TRISC = 0b00010011;    // RC0 & RC1 (TMR1 crystal) and RC4 (SDI) input, rest output
  //  TRISD = 0x00;          // Set direction to be output
  //  TRISE = 0x00;          // Set direction to be output

    T_pow_Direction = 0;      // Port to Temp sensor power pin set as output
    Tin_Direction = 1;        // Port to Temp sensor output pin set as input

    PORTA = 0x00;
    PORTB = 0x00;
    //PORTC = 0x00;          // Do not set SPI ports to 0!
    //PORTD = 0x00;
    //PORTE = 0x00;

    OSCCON = 0x71;         // Set internal oscillator to 8Mhz (p. 64)
                           //      page 187/261 of 'PIC Microcontrollers Book'
    //OSCCON = 0x61;       // Set internal oscillator to 4Mhz (p. 64)

    CSCON = 1;              // Deselect chips
    CSDAT = 1;

    //IRQ0_Direction = 1;     //IRQ0 pin direction input
    //IRQ1_Direction = 1;     //IRQ1 pin direction input
    //RST_Direction = 0;      //RST pin direction output
    CSCON_Direction = 0;    //CSCON pin direction output
    CSDAT_Direction = 0;    //CSDAT pin direction output

    // Setup Interrupts
    INTCON.GIE = 1;        // Enable global interrupt
    INTCON.PEIE = 1;       // Enable peripheral interrupt

    // Setup TMR0
    INTCON.T0IE = 0;       // Timer0 disabled

    // Setup Comparator1
    CM1CON0.C1ON = 0;      // Comparator 1 off (p.90)
    // Setup Comparator2
    CM2CON0.C2ON = 0;      // Comparator 2 off (p.90)

    // Set Comparator Voltage Reference
    VRCON.VREN = 0;        // Disable Comparator1 Voltage Reference

    // TMR1 setup
    PIE1.TMR1IE = 1;       // Enable Timer1 interrupt
    INTCON.PEIE = 1;
    INTCON.GIE = 1;        // Enable Global interrupts
    T1CON.TMR1CS = 1;      // TMR1 clock source is supplied externally (the 32.768 kHz crystal)
    T1CON.TMR1GE = 0;      // Disable TMR1 Gate control
    T1CON.T1OSCEN = 1;     // Enable external crystal connection on RA6 (T1OSO) & RA7 (T1OSI)
    T1CON.T1SYNC = 1;      // Do NOT synchronize TRM1 clock with internal clock so TMR1 works during sleep
    T1CON.TMR1ON = 0;      // Timer1 off
    TMR1H = 0x00;          // !!!! LOADING TRM1H & TMR1L with anything clears Timer1 prescaler to 1!!!!!!! Be careful if prescaler is set differently.
    TMR1L = 0x00;
    T1CON.T1CKPS0 = 1;     // TMR1 prescaler set to 1:8, TRM1 will interrupt every 16 seconds with 32.768 kHz crystal
    T1CON.T1CKPS1 = 1;

    Delay_ms(50);          // Delay to wait for TMR1 LP oscillator stabilizaton
                           // PS. minimum 10ms delay at POR or manual reset

    blink_led();
    blink_led();

    // Initialize SPI module
    // Max SPI clock for CONFIG mode is 6 MHz, for DATA mode (to read/write FIFO) is 1 MHz
    // for Fosc = 8MHz, _SPI_MASTER_OSC_DIV16 gives SPI clock of 0.5Mhz
    // Data is received by TRX through SDI pin and is clocked on the rising edge of SCK.
    // MRF89XA sends data through SDO pin and is clocked out on the falling edge of SCK
    SPI1_Init_Advanced(_SPI_MASTER_OSC_DIV16, _SPI_DATA_SAMPLE_MIDDLE, _SPI_CLK_IDLE_LOW, _SPI_LOW_2_HIGH);
    Delay_ms(100);
    initialize_TRX();        // Initialize TRX module

    //PLL_lock();            // Wait for the PLL lock, puts TRX into Freq Synth mode
    set_chip_mode(4);        // 4 (Standby mode)
    Delay_ms(15);            // Short delay

    while(1){

        measure_temperature();   // Measure temperature
        measure_level();         // Measure water level
        check_battery();         // Check battery voltage

        set_chip_mode(4);        // 4 (Standby mode)
        Delay_ms(15);            // Short delay
        PLL_lock();              // Wait for the PLL lock, puts TRX into Freq Synth mode

        // FIRST DATA TRANSMISSION
        transmit_data1();        // FIRST TRANSMISSION
        blink_led();             // Going to Sleep mode clears FIFO
        set_chip_mode(4);        // 4 (Standby mode)
        
        // SECONDS DATA TRANSMISSION if normal TX mode began
        if (initial_TX_mode >= TX_mode_counter) {
            Delay_ms(2500);      // Second transmission about 3 seconds later
            PLL_lock();          // Wait for the PLL lock, puts TRX into Freq Synth mode
            transmit_data2();    // SECOND TRANSMISSION
            blink_led();         // Going to Sleep mode clears FIFO
        }
        set_chip_mode(5);        // 5 (TRX put to Sleep mode)

        // Initial continuous transmit mode will continue for 10 minutes, PIC will transmit every 3 seconds
        //  so the counter will need to count till 200 (600 seconds / 3 = 200)
        if (initial_TX_mode >= TX_mode_counter) { // Go to SLEEP
            T1CON.TMR1ON = 1;      // Timer1 on
            while (TMR1_counter < TMR1_sleep) {
                asm {sleep};           // Go to sleep
            }
            TMR1_counter = 0;
        } else {
              Delay_ms(3500);      // Wait for ~3-4 seconds and transmit again
              initial_TX_mode++;   // Increase the counter
              T1CON.TMR1ON = 0;    // Timer1 off

              if (initial_TX_mode >= TX_mode_counter) {
                  blink_led();
                  blink_led();
                  blink_led();
                  blink_led();
                  T1CON.TMR1ON = 1;      // Timer1 on
              }
          }
    }
}