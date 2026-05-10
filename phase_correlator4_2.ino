#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// ── Hardware constants ────────────────────────────────────────────────────────
// Use constexpr to avoid colliding with the Arduino core's A0/A1 macros
static constexpr uint8_t ADC_PIN_0 = 36;
static constexpr uint8_t ADC_PIN_1 = 39;

static constexpr int     BUFFER_SIZE  = 64;
static constexpr int     SCREEN_WIDTH  = 128;
static constexpr int     SCREEN_HEIGHT = 64;
static constexpr uint8_t OLED_ADDR    = 0x3C;

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// ── Buffers ───────────────────────────────────────────────────────────────────
int16_t buffer0[BUFFER_SIZE];
int16_t buffer1[BUFFER_SIZE];

// WHT output: max magnitude = 64 × 1023 = 65472  →  fits int32_t
int32_t wht0[BUFFER_SIZE];
int32_t wht1[BUFFER_SIZE];

// Walsh coefficients are ±1 only → int8_t saves 3× RAM vs int
int8_t walsh[BUFFER_SIZE][BUFFER_SIZE];

// ── Walsh-Hadamard matrix ─────────────────────────────────────────────────────
static void buildWalsh() {
    memset(walsh, 0, sizeof(walsh));
    walsh[0][0] = 1;
    for (int n = 1; n < BUFFER_SIZE; n <<= 1) {
        for (int i = 0; i < n; i++) {
            for (int j = 0; j < n; j++) {
                walsh[i + n][j    ] =  walsh[i][j];   // lower-left quadrant
                walsh[i    ][j + n] =  walsh[i][j];   // upper-right quadrant
                walsh[i + n][j + n] = -walsh[i][j];   // lower-right quadrant (negated)
            }
        }
    }
}

// ── Pearson correlation on time-domain buffers ────────────────────────────────
// Must be computed on raw samples, not WHT coefficients.
static float pearson(const int16_t *a, const int16_t *b, int n) {
    float sa = 0, sb = 0, sab = 0, sa2 = 0, sb2 = 0;
    for (int i = 0; i < n; i++) {
        sa  += a[i];
        sb  += b[i];
        sab += (float)a[i] * b[i];
        sa2 += (float)a[i] * a[i];
        sb2 += (float)b[i] * b[i];
    }
    const float ma = sa / n, mb = sb / n;
    const float num   = sab - n * ma * mb;
    const float denom = sqrtf(fabsf(sa2 - n * ma * ma) *
                              fabsf(sb2 - n * mb * mb));
    return (denom > 0.001f) ? num / denom : 0.0f;
}

// ── Dominant sequency (skip index 0 = DC) ────────────────────────────────────
// Returns the WHT sequency index (zero-crossings count), NOT a frequency in Hz.
static int dominantSequency(const int32_t *wht, int n) {
    int     idx  = 1;
    int32_t best = 0;
    for (int i = 1; i < n; i++) {      // i=0 is DC — always largest, skip it
        int32_t av = abs(wht[i]);
        if (av > best) { best = av; idx = i; }
    }
    return idx;
}

// ── Setup ─────────────────────────────────────────────────────────────────────
void setup() {
    Serial.begin(115200);

    analogReadResolution(10);
    analogSetAttenuation(ADC_11db);

    if (!display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR)) {
        Serial.println("SSD1306 init failed — halting");
        for (;;) {}
    }
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);

    buildWalsh();
}

// ── Main loop ─────────────────────────────────────────────────────────────────
void loop() {
    // Sample both channels synchronously (≈1 kHz with delay(1))
    for (int i = 0; i < BUFFER_SIZE; i++) {
        buffer0[i] = analogRead(ADC_PIN_0);
        buffer1[i] = analogRead(ADC_PIN_1);
        delay(1);
    }

    // Walsh-Hadamard transform (unnormalised)
    for (int i = 0; i < BUFFER_SIZE; i++) {
        int32_t s0 = 0, s1 = 0;
        for (int j = 0; j < BUFFER_SIZE; j++) {
            s0 += (int32_t)buffer0[j] * walsh[i][j];
            s1 += (int32_t)buffer1[j] * walsh[i][j];
        }
        wht0[i] = s0;
        wht1[i] = s1;
    }

    const int   seq0 = dominantSequency(wht0, BUFFER_SIZE);
    const int   seq1 = dominantSequency(wht1, BUFFER_SIZE);
    const float corr = pearson(buffer0, buffer1, BUFFER_SIZE);

    Serial.printf("Seq0: %d  Seq1: %d  Corr: %.3f\n", seq0, seq1, corr);

    // ── OLED ─────────────────────────────────────────────────────────────────
    display.clearDisplay();

    display.setCursor(0, 0);
    display.printf("Seq0: %-3d  Seq1: %-3d", seq0, seq1);
    display.setCursor(0, 10);
    display.printf("Corr: %+.3f", corr);

    // Scatter plot: WHT[ch0] on X, WHT[ch1] on Y
    // Uses actual min/max for robust scaling — no fixed ±131072 assumption
    display.drawLine(0, 31, 127, 31, SSD1306_WHITE);
    display.drawLine(63, 31,  63, 63, SSD1306_WHITE);

    int32_t maxMag = 1;   // avoid division by zero
    for (int i = 0; i < BUFFER_SIZE; i++) {
        maxMag = max(maxMag, max(abs(wht0[i]), abs(wht1[i])));
    }

    for (int i = 0; i < BUFFER_SIZE; i++) {
        // constrain() clamps values that exceed the mapped range
        int x = constrain(map(wht0[i], -maxMag, maxMag,   0, 127),   0, 127);
        int y = constrain(map(wht1[i], -maxMag, maxMag,  63,  31),  31,  63);
        display.drawPixel(x, y, SSD1306_WHITE);
    }

    display.display();
    delay(500);
}
