# tmsolve

## Description

Powerful and lightweight command line calculator.

## Usage

### Command Line Arguments

Run `tmsolve --help` (or `tmsolve.exe --help` for Windows) to see supported command line arguments.

### Scientific Mode

Sample usage:

```
Current mode: Scientific
> 2-3/17
= 1.823529412
= 1 + 14 / 17
= 31 / 17

> integrate(1,exp,ln(x))
= 0.9999983613

> ans*2i
= 1.999996723 i
Modulus = 1.999996723, argument = 1.570796327 rad = 90 deg

> 5i
= 5i
Modulus = 5, argument = 1.570796327 rad = 90 deg

> exp^(i*pi/12)
= 0.9659258263+0.2588190451 i
Modulus = 1, argument = 0.2617993878 rad = 15 deg

> 0xff
= 255

> 0b101.01
= 5.25
= 5 + 1 / 4
= 21 / 4

> oct(700)
= 448

> rand()
= 342083192.2

> g(x)=x^2+2*x+4
Function set successfully

> g(3.5)
= 23.25
= 23 + 1 / 4
= 93 / 4
```

- Default mode when `tmsolve` is started without arguments.
- Supported functions are the ones provided by libtmsolve, check them [here](https://a-h-ismail.gitlab.io/libtmsolve-docs/md_FUNCTIONS.html).
- Supports user defined functions, syntax: `foo(x)=expression`.
- Supports hexadecimal, octal and binary represenation using prefixes `0x`,`0o`,`0b` or functions `hex()`, `oct()`, `bin()`.
- "ans" variable stores previous results.
- Supports complex numbers.
- Does not allow implied multiplication except for the imaginary "i" with numbers, where for example 5i is treated as (5*i).
- The syntax of this mode is used in function and equation mode when a value is requested.

### Base-N Mode

Uses only integers and supports bitwise operations, access by entering 'B' in any mode.

#### Supported Operators

priority: high to low, groups in `[]` have the same priority:

 `[ * / ] [ + - % ] [ < > ] & ^ |`

Where:

```
* /     Integer multiplication and division.
+ - %   Integer add, subtract, modulo.
< >     Bitwise left and right shift.
&       Bitwise AND.
^       Bitwise XOR.
|       Bitwise OR.
```

#### Supported Functions

All of the following functions perform bitwise operations:

- `not(value)`: Expects one argument, returns one's complement of `value`.
- `and, nand, or, xor, xnor`: Expects 2 arguments, name is self explanatory.
- `rrc(value, shift)`: Performs right circular rotation of `value` bits by `shift` bits.
- `rlc(value, shift)`: Performs left circular rotation of `value` bits by `shift` bits.

#### Variable Size

By default, this mode uses 32 bit signed integers, but can be changed to 8, 16, 32, or 64 bits of width during runtime using the command `set` and width specifier `w1`, `w2`, `w4`, `w8`.

Usage:

```
Current mode: Base-N
> set w2
Word size set to 16 bits.

> 0xFE & 0xa5
= 164 = 0xA4 = 0o244
= 0b10100100

> ans<2
= 656 = 0x290 = 0o1220
= 0b1010010000

> (ans<2)>4
= 164 = 0xA4 = 0o244
= 0b10100100

> not(0)
= -1 = 0xFFFF = 0o177777
= 0b1111111111111111

> set w8
Word size set to 64 bits.

> not(0)
= -1 = 0xFFFFFFFFFFFFFFFF = 0o1777777777777777777777
= 0b1111111111111111111111111111111111111111111111111111111111111111

> nor(0b1000,0b11)
= -12 = 0xFFFFFFFFFFFFFFF4 = 0o1777777777777777777764
= 0b1111111111111111111111111111111111111111111111111111111111110100

> 0xF00 | 0o61
= 3889 = 0xF31 = 0o7461
= 0b111100110001

> a=0b001101011
= 107 = 0x6B = 0o153
= 0b1101011

> a & 0xF
= 11 = 0xB = 0o13
= 0b1011
```

### Function Mode

Calculates a function over the specified interval. Access by entering 'F' in any other mode.
example: Calculating points across a parabola:

```
Current mode: Scientific
> F

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

### Tic-Tac-Toe

Play tic-tac-toe against the computer, enter 'G' in any mode to launch the game.

```
Current mode: Tic-Tac-Toe
Wins = 0
Losses = 0
Draws = 0
+-------+-------+-------+
|       |       |       |
|   1   |   2   |   3   |
|       |       |       |
+-------+-------+-------+
|       |       |       |
|   4   |   5   |   6   |
|       |       |       |
+-------+-------+-------+
|       |       |       |
|   7   |   8   |   9   |
|       |       |       |
+-------+-------+-------+

Enter position on board [1-9]: 
```

## Installation instructions

### Build from source

Install required packages:

#### For Debian/Ubuntu

`sudo apt install gcc libreadline-dev git cmake make`

#### For Fedora

`sudo dnf install gcc readline-devel git cmake make`

Clone the project repository and run the install script:

```
git clone --depth 1 https://gitlab.com/a-h-ismail/tmsolve
cd tmsolve
git submodule init libtmsolve
git submodule update
# Generate the makefile and use it to build the binary
cd ./tmsolve
cmake -S. -B./build -G "Unix Makefiles"
cd ./build
make
sudo make install
```

## Security practices

- The binary is run in `valgrind` memcheck tool with a variety of inputs to verify memory safety and possible leaks.
- The source code is inspected using `clang-tidy`.

## License

This project is licensed under the GNU GPL-3.0-or-later.
