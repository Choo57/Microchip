

// *************************************************************
//                       GENERAL
// *************************************************************
#define GCONREG         0x00    // General Configuration Register
#define DMODREG         0x01    // Data and Modulation Configuration Register
#define FDEVREG         0x02    // Frequency Deviation Control Register
#define BRSREG          0x03    // Bit Rate Set Register
#define FLTHREG         0x04    // Floor Threshold Control Register
#define FIFOCREG        0x05    // FIFO Configuration Register
#define R1CREG          0x06    // R1 Counter Set Register
#define P1CREG          0x07    // P1 Counter Set Register
#define S1CREG          0x08    // S1 Counter Set Register
#define R2CREG          0x09    // R2 Counter Set Register
#define P2CREG          0x0A    // P2 Counter Set Register
#define S2CREG          0x0B    // S2 Counter Set Register
#define PACREG          0x0C    // Power Amplifier Control Register
#define FTXRXIREG       0x0D    // FIFO, Transmit and Receive Interrupt Request Configuration Register

// *************************************************************
//                       INTERRUPT
// *************************************************************

#define FTPRIREG        0x0E    // FIFO Transmit PLL and RSSI Interrupt Configuration Register
#define RSTHIREG        0x0F    // RSSI Threshold Interrupt Request Configuration Register
#define FILCREG         0x10    // Filter Configuration Register

// *************************************************************
//                       RECEIVER
// *************************************************************
#define PFCREG          0x11    // Polyphase Filter Configuration Register
#define SYNCREG         0x12    // Sync Control Register
#define RESVREG         0x13    // Reserved Register
#define RSTSREG         0x14    // RSSI Status Read Register
#define OOKCREG         0x15    // OOK Configuration Register

// *************************************************************
//                       SYNC WORD
// *************************************************************
#define SYNCV31REG      0x16    // Sync Value 1st Byte Configuration Register
#define SYNCV23REG      0x17    // Sync Value 2nd Byte Configuration Register
#define SYNCV15REG      0x18    // Sync Value 3rd Byte Configuration Register
#define SYNCV07REG      0x19    // Sync Value 4th Byte Configuration Register

// *************************************************************
//                       TRANSMITTER
// *************************************************************
#define TXCONREG        0x1A    // Transmit Configuration Register

// *************************************************************
//                       CLOCK OUT
// *************************************************************
#define CLKOUTREG         0x1B    // Clock Output Control Register

// *************************************************************
//                       PACKET
// *************************************************************
#define PLOADREG        0x1C    // Payload Configuration Register
#define NADDSREG        0x1D    // Node Address Set Register
#define PKTCREG         0x1E    // Packet Configuration Register
#define FCRCREG         0x1F    // FIFO CRC Configuration Register