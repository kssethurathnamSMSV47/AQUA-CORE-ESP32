// ================================================================
// LAM RESEARCH CHALLENGE 3.0
// CASE 1 + CASE 2 DEDICATED SKETCH
//
// Dedicated Case 1/Case 2 use; inherited valve routines remain available
//
// FEATURES:
// 1. Physical ON/OFF button toggles system
// 2. Case 1 automatic operation
// 3. Case 1 float-switch control
// 4. Automatic excess/recovery volume calculation
// 5. Case 2 flush operation
// 6. FT-01 flow measurement and totalization
// 7. Ball-valve characterization
// 8. STOP command stops + immediately generates report
// 9. Physical button stop also stops + immediately generates report
// 10. REPORT command available at any time
// 11. No continuous FT-01 output while system is OFF
// 12. Ball-valve measurements are reported only when actually measured
// 13. Manual STOP locks system until physical button is pressed again
// ================================================================

// ---------------- PUMP 1 / U2 ----------------

#define P1_RPWM 33
#define P1_LPWM 32
#define P1_REN  26
#define P1_LEN  13


// ---------------- PUMP 2 / U3 ----------------

#define P2_RPWM 19
#define P2_LPWM 18
#define P2_REN  17
#define P2_LEN  16


// ---------------- PUMP 3 / U4 ----------------

#define P3_RPWM 25
#define P3_LPWM 23
#define P3_REN  22
#define P3_LEN  21


// ---------------- OTHER INTERFACES ----------------

#define FLOAT_PIN 35

#define FLOW_PIN  14

#define ONOFF_PIN 34

#define SOLENOID_PIN 27


// ================================================================
// SOLENOID LOGIC
// ================================================================

#define SOLENOID_OPEN_LEVEL   HIGH
#define SOLENOID_CLOSED_LEVEL LOW


// ================================================================
// PWM CONFIGURATION
// ================================================================

#define PWM_FREQ 10000
#define PWM_RES  8


// ================================================================
// PUMP DUTY CYCLES
// ================================================================

#define PUMP1_DUTY 128      // 50%
#define PUMP2_DUTY 255      // 100%
#define PUMP3_DUTY 255      // 100%

const float CAL_A = 0.02420192;
const float CAL_B = 0.02599480;


// ================================================================
// TIMING PARAMETERS
// ================================================================

const unsigned long CASE1_STABLE_BUFFER_MS = 5000;

const unsigned long CASE2_STARTUP_BUFFER_MS = 2000;

const float CASE2_LOW_FLOW_THRESHOLD = 0.35;

const unsigned long SOLENOID_OPEN_DELAY_MS = 1000;

const unsigned long SOLENOID_CLOSE_DELAY_MS = 500;

const unsigned long VALVE_STABILIZATION_MS = 4000;

const unsigned long VALVE_AVERAGING_MS = 5000;

const unsigned long DEBOUNCE_TIME = 50;


// ================================================================
// FLOW VARIABLES
// ================================================================

volatile unsigned long pulseCount = 0;

unsigned long previousFlowMillis = 0;

float currentFlowRate = 0.0;

float totalVolume = 0.0;


// ================================================================
// CASE 1 VARIABLES
// ================================================================

unsigned long case1StartTime = 0;

float normalFlowSum = 0.0;

unsigned long normalFlowSamples = 0;

float normalFlowAverage = 0.0;


// ================================================================
// EXCESS / RECOVERY VARIABLES
// ================================================================

bool excessFlowEvent = false;

float excessVolume = 0.0;

float totalExcessVolume = 0.0;

float eventStartVolume = 0.0;

unsigned long excessStartTime = 0;

unsigned long excessDuration = 0;

float excessFlowSum = 0.0;

unsigned long excessFlowSamples = 0;

float excessAverageFlow = 0.0;


// ================================================================
// CASE 2 VARIABLES
// ================================================================

unsigned long case2StartTime = 0;

bool case2ShutdownTriggered = false;


// ================================================================
// SYSTEM MODE
// ================================================================

enum SystemMode
{
  MODE_OFF,
  MODE_CASE1,
  MODE_CASE2
};

SystemMode systemMode = MODE_OFF;


// ================================================================
// MANUAL STOP LOCKOUT
// ================================================================

bool manualStopLockout = false;


// ================================================================
// FLOAT SWITCH STATE
// ================================================================

bool lastFloatTriggered = false;


// ================================================================
// PHYSICAL BUTTON VARIABLES
// ================================================================

bool lastButtonState = HIGH;

bool stableButtonState = HIGH;

unsigned long lastDebounceTime = 0;


// ================================================================
// BALL VALVE MEASUREMENT VARIABLES
// ================================================================

bool waitingForValvePosition = false;

bool valveMeasurementActive = false;

int selectedValvePosition = 0;

unsigned long valveStabilizationStart = 0;

unsigned long valveMeasurementStart = 0;

float valveFlowSum = 0.0;

unsigned long valveFlowSamples = 0;


// ================================================================
// BALL VALVE RESULTS
// ================================================================

float flow25 = 0.0;

float flow50 = 0.0;

float flow75 = 0.0;

float flow100 = 0.0;


// Measurement validity flags

bool valve25Measured = false;

bool valve50Measured = false;

bool valve75Measured = false;

bool valve100Measured = false;


// ================================================================
// OPERATING FLOW RANGE
// ================================================================

float minimumOperatingFlow = 9999.0;

float maximumOperatingFlow = 0.0;

// ================================================================
// FLOW SENSOR INTERRUPT
// ================================================================

void IRAM_ATTR flowPulse()
{
  pulseCount++;
}

// ================================================================
// SOLENOID CONTROL
// ================================================================

void openSolenoid()
{
  digitalWrite(
    SOLENOID_PIN,
    SOLENOID_OPEN_LEVEL
  );

  Serial.println(
    "SOLENOID: OPEN SIGNAL SENT"
  );
}

void closeSolenoid()
{
  digitalWrite(
    SOLENOID_PIN,
    SOLENOID_CLOSED_LEVEL
  );

  Serial.println(
    "SOLENOID: CLOSE SIGNAL SENT"
  );
}

// ================================================================
// PUMP 1
// ================================================================

void stopPump1()
{
  ledcWrite(P1_RPWM, 0);

  ledcWrite(P1_LPWM, 0);

  digitalWrite(P1_REN, LOW);

  digitalWrite(P1_LEN, LOW);
}


void startPump1()
{
  digitalWrite(P1_REN, HIGH);

  digitalWrite(P1_LEN, HIGH);

  ledcWrite(P1_LPWM, 0);

  ledcWrite(P1_RPWM, PUMP1_DUTY);
}


// ================================================================
// PUMP 2
// ================================================================

