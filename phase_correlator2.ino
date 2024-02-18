// Include libraries for ESP32 and OLED display
#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// Define pins for analog inputs 0 and 1
#define A0 36
#define A1 39

// Define constants for sampling and Walsh system
#define SAMPLES 64 // Number of samples per buffer
#define FREQUENCY 1000 // Sampling frequency in Hz
#define ORDER 6 // Order of Walsh system (must be a power of 2)

// Create an OLED display object
Adafruit_SSD1306 display = Adafruit_SSD1306(128, 64, &Wire);

// Declare global variables for buffers, Walsh matrix, and correlation
float buffer0[SAMPLES]; // Buffer for analog input 0
float buffer1[SAMPLES]; // Buffer for analog input 1
float walsh[ORDER][ORDER]; // Walsh matrix of order 6
float correlation[ORDER][ORDER]; // Correlation matrix of buffer0 and buffer1

// Initialize the ESP32 and the OLED display
void setup() {
  Serial.begin(115200); // Start serial communication
  pinMode(A0, INPUT); // Set analog input 0 as input
  pinMode(A1, INPUT); // Set analog input 1 as input
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C); // Initialize the OLED display
  display.clearDisplay(); // Clear the display
  display.setTextSize(1); // Set text size
  display.setTextColor(SSD1306_WHITE); // Set text color
  display.setCursor(0, 0); // Set cursor position
  display.println("Walsh Correlation"); // Print title
  display.display(); // Update the display
  generateWalsh(); // Generate the Walsh matrix
}

// Main loop of the program
void loop() {
  sampleInputs(); // Sample the analog inputs and store them in buffers
  correlateBuffers(); // Correlate the buffers using the Walsh matrix
  plotCorrelation(); // Plot the correlation matrix on the OLED display
  delay(1000); // Wait for 1 second
}

// Function to generate the Walsh matrix of order 6
void generateWalsh() {
  // Initialize the first row and column of the Walsh matrix
  for (int i = 0; i < ORDER; i++) {
    walsh[0][i] = 1;
    walsh[i][0] = 1;
  }
  // Generate the rest of the Walsh matrix using the recursive formula
  for (int n = 1; n < ORDER; n *= 2) {
    for (int i = 0; i < n; i++) {
      for (int j = 0; j < n; j++) {
        walsh[i + n][j] = walsh[i][j];
        walsh[i][j + n] = walsh[i][j];
        walsh[i + n][j + n] = -walsh[i][j];
      }
    }
  }
}

// Function to sample the analog inputs and store them in buffers
void sampleInputs() {
  // Calculate the sampling interval in microseconds
  int interval = 1000000 / FREQUENCY;
  // Loop through the samples
  for (int i = 0; i < SAMPLES; i++) {
    // Read the analog inputs and map them to [-1, 1] range
    buffer0[i] = map(analogRead(A0), 0, 4095, -1, 1);
    buffer1[i] = map(analogRead(A1), 0, 4095, -1, 1);
    // Wait for the sampling interval
    delayMicroseconds(interval);
  }
}

// Function to correlate the buffers using the Walsh matrix
void correlateBuffers() {
  // Loop through the rows and columns of the Walsh matrix
  for (int i = 0; i < ORDER; i++) {
    for (int j = 0; j < ORDER; j++) {
      // Initialize the correlation value to zero
      correlation[i][j] = 0;
      // Loop through the samples
      for (int k = 0; k < SAMPLES; k++) {
        // Multiply the buffers by the corresponding Walsh coefficients and add them to the correlation value
        correlation[i][j] += buffer0[k] * walsh[i][k % ORDER] + buffer1[k] * walsh[j][k % ORDER];
      }
      // Divide the correlation value by the number of samples
      correlation[i][j] /= SAMPLES;
    }
  }
}

// Function to plot the correlation matrix on the OLED display
void plotCorrelation() {
  // Clear the display
  display.clearDisplay();
  // Print the title
  display.setCursor(0, 0);
  display.println("Walsh Correlation");
  // Loop through the rows and columns of the correlation matrix
  for (int i = 0; i < ORDER; i++) {
    for (int j = 0; j < ORDER; j++) {
      // Map the correlation value to [0, 63] range
      int value = map(correlation[i][j], -1, 1, 0, 63);
      // Draw a pixel on the display with the corresponding brightness
      display.drawPixel(16 + i * 16, 16 + j * 16, value);
    }
  }
  // Update the display
  display.display();
}
