# iio-temp-mod
IIO Temperature Module streaming example 


 ```bash
	pi@raspberrypi:~ $ iio_temp_mod -h
	Usage: iio_temp_mod [-v] [-n] <count> -u <uri> <config-file>
	Options:
		-u Use the context at the provided URI.
		-n Capture n samples at the time
		-v Verbose information
	Example:
		iio_temp_mod -v -u serial:/dev/ttyAMA0,38400n8 defaultConfig.json
  ```

* Simple Build Instructions
 + Depends on: libiio, libjson0-dev
 ```bash
 	git clone https://github.com/analogdevicesinc/iio-temp-mod.git
	cd iio-temp-mod
	make
	sudo make install
 ```

* Automated Build and Setup Script for Raspberry PI / Raspbian
  - Download NOOBS (NOOBS_v2_0_0.zip) and install Raspbian GNU/Linux 8 (jessie)
  - Open Terminal CTRL-ALT-T
  - Type following commands:

```bash
	pi@raspberrypi:~ $ wget https://github.com/analogdevicesinc/iio-temp-mod/blob/master/rpi_setup.sh
	pi@raspberrypi:~ $ chomd +x rpi_setup.sh
	pi@raspberrypi:~ $ ./rpi_setup.sh
 ```

 * Example: iio_temp_mod

```bash
	pi@raspberrypi:~ $ iio_temp_mod -v -u serial:/dev/ttyAMA0,38400n8 iio-temp-mod/defaultConfig.json
	TEMP_MODULE-1 Version: 1.2
	DEV  [TemperatureUnit] = Celsius
	DEV  [OperationalMode] = ContinuousConversion
	DEV  [PowerMode] = Low
	DEV  [FilterType] = FIR
	DEV  [FirFrequency] = 25SPS
	CH thermocouple [SensorType] = T-Type
	CH thermocouple [Gain] = 64
	CH thermocouple [VBiasEnable] = 0
	CH thermocouple [TemperatureMin] = -200
	CH thermocouple [TemperatureMax] = 400
	CH cold_junction [Sensor] = RTD3Wire
	CH cold_junction [SensorType] = PT100
	CH cold_junction [Gain] = 32
	CH cold_junction [ExcitationCurrent] = 500
	CH cold_junction [ReferenceResistor] = 5110

	CJC-Temp, TC-Temp
	-242.020004, -243.970459
	-242.020004, -243.976624
	-242.020004, -243.919678
	-242.020004, -243.948090
	-242.020004, -243.955719
	-242.020004, -243.911011
	-242.020004, -243.904907
	-242.020004, -243.909973
	-242.020004, -243.966888
	-242.020004, -243.933289
	-242.020004, -243.944275
 ```

 * Example: iio_info
 
 ```bash
	pi@raspberrypi:~ $ iio_info -u serial:/dev/ttyAMA0,38400n8
	Library version: 0.8 (git tag: bb70e40)
	Compiled with backends: local xml network serial
	IIO context created with serial backend.
	Backend version: 0.1 (git tag: v0.1   )
	Backend description string: /dev/ttyAMA0: ttyAMA0
	IIO context has 1 devices:
		iio:device0: TEMP_MODULE (buffer capable)
			2 channels found:
				temp0: cold_junction (input, index: 0, format: le:S32/32>>0)
				7 channel-specific attributes found:
					attr 0: Sensor value: RTD3Wire
					attr 1: SensorType value: pt100
					attr 2: Gain value: 32
					attr 3: ExcitationCurrent value: 500
					attr 4: ReferenceResistor value: 5110
					attr 5: TemperatureMax value: 125
					attr 6: TemperatureMin value: -40
				temp1: thermocouple (input, index: 1, format: le:S32/32>>1)
				6 channel-specific attributes found:
					attr 0: Sensor value: Thermocouple
					attr 1: SensorType value: T
					attr 2: Gain value: 64
					attr 3: VBiasEnable value: 0
					attr 4: TemperatureMin value: -200
					attr 5: TemperatureMax value: 400
			6 device-specific attributes found:
					attr 0: PowerMode value: Low
					attr 1: OperationalMode value: Continuous
					attr 2: FilterType value: FIR
					attr 3: FirFrequency value: 25SPS
					attr 4: FS value: 384
					attr 5: TemperatureUnit value: Celsius
			1 debug attributes found:
					debug attr 0: direct_reg_access value: 16
 ```
