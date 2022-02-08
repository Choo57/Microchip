/*
   MRF89XAM8A Header file

   Defines:
          IC connection pins
          Register & Data SPI Read/Write functions
          TRX module initialization routine
*/



// MRF89XAM8A connections
//sbit IRQ0 at RD2_bit;    //INT0 pin
//sbit IRQ1 at RD0_bit;    //INT1 pin
//sbit RST at RD1_bit;     //RESET pin
sbit CSCON at RC6_bit;   //CSCON pin
sbit CSDAT at RC2_bit;   //CSDAT pin

//sbit IRQ0_Direction at TRISD2_bit;     //IRQ0 pin direction
//sbit IRQ1_Direction at TRISD0_bit;     //IRQ1 pin direction
//sbit RST_Direction at TRISD1_bit;      //RST pin direction
sbit CSCON_Direction at TRISC6_bit;    //CSCON pin direction
sbit CSDAT_Direction at TRISC2_bit;    //CSDAT pin direction


// Write to the CONFIG registers
// REMOVE the "return current_data" if you never use the existing value in the register
void write_register(unsigned short int address, unsigned short int data_r){
 unsigned short int dummy_data_r = 0;
 // unsigned short int current_data = 0;
 CSCON = 0;

 address = address << 1;     // p.24 Figure 2-12 of MRF89XA datasheet, can also try "address = ((address << 1) & 0b00111110);"
 SPI1_Write(address);        // addressing register
 SPI1_Write(data_r);         // write data in register
// current_data = SPI1_Read(dummy_data_r);   // Current value of the register can be read simultaneously

 CSCON = 1;
// return current_data;
}

// Read from the CONFIG registers
unsigned short int read_register(unsigned short int address){
 unsigned short int data_r = 0, dummy_data_r = 0;

 CSCON = 0;

 address = (address << 1) | 0b01000000;     // p.24 Figure 2-13 of MRF89XA datasheet, can also try "((address << 1) & 0b00111110) | 0b01000000;"
 SPI1_Write(address);                       // addressing register
 data_r = SPI1_Read(dummy_data_r);          // read data from register

 CSCON = 1;
 return data_r;
}

// Write Bytes to FIFO before/during TX
void write_byte(unsigned short int data_r){    // Data received is set as UNSIGNED SHORT INT, so is between 0-255
 CSDAT = 0;

 SPI1_Write(data_r);      // write byte

 CSDAT = 1;
}

// Read Bytes from FIFO after/during RX
unsigned short int read_byte(void){
 unsigned short int data_r = 0;

 CSDAT = 0;

 data_r = SPI1_Read(0x00);    // read data from register

 CSDAT = 1;
 return data_r;
}

// Select chip mode
//   Vtune determined by tank inductors values (default), using R1, S1, P1, Freq: 863- 870 MHz
void set_chip_mode (unsigned short int chip_mode) {
 switch(chip_mode){
     case 1:                         // Send 1 for Transmit mode
         write_register(GCONREG, 0b10010000);
         break;
     case 2:                         // Send 2 for Receive mode
         write_register(GCONREG, 0b01110000);
         break;
     case 3:                         // Send 3 for Frequency Synthesizer mode
         write_register(GCONREG, 0b01010000);
         break;
     case 4:                         // Send 4 for Stand-by mode
         write_register(GCONREG, 0b00110000);
         break;
     case 5:                         // Send 5 for Sleep mode
         write_register(GCONREG, 0b00010000);
         break;
  }
}

// Wait for the PLL lock
void PLL_lock (void) {
 unsigned short int PLL_locked = 0, reg_status = 0, i = 0;
// unsigned short int VCOT_select[4] = {0b01010000, 0b01010010, 0b01010100, 0b01010110};  // All in Freq. Synth. mode, increase VCOT value if PLL lock fails
     set_chip_mode(3);                        //  Go to Freq. Synth. mode

     do {
         reg_status = read_register(0x0E);               // Read FTPRIREG (0x0E)

         PLL_locked = (reg_status >> 1) & 0b00000001;    // Isolate LSTSPLL (Lock Status of PLL bit)
         Delay_ms(400);
     } while (PLL_locked == 0);


 reg_status = reg_status | 0b00000010;    // Write 1 to LSTSPLL to clear PLL lock flag, put the original register values back
 write_register(0x0E, reg_status);
 //set_chip_mode(4);                        //  Go to stand-by mode
}

