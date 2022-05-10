# ATtiny_PI_FANcontrol

Small program to control the fan for Raspberry Pi depending on temperature


**Files:**
- [ATtiny_fan_control](https://github.com/techniccontroller/ATtiny_PI_FANcontrol/tree/main/ATtiny_fan_control): contains code for ATtiny85
- [fan_control.py](https://github.com/techniccontroller/ATtiny_PI_FANcontrol/blob/main/fan_control.py): python script running on Raspberry Pi

### Pictures

<img src="https://user-images.githubusercontent.com/36072504/167672236-fb789b67-f7db-41ea-92d1-ed57bed98eb7.jpg" width="400px"/>
<img src="https://user-images.githubusercontent.com/36072504/167672731-e70874c9-d911-40d4-8603-45270ab03318.jpg" width="400px"/>


### Schematic

<img src="https://user-images.githubusercontent.com/36072504/167681456-2460f52a-1df9-4b14-ac1c-38699f486026.jpg" width="600px"/>

### Getting Started

1. Clone repository to Raspberry Pi
2. Upload ATtiny source to ATtiny85 (e.g with USBtinyISP programmer)
3. Connect ATtiny to Raspberry Pi as shown in schematic
4. Copy **fan_control.service** to **/etc/systemd/system/fan_control.service** (You may first need to adjust the paths inside the file)
```
$ sudo cp fan_control.service /etc/systemd/system/fan_control.service
```
6. Enable service with
```
$ sudo systemctl enable fan_control.service
```
7. Start service with
```
$ sudo systemctl start fan_control.service
```
8. Check if a file **temperature_log.txt** is created
```
$ cat temperature_log.txt
```
The output should look like this:
```
19:17:48: pi(47.2'C) attiny(32.0'C) -> False
19:18:03: pi(49.1'C) attiny(32.0'C) -> False
19:18:18: pi(48.2'C) attiny(32.0'C) -> False

```


