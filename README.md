# tmsolve
`tmsolve`, simple and lightweight CLI calculator.

# Features
## Scientific mode
- Default mode when started without arguments.
- Supports most used scientific functions (trigonometry and its variants, power, factorial...).
- "ans" variable stores previous results.
- Automatically detects complex number and switches the output accordingly.
- Supports numerical integration and derivation using `int(start,end,function)` and `der(function,x0)`.
- Does not support implied multiplication because it is ambiguous.

## Function mode
Calculates a function over the specified interval.

## Matrix mode
Basic matrix handling (determinant, multiplication, inversion...).

## Complex mode
Forces the calculator to work with complex numbers, not needed since scientific mode can handle complex well.

# Security practices
- The binary is run in `valgrind` memcheck tool with a variety of inputs to verify memory safety and possible leaks.
- The source code is inspected using `clang-tidy`.
