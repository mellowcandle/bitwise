# Bitwise
## Terminal based bitwise calculator in curses
[![Build Status](https://travis-ci.org/mellowcandle/bitwise.svg?branch=master)](https://travis-ci.org/mellowcandle/bitwise)[![codecov](https://codecov.io/gh/mellowcandle/bitwise/branch/master/graph/badge.svg)](https://codecov.io/gh/mellowcandle/bitwise)
[![Snap Status](https://build.snapcraft.io/badge/mellowcandle/bitwise.svg)](https://build.snapcraft.io/user/mellowcandle/bitwise)<a href="https://scan.coverity.com/projects/mellowcandle-bitwise">
  <img alt="Coverity Scan Build Status"
       src="https://img.shields.io/coverity/scan/18170.svg"/>
</a>[![GitSpo Mentions](https://gitspo.com/badges/mentions/mellowcandle/bitwise?style=flat-square)](https://gitspo.com/mentions/mellowcandle/bitwise)

_Bitwise_ is multi base interactive calculator supporting dynamic base convertion and bit manipulation.
It's a handy tool for low level hackers, kernel developers and device drivers developers.

Some of the features include:
* Interactive ncurses interface
* Command line calculator supporting all bitwise operations.
* Individual bit manipulator.
* Bitwise operations such as NOT, OR, AND, XOR, and shifts.

## IRC support
#bitwise @ irc.oftc.net

##

![Demo](https://github.com/mellowcandle/bitwise/raw/master/resources/bitwise.gif "Bitwise demo2")

## Usage
_bitwise_ can be used both Interactively and in command line mode.

### Command line calculator mode
In command line mode, bitwise will calculate the given expression and will output the result in all bases including binary representation.

_bitwise_ detects the base by the preface of the input (_0x/0X_ for hexadecimal, leading _0_ for octal, _b_ for binary, and the rest is decimal).

### Examples:

#### Simple base conversion

![conversion](https://github.com/mellowcandle/bitwise/raw/master/resources/cmdline.png "Bitwise conversion")


#### C style syntax Calculator

![calculator](https://github.com/mellowcandle/bitwise/raw/master/resources/conversion.png "Bitwise calculator")

### Interactive mode
_bitwise_ starts in interactive mode if no command line parameters are passed or if the _-i | --interactive_ flag is passed.
In this mode, you can input a number and manipulate it and see the other bases change dynamically.
It also allows changing individual bits in the binary.

#### Navigation in interactive mode
To move around use the arrow keys, or use _vi_ key bindings : <kbd> h </kbd> <kbd> j </kbd> <kbd> k </kbd> <kbd> l </kbd>.
Leave the program by pressing <kbd> q </kbd>.

##### Binary specific movement
You can toggle a bit bit using the <kbd> space </kbd> key.
You can jump a byte forward using <kbd> w </kbd> and backwards one byte using <kbd> b </kbd>. 

#### Bitwise operation in interactive mode

##### Setting the bit width:

Reducing or extending the bit width interactively is also very easy, just use:
<kbd> ! </kbd> for 8bit, <kbd> @ </kbd>  for 16Bit, <kbd> $ </kbd> for 32Bit and <kbd> * </kbd> for 64Bit.
When changing the bit width, the number is *masked* with the new width, so you might lost precision, use with care.

##### NOT:

Press <kbd> ~ </kbd> to perform the NOT operator.

#### expression calculator in interactive mode

You can enter expression calculator mode by typing <kbd> : </kbd> (Just like in vim).

To exit the mode, just press <kbd> ESC </kbd>.

In this mode, you can type any expression you like to be evaluated.
The result will be printed in the history window and also printed in the binary and various bases on top.

###### operators and functions
* All C operators are supported, additionaly, you can use the "$" symbol to refer to the last result.
* refer to a specific bit by using the function _BIT(x)_.

###### commands
* _clear_ - to clear the history window.
* _width_ [8 | 16 | 32 | 64] - set the required width mask
* _q_ - to exit

## Installation

### Linux
#### Ubuntu
```
sudo add-apt-repository ppa:ramon-fried/bitwise
sudo apt-get update
sudo apt-get install bitwise
```
#### Snap
If your distribution supports Snap just type:
`
sudo snap install bitwise
`
#### Arch
You can use the AUR repository: https://aur.archlinux.org/packages/bitwise/

### Windows
NCurses doesn't support windows. You can use the windows subsystem for linux as a workaround.

### Building from source

#### Prerequisites
* libreadline
* libncurses (with forms)
* libcunit (only needed for testing)

On Ubuntu/Debian system you can just paste:
```
sudo apt-get install build-essential
sudo apt-get install libncurses5-dev
sudo apt-get install libreadline-dev
sudo apt-get install libcunit1-dev
```

- Download [the latest release](https://github.com/mellowcandle/bitwise/releases/latest)

```sh
tar xfz RELEASE-FILE.TAR.GZ
cd RELEASE-DIR
./configure
make
sudo make install
```

Running unit tests by typing
``` make check ```

### Contribution
* Fork the repo
* Run ```./bootstrap.sh```
* Follow the building from source section.
* commit and send pull request
