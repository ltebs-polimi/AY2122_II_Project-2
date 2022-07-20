# ELECTRONIC TECHNOLOGIES AND BIOSENSORS LABORATORY, a.a. 2021/22

# Project 2 - Colombo L., D'Andrea M.

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

## PSoC code
The PSoC code is composed by:

- A global switch case handling the passage between the different OLED screens, up to the "Press the button to START the measurement" screen

- When the hardware button is pressed, the code automatically fetches potential values from the EEPROM and starts and amperometry scan. Basically "adcDacInterrupt" scans
a look-up table and sets the values to the DAC. At the same time the "adcAmpInterrupt" converts the values read by the ADC into current. In the end a glucose 
concentration value is retrieved by means of a proper calibration curve and the result is shown on the OLED screen.

- In case the GUI is started, the code allows to have a connection to the GUI and all the possible commands are managed. Among the different possibilities:
    - cyclic voltammetry (CV): another look up table is scanned by "dacInterrupt" and the "adcInterrupt" convertes the ADC readings into current. These values are sent to the GUI via bluetooth
    - chronoamperometry (CA): same as described before but in this case the current values are sent via bluetooth to the GUI
    - measurements history: measurements which were saved into the EEPROM (date, time and glucose value) at the end of a CA, are sent to the GUI via bluetooth

**see "GUI code" for further details**

## GUI code
The GUI is based on 3 different screens:

- first_screen.py: it allows to select between a user and a clinician

- user_screen.py:
    - Automatic COM port detection by sending a proper command to the PSoC and waiting for the expected answer
    - By pressing the "START MEASUREMENT" button, only a chronoamperometry will be started by sending a proper command to the psoc. The two ISRs needed for the chronoamperometry will be started. Finally the result of the measurement is displayed

- clinician_screen.py: 
    - Automatic COM port detection by sending a proper command to the PSoC and waiting for the expected answer
    - First tab (cyclic voltammetry): it is possible to setup all the required parameters for a cyclic voltammetry. Then it allows to visualize a real-time
    graph by pressing "DRAW"
    - Second tab (chronoamperometry): The clinician can decide to fetch a value of potential from the EEPROM or to start a CA based on the peak of potential identified 
    by the previous cyclic voltammetry. In both cases the real-time graph is shown
    - Third screen (Data visualization): it displays the result of the chronoamperometry scan, in terms of glucose concentration
    - Fourth tab (History): it allows to check the three latest measurements taken by the device. There is also the indication about the date and the time at which the 
    measurements have been taken

