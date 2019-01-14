# Security in automobiles: Vulnerability and security of the on-board diagnostics port (OBD-II)

Part of a master's thesis for a master of engineering: computer science.
KuLeuven, Department of Computer Science.

Student: Michiel Willems.
Thesis Assisants: Mahmoud Ammar and Hassaan Janjua.
Promotor: Prof. dr. Bruno Crispo.

## Features

This repository contains the source code for a 'proof-of-concept' of a newly proposed OBD-II access control system.
It is designed to run on two DVK90CAN1 boards (AT90CAN128 microcontroller) and the source code consists of two Atmel Studio C projects.
One functions as the tester device and the other as the gateway (tester authenticates to the gateway).
Both are connected via CAN (DB9 connector).

For more information on the proposed solution and the problem in general, see thesis.pdf.

## Contents
- **Articles:** Articles and papers that constitute the background of this research topic.
- **GccApplication2:** Source code of the tester device.
- **GccApplication3:** Source code of the automotive gateway.
- **Paper:** Contains the thesis text, i.e. thesis.pdf.
- **Presentation:** All presentations that were given on this subject can be found here.

