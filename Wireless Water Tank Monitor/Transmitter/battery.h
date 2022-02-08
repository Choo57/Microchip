/*
   Header file to check battery voltage

   Defines:
          Battery/MCU connection pin
          Function to measure battery voltage
*/


// Battery connection to MCU
static const Bat_ADC = 1;             // Battery connected to AN1 (RA1)
// * * * DO NOT FORGET TO SET THE ASSOCIATED POWER PIN AS DIGITAL & INPUT PIN ANALOG

// End of battery connections

unsigned short low_bat = 0;     // low_bat = 0 (battery is ok), low_bat = 1 (battery close to dying)

// Function to read water temperature from MCP9700A
void check_battery(void){
     unsigned bat_vol = 0;   // 2 byte (0-65535) ADC reading will be stored here
     char j=0;

     //ADC_Init();           // Initialize the ADC module

     for(j=0; j<8; j++) {    // Take 8 readings and average
          bat_vol = ADC_Read(Bat_ADC) + bat_vol;  // Read from the associated channel and sum up the 8 measurements
          Delay_ms(5);          // Short delay
      }
      bat_vol = bat_vol/8;
      
      if (bat_vol < lowbat_threshold) {  // ADC reference voltage is 3.3V (3300 mv / 1024 steps = 3.22)
          low_bat = 1;      // ADC results: Battery voltage 1.6V(497), 1.5V(465), 0.9V(279), 0.8V(248), 0.75V(232), 0.7V(216), 0.65V(201), 0.6V(186)
      } else {              // MCP1640 will not start up if Vin is lower than 0.7V for upto 40mA load
            low_bat = 0;
        }

      ADCON0.ADON = 0;        // ADC disabled to prevent current consumption
      bat_vol = 0;
}