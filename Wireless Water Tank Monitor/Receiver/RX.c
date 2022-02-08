/*
 * Project name:
     Wireless Water Level and Hot Water Temperature Indicator
* Copyright:
     Cagan Cerkez, August 2014
     Last update: 19 August 2015
  * Description:
     Code for the receiver unit. Values transmitted via a Microchip MRF89AM8A Transceiver
     unit are received. Data is displayed on a segmented LCD with a very low
     power profile. Device is battery powered. Temperature value is displayed
     when the button is pushed, otherwise the water level is displayed as the default value.
     Schematic is in the project folder. Buzzer sounds an alarm when water overflow or <30%
     water level is detected. Buzzer will continue alarming until water level changes to 30%-100%
     or till the user presses the "show water temperture" button. Battery voltage is also checked,
     together with teh battery voltage indicator byte receved from the transmitter; so when the
     "low bat" LED is lit on the LCD display, either RX units or TX unit's battery can be low.
 * Configuration:
     MCU:             PIC16F917
     Oscillator:      32.768 kHz crystal in LP mode
     TMR1:            Running on internal 8MHz crystal
     SW:              mikroC PRO for PIC
 * Connections:
     Batteries:              x2 AA batteries connected in series
     3.5 Digit LCD:          Static LCD with only COM0 used
     Temp. Display Switch:   RB0
     MRF89AM8A:              868MHz transceiver, conforms European ETSI standard

 * NOTES:
     - Tick "Use VPP first progam entry" on PICkit 2 under "Tools" menu before programming

*/

static const SYNC_word1 = 0xCC;               // Network address btye 1 (between 0-255)
static const SYNC_word2 = 0xCC;               // Network address btye 2 (between 0-255)
static const SYNC_word3 = 0xCC;               // Network address btye 3 (between 0-255)
static const SYNC_word4 = 0xCC;               // Network address btye 4 (between 0-255)
static const unique_node_adrs = 10;        // Node address (between 0-255)
static const payload_lngth = 0b00000110;   // Payload length set to 6 (1 node address byte, level byte, temp byte, battery chck byte, transmit # byte, TX mode byte. Bit 7 = 0 disables Manchester encoding

// x2 AA batteries will be used in series so the input battery voltage will  initially be 3.2. ADC reference voltage is 3.3V (3300 mv / 1024 steps = 3.22)
static const lowbat_threshold = 497; // ADC results: Battery voltage 1.6V (497), 1.5V(465), 0.9V(279), 0.8V(248), 0.75V(232), 0.7V(216), 0.65V(201), 0.6V(186)

static const lower_limit = 4096;        // Each TRM1 overflow is 16 seconds with a 32.768kHz crystal and 1:8 prescaler. TRM1 value = 4096 means 1s early wakeup
static const upper_limit = 16384;       // Each TRM1 overflow is 16 seconds with a 32.768kHz crystal and 1:8 prescaler. TRM1 value = 16384 means 4s extra waiting period in RX mode
static const TMR1_max_possible = 25;    // Max possible limit for x16s counter, to prevent integer multiples of real sleep period (eg. sleeping for 1 minutes instead of 5 minutes)

// Temp/Level display switch connection
sbit t_switch at RB0_bit;
sbit t_switch_Direction at TRISB0_bit;
// End of Temp/Level display switch connection

// Piezo Transducer connections
sbit buzzer at RB5_bit;
sbit buzzer_direction at TRISB5_bit;
// End of Piezo Transducer connections

// Battery level check connection
static const Bat_ADC = 2;             // Battery connected to AN2 (RA2)
// * * * DO NOT FORGET TO SET THE ASSOCIATED POWER PIN AS DIGITAL & INPUT PIN ANALOG
// End of battery level check connection

