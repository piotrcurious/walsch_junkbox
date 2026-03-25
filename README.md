# Walsh Junkbox - Phase Correlator Testing System

This repository contains various Arduino sketches for signal processing using Walsh functions and Hadamard transforms.
A comprehensive testing system is provided to verify the logic of these sketches without physical hardware.

## Testing System

The testing system consists of:
1.  **Mock Arduino Environment** (`tests/mock_arduino/`): A self-contained C++ mock for the Arduino core and common libraries (Wire, Adafruit_GFX, Adafruit_SSD1306).
2.  **Signal Simulation**: The mock environment simulates analog signals on specific pins (e.g., A0/A1). You can configure waveforms (SINE, SQUARE, SAWTOOTH), frequency, phase, and noise levels in `tests/mock_arduino/main.cpp`.
3.  **Test Runner** (`tests/test_runner.py`): An automated script that compiles every `.ino` file using g++ and verifies their execution.

### Running Tests

To run the automatic verification:
```bash
python3 tests/test_runner.py
```

To run a specific sketch in **interactive mode** (requires `tkinter`):
```bash
python3 tests/test_runner.py --interactive
```
In interactive mode, you can select a sketch to run, and its OLED display output and Serial output will be shown in a window.
