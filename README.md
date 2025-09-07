# Automated-Pet-Feeder
Developed an automated pet feeder using the TM4C123GH6Pm microcontroller. The system dispenses food in regards to a feeding schedule and water based on the settings the owner configures. The pet feeder also can detect motion for dispensing water if "Motion" mode is set.

# Hardware Components
|                                                    |
|----------------------------------------------------|
|TM4C123GH6PM Tiva Board|
|PIR motion sensor|
|Capacitive water sensing circuit|
|Water pump|
|Auger controlled by motor|
|AT-1127-ST-2-R 2730 Hz resonant transducer (Speaker)|
|PSMN4R3-30PL logic-level MOSFETs|

# Peripherals Used
|                   |
|-------------------|
| UART |
| TIMERS |
| Hibernation |
| Analog Comparator |
| PWM |
| EEPROM |

# Software Features
* `time HH:MM`: sets the time for the pet feeder to start counting at.
* `time`: displays the current time.
* `feed a b c d e`: adds a feeding schedule to save to EEPROM. a:schedule number, b: duration food will be dispensed on the bowl, c: motor speed, d is for hour, and e is for minutes.
* `Display`: shows the feeding schedule stored on EEPROM.
* `feed x delete`: deletes the selected feeding schedule from EEPROM.
* `water x`: sets a threshold for amount of water to be dispensed on the bowl.
* `fill x`: selects which mode to configure (either auto or motion) for dispensing water.
* `alert x`: either activates or deactivates the speaker. Speaker is used to alert user there is low water.
* `Show`: displays settings saved on EEPROM for water level, fill mode, and alert status.  
