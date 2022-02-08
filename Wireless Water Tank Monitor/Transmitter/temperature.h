/*
   MCP9700A Header file

   Defines:
          IC connection pins
          Function to measure temperature
*/


// Temperature Sensor connections
sbit T_pow at RB1_bit;               // Vsupply of sensor connected to RB1
sbit T_pow_Direction at TRISB1_bit;  // Vsupply will be set as output
sbit Tin_Direction at TRISB0_bit;    // Temperature input pin (RB0) direction
static const T_ADC = 12;             // Output of sensor connected to AN12 (RB0)
// * * * DO NOT FORGET TO SET THE ASSOCIATED POWER PIN AS DIGITAL & INPUT PIN ANALOG
// End of Temperature Sensor connections

unsigned short temperature = 0;                          // Temperature variable unsigned, assuming the temperature will be >0 always

// Function to read water temperature from MCP9700A
void measure_temperature(void){
     unsigned long temperature_4byte = 0;
     unsigned long total_temperature = 0;
     char j=0;

     T_pow = 1;                            // Power up the sensor
     //ADC_Init();                           // Initialize the ADC module
     Delay_ms(50);                        // Wait for the sensor to stabilise (min 1ms)

     for(j=0; j<8; j++) {
          temperature_4byte = ADC_Read(T_ADC);  // Read from the associated channel

         // ADC reference voltage is 3.3V! Scale up the result (3300 mv / 1024 steps = 3.22)
         // Convert to temperature to C by /10, take the decimal part by /100
         // MCP9700A has a 500mV offset, so the ADC reading will be 500/3.22=155.27 steps more
         // Just the 10-bit 0-3.3v ADC conversaion itself introduces a 0.028 - 0.374 C difference between the analog reading and the digital result
         // Rounding >0.5 UP and <0.5 down will give result in -0.4 - 0.8 C difference between the read analog temperature and the displayed (only decimal) temperture
         // Can display the first decimal of the temperture for better accuracy, which will yield 0-0.4 C difference
         temperature_4byte = temperature_4byte - 155;            // Subtracting 155 (not 155.27) steps for the 500mV offset makes the accuracy a bit better
         temperature_4byte = ((322*temperature_4byte)/100);      // If real temperature is 20 C, temperature_4byte is 199.64 at this stage (19.964 C x 10)

         // For the prototype, MCP9701 is used. Offset voltage = 400mV, temperature coefficient = 19.5 mV/C
         //temp_adc = temp_adc - 124;            // Exact offset steps = 124.12
         //temperature_4byte = ((322*temp_adc)/195)%100;   // This will give the first decimal point value

         if ((temperature_4byte%10) > 4) {                        // Find the first digit after the decimal and check if it is > 4
              temperature_4byte = (temperature_4byte/10) + 1;     // Round UP if the first decimal is greater than 0.49
         } else {
               temperature_4byte = temperature_4byte/10;          // Round DOWN if smaller than 0.5
         }

         total_temperature = total_temperature + temperature_4byte;
         Delay_ms(5);                               // Short delay
      }

      // AVERAGE 8 consecutive temperature readings for increased accuracy
      total_temperature = total_temperature*10;    //  Multiply by 10 to get the first decimal of the average and round accordingly

      total_temperature = (total_temperature)/8;   // ***** Room for improvement for a better algorithm *****
      if ((total_temperature%10) > 4) {
          total_temperature = (total_temperature)/10;                       // Round UP if higher than 0.49
          temperature = Lo(total_temperature)+1;
      } else {
            total_temperature = (total_temperature)/10;                     // Round down if lower than 0.5
            temperature = Lo(total_temperature);
        }

      T_pow = 0;                            // Power down the sensor
      ADCON0.ADON = 0;                      // ADC disabled to prevent current consumption
      total_temperature = 0;
      temperature_4byte = 0;
}