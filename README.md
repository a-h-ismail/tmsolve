# tmsolve

## Description

Powerful and lightweight command line calculator.

## Usage

### Scientific mode

Sample usage:

```
Current mode: Scientific
> 2-3/17
= 1.8235294117647
= 1 + 14 / 17
= 31 / 17

> int(1,exp,ln(x))
= 0.99999344540145

> ans*2i
= 1.9999868908029i
Modulus = 1.9999868908029, argument = 1.5707963267949 rad = 90 deg

> 5i
= 5i
Modulus = 5, argument = 1.5707963267949 rad = 90 deg

> exp^(i*pi/12)
= 0.96592582628907+0.25881904510252i
Modulus = 1, argument = 0.26179938779915 rad = 15 deg

> 1/(2-2)
Division by zero isn't defined.
1/(2-2)
~^
```

- Default mode when `tmsolve` is started without arguments.
- Supports most used scientific functions (trigonometry and its variants, power, factorial...).
- "ans" variable stores previous results.
- Automatically detects complex number and switches the output accordingly.
- Supports numerical integration and derivation using `int(start,end,function)` and `der(function,x0)`.
- Does not support implied multiplication except for the imaginary number with digits [0-9] where for example 5i is treated as (5*i).

## Function mode

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

## Matrix mode

Basic matrix handling (determinant, multiplication, inversion...). Access by entering 'M' in any mode.

## Complex mode

Forces the calculator to work with complex numbers, not needed since scientific mode can handle complex well. Access by entering 'C' in any mode.

## Installation instructions

### Linux desktop

#### Build from source

Install required packages:

##### For Debian/Ubuntu

`sudo apt install gcc libreadline-dev git`

##### For Fedora

`sudo dnf install gcc readline-devel git`

Clone the project repository and run the install script:

```
git clone --depth 1 https://gitlab.com/a-h-ismail/tmsolve
cd tmsolve
git submodule init libtmsolve
git submodule update
chmod +x install.sh
sudo ./install.sh
```

## Security practices

- The binary is run in `valgrind` memcheck tool with a variety of inputs to verify memory safety and possible leaks.
- The source code is inspected using `clang-tidy`.

## License

This project is licensed under the GNU GPL-3.0-or-later.
