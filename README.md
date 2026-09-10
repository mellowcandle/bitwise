<div align="center">

<h1>bitwise</h1>

<p><strong>Multi-base interactive calculator and bit manipulator for the terminal.</strong></p>

<p>Convert between bases as you type, flip individual bits by hand, and evaluate C-style
expressions — without leaving the console.</p>

<p>
<a href="https://github.com/mellowcandle/bitwise/actions/workflows/c-cpp.yml"><img alt="CI" src="https://github.com/mellowcandle/bitwise/actions/workflows/c-cpp.yml/badge.svg"></a>
<a href="https://scan.coverity.com/projects/mellowcandle-bitwise"><img alt="Coverity Scan" src="https://img.shields.io/coverity/scan/18170.svg"></a>
<a href="https://github.com/mellowcandle/bitwise/releases/latest"><img alt="Latest release" src="https://img.shields.io/github/v/release/mellowcandle/bitwise"></a>
<a href="LICENSE"><img alt="License: GPL-3.0-or-later" src="https://img.shields.io/badge/license-GPL--3.0--or--later-blue.svg"></a>
</p>

<img src="https://github.com/mellowcandle/bitwise/raw/master/resources/bitwise.gif" alt="Bitwise demo" width="800">

</div>

---

_bitwise_ is a handy tool for low level hackers, kernel developers and device driver developers:
anyone who spends their day staring at a register dump wondering which bit is set.

* **Every base at once** — decimal (signed and unsigned), hexadecimal, octal and binary, side by side.
* **Interactive ncurses interface** — edit a number in any field and watch the rest follow.
* **Individual bit manipulation** — walk the binary row and toggle bits with the cursor.
* **C-style expression calculator** — `&`, `|`, `^`, `~`, `<<`, `>>`, `*`, `/`, `%`, `+`, `-`, with C's precedence.
* **Selectable bit width** — 8, 16, 32 or 64 bit, switchable on the fly.
* **Extras that save a lookup** — human-readable sizes (IEC or SI), IPv4 in both byte orders, ASCII and Radix64.

## Quick tour

Pass a number and get every representation at once — signed and unsigned decimal, hexadecimal,
octal, a human-readable size, Radix64, both IPv4 byte orders, and a labelled bit grid:

