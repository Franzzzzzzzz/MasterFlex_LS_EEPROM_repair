This repository contains instructions and necessary files to repair a MasterFlex L/S pump model 07551-20 when an EEPROM failure prevent its from starting (errors 7, 8, 9).

The cost of repair from the manufacturer of this type of error is several thousand dollars, while the cost of repair of the method indicated here is a few dollars, several hours of work, and some tools (soldering iron etc.). You will need some soldering skills and programming knowledge. 

**No garanty is provided regarding the repair detailed below. It may make your pump permanently inoperable, dangerous, may result in lost of data, and certainly voids any warranty. You have been warned!**

Please read the codes provided before compiling them and using them. They most certainly require some changes and adjustements.

The process indicated here can probably be applied similarly to other pump models, but it would certainly require some changes. Basically, this repository is mainly a recording for myself on how to do the repair. 

## Overview of the process
1. Remove the power cord, open the pump, unplug the logic board (stack of 3 boards), identify the EEPROM, next to the microprocessor. All the cables are annoyingly short. 
2. Remove the original EEPROM and resolder it on a daughter board to extract the data. 
3. Extract the data from the original EEPROM with a Raspberry Pi
4. Solder a new EEPROM chip, reassemble temporarily and do a factory reset (left and right arrow pressed at power on). The pump will still not be working. 
5. Remove power to the pump. Solder jump wires on the newly soldered EEPROM (without removing it). Connect the EEPROM to the raspberry pi. Extract the factory data. 
6. Build the new ROM data by combining the dumps from the original ROM and the one from the new ROM. 
7. Upload the new ROM data to the new ROM. 
8. Unplug the raspberry pi, plug the pump and power. It should work now. 
9. Reassamble everything. 

### Reading / writing the EEPROM 93LC66B
A raspberry pi is used to read and write the EEPROM. The program 93LC66B.cpp handles the reading and writing. It always read the full content of the chip, but optionally writes a hex file before. Make sure you activate or deactivate the writing in the code as needed (using the `if (0)` or `if (1)` in the source for reading only or writing and reading, respectively. 

The defaults pins for the RPI are GPIO6 for CS, GPIO13 for CLK, GPIO19 for DI and GPIO26 for CI. These are conveniently aligned on the RPI3B. 

The program can be compiled simply with a `g++ -o 93LC66B 93LC66B.cpp`. It needs to run as `sudo` (to access the GPIO pins). 

Use the 3.3V power from the RPI to power the EEPROM. 

The RPI must be completely disconnected from the pump PCB when powering the pump with the EEPROM and the jump wired (step 4), or the pump and RPI conflict and weird stuff happens. 

### Preparing the ROM data to write to the EEPROM
If you feel extremely lucky, you could just try and write the file `ManufacturedWorking.bin` on the EEPROM using the program above, and try powering the pump. I would consider it unlikely to work well: the EEPROM seem to contain some calibration data which may differ from the one I have. This is also why the pump does not work on blank EEPROM, even after doing the factory reset (which does write some data on the EEPROM, but cannot rewrite the calibrtion that it does not have anymore).

My approach has been to combine a fresh dump from the factory reset, and the part of the original EEPROM that seem to contain calibration. To do all that:
1. Extract the original EEPROM
2. Wire it somehow to the RPI
3. Use the program 93LC66B in read only mode (**Make sure to deactivate writing!**). A `Dump.hex` is created.
4. Rename this `.hex` file to whatever you want, and save it somewhere. Disconnect the original EEPROM and store it somewhere just in case. 
5. Solder the new EEPROM on the pump board. Solder jump wires, but leave them disconnected from the RPI. Power on the pump pump. Turn it off. Power it again while maintaining the left and right arrow pressed at the same time. Power it off and disconnect it. 
6. Connect the jump wires to the RPI. 
7. Read the EEPROM without writing. Save the new `Dump.hex` created under a different name. 
8. Now, use the program CreateHexData (`g++ -o CreateHexData CreateHexData.cpp` to merge the part we want from the two hex file we dumped). The structure of the ROM seems to be roughly the following, identified from looking at the dump and sniffing the communication between the microcontroller and the ROM at initialisation (NB: the 93LC66B address 16 bits words, the address given there are in this space, you need to multiply by 2 if whatever hex viewer you are using is using 8bits words): 
  - Factory data from 0x00 to 0x7D (126 words, 252 bytes)
  - The word at address 0x7D (250 bytes) is almost certainly a checksum. I couldn't identify the algorithm used to calculate it. 
  - Calibration data from 0x7E to 0x9F (34 words, 68 bytes).
9. Create your new ROM data file to write by combining the data from the new EEPROM after factory reset, for the words before 0x7E, and from the original EEPROM for the words after 0x7E. The program `CreateHexData` can help you do that. 
10. Write this new ROM data using the program `93LC99B` with the writing unable now. 
11. Unplug the RPI. Power on the pump. Hopefully, it works now. 
12. If all looks good, desolder the jump wires and put everything back together. 

















