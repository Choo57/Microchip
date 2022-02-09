<h3>PROJECT NAME</h3>
Timer - Periodically turn on strobe lights
     
<h3>Author</h3>
Cagan Cerkez, 2012
     
<h3>DESCRIPTION</h3>
A friend asked if we can build a timer that will control the strobe lights in his gym. He created "stations" where athletes would rotate through in turns. 
He wouuld set the period he wants athletes to spend on each station (e.g. 5 minutes), and have the strobe lights "alarm" everyone in the room to switch to the 
next station after the set period expired. 

A simple timer circuit was designed with an HD44780 LCD display that operates a relay continuously until stopped for a set period. The period can be adjusted by stopping the counter and using the UP or DOWN buttons.

<h3>CONFIG</h3>
<b>MCU</b>: PIC16F887<br/>
<b>Oscillator</b>: 4.0000 MHz<br/>
<b>LCD</b>: HD44780, 2x16 blue character<br/>
<b>SW</b>: mikroC PRO for PIC<br/>

<h3>NOTES</h3>
- Start/Stop button connected to RB0<br/>
- UP button connected to RD3<br/>
- DOWN button connected to RD2<br/>
- Relay connected to RD1<br/>
- LCD connected to RB2-RB7<br/>

<h6>Schematic</h6>
<img src="https://github.com/Choo57/Microchip/blob/main/Timer%20Periodically%20Turn%20On%20Strobe%20Lights/schematic.png">

<h6>Etched board</h6>
<img src="https://github.com/Choo57/Microchip/blob/main/Timer%20Periodically%20Turn%20On%20Strobe%20Lights/photo_etched.png">

<h6>Mounted PCB</h6>
<img src="https://github.com/Choo57/Microchip/blob/main/Timer%20Periodically%20Turn%20On%20Strobe%20Lights/photo_pcb.png">

<h6>Case</h6>
<img src="https://github.com/Choo57/Microchip/blob/main/Timer%20Periodically%20Turn%20On%20Strobe%20Lights/photo_case.png">
