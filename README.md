# tmsolve

## Description

Powerful and lightweight command line calculator.

## Usage

### Scientific mode

Sample usage:

```
Current mode: Scientific
> 2-3/17
= 1.823529412
= 1 + 14 / 17
= 31 / 17

> int(1,exp,ln(x))
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
```

- Default mode when `tmsolve` is started without arguments.
- Supports most used scientific functions (trigonometry and its variants, power, factorial...).
- Supports hexadecimal, octal and binary represenation using prefixes `0x`,`0o`,`0b` or functions `hex()`, `oct()`, `bin()`.
- "ans" variable stores previous results.
- Supports complex numbers.
- Supports numerical integration and derivation using `int(start,end,function)` and `der(function,x0)`.
- Does not allow implied multiplication except for the imaginary "i" with numbers, where for example 5i is treated as (5*i).

### Function mode

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

### Matrix mode

Basic matrix handling (determinant, multiplication, inversion...). Access by entering 'M' in any mode.

### Tic-Tac-Toe

Play tic-tac-toe against the computer, enter 'G' in any mode to launch the game.

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
