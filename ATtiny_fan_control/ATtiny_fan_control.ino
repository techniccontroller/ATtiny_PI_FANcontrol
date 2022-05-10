/**
 * @file ATtiny_fan_control.ino
 * @author techniccontroller (mail@techniccontroller.com)
 * @brief ATiny code for fan control program for Raspberry Pi
 * @version 0.1
 * @date 2022-05-10
 * 
 * The program basically just provide a 4 byte large register where a master can read/write from/to via I2C
 * 
 * - The first two bytes holds the temperature of the internal temperatur sensor
 * - The third byte is there to trigger a new temperature measurement 
 *   -> the master just need to write a value other than 0x33 to trigger a new measurement.
 * - The fourth byte is the analog value to be output via PWM
 * 
 * 
 * ATtiny85 pinout:
 * 
 *           -------
 *        --|*      |-- 3.3V
 *        --|       |-- SCL
 *    PWM --|       |-- 
 *    GND --|       |-- SDA
 *           -------
 * 
 * Board settings in Arduino IDE:
 * - Board: ATtiny45/85 (Optiboot)
 * - Chip: ATtiny85
 * - Clock: 8 MHz (internal)
 * - LTO: Enabled
 * - B.O.D. Level: Disabled
 * - millis()/micros(): Enabled
 * - Timer 1 Clock: 64MHz
 * - Programmer: USBtinyISP
 * 
 * 
 * 
 * Most of the code is taken over from example code of TinyWireS library from 
 * https://github.com/rambo/TinyWire/blob/rollback/TinyWireS/examples/attiny85_i2c_slave/attiny85_i2c_slave.ino
 * 
 * I used following version of the TinyWireS library:
 * Commit: https://github.com/rambo/TinyWire/commit/6f118887c1b115e1af64246e4b10dc04bb7091a1
 * Date: 29. Jan. 2021 
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#include <Arduino.h>
#define I2C_SLAVE_ADDRESS 0x5 // the 7-bit address
// Get this from https://github.com/rambo/TinyWire
#include <TinyWireS.h>
// The default buffer size
#ifndef TWI_RX_BUFFER_SIZE
#define TWI_RX_BUFFER_SIZE ( 16 )
#endif

#define PWM_PIN 4

uint8_t analogValue = 255;

volatile uint8_t i2c_regs[] =
{
    0xDE, // lowbyte of temperature
    0xAD, // highbyte of temperatur
    0xBE, // trigger measurement
    0xEF, // PWM value
};
// Tracks the current register pointer position
volatile byte reg_position;
const byte reg_size = sizeof(i2c_regs);

/**
 * This is called for each read request we receive, never put more than one byte of data (with TinyWireS.send) to the 
 * send-buffer when using this callback
 */
void requestEvent()
{  
    TinyWireS.send(i2c_regs[reg_position]);
    // Increment the reg position on each read, and loop back to zero
    reg_position++;
    if (reg_position >= reg_size)
    {
        reg_position = 0;
    }
}

/**
 * The I2C data received -handler
 *
 * This needs to complete before the next incoming transaction (start, data, restart/stop) on the bus does
 * so be quick, set flags for long running tasks to be called from the mainloop instead of running them directly,
 */
void receiveEvent(uint8_t howMany)
{
    if (howMany < 1)
    {
        // Sanity-check
        return;
    }
    if (howMany > TWI_RX_BUFFER_SIZE)
    {
        // Also insane number
        return;
    }

    reg_position = TinyWireS.receive();
    howMany--;
    if (!howMany)
    {
        // This write was only to set the buffer for next read
        return;
    }
    while(howMany--)
    {
        i2c_regs[reg_position] = TinyWireS.receive();
        reg_position++;
        if (reg_position >= reg_size)
        {
            reg_position = 0;
        }
    }
}


void setup()
{
    pinMode(PWM_PIN, OUTPUT);

    analogReference(INTERNAL); // You must use the internal 1.1v bandgap reference when measuring temperature

    /**
     * Reminder: taking care of pull-ups is the masters job
     */

    TinyWireS.begin(I2C_SLAVE_ADDRESS);
    TinyWireS.onReceive(receiveEvent);
    TinyWireS.onRequest(requestEvent);
}

void loop()
{
    /**
     * This is the only way we can detect stop condition (http://www.avrfreaks.net/index.php?name=PNphpBB2&file=viewtopic&p=984716&sid=82e9dc7299a8243b86cf7969dd41b5b5#984716)
     * it needs to be called in a very tight loop in order not to miss any (REMINDER: Do *not* use delay() anywhere, use tws_delay() instead).
     * It will call the function registered via TinyWireS.onReceive(); if there is data in the buffer on stop.
     */
    TinyWireS_stop_check();

    /**
     * Check if PWM value in register has change -> update PWM output 
     */
    if(analogValue != i2c_regs[3]){
      analogWrite(PWM_PIN, i2c_regs[3]);
      analogValue = i2c_regs[3];
    }

    /**
     *  Check if new measurement was triggered
     */
    if(i2c_regs[2] != 0x33){
      // change in byte 2 of i2c register -> trigger temperature read
      int temp = analogRead(ADC_TEMPERATURE);
      cli();
      i2c_regs[0] = lowByte(temp);
      i2c_regs[1] = highByte(temp);
      i2c_regs[2] = 0x33;
      sei();
    }
}


