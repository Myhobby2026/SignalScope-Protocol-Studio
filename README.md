# SignalScope Teensy 4.1 Protocol Monitor

## App sections
The browser app has separate Monitor, Decode, Capture, and Hardware tabs. Start demo generates event rows and waveforms without a Teensy. Capture waveform runs a demo trace when hardware is disconnected; with the bridge connected and GPIO selected, it requests a Teensy capture. The Hardware tab embeds the PCB wiring diagram and links to the detailed pin guide.

## Start the app
From this project folder, run: python -m http.server 8000 --bind 127.0.0.1
Then open http://127.0.0.1:8000 in Chrome or Edge. Web Serial needs HTTPS or localhost.

## Teensy firmware
Open firmware/SignalScope_Teensy41.ino in Arduino IDE with Teensyduino. Select Teensy 4.1 and Tools > USB Type > Serial, then upload. In the app, click Connect Teensy and choose the board.

Included hardware capture: UART RX on pin 0 / RX1, optional UART TX on pin 1 / TX1, and GPIO edge/sample capture on pins 2-5. GPIO capture defaults to 512 samples at a nominal 50 us interval; software timing has jitter.

I2C, SPI, CAN FD, SWD, JTAG, LIN, Modbus RTU, and 1-Wire views are present, but their real hardware decoders are not included yet. Demo frames are marked DEMO.

## PCB reference
See hardware/teensy41-bridge.svg and hardware/pin-connections.md. Teensy GPIO is 3.3 V only and not 5 V tolerant. CAN requires an external transceiver. The diagram is a pin reference, not a fabrication-ready schematic.
