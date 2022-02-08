/*
   Transmit_Data Header file

   Includes the data transmission routines to simplify the main code
*/

unsigned short int TXDONE_flag = 0, reg_status = 0, initial_TX_mode = 0;

// FIRST TRANSMISSION
void transmit_data1() {
        write_byte(unique_node_adrs);               // Send the Node Address to FIFO

        // WRITE data to FIFO in Standby mode
        if (initial_TX_mode >= TX_mode_counter) {    // Normal operation mode, transmit temperature and water level values
            write_byte(water_level);
            write_byte(temperature);
            write_byte(low_bat);
            write_byte(1);              // This byte will indicate the first transmission
            write_byte(1);              // 1 will indicate normal transmit mode (Temp and water level will never be 203)

        } else {                        // Transmit a signal that will indicate the TX is continuously sending packets (total of 4 bytes will be sent to FIFO)
            write_byte(water_level);    // Send water level
            write_byte(temperature);    // Send water temperature
            write_byte(low_bat);        // Send battery voltage warning
            write_byte(1);              // This byte will indicate the first transmission
            write_byte(203);            // 203 will indicate continuous transmit mode (Temp and water level will never be 203)
         }

        reg_status = 0;
        TXDONE_flag = 0;

        // TRANSMIT the data
        set_chip_mode(1);        // 1 (Transmit mode)
        while(TXDONE_flag == 0) {
            reg_status = read_register(FTPRIREG);
            TXDONE_flag = (reg_status >> 5) & 0b00000001;   // Mask out TXDONE bit
            Delay_ms(1);
        }

        reg_status = reg_status & 0b11011111;      // Clear TXDONE
        write_register(FTPRIREG, reg_status);
}

// SECOND TRANSMISSION, only called when "initial_TX_mode >= TX_mode_counter"
void transmit_data2() {
        write_byte(unique_node_adrs);               // Send the Node Address to FIFO

        // WRITE data to FIFO in Standby mode
        // Normal operation mode, transmit temperature and water level values
        write_byte(water_level);
        write_byte(temperature);
        write_byte(low_bat);
        write_byte(2);              // This byte will indicate the second transmission
        write_byte(1);              // 1 will indicate normal transmit mode

        reg_status = 0;
        TXDONE_flag = 0;

        // TRANSMIT the data
        set_chip_mode(1);        // 1 (Transmit mode)
        while(TXDONE_flag == 0) {
            reg_status = read_register(FTPRIREG);
            TXDONE_flag = (reg_status >> 5) & 0b00000001;   // Mask out TXDONE bit
            Delay_ms(1);
        }

        reg_status = reg_status & 0b11011111;      // Clear TXDONE
        write_register(FTPRIREG, reg_status);
}