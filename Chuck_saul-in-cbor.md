


typedef int(*chuck_read_t)(const void *dev, void *buf, size_t len);

typedef int(*chuck_write_t)(const void *dev, void *buf, size_t len);

[array]     -> possibly more than a single val
  u16       -> physical unit, a standard!
  tag 5     -> fractional number
  [array]


phydat:
-> 8 byte

Temp:
{
    [
        0x123,
        -2,
        [ 2340 ]
    ]
}

0x83                -> array with 3 elements
0x19 0x00 0x01      -> unit
0x22                -> scaling -2
0x81                -> array with 1 element
0x19 0xYY 0xYY      -> 2340 as positive number
-> 9 byte vs 8byte

Acc:
[
    0x1591,     -> physical unit
    0,          -> no scaling
    [ 1023, 9, -3 ]
]

0x83
0x19 0xZZ 0xZZ      -> unit
0x00
0x83
0x19 0xAA 0xAA
0x19 0xBB 0xBB
0x19 0xCC 0xCC
~ 13 byte VS 8byte

BME (tmp, hum, pressure):
[
    0x1323,
    -2,
    [ 123],
    0x345,
    3,
    [ 1233 ],
    0x8839,
    -3,
    [ 2983 ],
]

0x89
0x19 0x00 0x01      -> unit as u16
0x22                -> scaling -2
0x81                -> array with 1 element
0x19 0xFF 0xFF      -> 123 as positive number
0x19 0x00 0x01      -> 0x0001 as u16
0x22                -> scaling -2
0x81                -> array with 1 element
0x19 0xFF 0xFF      -> 123 as positive number
0x19 0x00 0x01      -> 0x0001 as u16
0x22                -> scaling -2
0x81                -> array with 1 element
0x19 0xFF 0xFF      -> 123 as positive number
-> 25 byte
-> phydat: 24 byte



problems:
- positive vs negative numbers, have different types and therefor need handling
- endianess: multi-byte numbers are always big-endian

pros:
- not tied to 16-bit ints anymore
- read multiple endpoints in a single read operation

cons:
- big-endian encoding overkill for internal data handling