void stopPump2()
{
  ledcWrite(P2_RPWM, 0);

  ledcWrite(P2_LPWM, 0);

  digitalWrite(P2_REN, LOW);

  digitalWrite(P2_LEN, LOW);
}


void startPump2()
{
  digitalWrite(P2_REN, HIGH);

  digitalWrite(P2_LEN, HIGH);

  ledcWrite(P2_RPWM, 0);

  ledcWrite(P2_LPWM, PUMP2_DUTY);
}

// ================================================================
// PUMP 3
// ================================================================

void stopPump3()
{
  ledcWrite(P3_RPWM, 0);

  ledcWrite(P3_LPWM, 0);

  digitalWrite(P3_REN, LOW);

  digitalWrite(P3_LEN, LOW);
}

void startPump3()
{
  digitalWrite(P3_REN, HIGH);

  digitalWrite(P3_LEN, HIGH);

  ledcWrite(P3_LPWM, 0);

  ledcWrite(P3_RPWM, PUMP3_DUTY);
}

// ================================================================
// STOP ALL PUMPS
// ================================================================

void stopAllPumps()
{
  stopPump1();

  stopPump2();

  stopPump3();
}

// ================================================================
// RESET FLOW WINDOW
// ================================================================

void resetFlowWindow()
{
  noInterrupts();

  pulseCount = 0;

  interrupts();

  previousFlowMillis = millis();
}

// ================================================================
// FLOAT SWITCH
// LOW = TRIGGERED
// ================================================================

bool isFloatTriggered()
{
  return digitalRead(FLOAT_PIN) == LOW;
}

// ================================================================
// START CASE 1
// ================================================================

void startCase1()
{
  Serial.println();

  Serial.println(
    "=========================================="
  );

  Serial.println(
    "STARTING CASE 1"
  );

  Serial.println(
    "=========================================="
  );


  manualStopLockout = false;

  systemMode = MODE_CASE1;

  case1StartTime = millis();

  resetFlowWindow();


  // Reset Case 1 measurements

  normalFlowSum = 0.0;

  normalFlowSamples = 0;

  normalFlowAverage = 0.0;


  // Reset excess measurement

  excessFlowEvent = false;

  excessVolume = 0.0;

  totalExcessVolume = 0.0;

  eventStartVolume = 0.0;

  excessDuration = 0;

  excessFlowSum = 0.0;

  excessFlowSamples = 0;

  excessAverageFlow = 0.0;


  // Reset total volume

  totalVolume = 0.0;


  // Reset operating flow range

  minimumOperatingFlow = 9999.0;

  maximumOperatingFlow = 0.0;


  lastFloatTriggered =
    isFloatTriggered();


  // ------------------------------------------------
  // OPEN PROCESS TANK SOLENOID
  // ------------------------------------------------

  Serial.println();

  Serial.println(
    "Opening process tank solenoid..."
  );

  openSolenoid();

  delay(
    SOLENOID_OPEN_DELAY_MS
  );

  Serial.println(
    "Solenoid open command completed."
  );


  // ------------------------------------------------
  // START P2
  // ------------------------------------------------

  Serial.println();

  Serial.println(
    "Starting P2 @ 100%..."
  );

  startPump2();

  delay(300);


  // ------------------------------------------------
  // CHECK FLOAT
  // ------------------------------------------------

  if (!isFloatTriggered())
  {
    Serial.println(
      "Float NORMAL."
    );

    Serial.println(
      "Starting P1 @ 50%..."
    );

    startPump1();
  }
  else
  {
    Serial.println(
      "Float TRIGGERED."
    );

    Serial.println(
      "Closing solenoid."
    );

    closeSolenoid();

    Serial.println(
      "Starting P1 @ 50%..."
    );

    startPump1();
  }


  // P3 OFF in Case 1

  stopPump3();


  Serial.println();

  Serial.println(
    "------------------------------------------"
  );

  Serial.println(
    "CASE 1 ACTIVE"
  );

  Serial.println(
    "------------------------------------------"
  );

  Serial.println(
    "SOLENOID = OPEN"
  );

  Serial.println(
    "P1 = 50% when float normal"
  );

  Serial.println(
    "P2 = 100%"
  );

  Serial.println(
    "P3 = OFF"
  );

  Serial.println();

  Serial.println(
    "Float control:"
  );

  Serial.println(
    "NORMAL  -> P1 ON"
  );

  Serial.println(
    "TRIGGER -> SOLENOID CLOSED, P1/P2 CONTINUE"
  );

  Serial.println(
    "RETURN  -> SOLENOID OPENS"
  );

  Serial.println();

  Serial.println(
    "FT-01 measurement ACTIVE"
  );

  Serial.println(
    "------------------------------------------"
  );
}


// ================================================================
// START CASE 2
// ================================================================

void startCase2()
{
  if (manualStopLockout)
  {
    Serial.println();

    Serial.println(
      "=========================================="
    );

    Serial.println(
      "CASE 2 BLOCKED"
    );

    Serial.println(
      "=========================================="
    );

    Serial.println(
      "System was manually stopped."
    );

    Serial.println(
      "Press physical ON/OFF button"
    );

    Serial.println(
      "to re-arm system."
    );

    Serial.println(
      "=========================================="
    );

    return;
  }


  Serial.println();

  Serial.println(
    "STARTING CASE 2 - FLUSH SEQUENCE"
  );

  Serial.println(
    "=========================================="
  );


  // Close process tank solenoid

  Serial.println(
    "Closing process tank solenoid..."
  );

  closeSolenoid();

  delay(
    SOLENOID_CLOSE_DELAY_MS
  );

  Serial.println(
    "Solenoid CLOSED."
  );


  systemMode = MODE_CASE2;

  case2StartTime = millis();

  case2ShutdownTriggered = false;

  resetFlowWindow();


  // ------------------------------------------------
  // START CASE 2 PUMPS
  // ------------------------------------------------

  Serial.println(
    "Starting P1 @ 50%..."
  );

  startPump1();


  Serial.println(
    "Starting P2 @ 100%..."
  );

  startPump2();


  Serial.println(
    "Starting P3 @ 100%..."
  );

  startPump3();


  Serial.println();

  Serial.println(
    "------------------------------------------"
  );

  Serial.println(
    "CASE 2 FLUSH ACTIVE"
  );

  Serial.println(
    "------------------------------------------"
  );

  Serial.println(
    "SOLENOID = CLOSED"
  );

  Serial.println(
    "P1 = ON @ 50%"
  );

  Serial.println(
    "P2 = ON @ 100%"
  );

  Serial.println(
    "P3 = ON @ 100%"
  );

  Serial.println();

  Serial.println(
    "FT-01 startup buffer = 2 seconds"
  );

  Serial.println(
    "After buffer:"
  );

  Serial.println(
    "FLOW < 0.35 L/min -> SHUTDOWN"
  );

  Serial.println(
    "------------------------------------------"
  );
}


