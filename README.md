# Automated-Pet-Feeder
Developed an automated pet feeder using the TM4C123GH6Pm microcontroller. The system dispenses food in regards to a feeding schedule and water based on the settings the owner configures. The pet feeder also can detect motion for dispensing water if "Motion" mode is set.

![Automated Pet Feeder](https://github.com/user-attachments/assets/d0004f1b-4785-482b-bc7c-7fc9747d0579)

# Hardware Components
|                                                    |
|----------------------------------------------------|
|TM4C123GH6PM Tiva Board|
|PIR motion sensor|
|Capacitive water sensing circuit|
|Water pump|
|Auger controlled by motor|
|AT-1127-ST-2-R 2730 Hz resonant transducer (Speaker)|
|PSMN4R3-30PL logic-level MOSFETs (2)|

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
* `alert x`: enables or disables the speaker.
* `Show`: shows the current setting stored in EEPROM for water level, fill mode, and alert status.

![Pet Feeder Interface 1](https://github.com/user-attachments/assets/e9ef5b04-3b54-45f9-88e8-0a8d7ed9b68a)
![Pet Feeder Interface 2](https://github.com/user-attachments/assets/220a1c4c-d43c-4b93-963a-3e124b6d752e)
![Pet Feeder Interface 3](https://github.com/user-attachments/assets/807db67b-6887-4bd7-97b2-96726e5ae693)
![Pet Feeder Interface 4](https://github.com/user-attachments/assets/b7718a84-a3d2-4625-bca7-0963172368b8)




* `alert x`: either activates or deactivates the speaker. Speaker is used to alert user there is low water.
* `Show`: displays settings saved on EEPROM for water level, fill mode, and alert status.  
