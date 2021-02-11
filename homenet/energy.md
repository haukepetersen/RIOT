# Energy consumption

Test node: DD:9F:...

## Gasman
initial with DEBUG=1 (stdio_rtt, periph_timer, ....)
avg 570µA

First try with DEBUG=0:
avg 305µA




## Dongle test
For all: Suppply Voltage 3.305V in power profiler

- skald with 3 byte (+ 3 flags + 4 AD header) static data
avg: 26.3µA

- skald: same data, + ADC sampling battery (VDDH) once a second
avg: 26.9µA  | 27.7µA

- skald: same data, + ADC sampling battery (VDDH) once a second
 + RAM OFF for RAM sections 1-8, only section 0 enabled
avg: 25,7µA