// ================================================================
// CASE 2 SHUTDOWN
// ================================================================

void shutdownCase2(
  const char *reason
)
{
  if (case2ShutdownTriggered)
    return;


  case2ShutdownTriggered = true;


  Serial.println();

  Serial.println(
    "=========================================="
  );

  Serial.println(
    "CASE 2 SHUTDOWN"
  );

  Serial.println(
    "=========================================="
  );


  Serial.print(
    "Reason: "
  );

  Serial.println(
    reason
  );


  Serial.print(
    "Final FT-01 flow: "
  );

  Serial.print(
    currentFlowRate,
    3
  );

  Serial.println(
    " L/min"
  );


  stopAllPumps();

  closeSolenoid();


  systemMode = MODE_OFF;


  Serial.println();

  Serial.println(
    "P1 = OFF"
  );

  Serial.println(
    "P2 = OFF"
  );

  Serial.println(
    "P3 = OFF"
  );

  Serial.println(
    "SOLENOID = CLOSED"
  );

  Serial.println(
    "SYSTEM = OFF"
  );

  Serial.println();

  Serial.println(
    "Generating final report..."
  );


  printFinalReport();


  Serial.println(
    "=========================================="
  );
}


// ================================================================
// MANUAL STOP
// ================================================================
//
// This function is called by:
// 1. Physical ON/OFF button while running
// 2. STOP serial command
//
// It immediately:
// - finalizes excess measurement
// - stops all pumps
// - closes solenoid
// - locks system
// - prints report
// ================================================================

void stopSystem()
{
  Serial.println();

  Serial.println(
    "=========================================="
  );

  Serial.println(
    "MANUAL SYSTEM STOP"
  );

  Serial.println(
    "=========================================="
  );


  // ------------------------------------------------
  // FINALIZE ACTIVE EXCESS EVENT
  // ------------------------------------------------

  if (excessFlowEvent)
  {
    Serial.print("EVENT,FLOAT_NORMAL,");
    Serial.println(millis());
    excessFlowEvent = false;

    excessDuration =
      millis() -
      excessStartTime;


    excessVolume =
      totalVolume -
      eventStartVolume;


    if (excessVolume < 0)
      excessVolume = 0;


    totalExcessVolume +=
      excessVolume;


    if (excessFlowSamples > 0)
    {
      excessAverageFlow =
        excessFlowSum /
        excessFlowSamples;
    }
    else
    {
      excessAverageFlow = 0.0;
    }


    Serial.println();

    Serial.println(
      "------------------------------------------"
    );

    Serial.println(
      "EXCESS / RECOVERY EVENT FINALIZED"
    );

    Serial.println(
      "------------------------------------------"
    );


    Serial.print(
      "Excess duration = "
    );

    Serial.print(
      excessDuration / 1000.0,
      2
    );

    Serial.println(
      " s"
    );


    Serial.print(
      "Excess average flow = "
    );

    Serial.print(
      excessAverageFlow,
      3
    );

    Serial.println(
      " L/min"
    );


    Serial.print(
      "Excess volume = "
    );

    Serial.print(
      excessVolume,
      3
    );

    Serial.println(
      " L"
    );


    Serial.print(
      "Total excess volume = "
    );

    Serial.print(
      totalExcessVolume,
      3
    );

    Serial.println(
      " L"
    );


    Serial.println(
      "------------------------------------------"
    );
  }


  // ------------------------------------------------
  // STOP PHYSICAL OUTPUTS
  // ------------------------------------------------

  stopAllPumps();

  closeSolenoid();


  // ------------------------------------------------
  // STOP BALL VALVE MEASUREMENT
  // ------------------------------------------------

  waitingForValvePosition = false;

  valveMeasurementActive = false;

  selectedValvePosition = 0;

  valveMeasurementStart = 0;


  // ------------------------------------------------
  // SYSTEM OFF + LOCKED
  // ------------------------------------------------

  systemMode = MODE_OFF;

  manualStopLockout = true;


  Serial.println();

  Serial.println(
    "P1 = OFF"
  );

  Serial.println(
    "P2 = OFF"
  );

  Serial.println(
    "P3 = OFF"
  );

  Serial.println(
    "SOLENOID = CLOSED"
  );

  Serial.println(
    "SYSTEM = OFF"
  );

  Serial.println(
    "SYSTEM LOCKED."
  );


  // ------------------------------------------------
  // IMMEDIATE REPORT
  // ------------------------------------------------

  Serial.println();

  Serial.println(
    "Generating current system report..."
  );

  Serial.println();


  printFinalReport();


  Serial.println();

  Serial.println(
    "=========================================="
  );

  Serial.println(
    "SYSTEM STOP COMPLETE"
  );

  Serial.println(
    "=========================================="
  );


  Serial.println();

  Serial.println(
    "CASE1 and CASE2 commands are BLOCKED."
  );

  Serial.println(
    "Press physical ON/OFF switch"
  );

  Serial.println(
    "to re-arm and start Case 1."
  );

  Serial.println(
    "=========================================="
  );
}


// ================================================================
// CASE 1 FLOAT CONTROL
// ================================================================

