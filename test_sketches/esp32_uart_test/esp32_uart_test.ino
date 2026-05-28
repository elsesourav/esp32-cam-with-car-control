// ==============================================================================
// ESP32-CAM UART & GPIO14 Debug Test
// 
// Purpose: 
// 1. Verify that GPIO14 and GPIO15 are functioning physically.
// 2. Verify that UART communication works without the complex RC car logic.
// ==============================================================================

#include <Arduino.h>

// --- Configuration ---
const int TX2_PIN = 14;
const int RX2_PIN = 15;
const uint32_t BAUD_RATE = 115200;

// --- Timing Variables (Non-blocking) ---
unsigned long lastActionTime = 0;
const unsigned long interval = 1000; // 1 second

// --- State Variables ---
bool testCycle = false;

void setup() {
  // 1. Configure Serial at 115200 for debug logs to the Serial Monitor
  Serial.begin(BAUD_RATE);
  delay(1000); // Give Serial Monitor time to connect
  
  Serial.println("\n\n=======================================");
  Serial.println("   ESP32-CAM UART & GPIO Test Start");
  Serial.println("=======================================");

  // 2. Configure Serial2 (UART2) using TX=GPIO14 and RX=GPIO15
  Serial2.begin(BAUD_RATE, SERIAL_8N1, RX2_PIN, TX2_PIN);
  
  Serial.println("Setup complete. Starting 1-second interval loop...\n");
}

void loop() {
  unsigned long currentMillis = millis();

  // Non-blocking timer: execute every 1 second
  if (currentMillis - lastActionTime >= interval) {
    lastActionTime = currentMillis;

    // Toggle our test cycle state
    testCycle = !testCycle;

    if (testCycle) {
      // ---------------------------------------------------------
      // CYCLE A: UART TEST & IDLE HIGH
      // ---------------------------------------------------------
      // Re-attach the pin to the Hardware UART peripheral.
      // (Necessary because Cycle B detaches it to force it LOW).
      // UART naturally idles HIGH (3.3V), so your multimeter will read ~3.3V here.
      Serial2.begin(BAUD_RATE, SERIAL_8N1, RX2_PIN, TX2_PIN);
      
      Serial.println("[GPIO14] HIGH (UART Idle State)");

      // 3. Send "TEST_UART" through Serial2
      Serial2.println("TEST_UART");
      
      // 6. Print clear logs
      Serial.println("[UART] Sent: TEST_UART");
      
    } else {
      // ---------------------------------------------------------
      // CYCLE B: MULTIMETER LOW TEST
      // ---------------------------------------------------------
      // 5. Toggle GPIO14 LOW for multimeter testing.
      // Calling pinMode/digitalWrite on ESP32 temporarily detaches the pin 
      // from the UART hardware and turns it into a standard GPIO pin.
      pinMode(TX2_PIN, OUTPUT);
      digitalWrite(TX2_PIN, LOW);
      
      // 6. Print clear logs
      Serial.println("[GPIO14] LOW");
    }
    
    Serial.println("---------------------------------------");
  }

  // 4. If data is received from Serial2, print it to Serial Monitor
  // We check this continuously (every loop iteration) to ensure we don't miss data.
  if (Serial2.available()) {
    String incomingData = Serial2.readStringUntil('\n');
    
    // Clean up invisible carriage return characters if present
    incomingData.trim(); 
    
    if (incomingData.length() > 0) {
      // 6. Print clear logs
      Serial.print("[UART] Received: ");
      Serial.println(incomingData);
    }
  }
}
