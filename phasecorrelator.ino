// Include the libraries for ESP32 and OLED display
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// Define the pins for the OLED display
#define OLED_SDA 4
#define OLED_SCL 15
#define OLED_RST 16

// Create an object for the OLED display
Adafruit_SSD1306 display(OLED_SDA, OLED_SCL, OLED_RST);

// Define the pins for the analog inputs
#define ANALOG_IN_0 36
#define ANALOG_IN_1 39

// Define the sample size and the Walsh matrix size
#define SAMPLE_SIZE 64
#define WALSH_SIZE 64

// Define the base frequency in Hz
#define BASE_FREQ 1000

// Define the sampling frequency in Hz
#define SAMPLING_FREQ 8000

// Define the sampling period in microseconds
#define SAMPLING_PERIOD 1000000 / SAMPLING_FREQ

// Define the buffers for the analog inputs
int buffer_0[SAMPLE_SIZE];
int buffer_1[SAMPLE_SIZE];

// Define the buffer for the Walsh matrix
int walsh_matrix[WALSH_SIZE][WALSH_SIZE];

// Define the buffer for the correlated phase
int phase[SAMPLE_SIZE];

// Define the buffer for the frequency spectrum
int freq[SAMPLE_SIZE / 2];

// Define a variable to store the current sample index
int sample_index = 0;

// Define a variable to store the previous micros
unsigned long previous_micros = 0;

// Define a function to generate the Walsh matrix
void generate_walsh_matrix() {
  // Initialize the first row and column with 1
  for (int i = 0; i < WALSH_SIZE; i++) {
    walsh_matrix[0][i] = 1;
    walsh_matrix[i][0] = 1;
  }
  // Generate the rest of the matrix using the recursive formula
  for (int i = 1; i < WALSH_SIZE; i *= 2) {
    for (int j = 0; j < i; j++) {
      for (int k = 0; k < i; k++) {
        walsh_matrix[j + i][k] = walsh_matrix[j][k];
        walsh_matrix[j][k + i] = walsh_matrix[j][k];
        walsh_matrix[j + i][k + i] = -walsh_matrix[j][k];
      }
    }
  }
}

// Define a function to correlate the phase of the base frequency
void correlate_phase() {
  // Loop through the sample size
  for (int i = 0; i < SAMPLE_SIZE; i++) {
    // Initialize the phase to zero
    phase[i] = 0;
    // Loop through the Walsh matrix size
    for (int j = 0; j < WALSH_SIZE; j++) {
      // Multiply the buffers by the Walsh matrix and add to the phase
      phase[i] += buffer_0[i] * walsh_matrix[j][i];
      phase[i] += buffer_1[i] * walsh_matrix[j][i];
    }
    // Normalize the phase by dividing by the Walsh matrix size
    phase[i] /= WALSH_SIZE;
  }
}

// Define a function to calculate the frequency spectrum using FFT
void calculate_frequency() {
  // Loop through the sample size / 2
  for (int i = 0; i < SAMPLE_SIZE / 2; i++) {
    // Initialize the frequency to zero
    freq[i] = 0;
    // Loop through the sample size
    for (int j = 0; j < SAMPLE_SIZE; j++) {
      // Multiply the buffer by the cosine and sine waves and add to the frequency
      freq[i] += buffer_0[j] * cos(2 * PI * i * j / SAMPLE_SIZE);
      freq[i] -= buffer_0[j] * sin(2 * PI * i * j / SAMPLE_SIZE);
    }
    // Normalize the frequency by dividing by the sample size
    freq[i] /= SAMPLE_SIZE;
  }
}

// Define a function to plot the correlated phase and frequency on the OLED display
void plot_on_display() {
  // Clear the display
  display.clearDisplay();
  // Set the text size and color
  display.setTextSize(1);
  display.setTextColor(WHITE);
  // Print the labels for the phase and frequency
  display.setCursor(0, 0);
  display.print("Phase");
  display.setCursor(64, 0);
  display.print("Freq");
  // Loop through the sample size / 2
  for (int i = 0; i < SAMPLE_SIZE / 2; i++) {
    // Map the phase and frequency values to the display height
    int phase_y = map(phase[i], -1024, 1024, 63, 8);
    int freq_y = map(freq[i], 0, 1024, 63, 8);
    // Draw a vertical line for each value
    display.drawFastVLine(i, phase_y, 63 - phase_y, WHITE);
    display.drawFastVLine(i + 64, freq_y, 63 - freq_y, WHITE);
  }
  // Display the result on the OLED
  display.display();
}

// The setup function runs once when you press reset or power the board
void setup() {
  // Initialize serial communication at 9600 bits per second
  Serial.begin(9600);
  // Initialize the OLED display
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  // Generate the Walsh matrix
  generate_walsh_matrix();
}

// The loop function runs over and over again forever
void loop() {
  // Get the current micros
  unsigned long current_micros = micros();
  // Check if the sampling period has passed
  if (current_micros - previous_micros >= SAMPLING_PERIOD) {
    // Update the previous micros
    previous_micros = current_micros;
    // Read the analog inputs and store them in the buffers
    buffer_0[sample_index] = analogRead(ANALOG_IN_0);
    buffer_1[sample_index] = analogRead(ANALOG_IN_1);
    // Increment the sample index
    sample_index++;
    // Check if the sample index has reached the sample size
    if (sample_index == SAMPLE_SIZE) {
      // Reset the sample index
      sample_index = 0;
      // Correlate the phase of the base frequency
      correlate_phase();
      // Calculate the frequency spectrum
      calculate_frequency();
      // Plot the correlated phase and frequency on the OLED display
      plot_on_display();
    }
  }
}
