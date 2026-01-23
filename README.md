# tmsolve

## Description

Powerful and lightweight command line calculator.

## Usage

### Command Line Arguments

Run `tmsolve --help` (or `tmsolve.exe --help` for Windows if not in `PATH`) to see supported command line arguments.

### Modes

The calculator has the following modes:

- Scientific (S) (default)
- Integer (I)
- Function (F)
- Equation (E)
- Utility (U)

To switch between modes, add the correct letter after the command `mode`.

Example: `mode I` will switch to integer mode.

**Note:** All modes support emulating multi-line input using a semicolon `;`. For example, `a=5;b=10;a+b` would set `a` to 5, `b` to 10 then do the sum and display the result. In any mode, use `multiline show` to show intermediary results and `multiline hide` to show only the final result.

### Scientific Mode

#### Supported Operators

- Operator associativity: Left to right.
- Priority: see next, sorted by highest priority with operators on the same line having the same priority.

```
^ **       Power operators.
* / // %   Multiplication, division, integer division, modulo.
+ -        Addition and subtraction.
```

Assignment operations `+=` `-=` `*=` `/=` `%=` `//=` `**=` are also supported.

#### Supported Functions

Provided by `libtmsolve`, check them [here](https://a-h-ismail.gitlab.io/libtmsolve-docs/md_FUNCTIONS.html).

#### Features

- Supports user defined functions and variables.
- Supports hexadecimal, octal and binary represenation using prefixes `0x`,`0o`,`0b` or functions `hex()`, `oct()`, `bin()`.
- "ans" variable stores previous results.
- Supports complex numbers.
- Does not allow implied multiplication except for the imaginary "i" with numbers, where for example 5i is treated as (5*i).
- Attempts to find the reduced fractional form of the result.

#### Sample usage:

```
Current mode: Scientific
> 2-3/17
= 1.823529412
= 1 + 14 / 17
= 31 / 17

> integrate(1,e,ln(x))
= 0.9999983613

> ans*2i
= 1.999996723 i
Mod = 1.999996723, arg = 1.570796327 rad = 90 deg

> 5i
= 5 i
Mod = 5, arg = 1.570796327 rad = 90 deg

> exp(i*pi/12)
= 0.9659258263+0.2588190451 i
Mod = 1, arg = 0.2617993878 rad = 15 deg

> 0xff
= 255

> 0b101.01
= 5.25
= 5 + 1 / 4
= 21 / 4

> rand()
= 0.4630114902

> f(x,y)=x^y+2
Function set successfully.

> f(2,3)
= 10

> 38//5
= 7

> -38//5
= -7

> 2**8-2^8
= 0
```

### Integer Mode

Uses only integers and supports bitwise operations.

#### Supported Operators

- Operator associativity: Left to right.
- Priority: see next, sorted by highest priority with operators on the same line having the same priority.

```
**              Power operator
* / %           Multiplication, division and modulo.
+ -             Addition and subtraction.
<< >> <<< >>>   Shift left, shift right (arithmetic), rotate left, rotate right
&               Bitwise AND.
^               Bitwise XOR.
|               Bitwise OR.
```

Assignment operations `+=` `-=` `*=` `/=` `%=` `^=` `|=` `&=` `<<=` `>>=` `<<<=` `>>>=` `**=` are also supported.

#### Supported Functions

Provided by `libtmsolve`, check them [here](https://a-h-ismail.gitlab.io/libtmsolve-docs/md_INT_FUNCTIONS.html).

#### Variable Size

By default, this mode uses 32 bit signed integers, but can be changed to 8, 16, 32, or 64 bits of width during runtime using the command `set` and width specifier `w1`, `w2`, `w4`, `w8`.

#### Usage Examples

```
Current mode: Integer
> 0xFE & 0xa5
= 164
= 0xA4 = 0o244
= 0b 00000000 00000000 00000000 10100100
= 0.0.0.164

> ans >> 4
= 10
= 0xA = 0o12
= 0b 00000000 00000000 00000000 00001010
= 0.0.0.10

> ans << 5
= 320
= 0x140 = 0o500
= 0b 00000000 00000000 00000001 01000000
= 0.0.1.64

> set w2
Word size set to 16 bits.

> 0b1111 1111 >>> 12
= 4080
= 0xFF0 = 0o7 760
= 0b 00001111 11110000
= 15.240

> 14**3
= 2744
= 0xAB8 = 0o5 270
= 0b 00001010 10111000
= 10.184

> float(2.73)
= 1076803666
= 0x402E B852 = 0o10 013 534 122
= 0b 01000000 00101110 10111000 01010010
= 64.46.184.82

> not(0)
= -1
= 0xFFFF = 0o177 777
= 0b 11111111 11111111
= 255.255

> set w4
Word size set to 32 bits.

> ipv4(192.168.1.229)
= -1062731291
= 0xC0A8 01E5 = 0o30 052 000 745
= 0b 11000000 10101000 00000001 11100101
= 192.168.1.229

> ipv4(192.168.162.229) & ipv4_prefix(22)
= -1062690816
= 0xC0A8 A000 = 0o30 052 120 000
= 0b 11000000 10101000 10100000 00000000
= 192.168.160.0

> mask(8)
= 255
= 0xFF = 0o377
= 0b 00000000 00000000 00000000 11111111
= 0.0.0.255

> ipv4(194.43.236.139) & mask(12)
= 3211
= 0xC8B = 0o6 213
= 0b 00000000 00000000 00001100 10001011
= 0.0.12.139
```

### Function Mode

Calculates a function over the specified interval.
example: Calculating points across a parabola:

```
Current mode: Function
f(x) = x^2
Start: -4
-4
End: 4
4
Step: 1
1
f(-4) = 16
f(-3) = 9
f(-2) = 4
f(-1) = 1
f(0) = 0
f(1) = 1
f(2) = 4
f(3) = 9
f(4) = 16
```

You can use the `prev` keyword to get the last used function.

### Equation Mode

Used to solve simple equations up to and including third order.

```
Current mode: Equation
Degree? (n<=3)
3
a*x^3 + b*x^2 + c*x + d = 0
a = 1
b = 7
c = 16
d = 12

Equation:  1 x^3 + 7 x^2 + 16 x + 12 = 0
Solutions:
x1 = -3
x2 = x3 = -2


Degree? (n<=3)
2
a*x^2 + b*x + c = 0
a = 4
b = -3
c = -1

Equation: 4 x^2 - 3 x - 1 = 0
Solutions:
x1 = -0.25
x2 = 1
```

## Installation instructions

### Windows

Precompiled binaries are provided in the "Releases" section. You can also use CMake if you want to build from source. You need MSYS2 to install Mingw packages of `gcc` (NOT Cygwin), `readline`, `winpthread` and `ncurses`.

### Linux

You need to build from source.

Install required packages:

#### For Debian/Ubuntu

`sudo apt install gcc libreadline-dev git cmake`

#### For Fedora

`sudo dnf install gcc readline-devel git cmake`

---

Clone the project repository, configure, build and install the project:

```
git clone --depth 1 --branch v1.5.0 https://github.com/a-h-ismail/tmsolve.git
cd tmsolve && git submodule update --init

# Generate the makefile and use it to build the binary
cmake -S . -B build; cmake --build build
# Remove any older version and install the new one
sudo bash -c 'rm /usr/local/bin/tmsolve; cmake --install build'
```

## License

This project is licensed under the GNU GPL-3.0-or-later.