void updateCase1FloatControl()
{
  if (systemMode != MODE_CASE1)
    return;


  bool floatTriggered =
    isFloatTriggered();


  // ------------------------------------------------
  // FLOAT JUST TRIGGERED
  // ------------------------------------------------

  if (
    floatTriggered &&
    !lastFloatTriggered
  )
  {
    Serial.println();

    Serial.println(
      "------------------------------------------"
    );

    Serial.println(
      "FLOAT SWITCH TRIGGERED"
    );

    Serial.println(
      "------------------------------------------"
    );

    Serial.println(
      "SOLENOID -> CLOSED"
    );

    Serial.println(
      "P1 -> CONTINUES @ 50%"
    );

    Serial.println(
      "P2 -> CONTINUES @ 100%"
    );


    // Machine-readable event boundary for Python volume integration
    Serial.print("EVENT,FLOAT_TRIGGER,");
    Serial.println(millis());

    // Close solenoid
    closeSolenoid();

    // Start excess measurement

    excessFlowEvent = true;

    eventStartVolume =
      totalVolume;

    excessStartTime =
      millis();

    excessFlowSum = 0.0;

    excessFlowSamples = 0;

    excessVolume = 0.0;


    Serial.println();

    Serial.println(
      "RECOVERY / EXCESS FLOW MEASUREMENT STARTED"
    );

    Serial.println(
      "------------------------------------------"
    );
  }


  // ------------------------------------------------
  // FLOAT REMAINS TRIGGERED
  // ------------------------------------------------

  if (floatTriggered)
  {
    closeSolenoid();

    startPump1();

    startPump2();
  }


  // ------------------------------------------------
  // FLOAT RETURNS TO NORMAL
  // ------------------------------------------------

  if (
    !floatTriggered &&
    lastFloatTriggered
  )
  {
    Serial.println();

    Serial.println(
      "------------------------------------------"
    );

    Serial.println(
      "FLOAT SWITCH RETURNED TO NORMAL"
    );

    Serial.println(
      "------------------------------------------"
    );


    if (excessFlowEvent)
    {
      Serial.print("EVENT,FLOAT_NORMAL,");
      Serial.println(millis());
      excessFlowEvent = false;


      excessDuration =
        millis() -
        excessStartTime;


      excessVolume =
        totalVolume -
        eventStartVolume;


      if (excessVolume < 0)
        excessVolume = 0;


      totalExcessVolume +=
        excessVolume;


      if (excessFlowSamples > 0)
      {
        excessAverageFlow =
          excessFlowSum /
          excessFlowSamples;
      }
      else
      {
        excessAverageFlow = 0.0;
      }


      Serial.print(
        "Excess duration = "
      );

      Serial.print(
        excessDuration / 1000.0,
        2
      );

      Serial.println(
        " s"
      );


      Serial.print(
        "Excess average flow = "
      );

      Serial.print(
        excessAverageFlow,
        3
      );

      Serial.println(
        " L/min"
      );


      Serial.print(
        "Excess volume = "
      );

      Serial.print(
        excessVolume,
        3
      );

      Serial.println(
        " L"
      );


      Serial.print(
        "Total excess volume = "
      );

      Serial.print(
        totalExcessVolume,
        3
      );

      Serial.println(
        " L"
      );


      Serial.println(
        "------------------------------------------"
      );
    }


    // Re-open solenoid

    openSolenoid();


    Serial.println(
      "SOLENOID -> OPEN"
    );


    startPump1();

    startPump2();


    Serial.println(
      "P1 -> CONTINUES @ 50%"
    );

    Serial.println(
      "P2 -> CONTINUES @ 100%"
    );


    Serial.println(
      "------------------------------------------"
    );
  }


  lastFloatTriggered =
    floatTriggered;
}


// ================================================================
// CASE 2 FLOAT CONTROL
// ================================================================

void updateCase2FloatControl()
{
  if (systemMode != MODE_CASE2)
    return;


  if (isFloatTriggered())
  {
    shutdownCase2(
      "FLOAT SWITCH TRIGGERED DURING CASE 2"
    );
  }
}


// ================================================================
// FT-01 FLOW MEASUREMENT
// ================================================================
//
// IMPORTANT:
// When systemMode == MODE_OFF,
// the function does NOT print continuous
// FT-01 information.
//
// REPORT still gives stored measurements.
// ================================================================

void updateFlowMeasurement()
{
  unsigned long currentMillis =
    millis();


  if (
    currentMillis -
    previousFlowMillis <
    1000
  )
  {
    return;
  }


  unsigned long elapsedMillis =
    currentMillis -
    previousFlowMillis;


  previousFlowMillis =
    currentMillis;


  noInterrupts();

  unsigned long pulses =
    pulseCount;

  pulseCount = 0;

  interrupts();


  float elapsedSeconds =
    elapsedMillis / 1000.0;


  float frequency =
    pulses / elapsedSeconds;


  if (pulses > 0)
  {
    currentFlowRate =
      CAL_A * frequency +
      CAL_B;


    if (currentFlowRate < 0)
      currentFlowRate = 0;
  }
  else
  {
    currentFlowRate = 0;
  }


  // ------------------------------------------------
  // SYSTEM OFF
  // ------------------------------------------------

  if (systemMode == MODE_OFF)
  {
    return;
  }


  // ------------------------------------------------
  // VOLUME
  // ------------------------------------------------

  float volumeThisInterval =
    currentFlowRate *
    elapsedSeconds /
    60.0;


  totalVolume +=
    volumeThisInterval;

  // Machine-readable row consumed by lam_stage2_logger.py.
  // Fields: DATA,millis,mode,pulses,frequency_hz,flow_l_min,total_volume_l,float,event,solenoid
  Serial.print("DATA,");
  Serial.print(currentMillis);
  Serial.print(",");
  if (systemMode == MODE_CASE1) Serial.print("CASE1");
  else if (systemMode == MODE_CASE2) Serial.print("CASE2");
  else Serial.print("OFF");
  Serial.print(",");
  Serial.print(pulses);
  Serial.print(",");
  Serial.print(frequency, 4);
  Serial.print(",");
  Serial.print(currentFlowRate, 5);
  Serial.print(",");
  Serial.print(totalVolume, 6);
  Serial.print(",");
  Serial.print(isFloatTriggered() ? 1 : 0);
  Serial.print(",");
  Serial.print(excessFlowEvent ? 1 : 0);
  Serial.print(",");
  Serial.println(digitalRead(SOLENOID_PIN) == SOLENOID_OPEN_LEVEL ? 1 : 0);


  // ------------------------------------------------
  // OPERATING FLOW RANGE
  // ------------------------------------------------

  if (currentFlowRate > 0)
  {
    if (
      currentFlowRate <
      minimumOperatingFlow
    )
    {
      minimumOperatingFlow =
        currentFlowRate;
    }


    if (
      currentFlowRate >
      maximumOperatingFlow
    )
    {
      maximumOperatingFlow =
        currentFlowRate;
    }
  }


  // ------------------------------------------------
  // CASE 1 MEASUREMENT
  // ------------------------------------------------

  if (systemMode == MODE_CASE1)
  {
    unsigned long case1Elapsed =
      currentMillis -
      case1StartTime;


    // Normal flow averaging

    if (
      !isFloatTriggered() &&
      !excessFlowEvent &&
      case1Elapsed >=
      CASE1_STABLE_BUFFER_MS
    )
    {
      normalFlowSum +=
        currentFlowRate;


      normalFlowSamples++;


      if (normalFlowSamples > 0)
      {
        normalFlowAverage =
          normalFlowSum /
          normalFlowSamples;
      }
    }


    // Excess flow averaging

    if (excessFlowEvent)
    {
      excessFlowSum +=
        currentFlowRate;


      excessFlowSamples++;


      excessVolume =
        totalVolume -
        eventStartVolume;
    }
  }


  // ------------------------------------------------
  // LIVE FT-01 DISPLAY
  // ------------------------------------------------

  Serial.println();

  Serial.println(
    "--------------- FT-01 ----------------"
  );


  Serial.print(
    "Pulses       : "
  );

  Serial.println(
    pulses
  );


  Serial.print(
    "Frequency    : "
  );

  Serial.print(
    frequency,
    3
  );

  Serial.println(
    " Hz"
  );


  Serial.print(
    "Flow rate    : "
  );

  Serial.print(
    currentFlowRate,
    3
  );

  Serial.println(
    " L/min"
  );


  Serial.print(
    "Total volume : "
  );

  Serial.print(
    totalVolume,
    3
  );

  Serial.println(
    " L"
  );


  if (systemMode == MODE_CASE1)
  {
    Serial.print(
      "Normal avg   : "
    );

    Serial.print(
      normalFlowAverage,
      3
    );

    Serial.println(
      " L/min"
    );


    if (excessFlowEvent)
    {
      Serial.print(
        "Excess volume: "
      );

      Serial.print(
        excessVolume,
        3
      );

      Serial.println(
        " L"
      );
    }
  }


  Serial.println(
    "---------------------------------------"
  );


  // ------------------------------------------------
  // CASE 2 FLOW LOGIC
  // ------------------------------------------------

  if (systemMode == MODE_CASE2)
  {
    unsigned long case2Elapsed =
      currentMillis -
      case2StartTime;


    // Startup buffer

    if (
      case2Elapsed <
      CASE2_STARTUP_BUFFER_MS
    )
    {
      unsigned long remaining =
        CASE2_STARTUP_BUFFER_MS -
        case2Elapsed;


      Serial.print(
        "CASE 2 startup buffer: "
      );


      Serial.print(
        remaining / 1000.0,
        1
      );


      Serial.println(
        " s remaining"
      );


      return;
    }


    // Low-flow shutdown

    if (
      currentFlowRate <
      CASE2_LOW_FLOW_THRESHOLD
    )
    {
      Serial.println();

      Serial.println(
        "!!! CASE 2 LOW FLOW !!!"
      );


      Serial.print(
        "Measured flow = "
      );


      Serial.print(
        currentFlowRate,
        3
      );


      Serial.println(
        " L/min"
      );


      Serial.print(
        "Shutdown threshold = "
      );


      Serial.print(
        CASE2_LOW_FLOW_THRESHOLD,
        2
      );


      Serial.println(
        " L/min"
      );


      shutdownCase2(
        "FT-01 FLOW BELOW 0.35 L/min"
      );


      return;
    }


    Serial.println(
      "CASE 2: FLOW ABOVE 0.35 L/min"
    );
  }
}


