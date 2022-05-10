#!/usr/bin/env python

from smbus import SMBus
import time
from datetime import datetime
import os
import time

logfile = "temperature_log.txt"
i2caddress = 0x05
hysteresis = 5
target_temp = 55
fan_state = False

i2cbus = SMBus(22)


# returns temperature as float in degree celcius
def temperature_of_raspberry_pi():
    cpu_temp = os.popen("vcgencmd measure_temp").readline()
    return float(cpu_temp.replace("temp=", "").replace(r"'C", ""))

# returns temperature as float in degree celcius
def temperature_of_attiny():
    i2cbus.write_byte_data(i2caddress, 0x02, 0x34) # Trigger temp read
    time.sleep(0.01)  # Wait 10ms
    lowbyte = i2cbus.read_byte_data(i2caddress, 0x00)  # Read the value of lowbyte
    highbyte = i2cbus.read_byte_data(i2caddress, 0x01)  # Read the value of lowbyte
    
    # Temperature vs. Sensor Output Voltage (Typical Case)
    # Temp: -40'C    +25'C    +85'C
    # ADC:   230      300      370
    return float(highbyte * 256 + lowbyte - 275)

def turn_fan_on():
    global fan_state
    fan_state = True
    i2cbus.write_byte_data(i2caddress, 0x03, 0xff) # Set fan PWM to 255

def turn_fan_off():
    global fan_state
    fan_state = False
    i2cbus.write_byte_data(i2caddress, 0x03, 0x00) # Set fan PWM to 0   

def loop_cycle():   
    global fan_state
    # Read temperatur of Raspberry Pi
    pi_temp = temperature_of_raspberry_pi()

    # Read temperatur of ATtiny
    attiny_temp = temperature_of_attiny()
    
    if fan_state:
        if pi_temp < target_temp - hysteresis:
            turn_fan_off()
    else:
        if pi_temp > target_temp + hysteresis:
            turn_fan_on()
    
    current_time = datetime.now().strftime("%H:%M:%S")
    
    message = current_time + ": pi(" + str(pi_temp) + r"'C) attiny(" + str(attiny_temp) + "'C) -> " + str(fan_state)
    print(message)
    
    filesize = 0

    with open(logfile, "a") as myfile:
        myfile.write(message + "\n")
        filesize = myfile.tell()

    if filesize > 10000:
        print("filesize limit reached - clean file")
        with open(logfile, "w") as myfile:
            myfile.write(message + "\n")


def main():

    turn_fan_off()

    while True:
        loop_cycle()
        time.sleep(15)
    
if __name__ == "__main__":
    main()
