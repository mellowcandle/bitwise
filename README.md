# bitwise
Terminal based bitwise calculator in curses

[![Build Status](https://travis-ci.org/mellowcandle/bitwise.svg?branch=master)](https://travis-ci.org/mellowcandle/bitwise)<a href="https://scan.coverity.com/projects/mellowcandle-bitwise">
  <img alt="Coverity Scan Build Status"
       src="https://scan.coverity.com/projects/18170/badge.svg"/>
</a>

## 

## Usage
bitwise can be used both interactivly and in command line mode.
In command line mode, passing a number in any base (Decimal, Hexedical & Octal) will output the number in all bases including binary representation.
bitwise detects the base by the preface of the input (0x/0X for hexedecimal, leading 0 for octal, and the rest is decimal).

### Example:

$ bitwise 0x542
Decimal: 1346   Hexdecimal: 0x542       Octal:02502
0 0 0 0 0 0 0 0 | 0 0 0 0 0 0 0 0 | 0 0 0 0 0 0 0 0 | 0 0 0 0 0 0 0 0 | 0 0 0 0 0 0 0 0 | 0 0 0 0 0 0 0 0 | 0 0 0 0 0 1 0 1 | 0 1 0 0 0 0 1 0
$