// ================================================================
// START BALL VALVE MEASUREMENT
// ================================================================

void startValveMeasurement(
  int position
)
{
  selectedValvePosition =
    position;


  waitingForValvePosition =
    false;


  valveMeasurementActive =
    true;


  valveStabilizationStart =
    millis();


  valveMeasurementStart =
    0;


  valveFlowSum =
    0.0;


  valveFlowSamples =
    0;


  Serial.println();

  Serial.println(
    "=========================================="
  );


  Serial.print(
    "BALL VALVE POSITION = "
  );


  Serial.print(
    position
  );


  Serial.println(
    "%"
  );


  Serial.println(
    "=========================================="
  );


  Serial.println(
    "Stabilizing for 4 seconds..."
  );
}


// ================================================================
// UPDATE BALL VALVE MEASUREMENT
// ================================================================

void updateValveMeasurement()
{
  if (!valveMeasurementActive)
    return;


  unsigned long now =
    millis();


  // ------------------------------------------------
  // STABILIZATION
  // ------------------------------------------------

  if (
    now -
    valveStabilizationStart <
    VALVE_STABILIZATION_MS
  )
  {
    return;
  }


  // ------------------------------------------------
  // START AVERAGING
  // ------------------------------------------------

  if (valveMeasurementStart == 0)
  {
    valveMeasurementStart =
      now;


    valveFlowSum =
      0.0;


    valveFlowSamples =
      0;


    Serial.println(
      "Stabilization complete."
    );


    Serial.println(
      "Averaging flow for 5 seconds..."
    );


    return;
  }


  // ------------------------------------------------
  // AVERAGING
  // ------------------------------------------------

  if (
    now -
    valveMeasurementStart <
    VALVE_AVERAGING_MS
  )
  {
    valveFlowSum +=
      currentFlowRate;


    valveFlowSamples++;


    return;
  }


  // ------------------------------------------------
  // CALCULATE RESULT
  // ------------------------------------------------

  float averageFlow =
    0.0;


  if (valveFlowSamples > 0)
  {
    averageFlow =
      valveFlowSum /
      valveFlowSamples;
  }


  // ------------------------------------------------
  // SAVE RESULT
  // ------------------------------------------------

  if (selectedValvePosition == 25)
  {
    flow25 =
      averageFlow;

    valve25Measured =
      true;
  }
  else if (selectedValvePosition == 50)
  {
    flow50 =
      averageFlow;

    valve50Measured =
      true;
  }
  else if (selectedValvePosition == 75)
  {
    flow75 =
      averageFlow;

    valve75Measured =
      true;
  }
  else if (selectedValvePosition == 100)
  {
    flow100 =
      averageFlow;

    valve100Measured =
      true;
  }


  Serial.println();

  Serial.println(
    "------------------------------------------"
  );


  Serial.print(
    "BALL VALVE "
  );


  Serial.print(
    selectedValvePosition
  );


  Serial.println(
    "% RESULT"
  );


  Serial.print(
    "Stable flow = "
  );


  Serial.print(
    averageFlow,
    3
  );


  Serial.println(
    " L/min"
  );


  Serial.println(
    "------------------------------------------"
  );


  valveMeasurementActive =
    false;


  valveMeasurementStart =
    0;


  selectedValvePosition =
    0;
}


// ================================================================
// PHYSICAL ON/OFF BUTTON
// ================================================================
//
// OFF + button:
//     Start Case 1
//
// RUNNING + button:
//     Stop system + report
// ================================================================

void checkStartButton()
{
  bool reading =
    digitalRead(
      ONOFF_PIN
    );


  // Debounce

  if (
    reading !=
    lastButtonState
  )
  {
    lastDebounceTime =
      millis();
  }


  if (
    millis() -
    lastDebounceTime >
    DEBOUNCE_TIME
  )
  {
    if (
      reading !=
      stableButtonState
    )
    {
      stableButtonState =
        reading;


      // Button pressed

      if (
        stableButtonState ==
        LOW
      )
      {
        Serial.println();

        Serial.println(
          "PHYSICAL ON/OFF BUTTON PRESSED"
        );


        // ----------------------------------------
        // SYSTEM OFF
        // ----------------------------------------

        if (
          systemMode ==
          MODE_OFF
        )
        {
          manualStopLockout =
            false;


          startCase1();
        }


        // ----------------------------------------
        // SYSTEM RUNNING
        // ----------------------------------------

        else
        {
          stopSystem();
        }
      }
    }
  }


  lastButtonState =
    reading;
}


// ================================================================
// STATUS
// ================================================================

