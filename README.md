# ELECTRONIC TECHNOLOGIES AND BIOSENSORS LABORATORY, a.a. 2021/22

# Project 4 - Colombo L., D'Andrea M.

## Objective of the project

The objective of the project is to design and develop a system able to perform glucose monitoring using PSoC 5 as a potentiostat. Internal subdivision of the repository is as following:

- Case: contains the .stl files for the case design and printing
- Documents: contains all the bibliography related to the project and the datasheets of the hardware components
- Group2.cydsn: backup folder containing only the code related to glucose measurement, peripherals not included. **It is not to be intended as the folder of the whole project**
- PCB: contains schematic and board files for the PCB of the device
- Presentations: contains images of the project and final presentation
- Project_design: folder of the actual project
- PYTHON GUI: folder containing python scripts for the GUI
- Regression: folder containing the regression script used to find a calibration line and related images

**To open and run the project, open the Project_ETBL.cywrk file**

## Features implemented

Besides being capable of performing glucose measurements, the device hardware includes:

- RTC (DS3231): for real time tracking
- Bluetooth BR/EDR module (HC-05): for wireless communication
- External battery
- OLED screen (SSD1306 by Adafruit): to visualize data when GUI is not active
- External EEPROM (24LC512 by MicroChip): to store measurements

The assembled device and PCB are in the following images:
<br>

<p align="center">
<img alt="PCB" src="https://github.com/ltebs-polimi/AY2122_II_Project-2/blob/master/Presentations/Immagini/WhatsApp%20Image%202022-06-29%20at%2013.21.22.jpeg" />
</p>
<br><br>

<br>

<p align="center">
<img alt="DEVICE" src="https://github.com/ltebs-polimi/AY2122_II_Project-2/blob/master/Presentations/Immagini/Case.png" />
</p>
<br><br>
