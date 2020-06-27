# GSocArduino2020
Repository to store details about PRs and code for GSoC 2020

scanner.l contains the lexical analyser for the compiler. lex.yy.c is the output after using gcc -lfl lex.yy.c

scanner.l contains the following tokens:

- constant literals
- identifiers
- operators
- Keywords, taken from Arduino Language Reference: https://www.arduino.cc/reference/en/
- Comment Lines, both single and Multiline
- Strings
- Blank spaces, newlines
- punctuators, operators
- Anything else is recognised as an error (As of now)

Currently working with #include directives results in messing up, will figure out how to integrate later.

Continous work log here: https://docs.google.com/document/d/1z10PZ14lkHkTpayLLLew9qie6Hbd-CQ-pehbns72G6M/edit?usp=sharing

## Pull Requests

https://github.com/arduino/ArduinoCore-samd/pull/533

https://github.com/arduino/ArduinoCore-avr/pull/350

https://github.com/arduino/reference-en/pull/749

https://github.com/arduino/reference-en/pull/750

https://github.com/arduino/reference-en/pull/751

