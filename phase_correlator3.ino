// Include libraries for ESP32 and OLED display
#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// Define pins for analog inputs 0 and 1
#define A0 36
#define A1 39

// Define constants for sampling and processing
#define SAMPLES 64 // Number of samples per buffer
#define FREQ 1000 // Sampling frequency in Hz
#define PERIOD (1000000 / FREQ) // Sampling period in microseconds
#define ORDER 6 // Order of Walsh system and Hadamard matrix

// Declare global variables for buffers, Walsh system, and Hadamard matrix
int buffer0[SAMPLES]; // Buffer for analog input 0
int buffer1[SAMPLES]; // Buffer for analog input 1
int walsh[ORDER][SAMPLES]; // Walsh system of order 6
int hadamard[ORDER][ORDER]; // Hadamard matrix of order 6
int output[ORDER][ORDER]; // Output matrix after correlation

// Declare OLED display object
Adafruit_SSD1306 display = Adafruit_SSD1306(128, 64, &Wire);

// Function to initialize the Walsh system
void initWalsh() {
  // Loop through each row of the Walsh system
  for (int i = 0; i < ORDER; i++) {
    // Loop through each column of the Walsh system
    for (int j = 0; j < SAMPLES; j++) {
      // Calculate the value of the Walsh function at index j
      int value = 1;
      int index = j;
      // Loop through each bit of the row index i
      for (int k = 0; k < ORDER; k++) {
        // If the bit is 1, multiply the value by -1
        if (i & (1 << k)) {
          value *= -1;
        }
        // If the index is even, divide it by 2
        if (index % 2 == 0) {
          index /= 2;
        }
        // Otherwise, subtract 1 and divide by 2
        else {
          index = (index - 1) / 2;
        }
      }
      // Assign the value to the Walsh system
      walsh[i][j] = value;
    }
  }
}

// Function to initialize the Hadamard matrix
void initHadamard() {
  // Loop through each row of the Hadamard matrix
  for (int i = 0; i < ORDER; i++) {
    // Loop through each column of the Hadamard matrix
    for (int j = 0; j < ORDER; j++) {
      // Calculate the value of the Hadamard function at index (i, j)
      int value = 1;
      int index = i & j;
      // Loop through each bit of the index
      for (int k = 0; k < ORDER; k++) {
        // If the bit is 1, multiply the value by -1
        if (index & (1 << k)) {
          value *= -1;
        }
      }
      // Assign the value to the Hadamard matrix
      hadamard[i][j] = value;
    }
  }
}

// Function to sample the analog inputs and store them in the buffers
void sampleInputs() {
  // Loop through each sample index
  for (int i = 0; i < SAMPLES; i++) {
    // Read the analog values from inputs 0 and 1
    buffer0[i] = analogRead(A0);
    buffer1[i] = analogRead(A1);
    // Wait for the sampling period
    delayMicroseconds(PERIOD);
  }
}

// Function to correlate the buffers using the Walsh system and the Hadamard matrix
void correlateBuffers() {
  // Loop through each row of the output matrix
  for (int i = 0; i < ORDER; i++) {
    // Loop through each column of the output matrix
    for (int j = 0; j < ORDER; j++) {
      // Initialize the sum to 0
      int sum = 0;
      // Loop through each sample index
      for (int k = 0; k < SAMPLES; k++) {
        // Multiply the buffer values by the Walsh functions and the Hadamard function
        sum += buffer0[k] * walsh[i][k] * buffer1[k] * walsh[j][k] * hadamard[i][j];
      }
      // Assign the sum to the output matrix
      output[i][j] = sum;
    }
  }
}

// Function to visualize the output matrix on the OLED display
void visualizeOutput() {
  // Clear the display
  display.clearDisplay();
  // Set the text color to white
  display.setTextColor(SSD1306_WHITE);
  // Set the text size to 1
  display.setTextSize(1);
  // Display the title
  display.setCursor(0, 0);
  display.println("Phase-Frequency Correlation");
  // Display the x-axis label
  display.setCursor(0, 56);
  display.println("Signal 0");
  // Display the y-axis label
  display.setCursor(0, 8);
  display.println("S");
  display.setCursor(0, 16);
  display.println("i");
  display.setCursor(0, 24);
  display.println("g");
  display.setCursor(0, 32);
  display.println("n");
  display.setCursor(0, 40);
  display.println("a");
  display.setCursor(0, 48);
  display.println("l");
  display.setCursor(0, 56);
  display.println("1");
  // Loop through each row of the output matrix
  for (int i = 0; i < ORDER; i++) {
    // Loop through each column of the output matrix
    for (int j = 0; j < ORDER; j++) {
      // Map the output value to a brightness level between 0 and 255
      int brightness = map(output[i][j], -SAMPLES * SAMPLES, SAMPLES * SAMPLES, 0, 255);
      // Draw a pixel on the display with the corresponding brightness
      display.drawPixel(16 + j * 16, 8 + i * 8, brightness);
    }
  }
  // Display the buffer
  display.display();
}

// Setup function
void setup() {
  // Initialize serial communication
  Serial.begin(115200);
  // Initialize the Walsh system
  initWalsh();
  // Initialize the Hadamard matrix
  initHadamard();
  // Initialize the OLED display
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
}

// Loop function
void loop() {
  // Sample the analog inputs
  sampleInputs();
  // Correlate the buffers
  correlateBuffers();
  // Visualize the output
  visualizeOutput();
}
