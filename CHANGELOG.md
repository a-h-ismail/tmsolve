# Changelog

Note that most significant changes are in `libtmsolve` changelogs, so check them for most features and bugfixes.

## 1.3.0 - 2024-09-20

Built with `libtmsolve` version 3.0.0

### Changed

- Scientific mode now supports multi-argument user functions, adding it to Integer mode too.
- Autocompletion (Linux) now works much better using proper separators and `libtmsolve` new helper functions.
- On Windows, the terminal color is set to white background with black text.
- Display format of functions distinguishes between the 3 types.

### Added

- `del` keyword to unset user variables and functions.
- `reset` keyword to clear the state of the calculator.
- A simple solver benchmark to test the hashmap performance.

## 1.2.1 - 2024-05-10

Built with `libtmsolve` version 2.3.0

### Changed

- Use non locking parser and evaluator in benchmark mode.
- Avoid consecutive duplicated lines in input history (Linux only).
- Other minor changes.

### Fixed

- Some silent failures in utility mode.

## 1.2.0 - 2024-03-02

Built with `libtmsolve` version 2.2.0

### Added

- `debug` and `undebug` keywords in Scientific and Integer modes.


## 1.1.0 - 2024-02-25

Built with `libtmsolve` version 2.1.0

### Changed

- Help is now better and more concise.
- Command line argument parsing is a bit more robust.
- Integer mode can be directly used from the command line using the prefix `I:`.
- Function mode now calculates with complex function support.
- Error reporting is now much less verbose in function mode.

## 1.0.0 - 2024-02-21

Built with `libtmsolve` version 2.0.0

### Changed

- Mode switching is now done using the `mode` keyword.
- Rename Base-N mode to Integer mode.
- Print dot decimal notation in Integer mode.
- Add a startup prompt.
- Add help for every mode.
- Add `functions` and `variables` keywords to view currently defined functions and variables in Scientific and Integer modes.

## 0.6.0 - 2024-01-13

Built with `libtmsolve` version 1.4.0

### Changed

- Auto completion now works much better (Linux only).

## 0.5.0 - 2023-12-23

Built with `libtmsolve` version 1.3.0

### Fixed

- NULL dereference if `prev` was requested in function mode without any previous input.
- Silent failure in Utility Mode.
- Memory leaking in Function Mode.

### Changed

- Enable debug mode by default while running in test mode.

## 0.4.1 - 2023-12-16

Built with `libtmsolve` version 1.2.1

### Added

- Test mode for Base-N functions.
- Sign extend `ans` in Base-N mode.

## 0.4.0 - 2023-10-30

Built with `libtmsolve` version 1.2.0

### Added: Base-N mode

- Access using 'B' in any mode.
- Supports decimal, hexadecimal, octal, and binary representation using prefixes (none for decimal, `0x`, `0o`, `0b`).
- Supports most used bitwise operations using operators `& | ^` and functions.
- Supports normal integer arithmetic `+ - * / %`.
- Supports runtime variables.
- Word size length defaults to 32 bit during runtime, can be changed with the `set` keyword.
- Prints the answer in all representations.

## 0.3.0 - 2023-10-10

Built with `libtmsolve` version 1.1.0

### Added

- User function support, defined at runtime.
- Benchmark mode now displays rate in iterations per second.

### Fixed

- Incorrect time display in benchmark mode.

## 0.2.1 - 2023-09-12

### Changed

- Fix multiple memory leaks in function mode.
- Add `prev` keyword to use the previous function in function mode.

## 0.2.0 - 2023-08-22

Initial release of tmsolve.