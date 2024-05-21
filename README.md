# Traffic-Intersection-Lab
This repo showcases my completed driver for lab 10, the traffic intersection lab, of [Embedded Systems Shape the World](https://www.edx.org/learn/embedded-systems/the-university-of-texas-at-austin-embedded-systems-shape-the-world-microcontroller-input-output).

The most important file is /src/TableTrafficLight.c, and some sections of it to look at include:
- Lines 14-42: A finite state machine for the traffic intersection is implemented here. It defines how each traffic light should change based on the cars and pedestrians (the breadboard switches) and the other traffic lights.
- Lines 87-107: In this section, the main function progresses through the traffic intersection states based on the switches pressed, lighting up the LEDs on the breadboard.

### Resources Used
- TM4C123G Launchpad data sheet: https://www.ti.com/lit/ds/spms376e/spms376e.pdf
- Project template: https://github.com/pgimalac/tm4c-linux-template
