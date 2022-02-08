/*
   Piezo Transducer Header file

   Defines:
          Piezo Transducer connection pins
          Piezo Transducer on breadboard from Futurlec has Resonant Frequency 1.1 kHz
          Part from Digikey (445-5229-1-ND) has Resonant Frequency 4kHz, chart on the datasheet shows max output at 5kHz
*/

void beep() {                       // Execution of the while loop and the timer increment adds additional delays,
    unsigned int count_time = 0;    //    so the frequency will be faster than calculated.
    
    while (count_time < 1750) {     // count_time = 1750 will keep the buzzer on for ~350ms
        buzzer = ~buzzer;           // Toggle the pin
        count_time++;               // Without any delay functions, the buzzer pin toggles every 6us (167 kHz)
        Delay_us(500);
       // Delay_us(193);              // With 193us delay, a signle iteration of the while loop will take 200.5 us
    }                               //   which will toggle the buzzer pin at 5kHz
    buzzer = 0;                     // Measured with the Debugger
    
/*    Delay_ms(100);                  // 100ms delay
    count_time = 0;
    
    while (count_time < 1750) {     // count_time = 1750 will keep the buzzer on for ~350ms
        buzzer = ~buzzer;           // Toggle the pin
        count_time++;               // Without any delay functions, the buzzer pin toggles every 6us (167 kHz)
        Delay_us(500);
      //  Delay_us(193);              // With 193us delay, a signle iteration of the while loop will take 200.5 us
    }                               //   which will toggle the buzzer pin at 5kHz
    
    buzzer = 0;
*/
}