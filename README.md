# Walsh Junkbox - Phase Correlator Testing System

This repository contains various Arduino sketches for signal processing using Walsh functions and Hadamard transforms.
A comprehensive testing system is provided to verify the logic of these sketches without physical hardware.

## Testing System

The testing system consists of:
1.  **Mock Arduino Environment** (`tests/mock_arduino/`): A self-contained C++ mock for the Arduino core and common libraries (Wire, Adafruit_GFX, Adafruit_SSD1306).
2.  **Signal Simulation**: The mock environment simulates analog signals on pins A0 (36) and A1 (39). Waveforms (SINE, SQUARE, SAWTOOTH), frequency, phase, offset, and noise levels are configurable.
3.  **Test Runner** (`tests/test_runner.py`): An automated script that compiles every `.ino` file using g++ and verifies their execution.

### Running Tests

To run the automatic verification:
```bash
python3 tests/test_runner.py
```
Automatic mode saves the final display frame for each sketch to `tests/output/<sketch_name>_result.pbm`.

To run a specific sketch in **interactive mode** (requires `tkinter`):
```bash
python3 tests/test_runner.py --interactive
```
In interactive mode, you can:
- View a live simulation of the 128x64 OLED display.
- Monitor Serial output.
- Adjust signal parameters for A1 (Pin 39) in real-time using sliders and buttons to observe the correlator's response.

## Optimizations

Several sketches have been optimized with the **Fast Walsh-Hadamard Transform (FWHT)**, reducing complexity from $O(N^2)$ to $O(N \log N)$ and significantly lowering memory usage by removing the need for pre-calculated transformation matrices.