void initialize_TRX (void) {
 write_register(CLKOUTREG, 0x00);   // Disable Clockout to min current consumption, not used on the MRF89AM8A module

 set_chip_mode(3);         // 1 (Transmit mode), 2 (Receive mode), 3 (Freq Synth mode), 4 (Stand-by mode), 5 (Sleep mode)
                           //    Set chip mode CMOD<2:0>, frequency band FBS<1:0> & VCOT<1:0>

 write_register(0x01, 0x8C);    // FSK, DMODE0:DMODE1 0:1 (Packet)
 write_register(0x02, 0x03);    // fdev = 100 kHz
 write_register(0x03, 0x63);    // Bit Rate = 2 kbps NRZ
 write_register(0x04, 0x0C);    // OOK Floor Threshold = 6dB (default)
 //write_register(0x05, 0x42);    // FIFO size = 32 bytes, FIFO Threshold = 2
 write_register(0x05, 0x02);    // FIFO size = 16 bytes, FIFO Threshold = 2
 write_register(0x06, 0x77);    // R1 = 0x77 (119 decimal)
 write_register(0x07, 0x5F);    // P1 = 0x5F (95 decimal)
 write_register(0x08, 0x24);    // S1 = 0x24 (36 decimal)
 write_register(0x09, 0x77);    // R2 = 0x77 (119 decimal)
 write_register(0x0A, 0x5F);    // P2 = 0x5F (95 decimal)
 write_register(0x0B, 0x24);    // S2 = 0x24 (36 decimal)
 write_register(0x0C, 0x38);    // Power Amplifier Ramp = 23 us (default)
 //write_register(0x0D, 0xB8);    // IRQ0RXS (/FIFOEMPTY), IRQ1RXS (FIFO_THRESHOLD), IRQ1TX (TXDONE)
 write_register(0x0D, 0x08);    // IRQ0RXS (PLREADY), IRQ1RXS (CRCOK), IRQ1TX (TXDONE)
 write_register(0x0E, 0x11);    // IRQ0TXST (Transmit starts if FIFO is not empty, IRQ0 mapped to FIFOEMPTY), PLL lock enabled, FIFO fill start automatically when sync word detected, Writing: stop filling FIFO
 write_register(0x0F, 0x00);    // RSSI threshold interrupt = 0 (default)
 write_register(0x10, 0b10100101);    // Passive filter = 378 kHz, Butterworth Filter = 150 kHz (calculated for the designed bit rate, fdev and local osc drift)
 write_register(0x11, 0x38);    // Polyphase Centre Frequency = 100 kHz (default)
 write_register(0x12, 0x38);    // 32 bits SYNC word, 0 Errors tolerated (default), Polyphase filter disabled (must for FSK!), Bit Synchronizer enabled (default), SYNC Word Recognition enabled,
 write_register(0x13, 0x07);    // Reserved register (default value loaded)
 write_register(0x14, 0x00);    // RSSI read only status register
 write_register(0x15, 0x00);    // OOK config register (default values loaded)
 write_register(0x16, SYNC_word1);    // SYNC Value 1st byte
 write_register(0x17, SYNC_word2);    // SYNC Value 2nd byte
 write_register(0x18, SYNC_word3);    // SYNC Value 3rd byte
 write_register(0x19, SYNC_word4);    // SYNC Value 4th byte
 write_register(0x1A, 0b01110000);    // Transmit power = 13 dBm, Transmission Interpolation Filter Cut Off Frequency = 200 kHz
 write_register(0x1B, 0x3C);    // Clock Output disabled
 write_register(0x1C, payload_lngth);    // Manchester Encoding disabled, PLDPLEN payload length (including address byte)
 write_register(0x1D, unique_node_adrs);    // Node Local Address set
 write_register(0x1E, 0b01011010);    // Fixed packet length, Preamble 3 bytes (default), Whitening ON, CRC enabled (default), address filtering on
 write_register(0x1F, 0b00000000);    // Auto Clear FIFO CRC enabled, write access to FIFO during standby
}