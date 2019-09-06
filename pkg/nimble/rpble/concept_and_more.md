rpl ---> rpble

inst_id
dodag
version
role
rank


Events:
on initialization:
- scanning:   0  -> 1
- connecting: 0  -> 0
- accepting:  0  -> 0

parent selected:
- scanning:   1  -> 0
- connecting: 1  -> 0
- accepting:  0  -> 1*

## parent lost
- scanning:   0  -> 1
- connecting: 0  -> 0
- accepting:  1* -> 0

## slave lost
- scanning:   0  -> 0
- connecting: 0  -> 0
- accepting:  1* -> 1*

parent connect abort
- scanning:   0 ->
- connecting: 1 -> 0
- accepting:  0 -> 1*

slave connected
- scanning:   1 -> 0
- connecting: 1 -> 0
- accepting:  0 -> 1*



eval timeout
- scanning:   1 -> 0
- connecting: 1 -> 0
- accepting:  0 -> 1*