void printStatus()
{
  Serial.println();

  Serial.println(
    "=========================================="
  );

  Serial.println(
    "SYSTEM STATUS"
  );

  Serial.println(
    "=========================================="
  );


  Serial.print(
    "Mode: "
  );


  if (
    systemMode ==
    MODE_OFF
  )
  {
    Serial.println(
      "OFF"
    );
  }
  else if (
    systemMode ==
    MODE_CASE1
  )
  {
    Serial.println(
      "CASE 1"
    );
  }
  else if (
    systemMode ==
    MODE_CASE2
  )
  {
    Serial.println(
      "CASE 2"
    );
  }


  Serial.print(
    "STOP Lockout: "
  );


  if (manualStopLockout)
    Serial.println("ACTIVE");
  else
    Serial.println("NO");


  Serial.print(
    "Solenoid command: "
  );


  if (
    digitalRead(
      SOLENOID_PIN
    ) ==
    SOLENOID_OPEN_LEVEL
  )
  {
    Serial.println(
      "OPEN"
    );
  }
  else
  {
    Serial.println(
      "CLOSED"
    );
  }


  Serial.print(
    "Float: "
  );


  if (isFloatTriggered())
    Serial.println("TRIGGERED");
  else
    Serial.println("NORMAL");


  Serial.print(
    "Flow: "
  );


  Serial.print(
    currentFlowRate,
    3
  );


  Serial.println(
    " L/min"
  );


  Serial.print(
    "Total volume: "
  );


  Serial.print(
    totalVolume,
    3
  );


  Serial.println(
    " L"
  );


  Serial.println();

  Serial.println(
    "PUMP STATUS:"
  );


  if (
    systemMode ==
    MODE_CASE1
  )
  {
    if (isFloatTriggered())
    {
      Serial.println(
        "P1 = ON @ 50% - FLOAT TRIGGERED"
      );
    }
    else
    {
      Serial.println(
        "P1 = ON @ 50%"
      );
    }


    Serial.println(
      "P2 = ON @ 100%"
    );


    Serial.println(
      "P3 = OFF"
    );
  }


  else if (
    systemMode ==
    MODE_CASE2
  )
  {
    Serial.println(
      "P1 = ON @ 50%"
    );


    Serial.println(
      "P2 = ON @ 100%"
    );


    Serial.println(
      "P3 = ON @ 100%"
    );
  }


  else
  {
    Serial.println(
      "P1 = OFF"
    );


    Serial.println(
      "P2 = OFF"
    );


    Serial.println(
      "P3 = OFF"
    );
  }


  Serial.println(
    "=========================================="
  );
}


// ================================================================
// FINAL / CURRENT SYSTEM REPORT
// ================================================================
//
// This report NEVER waits for ball-valve testing.
//
// Unmeasured valve positions are shown as
// "NOT MEASURED".
//
// Can be called:
// - automatically after STOP
// - automatically after physical button STOP
// - automatically after Case 2 shutdown
// - manually using REPORT
// ================================================================

void printFinalReport()
{
  Serial.println();

  Serial.println();

  Serial.println(
    "================================================"
  );


  Serial.println(
    "              CURRENT SYSTEM REPORT"
  );


  Serial.println(
    "================================================"
  );


  // ============================================================
  // FT-01 CALIBRATION
  // ============================================================

  Serial.println();

  Serial.println(
    "FT-01 CALIBRATION"
  );


  Serial.println(
    "--------------------------------"
  );


  Serial.print(
    "Q = "
  );


  Serial.print(
    CAL_A,
    8
  );


  Serial.print(
    " * f + "
  );


  Serial.println(
    CAL_B,
    7
  );


  // ============================================================
  // CASE 1 NORMAL
  // ============================================================

  Serial.println();

  Serial.println(
    "CASE 1 - NORMAL OPERATION"
  );


  Serial.println(
    "--------------------------------"
  );


  if (
    normalFlowSamples > 0
  )
  {
    Serial.print(
      "Normal sustained flow = "
    );


    Serial.print(
      normalFlowAverage,
      3
    );


    Serial.println(
      " L/min"
    );
  }
  else
  {
    Serial.println(
      "Normal sustained flow = NOT MEASURED"
    );
  }


  // ============================================================
  // CASE 1 EXCESS / RECOVERY
  // ============================================================

  Serial.println();

  Serial.println(
    "CASE 1 - EXCESS / RECOVERY"
  );


  Serial.println(
    "--------------------------------"
  );


  if (
    totalExcessVolume > 0 ||
    excessDuration > 0
  )
  {
    Serial.print(
      "Total excess volume = "
    );


    Serial.print(
      totalExcessVolume,
      3
    );


    Serial.println(
      " L"
    );


    Serial.print(
      "Last excess duration = "
    );


    Serial.print(
      excessDuration / 1000.0,
      2
    );


    Serial.println(
      " s"
    );


    Serial.print(
      "Last excess average flow = "
    );


    Serial.print(
      excessAverageFlow,
      3
    );


    Serial.println(
      " L/min"
    );
  }
  else
  {
    Serial.println(
      "Excess event = NOT MEASURED"
    );
  }


  // ============================================================
  // BALL VALVE CHARACTERIZATION
  // ============================================================

  Serial.println();

  Serial.println(
    "BALL VALVE CHARACTERIZATION"
  );


  Serial.println(
    "--------------------------------"
  );


  // 25%

  Serial.print(
    "25%  = "
  );


  if (valve25Measured)
  {
    Serial.print(
      flow25,
      3
    );


    Serial.println(
      " L/min"
    );
  }
  else
  {
    Serial.println(
      "NOT MEASURED"
    );
  }


  // 50%

  Serial.print(
    "50%  = "
  );


  if (valve50Measured)
  {
    Serial.print(
      flow50,
      3
    );


    Serial.println(
      " L/min"
    );
  }
  else
  {
    Serial.println(
      "NOT MEASURED"
    );
  }


  // 75%

  Serial.print(
    "75%  = "
  );


  if (valve75Measured)
  {
    Serial.print(
      flow75,
      3
    );


    Serial.println(
      " L/min"
    );
  }
  else
  {
    Serial.println(
      "NOT MEASURED"
    );
  }


  // 100%

  Serial.print(
    "100% = "
  );


  if (valve100Measured)
  {
    Serial.print(
      flow100,
      3
    );


    Serial.println(
      " L/min"
    );
  }
  else
  {
    Serial.println(
      "NOT MEASURED"
    );
  }


  // ============================================================
  // OPERATING FLOW RANGE
  // ============================================================

  Serial.println();

  Serial.println(
    "TOTAL OPERATING FLOW RANGE"
  );


  Serial.println(
    "--------------------------------"
  );


  if (
    minimumOperatingFlow <
    9999.0
  )
  {
    Serial.print(
      "Minimum = "
    );


    Serial.print(
      minimumOperatingFlow,
      3
    );


    Serial.println(
      " L/min"
    );


    Serial.print(
      "Maximum = "
    );


    Serial.print(
      maximumOperatingFlow,
      3
    );


    Serial.println(
      " L/min"
    );


    Serial.print(
      "Range = "
    );


    Serial.print(
      maximumOperatingFlow -
      minimumOperatingFlow,
      3
    );


    Serial.println(
      " L/min"
    );
  }
  else
  {
    Serial.println(
      "No valid operating-flow data."
    );
  }


  // ============================================================
  // TOTAL FT-01 VOLUME
  // ============================================================

  Serial.println();

  Serial.println(
    "TOTAL FT-01 VOLUME"
  );


  Serial.println(
    "--------------------------------"
  );


  Serial.print(
    "Total measured volume = "
  );


  Serial.print(
    totalVolume,
    3
  );


  Serial.println(
    " L"
  );


  // ============================================================
  // FINAL STATE
  // ============================================================

  Serial.println();

  Serial.println(
    "FINAL STATE"
  );


  Serial.println(
    "--------------------------------"
  );


  if (
    systemMode ==
    MODE_OFF
  )
  {
    Serial.println(
      "P1 = OFF"
    );


    Serial.println(
      "P2 = OFF"
    );


    Serial.println(
      "P3 = OFF"
    );


    Serial.println(
      "SOLENOID = CLOSED"
    );


    Serial.println(
      "SYSTEM = OFF"
    );
  }
  else if (
    systemMode ==
    MODE_CASE1
  )
  {
    Serial.println(
      "P1 = ON @ 50%"
    );


    Serial.println(
      "P2 = ON @ 100%"
    );


    Serial.println(
      "P3 = OFF"
    );


    Serial.println(
      "SYSTEM = CASE 1 ACTIVE"
    );
  }
  else if (
    systemMode ==
    MODE_CASE2
  )
  {
    Serial.println(
      "P1 = ON @ 50%"
    );


    Serial.println(
      "P2 = ON @ 100%"
    );


    Serial.println(
      "P3 = ON @ 100%"
    );


    Serial.println(
      "SYSTEM = CASE 2 ACTIVE"
    );
  }


  Serial.println();

  Serial.println(
    "================================================"
  );


  Serial.println(
    "              REPORT COMPLETE"
  );


  Serial.println(
    "================================================"
  );
}