unsigned int level = 1000, temperature = 1000;    // Set to 1000 so the unit will display 00 until the first signal is received
unsigned short RX_mode = 0;
unsigned short node_adrs = 0;    // This will hold the address (first) byte sent by the transmitter
unsigned int TMR1_counter = 0;    // Counter for TIMER1
unsigned int TMR1_counter_hold = 0;
unsigned int TMR1_bytes = 0;
unsigned int TMR1_bytes_current = 0;
unsigned int TMR1_overflow = 0;
unsigned int sleep_time = 0;     // Unsigned int max 65,535. If TRM1_counter is incremented every 2 s, sleep_time will be ~150 for 5 minutes (300 seconds)
char display_temp = 0;
char mute_alarm = 0;
char overflow = 0;    // Flag to monitor water overflow
unsigned short low_batTX = 0;     // low_bat = 0 (battery is ok), low_bat = 1 (battery close to dying)
unsigned short tx_no = 0;  // 1 will indicate first transmission is captured, 2 will show 2nd is received

unsigned short reg_status = 0;
unsigned short CRC_OK = 0, package_missed = 0;
unsigned short first_signal = 0;

unsigned short i = 0; //*** FOR TESTING ***
unsigned int TEST_MISSES = 0;  //*** FOR TESTING ***

#include "built_in.h"
#include "Registers.h"
#include "LCD_setup.h"
#include "battery.h"
#include "TRX_config.h"
#include "Display_data.h"
#include "Piezo_trans.h"
#include "alarm.h"
#include "receive_package.h"

void interrupt() {
    if (INTF_bit) {        // First check the RB0/INT interrupt
        display_temp = 1;  // Set the "Display water temperature" flag
        mute_alarm = 1;    // Low water level alarm will be muted until water tank is filled again or in case of overflow, until level drops to <=100
        overflow = 0;      // Button pressed, stop sounding the buzzer
        INTF_bit = 0;      // Clear the RB0/INT flag
    }

    if (TMR1IF_bit) {     // Check if interrupt is due to TMR1 overflow
        TMR1_counter++;   // Counter incremented on every TRM1 overflow
        TMR1IF_bit = 0;   // Clear TMR1IF

        if ((sleep_time > 1) && (TMR1_counter == (sleep_time)) ) {
            TMR1H = Hi(TMR1_bytes);
            TMR1L = Lo(TMR1_bytes);

            T1CON.T1CKPS0 = 1;     // TMR1 prescaler set to 1:8, TRM1 will interrupt every 16 seconds with 32.768 kHz crystal
            T1CON.T1CKPS1 = 1;     // Refer to AN580 (Using Timer1 in Asynchronous Clock Mode) for TMR1 & Sleep mode
        }   // !!!! LOADING TRM1H & TMR1L with anything clears Timer1 prescaler to 1!!!!!!! Be careful if prescaler is set differently.
    }
}


