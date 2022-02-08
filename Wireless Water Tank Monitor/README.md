<h3>PROJECT NAME</h3>
Wireless Water Tank Monitor
     
<h3>Author</h3>
Cagan Cerkez, 2014
     
<h3>DESCRIPTION</h3>
As the water supply in Cyprus is not continuous, every house has a +2 ton water tank, equipped with an extra hot water tank that is connected to solar water heaters. As these water tanks are usually installed on roof tops, they are hard to reach, so it is not easy to know how much water you have and if you are about to run out. Also, the float switch that shuts off incoming water when the tank is full occasionally fails, and by the time the user notices the issue, water keeps running. The other challenge is with the solar panel connected hot water tank; during winter when the weather can be cloudy, users do not know how hot the water is, so they turn on the heating element to make sure their water is warm enough, which might not be necessary and hence increase their electricity bill.

To address these issues, two wireless, battery powered units are designed; one working as a transmitter and the other as a receiver. Transmitter unit will be installed on the water tank and will continuously measure water level and hot water temperature.

Receiver unit will have a 3.5 digit segmented LCD which will show the water level and temperature. A buzzer is installed on the receiver unit which will alarm if the float switch failed (i.e. water level exceeded the expected 100% level) or if water level is less than 30%, so the remaining water can be used more carefully or a refill might be requested.

Power consumption of both units is optimized to have +1 year battery life. Both the transmitter and the receiver sleep for 5 minutes. Transmitter wakes up, takes measurements, transmits and then goes back to sleep. Receiver automatically synchronizes sleep times based on the transmission times.


<h3>RECEIVER UNIT</h3>
Data transmitted via a Microchip MRF89AM8A Transceiver are received and displayed on a segmented LCD with a very low power profile. Device is battery powered. Hot water temperature in the water tank is displayed when the button is pushed, otherwise the water level is displayed as the default value. Buzzer sounds an alarm when water overflows (indicating a problem with the overflow mechanism that should have cut water supply) or if water level is <30%. Buzzer will continue alarming until water level goes above 30% or till the user presses the "show water temperature" button to acknowledge the alarm. Battery voltage is also checked together with the battery voltage indicator byte received from the transmitter; so when the "low bat" LED is lit on the LCD display, either the RX unit's or TX unit's battery can be low (same indicator used to alert for both units).

<h3>RECEIVER CONFIG</h3>
**MCU**: PIC16F917<br/>
**Oscillator**: 32.768 kHz crystal in LP mode<br/>
**TMR1**: Running on internal 8MHz crystal<br/>
**SW**: mikroC PRO for PIC<br/>
**Batteries**: x2 AA batteries connected in series<br/>
**3.5 Digit LCD**: Static LCD with only COM0 used<br/>
**Temperature Display Switch Port**: RB0<br/>
**MRF89AM8A**: 868MHz transceiver, conforms European ETSI standard<br/>

<h6>NOTES</h6>
Tick "Use VPP first program entry" on PICkit 2 under "Tools" menu before programming

<h6>Receiver Schematic</h6>
<img src="https://github.com/Choo57/Microchip/blob/main/Wireless%20Water%20Tank%20Monitor/receiver_schematic.png" width=65% height=65%>

<h3>TRANSMITTER UNIT</h3>
Water level is measured using a self-made 10-level reed switch float level probe. Reed switches are sealed inside a plastic pipe, so no contact with water. A magnet, also sealed inside a float around the vertically placed reed switch pipe turns on reed switches, which are read to find the water level. Hot water temperature is measured using an MCP9700A temperature sensor. Measured values are transmitted via a Microchip MRF89XAM8A Transciever. Device is battery powered. Timer1 and 32.768kHz crystal is used during sleep for improved/consistent sleep periods so the RX unit can synchronize reception more successfully.

<h3>TRANSMITTER CONFIG</h3>
**MCU**: PIC16F883<br/>
**Oscillator**: 8.000 MHz (using internal oscillator)<br/>
**SW**: mikroC PRO for PIC<br/>
**Battery**: x1 D battery<br/>
**Level probe Ports**: AN11 & AN13<br/>
**MCP9700A**: Vs (RB1), Vout (RB0, AN12)<br/>
**MRF89XAM8A**: CSCON (RC6), CSDATA (RC2), SPI (RC3 RC4 RC5)<br/>
**Low-voltage ICSP**: RB6 and RB7 reserved for PGD and PGC<br/>

<h6>NOTES</h6>
Send 203 at the 1st, 2nd, 9th and 10th packets to indicate continuous transmit mode during the initial power on
RB3 should be grounded during low voltage ICSP


<h6>Transmitter Schematic</h6>
<img src="https://github.com/Choo57/Microchip/blob/main/Wireless%20Water%20Tank%20Monitor/transmitter_schematic.png" width=65% height=65%>
