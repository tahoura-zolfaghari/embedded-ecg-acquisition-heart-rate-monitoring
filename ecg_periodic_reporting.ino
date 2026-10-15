```cpp
// AD8232 ECG - Full Report Version
// Prints a report every 3 seconds

// Pins used for the ECG module and alarm
const int LO_PLUS_PIN = 10;       // Positive lead-off signal
const int LO_MINUS_PIN = 11;      // Negative lead-off signal
const int ECG_PIN = A0;            // ECG signal input
const int ALARM_PIN = 13;          // Alarm output pin

// Variables used to calculate heart rate
unsigned long lastBeatTime = 0;    // Time of the last detected beat
unsigned long rrInterval = 0;      // Time between two beats
int bpm = 0;                       // Current heart rate

int threshold = 550;               // Threshold used to detect ECG peaks
const int minBeatInterval = 300;   // Minimum time between detected beats
bool aboveThreshold = false;       // Checks if the signal is already above the threshold

// Heart rate limits used for status and alarm
const int LOW_BPM_LIMIT = 60;      // Lower heart rate limit
const int HIGH_BPM_LIMIT = 100;    // Upper heart rate limit

// Only BPM values within this range are accepted
const int MIN_VALID_BPM = 50;      // Minimum accepted BPM
const int MAX_VALID_BPM = 130;     // Maximum accepted BPM

// Store recent BPM values to get a smoother average
const int BPM_HISTORY_SIZE = 5;    // Number of values used for averaging
int bpmHistory[BPM_HISTORY_SIZE];  // Stores recent BPM values
int bpmHistoryIndex = 0;           // Position for the next value
int bpmHistoryCount = 0;           // Number of values stored so far
int avgBpm = 0;                    // Average heart rate

// Track the signal range during each 3-second window
int ecgMinWindow = 1023;            // Lowest ECG value in the current window
int ecgMaxWindow = 0;               // Highest ECG value in the current window
String signalQuality = "---";       // Current signal quality result

// Keep track of detected beats and lead disconnections
int beatCountInWindow = 0;          // Beats detected in the current window
int leadOffCountInWindow = 0;       // Lead disconnections during the current window
bool leadCurrentlyOff = false;      // True while a lead is disconnected

// Settings for periodic reports
unsigned long lastReportTime = 0;   // Time of the last report
const unsigned long reportInterval = 3000; // Report interval in milliseconds


void setup() {

  Serial.begin(9600);               // Start serial communication

  pinMode(LO_PLUS_PIN, INPUT);       // Read the positive lead-off signal
  pinMode(LO_MINUS_PIN, INPUT);      // Read the negative lead-off signal

  pinMode(ALARM_PIN, OUTPUT);       // Set the alarm pin as output

  digitalWrite(ALARM_PIN, LOW);     // Keep the alarm off at startup
}


// Decide how to describe the current heart rate
String getHeartRateStatus(int currentBpm) {

  if (currentBpm == 0) return "Unknown";  // No valid BPM available

  else if (currentBpm < LOW_BPM_LIMIT) return "LOW (Bradycardia)";

  else if (currentBpm > HIGH_BPM_LIMIT) return "HIGH (Tachycardia)";

  else return "NORMAL";
}


// Calculate the average using the most recent BPM values
void updateAverageBpm(int newBpm) {

  if (newBpm <= 0) return;          // Ignore invalid values

  bpmHistory[bpmHistoryIndex] = newBpm; // Save the new BPM

  // Move to the next position and start over when the array is full
  bpmHistoryIndex = (bpmHistoryIndex + 1) % BPM_HISTORY_SIZE;

  if (bpmHistoryCount < BPM_HISTORY_SIZE) bpmHistoryCount++; // Update the number of stored values

  // Add up the values currently stored in the array
  long sum = 0;

  for (int i = 0; i < bpmHistoryCount; i++)
    sum += bpmHistory[i];

  avgBpm = sum / bpmHistoryCount;   // Calculate the average
}


// Keep track of the lowest and highest signal values
void trackSignalQuality(int ecgValue) {

  if (ecgValue < ecgMinWindow) ecgMinWindow = ecgValue; // Update the minimum

  if (ecgValue > ecgMaxWindow) ecgMaxWindow = ecgValue; // Update the maximum
}


// Check the signal range and prepare for the next window
void resetSignalQualityWindow() {

  int range = ecgMaxWindow - ecgMinWindow; // Difference between maximum and minimum

  if (range < 15) signalQuality = "Poor (Flat / No Contact)";

  else if (range > 600) signalQuality = "Poor (Noisy / Motion)";

  else if (range < 100) signalQuality = "Weak";

  else signalQuality = "Good";

  // Start collecting values for the next 3-second window
  ecgMinWindow = 1023;
  ecgMaxWindow = 0;
}


// Check whether the current heart rate should trigger the alarm
bool checkAlarm(int currentBpm) {

  if (currentBpm == 0) return false; // No BPM, no alarm

  return (currentBpm < LOW_BPM_LIMIT || currentBpm > HIGH_BPM_LIMIT);
}


// Print the current measurements and status
void printReport(bool alarm) {

  resetSignalQualityWindow();       // Update signal quality for this window

  String status = getHeartRateStatus(bpm); // Get the heart rate status

  Serial.println(F("========== HEART RATE REPORT =========="));

  // Show whether the leads are connected
  if (leadCurrentlyOff || leadOffCountInWindow > 0)
    Serial.println(F("Lead Status     : DISCONNECTED (during this window)"));
  else
    Serial.println(F("Lead Status     : Connected"));

  // Print the latest BPM value
  Serial.print(F("BPM (latest)    : "));
  Serial.println(bpm);

  // Print the average of the stored BPM values
  Serial.print(F("Avg BPM (last "));
  Serial.print(bpmHistoryCount);
  Serial.print(F(")  : "));
  Serial.println(avgBpm);

  // Print the heart rate status
  Serial.print(F("Status          : "));
  Serial.println(status);

  // Show the time between the last two accepted beats
  Serial.print(F("RR Interval     : "));
  Serial.print(rrInterval);
  Serial.println(F(" ms"));

  // Show how many beats were detected in this window
  Serial.print(F("Beats detected  : "));
  Serial.print(beatCountInWindow);
  Serial.println(F(" (in this 3s window)"));

  // Print the estimated signal quality
  Serial.print(F("Signal Quality  : "));
  Serial.println(signalQuality);

  // Show a warning if the heart rate is outside the selected range
  if (alarm)
    Serial.println(F(">>> ALARM: Heart rate out of normal range! <<<"));

  Serial.println(F("========================================="));
  Serial.println();

  // Reset the counters before the next report
  beatCountInWindow = 0;
  leadOffCountInWindow = 0;
}


// Main loop: read the ECG signal and update the report
void loop() {

  unsigned long currentTime = millis(); // Current time since startup

  // Check whether either ECG lead is disconnected
  if ((digitalRead(LO_PLUS_PIN) == 1) || (digitalRead(LO_MINUS_PIN) == 1)) {

    digitalWrite(ALARM_PIN, LOW);     // Turn off the alarm if a lead is disconnected

    // Count the disconnection only when it first happens
    if (!leadCurrentlyOff) {
      leadCurrentlyOff = true;
      leadOffCountInWindow++;
    }

    // Print a short message every 3 seconds while disconnected
    if (currentTime - lastReportTime >= reportInterval) {

      Serial.println(F("========== HEART RATE REPORT =========="));
      Serial.println(F("Lead Status     : DISCONNECTED"));
      Serial.println(F("========================================="));
      Serial.println();

      lastReportTime = currentTime;   // Update the report time
    }

    delay(5);                         // Brief pause before checking again

    return;                           // Skip ECG processing while disconnected
  }

  // Reset the lead status when the connection is restored
  else {

    if (leadCurrentlyOff) leadCurrentlyOff = false;
  }

  int ecg = analogRead(ECG_PIN);      // Read the current ECG sample

  trackSignalQuality(ecg);            // Update the signal range

  // Detect when the signal crosses the threshold
  if (ecg > threshold && !aboveThreshold) {

    // Calculate the interval if a previous beat was detected
    if (lastBeatTime > 0) {

      unsigned long beatInterval = currentTime - lastBeatTime; // Time since the previous beat

      // Ignore detections that are too close together
      if (beatInterval >= minBeatInterval) {

        int calculatedBpm = 60000 / beatInterval; // Convert the interval to BPM

        // Accept only values within the selected BPM range
        if (calculatedBpm >= MIN_VALID_BPM && calculatedBpm <= MAX_VALID_BPM) {

          rrInterval = beatInterval;    // Save the beat interval

          bpm = calculatedBpm;          // Update the current BPM

          updateAverageBpm(bpm);        // Update the average

          beatCountInWindow++;          // Count this accepted beat
        }
      }
    }

    lastBeatTime = currentTime;        // Save the time of this detection

    aboveThreshold = true;             // Prevent repeated detection above the threshold
  }

  // Allow the next peak to be detected once the signal falls below the threshold
  if (ecg < threshold) aboveThreshold = false;

  bool alarm = checkAlarm(bpm);        // Check whether the alarm should be active

  digitalWrite(ALARM_PIN, alarm ? HIGH : LOW); // Update the alarm output

  // Print a report when 3 seconds have passed
  if (currentTime - lastReportTime >= reportInterval) {

    printReport(alarm);               // Print the current report

    lastReportTime = currentTime;     // Save the time of this report
  }

  delay(5);                            // Brief pause before the next loop
}
```