void main() {
    buzzer = 0;   // Trying to stop the buzzer's ~1ms beep at every power cycle

    ANSEL = 0b00000100;    // RA2 (AN2) (Battery voltage check) analog, rest digital
    CMCON0 = 0x00;         // Disable the Comparator

    TRISA = 0b11000100;    // RA2 input for battery voltage check, RA6 & RA7 input for external crystal, rest output
    TRISB = 0b00000001;    // RB0/INT set as input, rest output
    TRISC = 0b10000000;    // RC7 input (SDI), rest output
    TRISD = 0x00;          // Set direction to be output
    TRISE = 0x00;          // Set direction to be output

    t_switch_Direction = 1;        // "Show temperature" switch port set as input
    buzzer_direction = 0;          // Piezo transducer pin set as output

    PORTA = 0;              // Set all ports connected to LCD to 0 initially
    PORTB = 0;
    PORTD = 0;              // DO NOT set PORTC (SPI ports) to 0, causes some issues with PLL Lock
    PORTE = 0;

    OSCCON = 0b01110001;         // Set internal oscillator to 8Mhz (p. 62 of datasheet)

    //OSCCON.SCS = 0;         // Clock source defined by FOSC<2:0> of the Configuration Word
                            //  this will enable two-speed startup to save energy (p. 97)

    CSCON = 1;              // Deselect chips
    CSDAT = 1;

    //IRQ0_Direction = 1;   //IRQ0 pin direction input
    //IRQ1_Direction = 1;   //IRQ1 pin direction input
    //RST_Direction = 0;    //RST pin direction output
    CSCON_Direction = 0;    //CSCON pin direction output
    CSDAT_Direction = 0;    //CSDAT pin direction output

    // LCD setup
    LCD_initialize();
    displayNumber(2,12);   // Show CC at startup until first data is received
    displayNumber(3,12);

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

    // RB0 INT setup
    OPTION_REG.INTEDG = 0; // Interrupt trigerred on the falling edge (RB0 is normally high, when the button is first pressed it will fall and then rise after the user releases his finger
    INTCON.INTF = 0;       // Clear the RB0 Interrupt flag
    INTCON.INTE = 1;       // Enable RB0 Interrupt

    // Initialize SPI module
    // Max SPI clock for CONFIG mode is 6 MHz, for DATA mode (to read/write FIFO) is 1 MHz
    // for Fosc = 8MHz, _SPI_MASTER_OSC_DIV16 gives SPI clock of 0.5Mhz
    // Data is received by TRX through SDI pin and is clocked on the rising edge of SCK.
    // MRF89XA sends data through SDO pin and is clocked out on the falling edge of SCK
    SPI1_Init_Advanced(_SPI_MASTER_OSC_DIV16, _SPI_DATA_SAMPLE_MIDDLE, _SPI_CLK_IDLE_LOW, _SPI_LOW_2_HIGH);
    // Was using the following when 32.786 kHz crystal is set as the system clock
    //SPI1_Init_Advanced(_SPI_MASTER_OSC_DIV4, _SPI_DATA_SAMPLE_MIDDLE, _SPI_CLK_IDLE_LOW, _SPI_LOW_2_HIGH);

    initialize_TRX();        // Initialize TRX module
    set_chip_mode(4);        // 4 (Standby mode)
    Delay_ms(15);            // Short delay
    //PLL_lock();              // Wait for the PLL lock

    while(1) {

        set_chip_mode(5);         // TRX goes to sleep (5- Sleep mode)

        // Wait here until sleep time is set, sleep TRX and MCU
        while ((sleep_time > 1) && (TMR1_counter < (sleep_time + 1))) {    // If sleep_time is not set yet OR if RX should wake up, wait for the package
            asm{sleep};               // MCU goes to sleep, and wakes up with every TMR1 overflow and goes out of the loop when the last increment of TRM! (sleep_time +1) is adjusted for
            Display_data();     // Second Display_data() ensures water level will be displayed after RB1 button is pressed
            if (((level <= 30) || (overflow == 1)) && (mute_alarm == 0)) {       // Sound the buzzer every 2 seconds until show temp button is pressed
                beep();
            }
            // *** TESTING ***
            if (i == 0) {
               level = level + 10;
               i = 1;
            } else {
                  level = level - 10;
                  i = 0;
              }
            // *** TESTING ***
                                  //  RB0/INT interrupt OR Timer1 overflow wakes the MCU from sleep
        }                         // MCU will wake up every 2 seconds, then will go to sleep until the expected signal reception time

        // RECEIVE PACKAGE LOOP
        receive_package();        // Wait here until the package is received, or time-out and set package_missed to 1

        if (sleep_time >= 1) {
            T1CON.TMR1ON = 0;                           // Stop TMR1, record TMR1L and TRM1H
            if (sleep_time == 1) {
                TMR1_bytes = (((int)TMR1H << 8) | TMR1L);   // Measure TMR1_bytes when sleep_time=1 & adjust it +- during following runs
            } else {
                  TMR1_bytes_current = (((int)TMR1H << 8) | TMR1L);   // Record TMR1L and TMR1H values
            }
            TMR1_counter_hold = TMR1_counter;           // Record TMR1_counter in case it overflows the moment TMR1 is turned on again
            TMR1H = 0;     // Reset TMR1
            TMR1L = 0;
            T1CON.T1CKPS0 = 1;     // TMR1 prescaler set to 1:8, TRM1 will interrupt every 16 seconds with 32.768 kHz crystal
            T1CON.T1CKPS1 = 1;     // Refer to AN580 (Using Timer1 in Asynchronous Clock Mode) for TMR1 & Sleep mode
                                   // !!!! LOADING TRM1H & TMR1L with anything clears Timer1 prescaler to 1!!!!!!! Be careful if prescaler is set differently.
            T1CON.TMR1ON = 1;
            TMR1_counter = 0;           // Reset TMR1_counter and start counting again
            
            // ADJUST TMR1_bytes with lower_limit & SET sleep_time
            if (sleep_time == 1) {
                if (TMR1_bytes < lower_limit) {          // If TMR1_bytes is less than lower_limit, TMR1_counter should be decremented
                      TMR1_bytes = (65536 - lower_limit) + TMR1_bytes;;
                      sleep_time = TMR1_counter_hold - 1;  // Because lower_limit > TMR1_bytes, the subtraction in the interrupt will yield a negative result so decrement TMR1_counter
                  } else {
                        sleep_time = TMR1_counter_hold;    // For the initial sleep_time=1 run, make sleep_time match TRM1_counter
                        TMR1_bytes = (65536 - TMR1_bytes) + lower_limit;
                    }
            } else {   // ADJUST TMR1_bytes by adding/subtracting the measured difference to/from it
                  if (TMR1_bytes_current < lower_limit) {                             // TRX will send ever x.5 interrupts, so the 1/2 interrupt will ensure the TMR1_bytes will be around 32768
                      TMR1_bytes = TMR1_bytes + (lower_limit - TMR1_bytes_current);   //   Therefore as there will be minor movements above or below this number, there is no overflow danger (i.e. TMR1_bytes will not go below 0 or above 65536)
                      sleep_time = TMR1_counter_hold - 1;
                  } else {
                        sleep_time = TMR1_counter_hold - 1;
                        TMR1_bytes = TMR1_bytes - (TMR1_bytes_current - lower_limit);
                    }
              }
        }
        
        if (package_missed == 0) {      // If payload is received, read it from FIFO, if missed, skip reading FIFO
            node_adrs = read_byte();    // Read byte from FIFO, 1st byte is the address byte
            level = read_byte();        // Following byte is the water level
            temperature = read_byte();  // The other byte is the temperature
            low_batTX = read_byte();    // This byte is the low battery warning flag
            tx_no = read_byte();        // This byte will show which transmission is received (1st or 2nd)
            RX_mode = read_byte();      // Last byte will either be 203 or 1
            displayNumber(0, 2);        // Package is received, so clear the SINE segment
        }// End of READ PACKAGE

        set_chip_mode(5);         // 5 (Sleep mode)

        // ******************* INITIAL RECEIVE MODE *******************
        if (RX_mode == 203) {

            check_alarm();
            check_RXbattery();     // Check battery of RX unit
            display_bat();

            clear_display();     // Clear LCD (excluding SINE and BAT leds) for a short period to make the new reception visible
            Display_data();      // Update the data displayed on LCD

        // *********************** NORMAL OPERATION MODE ***********************
        } else if (RX_mode == 1) {
              switch (sleep_time) {
                  case 0:
                      T1CON.TMR1ON = 1;           // Timer1 is turned on when initial continuous receive mode is over
                      sleep_time = 1;             // Set sleep_time to 1 for the very first run of TRM1

                      check_alarm();              // Sleep_time is measured when sleep_time = 1
                      check_RXbattery();
                      display_bat();

                      clear_display();   // Clear LCD (excluding SINE and BAT leds) for a short period to make the new reception visible
                      Display_data();    // Update the data displayed on LCD

                      if (package_missed == 0) {      // Do not delay if the package is missed
                          Delay_ms(6000);             // Delay to skip the second transmission of the TX so TMR1 measures correct TX sleep time
                      }
                      break;
                  default:
                      if (tx_no == 2) {               // Decrement TMR1_bytes if the second transmission is captured
                          if ((65536 - TMR1_bytes) < upper_limit) {          // Check if TRM1_bytes + upper_limit will overflow
                             TMR1_bytes = 65536-(upper_limit -(65536-TMR1_bytes));
                             sleep_time = sleep_time - 1;
                          } else {
                                TMR1_bytes = TMR1_bytes + upper_limit;
                          }
                      }
                      check_alarm();
                      check_RXbattery();
                      display_bat();

                      clear_display();   // Clear LCD (excluding SINE and BAT leds) for a short period to make the new reception visible
                      Display_data();    // Update the data displayed on LCD
                      break;
              }
          }
    }
}