// Include libraries for ESP32 and OLED display
#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// Define pins for analog inputs 0 and 1
#define A0 36
#define A1 39

// Define constants for sampling and buffer size
#define SAMPLE_RATE 1000 // Hz
#define BUFFER_SIZE 256 // Must be a power of 2

// Define constants for Walsh system and Hadamard matrix
#define N 8 // Number of Walsh functions
#define M 32 // Number of Hadamard matrix rows

// Define constants for OLED display
#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
#define OLED_RESET -1 // Reset pin # (or -1 if sharing Arduino reset pin)

// Declare global variables for buffers, Walsh system, Hadamard matrix, and OLED display
int buffer0[BUFFER_SIZE]; // Buffer for analog input 0
int buffer1[BUFFER_SIZE]; // Buffer for analog input 1
int index = 0; // Index for buffer filling
int walsh[N][BUFFER_SIZE]; // Walsh system matrix
int hadamard[M][N]; // Hadamard matrix
int freq0[N]; // Frequency vector for signal 0
int freq1[N]; // Frequency vector for signal 1
int phase[N]; // Phase vector for signals 0 and 1
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET); // OLED display object

// Function to initialize the Walsh system matrix
void initWalsh() {
  // Loop through the rows of the Walsh matrix
  for (int i = 0; i < N; i++) {
    // Loop through the columns of the Walsh matrix
    for (int j = 0; j < BUFFER_SIZE; j++) {
      // Compute the Walsh function value using bitwise operations
      int x = i & j;
      int y = 0;
      while (x > 0) {
        y ^= x & 1;
        x >>= 1;
      }
      // Assign the Walsh function value to the matrix element
      walsh[i][j] = y == 0 ? 1 : -1;
    }
  }
}

// Function to initialize the Hadamard matrix
void initHadamard() {
  // Loop through the rows of the Hadamard matrix
  for (int i = 0; i < M; i++) {
    // Loop through the columns of the Hadamard matrix
    for (int j = 0; j < N; j++) {
      // Compute the Hadamard function value using bitwise operations
      int x = i & j;
      int y = 0;
      while (x > 0) {
        y ^= x & 1;
        x >>= 1;
      }
      // Assign the Hadamard function value to the matrix element
      hadamard[i][j] = y == 0 ? 1 : -1;
    }
  }
}

// Function to compute the frequency vector for a signal using the Walsh system
void computeFreq(int buffer[], int freq[]) {
  // Loop through the rows of the Walsh matrix
  for (int i = 0; i < N; i++) {
    // Initialize the frequency value to zero
    freq[i] = 0;
    // Loop through the columns of the Walsh matrix
    for (int j = 0; j < BUFFER_SIZE; j++) {
      // Multiply the buffer value and the Walsh function value and add to the frequency value
      freq[i] += buffer[j] * walsh[i][j];
    }
    // Divide the frequency value by the buffer size
    freq[i] /= BUFFER_SIZE;
  }
}

// Function to compute the phase vector for two signals using the Hadamard matrix transform
void computePhase(int freq0[], int freq1[], int phase[]) {
  // Loop through the rows of the Hadamard matrix
  for (int i = 0; i < M; i++) {
    // Initialize the phase value to zero
    phase[i] = 0;
    // Loop through the columns of the Hadamard matrix
    for (int j = 0; j < N; j++) {
      // Multiply the frequency values and the Hadamard function value and add to the phase value
      phase[i] += freq0[j] * freq1[j] * hadamard[i][j];
    }
    // Divide the phase value by the number of Walsh functions
    phase[i] /= N;
  }
}

// Function to visualize the correlated phase along with frequency on the OLED display
void visualizePhase(int freq0[], int freq1[], int phase[]) {
  // Clear the display buffer
  display.clearDisplay();
  // Set the text color to white
  display.setTextColor(SSD1306_WHITE);
  // Set the text size to 1
  display.setTextSize(1);
  // Set the cursor position to the top left corner
  display.setCursor(0, 0);
  // Print the title
  display.println("Phase Correlator");
  // Print the frequency values for signal 0
  display.print("Freq0: ");
  for (int i = 0; i < N; i++) {
    display.print(freq0[i]);
    display.print(" ");
  }
  display.println();
  // Print the frequency values for signal 1
  display.print("Freq1: ");
  for (int i = 0; i < N; i++) {
    display.print(freq1[i]);
    display.print(" ");
  }
  display.println();
  // Draw the x and y axes for the phase plot
  display.drawLine(0, 63, 127, 63, SSD1306_WHITE); // x axis
  display.drawLine(0, 63, 0, 31, SSD1306_WHITE); // y axis
  // Draw the phase plot using the output of the Hadamard matrix
  for (int i = 0; i < M; i++) {
    // Map the phase value to a pixel coordinate
    int x = map(i, 0, M - 1, 1, 126);
    int y = map(phase[i], -N, N, 62, 32);
    // Draw a pixel at the coordinate
    display.drawPixel(x, y, SSD1306_WHITE);
  }
  // Display the buffer contents on the OLED
  display.display();
}

// Function to fill the buffers with analog input values
void fillBuffer() {
  // Read the analog input values from pins A0 and A1
  int value0 = analogRead(A0);
  int value1 = analogRead(A1);
  // Store the values in the buffers at the current index
  buffer0[index] = value0;
  buffer1[index] = value1;
  // Increment the index and wrap around if necessary
  index++;
  if (index == BUFFER_SIZE) {
    index = 0;
  }
}

// The setup function runs once when you press reset or power the board
void setup() {
  // Initialize serial communication at 9600 bits per second
  Serial.begin(9600);
  // Initialize the Walsh system matrix
  initWalsh();
  // Initialize the Hadamard matrix
  initHadamard();
  // Initialize the OLED display
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { // Address 0x3C for 128x64
    Serial.println("SSD1306 allocation failed");
    for (;;); // Don't proceed, loop forever
  }
  // Clear the display buffer
  display.clearDisplay();
  // Display a welcome message
  display.setTextSize(2); // Draw 2X-scale text
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(10, 10);
  display.println("Hello!");
  display.display(); // Show initial text
  delay(1000); // Pause for 1 second
}

// The loop function runs over and over again forever
void loop() {
  // Fill the buffers with analog input values
  fillBuffer();
  // Compute the frequency vectors for the signals using the Walsh system
  computeFreq(buffer0, freq0);
  computeFreq(buffer1, freq1);
  // Compute the phase vector for the signals using the Hadamard matrix transform
  computePhase(freq0, freq1, phase);
  // Visualize the correlated phase along with frequency on the OLED display
  visualizePhase(freq0, freq1, phase);
  // Wait for the next sampling interval
  delay(1000 / SAMPLE_RATE);
}

