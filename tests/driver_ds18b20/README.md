Expected result
===============
This 1-wire driver test application reads and writes some test bytes cyclically
to the bus. If you connect a logic analyzer or a scope to the used pin, you
should observe the sequence
```
reset 0x01 0x0f 0x10 0xf0 0x11 0xaa
```
written to the bus once about every second.

Background
==========

