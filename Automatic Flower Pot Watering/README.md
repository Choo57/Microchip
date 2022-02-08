<h3>PROJECT NAME</h3>
Automatic Flower Pot watering
     
<h3>Author</h3>
Cagan Cerkez, 2014
     
<h3>DESCRIPTION</h3>
PIC12F1501 will run a small submersible pump, once every 24 hours. User will
press the button on the board, TMR1 will measure the time the button is pressed for and
right after the button is released, the LED will be lit for that long to confirm with 
the user the time the button was pressed for. MCU will then operate the pump for that
duration every 24 hours. The LED on the board will blink periodically to inform the 
user that the device is working fine and to indicate that the batteries have not died.
If the user presses the button again, the 24-hour period will be restarted
and the watering period will be updated.

(To be included in the next version: WDT will be used to prevent any unexpected 
freezes/failures as much as possible.)
     
WDT and the uncalibrated 31 kHz internal oscillator will be used to minimize the
required external components (e.g. external 32.768kHz crystal) as an exact 24-hour 
period is not required. According to PIC12F1501 datasheet, page 264-265, LFINTOSC
accuracy is between 35kHz-27kHz (+-13%, +-3 hour difference from expected 24-hour period) 
and HFINTOSC is +-2% accurate (+-0.5 hour difference from expected 24-hour period).
     
<h3>CONFIG</h3>
MCU:                      PIC12F1501<br/>
Oscillator:               31 KHz internal uncalibrated<br/>
SW:                       mikroC PRO for PIC<br/>

<h6>Schematic</h6>
<img src="https://github.com/Choo57/Microchip/blob/main/Automatic%20Flower%20Pot%20Watering/Schematic.PNG" width=65% height=65%>

<h6>Board</h6>
<img src="https://github.com/Choo57/Microchip/blob/main/Automatic%20Flower%20Pot%20Watering/photo%20-%20board.png" width=35% height=35%>

<h6>Photo</h6>
<img src="https://github.com/Choo57/Microchip/blob/main/Automatic%20Flower%20Pot%20Watering/photo%20-%20complete.png" width=40% height=40%>
