/*
   LPG Level Sensor Header file

   Measures the temperature of the LPG tank at 2 vertically placed points and tries to find the
   LPG level by looking at a temperature difference between the two temperture sensors;
   Evaporating LPG will decrease the temperature of the lower sensor while the upper sensor
   will show a higher temperature if it is above the liquid level

   Defines:
          IC connection pins
          Function to measure temperature
*/


// LPG Level Measurement Temperature Sensor connections
sbit LPG_pow at RA3_bit;               // Vsupply of sensors connected to RA3
sbit LPG_pow_Direction at TRISA3_bit;  // Vsupply will be set as output
sbit LPG_low_Direction at TRISA2_bit;  // Lower temperature sensor input pin (RA2) direction
sbit LPG_up_Direction at TRISA1_bit;   // Upper temperature sensor input pin (RA1) direction
static const LPGlo_ADC = 2;            // Output of lower temperture sensor connected to AN2 (RA2)
static const LPGhi_ADC = 1;            // Output of lower temperture sensor connected to AN1 (RA1)
// * * * DO NOT FORGET TO SET THE ASSOCIATED POWER PIN AS DIGITAL & INPUT PIN ANALOG

// End of LPG Level Measurement Temperature Sensor connections

static const LPG_threshold = 15;      // Expected temperature difference between lower and higher temperature sensors when the
                                      //   LPG level is inbetween the two sensors. eg: if set as 15, when upper sensor is at least
                                      //   1.5C warmer than the lower sensor, the alarm will be triggered

unsigned short LPG_empty = 0;       // 0 = LPG tank NOT empty,  1 = LPG tank empty

// Function to read lower temperature via MCP9700A
void measure_LPG(void){
     unsigned long temperature_4byte = 0;
     unsigned long total_temperature = 0;
     unsigned short lo_temp = 0;       // Temperature variable unsigned, assuming the temperature will be >0 always
     unsigned short hi_temp = 0;       // Temperature variable unsigned, assuming the temperature will be >0 always

     char j=0;

     LPG_pow = 1;                         // Power up the sensor
     //ADC_Init();                           // Initialize the ADC module
     Delay_ms(50);                        // Wait for the sensor to stabilise (min 1ms)

     for(j=0; j<8; j++) {
          temperature_4byte = ADC_Read(LPGlo_ADC);  // Read from the associated channel

         // ADC reference voltage is 3.3V! Scale up the result (3300 mv / 1024 steps = 3.22)
         // Convert to temperature to C by /10, take the decimal part by /100
         // MCP9700A has a 500mV offset, so the ADC reading will be 500/3.22=155.27 steps more
         // Just the 10-bit 0-3.3v ADC conversaion itself introduces a 0.028 - 0.374 C difference between the analog reading and the digital result
         // Rounding >0.5 UP and <0.5 down will give result in -0.4 - 0.8 C difference between the read analog temperature and the displayed (only decimal) temperture
         // Can display the first decimal of the temperture for better accuracy, which will yield 0-0.4 C difference
         temperature_4byte = temperature_4byte - 155;            // Subtracting 155 (not 155.27) steps for the 500mV offset makes the accuracy a bit better
         temperature_4byte = ((322*temperature_4byte)/100);      // If real temperature is 20 C, temperature_4byte is is 199 (199.64) at this stage (19.964 C x 10)

         total_temperature = total_temperature + temperature_4byte;
         Delay_ms(5);                               // Short delay
      }

      // AVERAGE 8 consecutive temperature readings for increased accuracy
      lo_temp = (total_temperature)/8;   // If real temp is 19.3 C, lo_temp = 193 here

      total_temperature = 0;
      temperature_4byte = 0;
      j=0;


    // Read UPPER temperature via MCP9700A

     for(j=0; j<8; j++) {
          temperature_4byte = ADC_Read(LPGhi_ADC);  // Read from the associated channel

         // ADC reference voltage is 3.3V! Scale up the result (3300 mv / 1024 steps = 3.22)
         // Convert to temperature to C by /10, take the decimal part by /100
         // MCP9700A has a 500mV offset, so the ADC reading will be 500/3.22=155.27 steps more
         // Just the 10-bit 0-3.3v ADC conversaion itself introduces a 0.028 - 0.374 C difference between the analog reading and the digital result
         // Rounding >0.5 UP and <0.5 down will give result in -0.4 - 0.8 C difference between the read analog temperature and the displayed (only decimal) temperture
         // Can display the first decimal of the temperture for better accuracy, which will yield 0-0.4 C difference
         temperature_4byte = temperature_4byte - 155;            // Subtracting 155 (not 155.27) steps for the 500mV offset makes the accuracy a bit better
         temperature_4byte = ((322*temperature_4byte)/100);      // If real temperature is 20 C, temperature_4byte is 199.64 at this stage (19.964 C x 10)

         total_temperature = total_temperature + temperature_4byte;
         Delay_ms(5);                               // Short delay
      }

      // AVERAGE 8 consecutive temperature readings for increased accuracy
      hi_temp = (total_temperature)/8;   // If real temp is 19.3 C, hi_temp = 193 here

      LPG_pow = 0;                          // Power down the sensor
      ADCON0.ADON = 0;                      // ADC disabled to prevent current consumption
      total_temperature = 0;
      temperature_4byte = 0;
      
      if ((hi_temp - lo_temp) > LPG_threshold) {
          LPG_empty = 1;
      } else {
            LPG_empty = 0;
      }
}