// ================================================================
// RESET ALL MEASUREMENTS
// ================================================================

void resetMeasurements()
{
  noInterrupts();

  pulseCount = 0;

  interrupts();


  totalVolume = 0.0;

  currentFlowRate = 0.0;


  normalFlowSum = 0.0;

  normalFlowSamples = 0;

  normalFlowAverage = 0.0;


  excessFlowEvent = false;

  excessVolume = 0.0;

  totalExcessVolume = 0.0;

  eventStartVolume = 0.0;

  excessDuration = 0;

  excessFlowSum = 0.0;

  excessFlowSamples = 0;

  excessAverageFlow = 0.0;


  minimumOperatingFlow = 9999.0;

  maximumOperatingFlow = 0.0;


  flow25 = 0.0;

  flow50 = 0.0;

  flow75 = 0.0;

  flow100 = 0.0;


  valve25Measured = false;

  valve50Measured = false;

  valve75Measured = false;

  valve100Measured = false;


  Serial.println();

  Serial.println(
    "ALL FLOW MEASUREMENTS RESET."
  );
}


// ================================================================
// SERIAL COMMAND PROCESSING
// ================================================================

void processSerialCommand()
{
  if (!Serial.available())
    return;


  String command =
    Serial.readStringUntil('\n');


  command.trim();

  command.toUpperCase();


  // ============================================================
  // CASE 1
  // ============================================================

  if (command == "CASE1")
  {
    if (manualStopLockout)
    {
      Serial.println();

      Serial.println(
        "CASE1 BLOCKED."
      );

      Serial.println(
        "System was stopped using STOP."
      );

      Serial.println(
        "Press physical ON/OFF switch first."
      );
    }
    else if (
      systemMode ==
      MODE_OFF
    )
    {
      startCase1();
    }
    else
    {
      Serial.println(
        "System already running."
      );
    }

    return;
  }


  // ============================================================
  // CASE 2
  // ============================================================

  if (command == "CASE2")
  {
    startCase2();

    return;
  }


  // ============================================================
  // STOP
  // ============================================================

  if (command == "STOP")
  {
    if (systemMode != MODE_OFF)
    {
      stopSystem();
    }
    else
    {
      Serial.println();

      Serial.println(
        "SYSTEM ALREADY OFF."
      );

      Serial.println(
        "Generating current report..."
      );

      printFinalReport();
    }

    return;
  }


  // ============================================================
  // STATUS
  // ============================================================

  if (command == "STATUS")
  {
    printStatus();

    return;
  }


  // ============================================================
  // REPORT
  // ============================================================

  if (command == "REPORT")
  {
    printFinalReport();

    return;
  }


  // ============================================================
  // RESET
  // ============================================================

  if (command == "RESET")
  {
    resetMeasurements();

    return;
  }


  // ============================================================
  // VALVE
  // ============================================================

  if (command == "VALVE")
  {
    Serial.println();

    Serial.println(
      "=========================================="
    );

    Serial.println(
      "BALL VALVE FLOW MEASUREMENT"
    );

    Serial.println(
      "=========================================="
    );

    Serial.println(
      "Enter valve position:"
    );

    Serial.println(
      "25"
    );

    Serial.println(
      "50"
    );

    Serial.println(
      "75"
    );

    Serial.println(
      "100"
    );

    Serial.println();


    waitingForValvePosition =
      true;


    return;
  }


  // ============================================================
  // VALVE POSITION
  // ============================================================

  if (waitingForValvePosition)
  {
    int position =
      command.toInt();


    if (
      position == 25 ||
      position == 50 ||
      position == 75 ||
      position == 100
    )
    {
      waitingForValvePosition =
        false;


      startValveMeasurement(
        position
      );
    }
    else
    {
      Serial.println(
        "Invalid valve position."
      );

      Serial.println(
        "Enter 25, 50, 75 or 100."
      );
    }


    return;
  }


  // ============================================================
  // HELP
  // ============================================================

  if (command == "HELP")
  {
    Serial.println();

    Serial.println(
      "=========================================="
    );

    Serial.println(
      "AVAILABLE COMMANDS"
    );

    Serial.println(
      "=========================================="
    );

    Serial.println(
      "CASE1  -> Start Case 1"
    );

    Serial.println(
      "CASE2  -> Start Case 2"
    );

    Serial.println(
      "STOP   -> Stop + generate report"
    );

    Serial.println(
      "STATUS -> Show system status"
    );

    Serial.println(
      "REPORT -> Show current report"
    );

    Serial.println(
      "RESET  -> Reset measurements"
    );

    Serial.println(
      "VALVE  -> Measure ball valve flow"
    );

    Serial.println(
      "HELP   -> Show commands"
    );

    Serial.println(
      "=========================================="
    );


    return;
  }


  // ============================================================
  // UNKNOWN COMMAND
  // ============================================================

  Serial.print(
    "Unknown command: "
  );

  Serial.println(
    command
  );

  Serial.println(
    "Type HELP for commands."
  );
}


