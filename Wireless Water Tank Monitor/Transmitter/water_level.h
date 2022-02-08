/*
   Reed switch water level sensor Header file

   Defines:
          Sensor construction
          Function to measure water level

          RB4:          Digital output pin, will provide Vsupply for the sensor
          RB5 (AN13):   Analog input of the sensor connection
   
   NOTES:
          - Check the Excel file "Reed switch resistor values.xlsx" for a schmatic
             of the sensor and the associated resistor values
             First leg of the voltage divider: 100 kohm resistor
             Resistance       Rvoltage     ADC result
             R1 9.09 kohm        0.27         85
             R2 18.2 kohm        0.51         158
             R3 34 kohm          0.84         260
             R4 52.3 kohm        1.13         352
             R5 76.8 kohm        1.43         445
             R6 110 kohm         1.73         536
             R7 162 kohm         2.04         633
             R8 249 kohm         2.35         731
             R9 392 kohm         2.63         816

             - Show 200 is OVERFLOW is detected
             - Show 250 if an ERROR is detected
*/


// Transmit 200 if water overflow is detected

// Water Level Sensor connections
sbit Lev_pow at RB4_bit;                // Vsupply of sensor connected to RB4
sbit Lev_pow_Direction at TRISB4_bit;   // Vsupply will be set as output
sbit Lev_in_Direction at TRISB5_bit;    // Level sensor's ADC input pin (RB5) direction
static const Lev_ADC_channel = 13;            // Level sensor's pin connected to AN13 (RB5)
// * * * DO NOT FORGET TO SET THE ASSOCIATED POWER PIN AS DIGITAL & INPUT PIN ANALOG

unsigned int level_ADC = 0;              // ADC result will vary between 0-1024
unsigned short water_level = 0;          // Water level

// Function to read Water Level Sensor
void measure_level (void) {
    unsigned short i=0, j=0, mode=0, mode_target=0;
    unsigned short level_array[8] = {0,0,0,0,0,0,0,0};

    Lev_pow = 1;          // Power the voltage divider
    Delay_ms(50);         // Wait for the sensor to stabilise

     for(j=0; j<8; j++) {
         level_ADC = ADC_Read(Lev_ADC_channel);  // Read from the associated channel

         if (level_ADC < 45) {
             level_array[j] = 250;          // 250 will indicate ERROR, ADC reading should never be 0 or below 180?!
         } else if (level_ADC < 72) {
              level_array[j] = 35;
         } else if (level_ADC < 97) {
              level_array[j] = 30;
         }  else if (level_ADC < 133) {
              level_array[j] = 45;
         }  else if (level_ADC < 166) {
              level_array[j] = 40;
         }  else if (level_ADC < 209) {
              level_array[j] = 55;
         }  else if (level_ADC < 251) {
              level_array[j] = 65;
         }  else if (level_ADC < 289) {
              level_array[j] = 50;
         }  else if (level_ADC < 335) {
              level_array[j] = 75;
         }  else if (level_ADC < 378) {
              level_array[j] = 60;
         }  else if (level_ADC < 425) {
              level_array[j] = 85;
         }  else if (level_ADC < 476) {
              level_array[j] = 70;
         }  else if (level_ADC < 522) {
              level_array[j] = 95;
         }  else if (level_ADC < 577) {
              level_array[j] = 80;
         }  else if (level_ADC < 626) {
              level_array[j] = 200;      // 200 will indicate OVERFLOW
         }  else if (level_ADC < 682) {
              level_array[j] = 90;
         }  else if (level_ADC < 773) {
              level_array[j] = 100;
         }  else if (level_ADC < 920) {
              level_array[j] = 200;      // 200 will indicate OVERFLOW
         }  else {                       // else do not change water level, keep the old value
               level_array[j] = water_level;           // Magnet float will be physically stopped at the last reed switch level
         }                                //  Here all reed switches are probably open, so ADC reading will be close to 1024
                                         //  This will indicate the float is within the "blind spot" of one of the reed switches, so last read value will be displayed
         Delay_ms(5);    // Short delay
      }

      i = 0;
      mode = 0;
      mode_target = 5;       // At least 4 (algorithm counts same value twice) of the 8 readings should be the same to be takens as the reading

      while (mode < mode_target) {       // Take 8 readings, find the one that repeated at least 4 times
          while (i < 5) {
              for(j=0; j<8; j++) {
                  if(level_array[i] == level_array[j]) {
                      mode++;
                   }
              }

              if (mode >= mode_target ) {
                  water_level = level_array[i];
              } else {
                    mode = 0;
                    i++;
                }
          }
          mode_target--;    // Decrease occurence target to prevent an infinite loop
      }                     //   If target mode not reached, previous reading will be kept

      Lev_pow = 0;                          // Power down the sensor
      ADCON0.ADON = 0;                      // ADC disabled to prevent current consumption
      level_ADC = 0;
}