/*
   Description: Routine where the MCU waits in RX mode until the packages are received
                  or times-out after a determined period and exits the reception mode
                  and enters a continuous reception to recalculate the wake up time

*/

// ********** MRF89XAM8A set to RECEIVE MODE and waits for the package *********
void receive_package() {

        package_missed = 0;
        CRC_OK = 0;         // In RX mode, the STSCRCEN bit is cleared when the complete payload has been read from the FIFO. If the payload is read in Stand-by mode,
                            //   the STSCRCEN bit is cleared when the user goes back to RX mode and a new Sync word is detected.
        Display_data();

        set_chip_mode(4);   // 4 (Standby mode)
        PLL_lock();         // Puts mode to 3 (Freq Synth), then waits for PLL lock
        Delay_ms(10);
        set_chip_mode(2);   // 2 (Receive mode)
        Delay_ms(5);
        
        // RX MODE & wait for the package
        while ((CRC_OK == 0) && (package_missed == 0)) {       // Wait here until the package is received with the correct check-sum, if missed, reset timers
             PORTD.F3 = 1;            // *** TESTING *** Light the LED during receive mode
            // Display CC at startup or show the readings
            if (first_signal > 0) {
                Display_data();
                //check_alarm();         // If this line is commented out, RX unit will stop the alarm during tha signal waiting period.
            } else {                     //    If it is left, make sure the overflow alarm is not falsely triggeres as level=200 was set as 100 before and this will clear mute_alarm flag as mute_alarm=0
                  displayNumber(2,12);   // Show CC at startup until first data is received
                  displayNumber(3,12);
            }

            reg_status = read_register(PKTCREG);
            CRC_OK = reg_status & 0b00000001;     // Mask out STSCRCEN bit

            TMR1_bytes_current = (((int)TMR1H << 8) | TMR1L);   // Record TRM1L and TRM1H values

            // CHECK IF UPPER_LIMIT WAITING PERIOD IS EXCEEDED, STOP TMR1 IF TX IS MISSED & START OVER
            if ((sleep_time > 1) && (TMR1_bytes_current > (upper_limit + lower_limit)) && (TMR1_counter >= (sleep_time + 1)) && (TMR1_counter < TMR1_max_possible) ) {  // RX unit wakes up with an interruot and ideally captures the signal +lower_limit seconds later (when TMR1H:L starts from 0 and gets to lower_limit. So max wait for TMR1_current > upper_limit
                T1CON.TMR1ON = 0;               // Timer1 turned off, all counters reset
                sleep_time = 0;                 // Probably missed the transmission, will reset sleep_time so it will be re-measured as otherwise a wrong (too long) sleep time will be used
                displayNumber(0, 1);            // Display "SINE" sign to indicate missed reception
                TMR1_counter = 0;
                TMR1H = 0x00;
                TMR1L = 0x00;
                T1CON.T1CKPS0 = 1;     // TMR1 prescaler set to 1:8, TRM1 will interrupt every 16 seconds with 32.768 kHz crystal
                T1CON.T1CKPS1 = 1;     // Refer to AN580 (Using Timer1 in Asynchronous Clock Mode) for TMR1 & Sleep mode
                // !!!! LOADING TRM1H & TMR1L with anything clears Timer1 prescaler to 1!!!!!!! Be careful if prescaler is set differently.
                package_missed = 1;

                TEST_MISSES++;            // *** TESTING ***
            }
        }
        set_chip_mode(4);         // 4 (Standby mode), read FIFO in Standby mode
        first_signal = 1;         // At this stage it means a signal is received, so LCD will stop showing CC
        
        PORTD.F3 = 0;            // *** TESTING *** Light the LED during receive mode
}