examples/sensebox_assembly
================
This application collects data from various sensors and posts the data to The
Things Network. The data can be viewed in OpenSenseMap if the integration
between the two is set up.

Overview
========

This application runs on a plant monitoring hardware setup constructed in FU
Informatik department. The hardware consists of:

- A sensebox
- The following sensors:
  - A Sensebox I2C temperature/humidity sensor, based on the HDC1000
  - A Sensebox I2C light sensor, based on the TSL45315
  - A Sensebox SPI LoRa "bee"
  - Three soil moisture sensors, of the common, low-cost, two-pronged resistive
    kind
  - One DS18 soil temperature sensor
- The following watering apparatus:
  - Two water valves, with a 12V opening voltage
  - Water tank and tubes (6mm internal diameter) to supply water to the plants

The circuitry for each of these, along with description of the drivers, is
described in more detail below.

### Sensebox I2C sensors

The temperature/humidity and light sensors are simply plugged into the board
using the same wiring as in the wires supplied with the sensors and sensebox.
The wires have only been extended.

The HDC1000 sensor uses RIOT's HDC1000 driver, and the TSL45315 sensor uses
RIOT's tsl4531x driver.

### Sensebox LoRa "bee"

This plugs directly into the header for XBEE1 on the sensebox board.

It uses RIOT's semtech loramac package/driver. The data is serialized into the
format required by OpenSenseMap using RIOT's lora_serialization module.

### Soil moisture sensors (Referred to in the code as "hygrometers")

These act as voltage dividers, supplying a voltage out between their Vcc and
ground according to the amount of moisture in the soil.

There is no driver for these. The output voltage is fed into the SenseBox
digital pins, which are configured as ADCs. The ADCs can only read between 0
and 3.3V, so Vcc has to be 3.3V. Therefore, power has to be taken from the SWD
header.

To minimize corrosion, these are only powered when a reading is being taken.
The power is switched on the ground side, using a transistor and a UART/serial
pin reconfigured as a GPIO pin to switch on and off. The moisture sensors take
about 5s for their readings to stablise after turn-on; 10s is given so there is
some margin. In the FU Informatik setup, the switch also cuts the ground to the
water valves. In the FU Biology department setup, the switch also cuts the
ground to the soil temperature sensor (but not the water valves). This physical
design was convenient from a wiring perspective. From a software perspective,
it means you need to switch on the ground when operating the valves or soil
temperature sensors as well. So, some added complexity in the software as a
tradeoff.

### Soil temperature sensor

This communicates via the 1-wire protocol. The one comms wire plugs into the
UART2 Rx pin, which is actually configured as a GPIO.

It uses RIOT's ds18 driver.

### Water valves

The water valves are supplied with 12V from a wall socket. These are switched on
the ground side using transistors and SenseBox digital pins as control pins.

## Application logic overview
 
Periodically, the software measures the soil moisture, and if the moisture is
too low, as indicated by a moisture sensor reading which is too high, the
valves open for a short period of time. At a different rate, the collected data
is sent to the TTN servers. The rate of data collection/watering, sending to
the TTN servers, and the watering levels are all configurable.

In addition, there are safety limits which trip when moisture sensor readings
go above and below certain levels. These are also configurable. Details of the
hardware are also configurable in the software, for example which pins are used
for certain sensors, and for power switching.

There is a "hardware test" function, which can be switched off by
commenting out a define, which toggles and reads all the sensors and actuators
in such a way that allows you to check whether they are working properly. If
on, it's run on startup.

## Things network/OpenSenseMap setup

### The Things Network

Go to [the website](https://www.thethingsnetwork.org/) and create an account.
On the [console section](https://console.thethingsnetwork.org/) you will be
able to create a new application.Once the application has been created register
a new device.

 Finally on the _Integrations_ section of your application add a new
'HTTP Integration'. **Make sure that URL points to
https://ttn.opensensemap.org/v1.1 and that the Method is POST**.

 ### openSenseMap
First, you need to register a new box. For that go to the
[openSenseMap website](https://opensensemap.org/), register and create a new
senseBox.

 You will need to choose the station name, the exposure, location and type of
hardware (you can choose either of them). Under _Manual configuration_ you can
add sensors to your box. They need to be added in the same order as the data is
written into the serialization data structure. For this application: 

Phenomonon: Air temperature
Unit: °C
Type: temperature

Phenomenon: Air humidity
Unit: %
Type: humidity

Phenomenon: Visible light intensity
Unit: lux
Type: uint8

Phenomenon: Soil temperature
Unit: °C
Type: temperature

Phenomenon: Soil moisture - plant 1
Unit: ADC val
Type: uint16

Phenomenon: Soil moisture - plant 2
Unit: ADC val
Type: uint16

Phenomenon: Soil moisture - plant 3
Unit: ADC val
Type: uint16

Once the box is created go to the dashboard and click the 'Edit' button. On the
'TheThingsNetwork' section choose:

- **Decoding Profile**: LoRa serialization
- **TTN application ID**: (found on the TTN 'Application Overview' section)
- **TTN device ID**: (found on the TTN 'Device Overview' section as Device ID)
- **Port**: (LoRa port where the data is being sent to The Things Network. Found
in Application -> [your application] -> Devices -> [your device] -> Data -> 
Port column)
- **Decoder Options**:

```json
[
    {
        "sensor_unit":"°C",
        "decoder":"temperature",
        "sensor_title":"Air temperature"
    },
    {
        "sensor_unit":"%",
        "decoder":"humidity",
        "sensor_title":"Air humidity"
    },
    {
        "sensor_unit":"lux",
        "decoder":"uint16",
        "sensor_title":"Visible light intensity"
    },
    {
        "sensor_unit":"°C",
        "decoder":"temperature",
        "sensor_title":"Soil temperature"
    },
    {
        "sensor_unit":"ADC val",
        "decoder":"uint16",
        "sensor_title":"Soil moisture - plant 1"
    },
    {
        "sensor_unit":"ADC val",
        "decoder":"uint16",
        "sensor_title":"Soil moisture - middle plant"
    },
    {
        "sensor_unit":"ADC val",
        "decoder":"uint16",
        "sensor_title":"Soil moisture - Gaetan's plant"
    }
]
```

**Make sure you save your changes before leaving**

### LoRaWAN keys

To join the LoRaWAN network using OTAA, edit the application `Makefile` with the
information found on the 'Device Overview' section at The Things Network
website.

    DEVEUI ?= FFFFFFFFFFFFFFFF
    APPEUI ?= FFFFFFFFFFFFFFFF
    APPKEY ?= FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF

Notes and further work
======================

### Soil moisture sensors

After investigation, it was found that the soil moisture sensors were
inadequate to give readings other than rough qualitiative ones. The readings
are highly dependent on insertion depth and contact area with the actual soil.
The latter changes as the soil dries out, leading to highly nonlinear
behaviour. The readings also change as they corrode, which they do quickly.
Capacitive soil moisture sensors exist for a similar price but longer lead
times. They are generally regarded to be better and do not corrode.

There are several such sensors in the FU Informatik department, please see Juan
Carrano if you wish to obtain them.

### State of software/various branches

The head of this branch compiles but needs some more testing before it can be
run overnight. For example, checking that the safety levels trip as expected
and that the valves water as expected. The version that is tested and verified
for long term operation is tagged "working_version".

There is also a branch in this repo called
examples/sensebox_assembly_with_registry which has the start of an
implementation which uses the RIOT registry to set configuration variables.
Currently variables can be set and read but the correct operation of those
variables is not tested.
