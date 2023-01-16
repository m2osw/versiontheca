
# Format

Implement a "format" so the next & previous functions know of all the
possible fields in that version.

For example:

    9999.9999zzz

means:

    +------------+------------+--------+--------+--------+--------+
    |  original  | equivalent |      next       |     previous    |
    |            |            |   2    |   3    +   2    |   3    |
    +------------+------------+--------+--------+--------+--------+
    | 1.3c       | 1.3caa     | 1.4    | 1.3cab | 1.3cab | 1.3bzz |
    | 1.4zzz     | 1.4zzz     | 1.5    | 1.5    | 1.3    | 1.4zzy |
    +------------+------------+--------+--------+--------+--------+

Right now we just have a limit which does not really cut it because it limits
everything to the same thing which is unlikely to work in all circumstances.

