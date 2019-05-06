# bitwise (v0.13)
Terminal based bitwise calculator in curses

[![Build Status](https://travis-ci.org/mellowcandle/bitwise.svg?branch=master)](https://travis-ci.org/mellowcandle/bitwise)[![codecov](https://codecov.io/gh/mellowcandle/bitwise/branch/master/graph/badge.svg)](https://codecov.io/gh/mellowcandle/bitwise)
[![Snap Status](https://build.snapcraft.io/badge/mellowcandle/bitwise.svg)](https://build.snapcraft.io/user/mellowcandle/bitwise)<a href="https://scan.coverity.com/projects/mellowcandle-bitwise">
  <img alt="Coverity Scan Build Status"
       src="https://img.shields.io/coverity/scan/18170.svg"/>
</a>

## 

![alt text](https://github.com/mellowcandle/bitwise/raw/master/resources/bitwise.gif "Bitwise demo")


## Usage
_bitwise_ can be used both Interactively and in command line mode.

### Command line mode
In command line mode, passing a number in any base (Decimal, Hexadecimal & Octal) will output the number in all bases including binary representation.

_bitwise_ detects the base by the preface of the input (0x/0X for hexadecimal, leading 0 for octal, and the rest is decimal).

#### Example:

```
$ ./bitwise 0x12312311
Decimal: 305210129
Hexdecimal: 0x12312311
Octal: 02214221421
Size: 291.07MB
Binary:
0 0 0 1 0 0 1 0 | 0 0 1 1 0 0 0 1 | 0 0 1 0 0 0 1 1 | 0 0 0 1 0 0 0 1 
    31 - 24           23 - 16           15 -  8            7 -  0     
```

### Interactive mode
_bitwise_ starts in interactive mode if no command line parameters are passed or if the _-i | --interactive_ flag is passed.
In this mode, you can input a number and manipulate it and see the other bases change dynamically.
It also allows changing individual bits in the binary.

#### Navigation in interactive mode
To move around use the arrow keys, or use _vi_ key bindings : <kbd> h </kbd> <kbd> j </kbd> <kbd> k </kbd> <kbd> l </kbd>.

You can toggle a bit bit using the <kbd> space </kbd> key.

Reducing or extending the bit width interactively is also very easy, just use:

<kbd> ! </kbd> for 8bit, <kbd> @ </kbd>  for 16Bit, <kbd> $ </kbd> for 32Bit and <kbd> * </kbd> for 64Bit.

When changing the bit width, the number is *masked* with the new width, so you might lost precision, use with care.

Leave the program by pressing <kbd> q </kbd>.

## Installation

## Snap
If your distribution supports Snap just type:
`
sudo snap install bitwise
`

## Arch
Your can use the AUR repository: https://aur.archlinux.org/packages/bitwise/

## Building from source
*Make sure you have ncurses package installed*
1. Download latest release.
2. unpack release
3. ./configure
4. make
5. sudo make install


