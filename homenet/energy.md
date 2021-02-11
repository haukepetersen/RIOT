# Energy consumption

Test node: DD:9F:...

## Gasman
initial with DEBUG=1 (stdio_rtt, periph_timer, ....)
avg 570µA

First try with DEBUG=0:
avg 305µA


RUN#1: in place, 3.3V via breakout VDD pins
avg: 720µA


OLD: b2:32:20:88:a8:52
NEW: a5:c9:39:e0:3b:b6



## Dongle test
For all: Suppply Voltage 3.305V in power profiler

- skald with 3 byte (+ 3 flags + 4 AD header) static data
avg: 26.3µA

- skald: same data, + ADC sampling battery (VDDH) once a second
avg: 26.9µA  | 27.7µA

- skald: same data, + ADC sampling battery (VDDH) once a second
 + RAM OFF for RAM sections 1-8, only section 0 enabled
avg: 25,7µA



## Testing

addr of xenon: dd:9f:41:8c:67:a2