// ================================================================
// SETUP
// ================================================================

void setup()
{
  Serial.begin(115200);

  delay(1000);


  Serial.println();

  Serial.println();


  Serial.println(
    "================================================"
  );

  Serial.println(
    "       LAM RESEARCH CHALLENGE 3.0"
  );

  Serial.println(
    "       STAGE 2 ESP32 CONTROL SYSTEM"
  );

  Serial.println(
    "================================================"
  );


  // ============================================================
  // PIN MODES
  // ============================================================

  pinMode(P1_RPWM, OUTPUT);

  pinMode(P1_LPWM, OUTPUT);

  pinMode(P1_REN, OUTPUT);

  pinMode(P1_LEN, OUTPUT);


  pinMode(P2_RPWM, OUTPUT);

  pinMode(P2_LPWM, OUTPUT);

  pinMode(P2_REN, OUTPUT);

  pinMode(P2_LEN, OUTPUT);


  pinMode(P3_RPWM, OUTPUT);

  pinMode(P3_LPWM, OUTPUT);

  pinMode(P3_REN, OUTPUT);

  pinMode(P3_LEN, OUTPUT);


  pinMode(
    FLOAT_PIN,
    INPUT
  );


  pinMode(
    FLOW_PIN,
    INPUT
  );


  pinMode(
    ONOFF_PIN,
    INPUT
  );


  pinMode(
    SOLENOID_PIN,
    OUTPUT
  );


  // ============================================================
  // SOLENOID SAFE POWER-UP STATE
  // ============================================================

  digitalWrite(
    SOLENOID_PIN,
    SOLENOID_CLOSED_LEVEL
  );


  // ============================================================
  // PWM
  // ============================================================

  ledcAttach(
    P1_RPWM,
    PWM_FREQ,
    PWM_RES
  );

  ledcAttach(
    P1_LPWM,
    PWM_FREQ,
    PWM_RES
  );


  ledcAttach(
    P2_RPWM,
    PWM_FREQ,
    PWM_RES
  );

  ledcAttach(
    P2_LPWM,
    PWM_FREQ,
    PWM_RES
  );


  ledcAttach(
    P3_RPWM,
    PWM_FREQ,
    PWM_RES
  );

  ledcAttach(
    P3_LPWM,
    PWM_FREQ,
    PWM_RES
  );


  // ============================================================
  // STOP ALL PUMPS AT POWER-UP
  // ============================================================

  stopAllPumps();


  // ============================================================
  // FT-01 INTERRUPT
  // ============================================================

  attachInterrupt(
    digitalPinToInterrupt(FLOW_PIN),
    flowPulse,
    FALLING
  );


  // ============================================================
  // INITIAL SYSTEM STATE
  // ============================================================

  systemMode =
    MODE_OFF;


  manualStopLockout =
    false;


  totalVolume =
    0.0;


  currentFlowRate =
    0.0;


  excessFlowEvent =
    false;


  excessVolume =
    0.0;


  totalExcessVolume =
    0.0;


  case2ShutdownTriggered =
    false;


  // ============================================================
  // INITIAL FLOAT / BUTTON STATES
  // ============================================================

  lastFloatTriggered =
    isFloatTriggered();


  lastButtonState =
    digitalRead(
      ONOFF_PIN
    );


  stableButtonState =
    lastButtonState;


  lastDebounceTime =
    millis();


  previousFlowMillis =
    millis();


  // ============================================================
  // STARTUP INFORMATION
  // ============================================================

  Serial.println(
    "-----------------------------------------------"
  );

  Serial.println(
    "SYSTEM INITIALIZED"
  );

  Serial.println(
    "-----------------------------------------------"
  );


  Serial.println(
    "P1 : GPIO 33/32, EN 26/13"
  );


  Serial.println(
    "P2 : GPIO 19/18, EN 17/16"
  );


  Serial.println(
    "P3 : GPIO 25/23, EN 22/21"
  );


  Serial.println(
    "FLOAT : GPIO 35"
  );


  Serial.println(
    "FT-01 : GPIO 14"
  );


  Serial.println(
    "START : GPIO 34"
  );


  Serial.println(
    "SOLENOID : GPIO 27"
  );


  Serial.println(
    "-----------------------------------------------"
  );


  Serial.println(
    "POWER-UP SOLENOID = CLOSED"
  );


  Serial.println(
    "P1 = 50%"
  );


  Serial.println(
    "P2 = 100%"
  );


  Serial.println(
    "P3 = 100%"
  );


  Serial.println(
    "-----------------------------------------------"
  );


  // ============================================================
  // CALIBRATION INFORMATION
  // ============================================================

  Serial.println(
    "FT-01 calibration:"
  );


  Serial.print(
    "a = "
  );


  Serial.println(
    CAL_A,
    8
  );


  Serial.print(
    "b = "
  );


  Serial.println(
    CAL_B,
    7
  );


  Serial.println(
    "-----------------------------------------------"
  );


  Serial.print(
    "CASE 2 startup buffer = "
  );


  Serial.print(
    CASE2_STARTUP_BUFFER_MS / 1000
  );


  Serial.println(
    " seconds"
  );


  Serial.print(
    "CASE 2 shutdown threshold = "
  );


  Serial.print(
    CASE2_LOW_FLOW_THRESHOLD,
    2
  );


  Serial.println(
    " L/min"
  );


  Serial.println(
    "-----------------------------------------------"
  );


  Serial.println(
    "SYSTEM READY"
  );


  Serial.println();


  Serial.println(
    "Press physical momentary button"
  );


  Serial.println(
    "to start CASE 1."
  );


  Serial.println();


  Serial.println(
    "Type HELP for commands."
  );


  Serial.println(
    "================================================"
  );
}


// ================================================================
// MAIN LOOP
// ================================================================

void loop()
{
  // Physical button

  checkStartButton();


  // Serial commands

  processSerialCommand();


  // FT-01 measurement

  updateFlowMeasurement();


  // Case 1 float logic

  if (
    systemMode ==
    MODE_CASE1
  )
  {
    updateCase1FloatControl();
  }


  // Case 2 float logic

  if (
    systemMode ==
    MODE_CASE2
  )
  {
    updateCase2FloatControl();
  }

  // Ball valve measurement

  updateValveMeasurement();
}
