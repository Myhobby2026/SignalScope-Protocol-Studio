# Teensy 4.1 bridge pin connections

Reference wiring only. Confirm your target voltage and board revision before laying out a PCB.

## USB and power
- Teensy USB device connector -> computer; browser uses the firmware's USB Serial link.
- Teensy GND -> target GND (shared reference).
- Teensy 3.3 V output is for small logic loads only; PJRC specifies a 250 mA maximum on the reference card. Do not power the target board from this bridge.

## UART (included firmware)
- Target TX -> Teensy pin 0 / RX1. This is the passive receive path.
- Teensy pin 1 / TX1 -> target RX only when transmitting is needed. Leave disconnected for strictly passive capture.
- Target GND -> Teensy GND.
- Firmware default is 115200, 8-N-1. Change baud from the app before capture.
- Both directions must be 3.3 V compatible. Use a level translator/protection stage for 5 V or unknown levels.

## GPIO waveform capture (included firmware)
- Up to four input signals -> Teensy pins 2, 3, 4, 5 respectively.
- Signal ground -> Teensy GND.
- Inputs must stay within 0-3.3 V. Firmware captures 512 samples at a requested nominal interval (default 50 us); software timing has jitter.

## Future decoder connector suggestions (pin card functions)
- I2C Wire: pin 18 SDA, pin 19 SCL, GND. Tap the existing bus; do not add pull-ups if the target already has them. True passive decode firmware is not included yet.
- SPI: pin 11 MOSI, pin 12 MISO, pin 13 SCK, pin 10 CS0, GND. These are logic taps; passive SPI capture firmware is not included yet.
- CAN1 logic side: pin 22 CTX1 -> transceiver TXD; transceiver RXD -> pin 23 CRX1. Transceiver CANH/CANL -> external CAN bus. Add the transceiver and bus protection/termination appropriate to your topology. Do not connect CANH/CANL directly to Teensy GPIO.
- SWD/JTAG need dedicated level-safe driver/decoder design; these pins are not assigned by this starter.

For a PCB, consider ESD protection at external connectors, series resistors/current limiting, selectable termination only where appropriate, and separate target power from Teensy USB power. Route USB differential pairs per USB layout guidance. Have the schematic and net classes reviewed before fabrication.
