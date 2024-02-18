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

// Declare the arrays for storing the transformed signals
int signal0[BUFFER_SIZE];
int signal1[BUFFER_SIZE];

// Declare the variables for storing the frequency and phase of the signals
int freq0;
int freq1;
int phase0;
int phase1;

// Declare the variable for storing the correlation coefficient
float correlation;

// Initialize the ESP32 and the OLED display
void setup() {
  // Initialize the serial monitor
  Serial.begin(115200);

  // Initialize the analog inputs
  analogReadResolution(10); // Set the resolution to 10 bits
  analogSetAttenuation(ADC_11db); // Set the attenuation to 11 dB

  // Initialize the OLED display
  display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR); // Start the display with the given address
  display.clearDisplay(); // Clear the display buffer
  display.setTextSize(1); // Set the text size to 1
  display.setTextColor(SSD1306_WHITE); // Set the text color to white
  display.setCursor(0, 0); // Set the cursor to the top-left corner

  // Generate the Walsh matrix using the recursive algorithm
  walsh[0][0] = 1; // Set the first element to 1
  for (int n = 1; n < BUFFER_SIZE; n *= 2) { // Loop through the powers of 2
    for (int i = 0; i < n; i++) { // Loop through the rows
      for (int j = 0; j < n; j++) { // Loop through the columns
        walsh[i + n][j] = walsh[i][j]; // Copy the top-left quadrant
        walsh[i][j + n] = walsh[i][j]; // Copy the top-right quadrant
        walsh[i + n][j + n] = -walsh[i][j]; // Copy the bottom-left quadrant with negation
      }
    }
  }
}

// Main loop of the program
void loop() {
  // Sample the analog inputs and store them in the buffers
  for (int i = 0; i < BUFFER_SIZE; i++) { // Loop through the buffer indices
    buffer0[i] = analogRead(A0); // Read from analog input 0
    buffer1[i] = analogRead(A1); // Read from analog input 1
    delay(1); // Wait for 1 millisecond
  }

  // Apply the Hadamard transform to the buffers using the Walsh matrix
  for (int i = 0; i < BUFFER_SIZE; i++) { // Loop through the buffer indices
    signal0[i] = 0; // Initialize the transformed signal 0
    signal1[i] = 0; // Initialize the transformed signal 1
    for (int j = 0; j < BUFFER_SIZE; j++) { // Loop through the Walsh matrix indices
      signal0[i] += buffer0[j] * walsh[i][j]; // Multiply and add the buffer 0 and the Walsh matrix
      signal1[i] += buffer1[j] * walsh[i][j]; // Multiply and add the buffer 1 and the Walsh matrix
    }
  }

  // Find the base frequency and phase of the signals by finding the maximum absolute value in the transformed signals
  int max0 = 0; // Initialize the maximum value for signal 0
  int max1 = 0; // Initialize the maximum value for signal 1
  for (int i = 0; i < BUFFER_SIZE; i++) { // Loop through the buffer indices
    if (abs(signal0[i]) > abs(max0)) { // If the absolute value of the signal 0 is greater than the current maximum
      max0 = signal0[i]; // Update the maximum value
      freq0 = i; // Update the frequency index
      phase0 = (max0 > 0) ? 0 : 180; // Update the phase angle (0 or 180 degrees)
    }
    if (abs(signal1[i]) > abs(max1)) { // If the absolute value of the signal 1 is greater than the current maximum
      max1 = signal1[i]; // Update the maximum value
      freq1 = i; // Update the frequency index
      phase1 = (max1 > 0) ? 0 : 180; // Update the phase angle (0 or 180 degrees)
    }
  }

  // Calculate the correlation coefficient between the signals using the Pearson formula
  float sum0 = 0; // Initialize the sum of signal 0
  float sum1 = 0; // Initialize the sum of signal 1
  float sum01 = 0; // Initialize the sum of the product of signal 0 and 1
  float sum00 = 0; // Initialize the sum of the square of signal 0
  float sum11 = 0; // Initialize the sum of the square of signal 1
  float mean0 = 0; // Initialize the mean of signal 0
  float mean1 = 0; // Initialize the mean of signal 1
  for (int i = 0; i < BUFFER_SIZE; i++) { // Loop through the buffer indices
    sum0 += signal0[i]; // Add the signal 0 to the sum
    sum1 += signal1[i]; // Add the signal 1 to the sum
    sum01 += signal0[i] * signal1[i]; // Add the product of signal 0 and 1 to the sum
    sum00 += signal0[i] * signal0[i]; // Add the square of signal 0 to the sum
    sum11 += signal1[i] * signal1[i]; // Add the square of signal 1 to the sum
  }
  mean0 = sum0 / BUFFER_SIZE; // Calculate the mean of signal 0
  mean1 = sum1 / BUFFER_SIZE; // Calculate the mean of signal 1
  correlation = (sum01 - BUFFER_SIZE * mean0 * mean1) / sqrt((sum00 - BUFFER_SIZE * mean0 * mean0) * (sum11 - BUFFER_SIZE * mean1 * mean1)); // Calculate the correlation coefficient

  // Print the frequency and phase of the signals and the correlation coefficient to the serial monitor
  Serial.print("Frequency of signal 0: ");
  Serial.println(freq0);
  Serial.print("Phase of signal 0: ");
  Serial.println(phase0);
  Serial.print("Frequency of signal 1: ");
  Serial.println(freq1);
  Serial.print("Phase of signal 1: ");
  Serial.println(phase1);
  Serial.print("Correlation coefficient: ");
  Serial.println(correlation);

  // Visualize the correlated phase along with frequency on the OLED display by plotting the output of the Walsh matrix
  display.clearDisplay(); // Clear the display buffer
  display.setCursor(0, 0); // Set the cursor to the top-left corner
  display.print("Freq0: "); // Print the label for frequency 0
  display.print(freq0); // Print the value of frequency 0
  display.print(" Hz"); // Print the unit of frequency
  display.print("  Phase0: "); // Print the label for phase 0
  display.print(phase0); // Print the value of phase 0
  display.println(" deg"); // Print the unit of phase
  display.print("Freq1: "); // Print the label for frequency 1
  display.print(freq1); // Print the value of frequency 1
  display.print(" Hz"); // Print the unit of frequency
  display.print("  Phase1: "); // Print the label for phase 1
  display.print(phase1); // Print the value of phase 1
  display.println(" deg"); // Print the unit of phase
  display.print("Corr: "); // Print the label for correlation coefficient
  display.println(correlation); // Print the value of correlation coefficient
  display.drawLine(0, 31, 127, 31, SSD1306_WHITE); // Draw a horizontal line to separate the text and the plot
  display.drawLine(63, 31, 63, 63, SSD1306_WHITE); // Draw a vertical line to mark the origin of the plot
  for (int i = 0; i < BUFFER_SIZE; i++) { // Loop through the buffer indices
    int x = map(signal0[i], -1024, 1024, 0, 127); // Map the signal 0 to the x-axis of the plot
    int y = map(signal1[i], -1024, 1024, 31, 63); // Map the signal 1 to the y-axis of the plot
    display.drawPixel(x, y, SSD1306_WHITE); // Draw a pixel at the corresponding coordinates
  }
  display.display(); // Display the buffer on the screen
  delay(1000); // Wait for 1 second
}