![Simple base conversion](https://github.com/mellowcandle/bitwise/raw/master/resources/cmdline.png "Bitwise conversion")

Or pass a whole C-style expression and let bitwise evaluate it first:

![C style syntax calculator](https://github.com/mellowcandle/bitwise/raw/master/resources/conversion.png "Bitwise calculator")

Run it with no arguments for the full-screen interactive mode.

## Installation

<table>
<tr>
<td valign="top" width="52%">

| Platform | Install |
| --- | --- |
| Debian / Ubuntu 20.04+ | `sudo apt-get install bitwise` |
| Fedora | `sudo dnf install bitwise` |
| openSUSE | `zypper install bitwise` |
| Void | `sudo xbps-install -S bitwise` |
| Arch | [AUR package](https://aur.archlinux.org/packages/bitwise/) |
| Snap | `sudo snap install bitwise` |
| Nix | `nix-env -i bitwise` |
| macOS (Homebrew) | `brew install bitwise` |
| macOS (MacPorts) | `sudo port install bitwise` |
| Buildroot / Yocto | In-tree; see their docs |
| Windows | [From source](#building-from-source), under mingw or msys2 |

<details>
<summary>Ubuntu older than 20.04</summary>

```sh
sudo add-apt-repository ppa:ramon-fried/bitwise
sudo apt-get update
sudo apt-get install bitwise
```

</details>

</td>
<td valign="top" width="48%">

<a href="https://repology.org/project/bitwise/versions">
    <img src="https://repology.org/badge/vertical-allrepos/bitwise.svg?exclude_unsupported=1&amp;columns=2" alt="Packaging status" width="100%">
</a>

</td>
</tr>
</table>

## Command line mode

In command line mode, bitwise calculates the given expression and prints the result in all bases,
including the binary representation.

The base is detected from the prefix of the input: _0x/0X_ for hexadecimal, a leading _0_ for octal,
_b_ for binary, and everything else is decimal. IPv4 addresses are parsed too, and are reported in
both network and reversed byte order.

| Option | Meaning |
| --- | --- |
| `-i`, `--interactive` | Load interactive mode (the default when there is no input) |
| `-w`, `--width [b\|w\|l\|d]` | Set bit width: `b`yte (8), `w`ord (16), `l`ong (32) or `d`ouble (64). Default: `l` |
| `-s`, `--si` | Print sizes using the SI standard (default: IEC) |
| `--no-color` | Start without color support |
| `-v`, `--version` | Output version information and exit |
| `-h`, `--help` | Display help and exit |

An expression starting with a minus sign would be taken for a command line option, so separate it
with `--`:

```sh
bitwise -- '-5'
```

## Interactive mode

_bitwise_ starts in interactive mode if no command line parameters are passed, or if the
`-i | --interactive` flag is given. Input a number, manipulate it, and watch the other bases change
as you go — including individual bits in the binary row. Press <kbd>F1</kbd> for the help screen.

### Navigation

| Key | Action |
| --- | --- |
| <kbd>←</kbd> <kbd>↓</kbd> <kbd>↑</kbd> <kbd>→</kbd> or <kbd>h</kbd> <kbd>j</kbd> <kbd>k</kbd> <kbd>l</kbd> | Move around |
| <kbd>space</kbd> | Toggle the bit under the cursor |
| <kbd>w</kbd> / <kbd>b</kbd> | Jump one byte forward / backward |
| <kbd>F1</kbd> | Show the help screen |
| <kbd>q</kbd> | Quit |

### Operations

| Key | Action |
| --- | --- |
| <kbd>~</kbd> | NOT |
| <kbd>r</kbd> | Reverse endianness |
| <kbd>&lt;</kbd> / <kbd>&gt;</kbd> | Shift left / right |
| <kbd>!</kbd> <kbd>@</kbd> <kbd>$</kbd> <kbd>*</kbd> | Set the width to 8, 16, 32 or 64 bit |

> [!WARNING]
> When changing the bit width, the number is *masked* with the new width, so you might lose
> precision. Use with care.

### Expression calculator

Enter the expression calculator by typing <kbd>:</kbd> (just like in vim), and leave it with
<kbd>ESC</kbd>. Anything you type there is evaluated; the result is printed in the history window
and also shown in binary and the various bases at the top.

#### Operators

The expression syntax is a subset of C, plus Verilog's bit select (below). These operators are
supported, in order of precedence, tightest binding first:

| Precedence | Operators | Notes |
| --- | --- | --- |
| 1 | `!` `~` `-` `+` | unary, right associative |
| 2 | `*` `/` `%` | |
| 3 | `+` `-` | |
| 4 | `<<` `>>` | |
| 5 | `&` | |
| 6 | `^` | |
| 7 | `\|` | |

Parentheses group as usual, and multiplication can be written implicitly: `2(3)` and `(2)(3)` both
give 6.

This is the same ordering a C compiler uses, so an expression lifted out of source evaluates to the
same thing here.

One difference from C: `&=`, `^=` and `|=` are accepted, but there is nothing to assign to, so they
evaluate exactly like `&`, `^` and `|`.

Not supported: comparison and equality (`<` `>` `<=` `>=` `==` `!=`), logical `&&` and `||`, the
ternary `?:`, and exponentiation.

#### Verilog bit select

Fields can be pulled out with Verilog's notation, the same way a datasheet writes them:

| Expression | Meaning |
| --- | --- |
| `v[msb:lsb]` | The bits from `msb` down to `lsb` |
| `v[bit]` | A single bit |

```
bitwise '0x875423[31:23]'
bitwise '0x875423[15]'
```

The field comes back **right-aligned**, as it does in Verilog and as a datasheet means it — not
masked in place. So `0xdeadbeef[31:16]` is `0xdead`, not `0xdead0000`, and reading a field no longer
means working out its mask by hand:

```
0xdeadbeef[31:16]        instead of      (0xdeadbeef >> 16) & 0xffff
```

The indices are ordinary expressions, so `v[15 + 16:16]` works, and a slice is itself a value, so it
can be sliced or combined further. Both indices must fit the 64-bit value and `msb` must not be
below `lsb`; anything else is reported rather than quietly returning zero.

This is the one place the syntax is not C's — in C, `v[3]` would be array indexing. It is deliberate,
and it is the notation the registers being decoded are documented in.

#### Bits and the last result

* Refer to the last result with the `$` symbol.
* Refer to a specific bit with the function `BIT(x)`.
* Set, clear, toggle or test a bit using `$ |= BIT(n)`, `$ &= ~BIT(n)`, `$ ^= BIT(n)` and `$ & BIT(n)`.

#### Commands

| Command | Action |
| --- | --- |
| `h(elp)` | Show the help screen |
| `c(lear)` | Clear the history window |
| `w(idth) [8 \| 16 \| 32 \| 64]` | Set the required width mask |
| `o(utput) [dec(imal) \| hex(adecimal) \| oct(al) \| bin(ary) \| all]` | Set the default output for results |
| `q(uit)` | Exit |

## Editor integration

### Vim and Neovim

[vim-bitwise](https://github.com/mellowcandle/vim-bitwise "vim-bitwise") runs bitwise on the number
under your cursor, without leaving the editor.

![vim-bitwise](https://github.com/mellowcandle/vim-bitwise/raw/master/assets/demo.gif "vim-bitwise demo")

Rest the cursor on a numeric literal and its representations appear beside it. There is also an
operator, so `<Leader>biw` runs bitwise on the word under the cursor and `<Leader>bi(` on the
expression inside the parentheses, plus a `:Bitwise` command for anything you want to type out.
Hexadecimal, binary, octal and decimal literals are all recognised, including digit separators and
size suffixes such as `0xFF_u8`.

Install it with your plugin manager:

```lua
-- lazy.nvim. Not lazy-loaded on a key: the hover is driven by an autocmd.
{ "mellowcandle/vim-bitwise", lazy = false }
```

```vim
" vim-plug
Plug 'mellowcandle/vim-bitwise'
```

It needs this `bitwise` binary on your `$PATH`, which you have already installed if you are reading
this. See the plugin's README for configuration and `:checkhealth bitwise` if anything looks wrong.

## Building from source

### Prerequisites

* libreadline
* libncurses (with forms)
* libcunit (only needed for testing)

<details>
<summary>Ubuntu / Debian</summary>

```sh
sudo apt-get install build-essential
sudo apt-get install libncurses5-dev
sudo apt-get install libreadline-dev
sudo apt-get install libcunit1-dev
```

</details>

<details>
<summary>macOS</summary>

```sh
brew install automake
brew install autoconf
brew install readline
brew install cunit
export LDFLAGS="-L$(brew --prefix readline)/lib -L$(brew --prefix cunit)/lib"
export CPPFLAGS="-I$(brew --prefix readline)/include -I$(brew --prefix cunit)/include"
```

</details>

### Build

Download [the latest release](https://github.com/mellowcandle/bitwise/releases/latest), then:

```sh
tar xfz RELEASE-FILE.TAR.GZ
cd RELEASE-DIR
./configure
make
sudo make install
```

Run the unit tests with:

```sh
make check
```

## Contributing

* Install the prerequisites above.
* Fork the repo.
* Run `./bootstrap.sh`.
* Follow the [building from source](#building-from-source) section.
* Commit and send a pull request.

See [CONTRIBUTING.md](CONTRIBUTING.md) for the style, testing and commit conventions patches are
expected to follow.

## License

GPL-3.0-or-later. See [LICENSE](LICENSE).
