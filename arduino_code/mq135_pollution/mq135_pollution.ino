#define MQ135_PIN A0  // Define the analog pin connected to MQ-135
#define RL 10.0       // Load resistance in kΩ

float RO;             // Sensor resistance in clean air (must be calibrated)
const float A = 116.602;   // Predefined constant for CO2 approximation
const float B = -2.769;    // Predefined constant for CO2 approximation

unsigned long startTime;  // Variable to store start time

void setup() {
    Serial.begin(9600);  // Start serial communication
    Serial.println("MQ-135 Air Quality Sensor Readings with Timestamps");

    calibrateMQ135();  // Call calibration function before starting readings

    startTime = millis();  // Record the initial time
    delay(2000); // Allow sensor to stabilize
}

void loop() {
    unsigned long elapsedTime = (millis() - startTime) / 60000;  // Convert to minutes
    unsigned long roundedTime = ((elapsedTime / 15) + 1) * 15;  // Round to the next 15-minute mark

    int rawValue = analogRead(MQ135_PIN);  // Read raw sensor value
    float voltage = (rawValue / 1023.0) * 5.0;  // Convert raw value to voltage

    // Corrected Rs calculation
    float Rs = ((5.0 - voltage) * RL) / voltage;
    float ratio = Rs / RO;  // Corrected Ratio calculation
    float ppm = A * pow(ratio, B);  // Convert to ppm using approximation

    Serial.print("Time (minutes): ");
    Serial.print(roundedTime);
    Serial.print(" | Raw Sensor Value: ");
    Serial.print(rawValue);
    Serial.print(" | Voltage: ");
    Serial.print(voltage, 2);
    Serial.print("V | Estimated CO2: ");
    Serial.print(ppm, 2);
    Serial.println(" ppm");

    delay(900000); // Wait for 15 minutes before next reading (900,000 ms)
}

// Calibration function
void calibrateMQ135() {
    Serial.println("Calibrating MQ-135...");
    float val = 0;
    for (int i = 0; i < 500; i++) {
        val += analogRead(MQ135_PIN);
        delay(10);
    }
    val = val / 500.0;
    float voltage = (val / 1023.0) * 5.0;
    float Rs = ((5.0 - voltage) * RL) / voltage;
    RO = Rs / pow(400.0 / A, 1.0 / B);  // 400 ppm standard for fresh air

    Serial.print("Calibrated Ro: ");
    Serial.println(RO);
}
