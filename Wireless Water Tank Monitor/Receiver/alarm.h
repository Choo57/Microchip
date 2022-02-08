/*
    Alarm Header file

    Header to check for the conditions to sound the buzzer
    Will minimize code repetition and make it easier to modify
*/


void check_alarm() {
    if ((level >= 35) && (level <=100)) {       // Overflow or low water level condition ended, enable warning
        mute_alarm = 0;                         //  Alarm will be set until either tank is filled (or ovewflow stopped) or the show temperature
    }                                           //    button is pressed (indicating someone saw the low water level)

    if (level == 200) {                        // Water level = 200 means water is overflowing
        overflow = 1;                         //  Set the corresponding alarm flag and make LCD show 100% as the water level
        level = 100;
    } else {                                  // Stop the alarm if water level is <= 100%
        overflow = 0;
    }

    if (((level <= 30) || (overflow == 1)) && (mute_alarm == 0)) {   // Sound the buzzer if water level is low or if there is an overflow
        beep();
    }
}

void display_bat() {
    if ((low_batRX == 1) || (low_batTX == 1)) {
        displayNumber(0, 3);    // Light the BAT warning led
    } else {
          displayNumber(0, 4);  // Turn off the BAT warning led
      }
}