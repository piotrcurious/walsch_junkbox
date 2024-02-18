// Include the libraries for ESP32 and OLED display
#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// Define the pins for analog inputs 0 and 1
#define A0 36
#define A1 39

// Define the size of the buffers for sampling
#define BUFFER_SIZE 64

// Define the size and address of the OLED display
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_ADDR 0x3C

// Create an object for the OLED display
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// Declare the buffers for storing the samples
int buffer0[BUFFER_SIZE];
int buffer1[BUFFER_SIZE];

// Declare the Walsh matrix for the Hadamard transform
int walsh[BUFFER_SIZE][BUFFER_SIZE];

// Declare the variables for storing the base frequencies and phases
int freq0, freq1;
float phase0, phase1;

// Declare the variables for storing the correlation and the plot coordinates
float correlation;
int x, y;

// Initialize the ESP32 and the OLED display
void setup() {
  // Set the analog input pins to input mode
  pinMode(A0, INPUT);
  pinMode(A1, INPUT);

  // Initialize the serial communication
  Serial.begin(115200);

  // Initialize the OLED display
  display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR);
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0, 0);
  display.println("ESP32 Phase Correlation");
  display.display();

  // Generate the Walsh matrix using the recursive formula
  walsh[0][0] = 1;
  for (int n = 1; n < BUFFER_SIZE; n *= 2) {
    for (int i = 0; i < n; i++) {
      for (int j = 0; j < n; j++) {
        walsh[i + n][j] = walsh[i][j];
        walsh[i][j + n] = walsh[i][j];
        walsh[i + n][j + n] = -walsh[i][j];
      }
    }
  }
}

// Perform the sampling, transformation, correlation, and visualization
void loop() {
  // Sample the analog inputs and store them in the buffers
  for (int i = 0; i < BUFFER_SIZE; i++) {
    buffer0[i] = analogRead(A0);
    buffer1[i] = analogRead(A1);
  }

  // Apply the Hadamard transform to the buffers using the Walsh matrix
  for (int i = 0; i < BUFFER_SIZE; i++) {
    int sum0 = 0;
    int sum1 = 0;
    for (int j = 0; j < BUFFER_SIZE; j++) {
      sum0 += buffer0[j] * walsh[i][j];
      sum1 += buffer1[j] * walsh[i][j];
    }
    buffer0[i] = sum0;
    buffer1[i] = sum1;
  }

  // Find the base frequencies and phases of the signals from the transformed buffers
  freq0 = 0;
  freq1 = 0;
  phase0 = 0;
  phase1 = 0;
  for (int i = 0; i < BUFFER_SIZE; i++) {
    if (abs(buffer0[i]) > abs(buffer0[freq0])) {
      freq0 = i;
    }
    if (abs(buffer1[i]) > abs(buffer1[freq1])) {
      freq1 = i;
    }
  }
  phase0 = atan2(buffer0[freq0], buffer0[BUFFER_SIZE - freq0]);
  phase1 = atan2(buffer1[freq1], buffer1[BUFFER_SIZE - freq1]);

  // Calculate the correlation between the phases of the signals
  correlation = cos(phase0 - phase1);

  // Map the frequencies and correlation to the plot coordinates
  x = map(freq0, 0, BUFFER_SIZE / 2, 0, SCREEN_WIDTH);
  y = map(freq1, 0, BUFFER_SIZE / 2, 0, SCREEN_HEIGHT);
  correlation = map(correlation, -1, 1, 0, 255);

  // Print the frequencies, phases, and correlation to the serial monitor
  Serial.print("Frequency 0: ");
  Serial.println(freq0);
  Serial.print("Frequency 1: ");
  Serial.println(freq1);
  Serial.print("Phase 0: ");
  Serial.println(phase0);
  Serial.print("Phase 1: ");
  Serial.println(phase1);
  Serial.print("Correlation: ");
  Serial.println(correlation);

  // Draw a pixel on the OLED display with the plot coordinates and correlation
  display.drawPixel(x, y, correlation);
  display.display();
}
