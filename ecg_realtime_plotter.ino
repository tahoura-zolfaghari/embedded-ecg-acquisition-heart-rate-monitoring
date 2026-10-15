```cpp
// AD8232 ECG - Real-Time Plotter + BPM

// Pins connected to the AD8232 module
const int LO_PLUS_PIN  = 10;  // Positive lead-off detection
const int LO_MINUS_PIN = 11;  // Negative lead-off detection
const int ECG_PIN      = A0;  // Analog ECG signal

// Variables used for heart rate calculation
unsigned long lastBeatTime = 0;  // Time of the last detected beat
int bpm = 0;                     // Last valid BPM value

int threshold = 550;             // Value used to detect a possible beat
const int minBeatInterval = 300; // Ignore detections that are too close

bool aboveThreshold = false;     // Prevents counting the same peak more than once


void setup() {

  Serial.begin(9600);

  pinMode(LO_PLUS_PIN, INPUT);
  pinMode(LO_MINUS_PIN, INPUT);
}


void loop() {

  // Check if either ECG lead is disconnected
  if ((digitalRead(LO_PLUS_PIN) == 1) || (digitalRead(LO_MINUS_PIN) == 1))
  {
    Serial.println("Lead Disconnected");

    delay(5);
    return;
  }

  int ecg = analogRead(ECG_PIN);

  // Get the current time to measure the interval between beats
  unsigned long currentTime = millis();

  // Detect a new beat when the signal crosses the threshold
  if (ecg > threshold && !aboveThreshold) {

    // We need a previous detection to calculate the beat interval
    if (lastBeatTime > 0) {

      // Calculate the time between the current and previous detections
      unsigned long beatInterval = currentTime - lastBeatTime;

      // Ignore very short intervals to reduce false detections
      if (beatInterval >= minBeatInterval) {

        // Convert the time between beats to beats per minute
        int calculatedBPM = 60000 / beatInterval;

        // Keep only BPM values within the selected range
        if (calculatedBPM >= 60 && calculatedBPM <= 140) {
          bpm = calculatedBPM;  // Update BPM only when the value is valid
        }
      }
    }

    // Save the detection time for the next calculation
    lastBeatTime = currentTime;

    // Wait for the signal to drop before detecting another beat
    aboveThreshold = true;
  }

  // Reset the flag when the signal goes below the threshold
  if (ecg < threshold) {
    aboveThreshold = false;
  }

  // Send the ECG value and BPM for real-time plotting
  Serial.print("ECG:");
  Serial.print(ecg);
  Serial.print(",BPM:");
  Serial.println(bpm);

  delay(5);
}
```