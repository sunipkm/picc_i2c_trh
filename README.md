# PICTURE-C Temperature Controller on I2C Bus

Controller driver for [HDC1010 Temperature and Humidity Sensor from Texas Instruments](http://www.ti.com/product/HDC1010) written in C.
The system is on an I2C bus.

#

# How to get

On command line execute `git clone https://github.com/sunipkmukherjee/picc_i2c_trh.git`

# How to build

After cloning the repository, `cd` into the directory and execute `make`.
This will built the shared library `build/libpicc_i2c_thsensor.so` and the example binary `example/example`. To execute the example binary you can execute `make install` (with `sudo` if you do not have write permission in `/usr/local/lib`) to install the library in `/usr/local/lib` and then fire up the example binary. This will work if you have your `LD_LIBRARY_PATH` variable set to include `/usr/local/lib`. Otherwise, you can execute 
```
export LD_LIBRARY_PATH=$PWD/build:$LD_LIBRARY_PATH
``` 
on a BASH shell or 
```
setenv LD_LIBRARY_PATH $PWD/build:$LD_LIBRARY_PATH
```
on a Corn shell to append the current build path to the shared library file search path, and then execute the example binary.

You can execute `make clean` to delete the built objects, and `make spotless` to restore the directory to the state when it was cloned.

`make uninstall` can be executed to delete the installed shared library file from `/usr/local/lib` if it was installed.

The debug signals can be turned on by specifying the `HDC1010_DEBUG` option during make in `CFLAGS` using 
```
make CFLAGS=-DHDC1010_DEBUG
```

Further, the option `I2C_DEV_FILE` can be used to provide the I2C device file name to the library.

Note: Since the variable is being passed to the preprocessor through make, it should be done in the following format:
```
make CFLAGS="-DI2C_DEV_FILE=\\\"/path/to/i2c\\\""
```
This ensures that the string is passed properly to the preprocessor.


# Functionalities provided by the library:

The driver is written in C, and the device specific variables are stored inside a struct named `hdc1010_dev`. The following methods are available to access device functionalities:
  
1. `hdc1010_dev * hdc1010_begin(unsigned char)`:
   This method opens the slave device address (e.g. `0x41`) on the I2C bus specified by `I2C_DEV_FILE` macro during compilation. Returns the pointer to the device struct on success. This should be tracked in order to obtain reliable information from the device. Returns `NULL` if the device fails to respond to `ioctl` requests or if the I2C bus fails to open. After the device is opened, it is by default set to measure temperature and humidity independently. The input address should be 7-bit following I2C bus addressing scheme.  
   The macro `I2C_ADDR` can be provided to the compiler through `make` through `CFLAGS` to set the base address, which is predefined to be `0x40`. (Testing has been made with two devices `0x41` and `0x43`, other possible device addresses are `0x40` and `0x42`.)
   Memory for each device struct is allocated in this function, and must only be freed with the corresponding destructor function.
  
2. `hdc1010_acquisition_mode(hdc1010_dev * dev , bool)`:
   This method sets the acquisition mode of the device pointed to by `dev`. If this method is called with `false` as the input, then it sets the device for simultaneous measurement of temperature and relative humidity. If this method is called with `true` as the input, then it sets the device for independent measurement of temperature and relative humidity. The functions to retrieve the temperature and relative humidity from the device are agnostic to the acquisition method used.
   
3. `hdc1010_readMfID(hdc1010_dev * dev)`:
   This method returns the manufacturer ID of the device `dev` (`0x5449` for Texas Instrument).
   
4. `hdc1010_readDevID(hdc1010_dev * dev)`:
   This method returns the base address of the device `dev` on the I2C bus (`0x40`).
   
5. `hdc1010_readReg(hdc1010_dev * dev)`:
   This method reads the configuration register (register `0x02`) of the device `dev` and returns its contents as `hdc1010_regs` structure. The structure has the following accessible members:
      * `TemperatureMeasurementResolution` (1 bit):
         0 indicates 14 bit resolution, 1 indicates 11 bit resolution.
      * `HumidityMeasurementResolution` (2 bit):
         00 indicates 14 bit resolution, 01 indicates 11 bit resolution, 10 indicates 8 bit resolution.
      * `BatteryStatus` (1 bit):
         High value of this bit indicates that the system is not getting enough power to operate.
      * `ModeOfAcquisition` (1 bit):
         0 indicates stand-alone measurement for temperature and relative humidity. 1 indicates simultaneous measure of temperature and relative humidity.
      * `Heater` (1 bit):
         Turns on an included resistive element to heat up the chip. This is used to get rid of any condensation on the chip or for testing purposes.
      * `SoftwareReset` (1 bit):
         Resets the system to its initial state.

6. `hdc1010_writeReg(hdc1010_dev * dev , hdc1010_regs reg )`:
   This method writes an input of the `hdc1010_regs` datatype to the `CONFIGURATION` register of the device `dev`.
   
7. `hdc1010_heatUp(hdc1010_dev * dev , unsigned char seconds )`:
   This method takes in a number between 0 and 255, and turns on the resistive heating element of `dev` for that many seconds.
   This functionality is helpful for testing and to eliminate possible condensation on the sensor which will produce erroneous relative humidity measurements.
   
8. `hdc1010_readT( hdc1010_dev * dev )`:
   This method makes a measurement of temperature sensed by `dev` and returns the value in floating point format.
   
9. `hdc1010_readH(hdc1010_dev * dev)`:
   This method makes a measurement of relative humidity sensed by `dev` and returns the value in floating point format.
   
   Note: If the device is in simultaneous measurement mode, it does not make a measurement until both temperature and humidity measurements have been read. 

10. `hdc1010_free(hdc1010_dev * dev)`:
  This method closes the bus open for the device and clears all allocated memory.
   
   
There is also a function `hdc1010_sleep(unsigned long)` defined, which takes in the number of milliseconds to sleep. This is a wrapper around the LINUX system call `usleep`.

A full precision (14-bit) temperature or relative humidity measurement takes 6.35 ms according to the TI documentation (reference I2C clock 400 kHz). The system sleeps for 30 ms after each measurement is read before reading the output (as the data ready indicator is unused). This should give the system ample time to perform the measurement. The time-lapse between request and read can be altered by providing the value for the macro `PICC_TIME_USEC` (in microseconds) in `CFLAGS` during `make`.
