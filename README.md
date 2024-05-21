# Traffic-Intersection-Lab
This repo showcases my completed driver for lab 10, the traffic intersection lab, of [Embedded Systems Shape the World](https://www.edx.org/learn/embedded-systems/the-university-of-texas-at-austin-embedded-systems-shape-the-world-microcontroller-input-output).

The most important file is [TableTrafficLight.c](https://github.com/BrianTibbetts/Traffic-Intersection-Lab/blob/main/src/TableTrafficLight.c), and some sections of it to read include:
- Lines 14-42: A finite state machine for the traffic intersection is implemented here. It defines how each traffic light should change based on the cars, pedestrians, and the other traffic lights. The repo includes a [diagram concept](https://github.com/BrianTibbetts/Traffic-Intersection-Lab/blob/main/Lab10_traffic_intersection.png) of the intersection.
- Lines 87-107: The main function progresses through the traffic intersection states based on the switches pressed, cycling through the different colored LEDs on a breadboard like a set of traffic lights would. See my [completed breadboard](https://github.com/BrianTibbetts/Traffic-Intersection-Lab/blob/main/Lab10_complete_breadboard.jpg) showing the yellow switches and red, yellow, and green LEDs which represent the intersection.

### Resources Used
- TM4C123G Launchpad data sheet: https://www.ti.com/lit/ds/spms376e/spms376e.pdf
- Project template: https://github.com/pgimalac/tm4c-linux-template
