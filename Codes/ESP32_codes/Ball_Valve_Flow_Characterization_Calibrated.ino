/*
  LAM Stage 2 - Standalone Ball-Valve Flow Characterization
  Serial baud: 115200
  Commands: HELP, VALVE, 25, 50, 75, 100, STATUS, REPORT, RESET, STOP

  Safety:
  - This sketch only counts FT-01 pulses and reports flow/volume.
  - It does NOT automatically move a manual ball valve.
  - Operator must physically set valve opening before confirming position.
  - Pump outputs are held LOW; use the rig's independently verified safe pump
    operating arrangement. This standalone sketch intentionally does not start pumps.
  - Verify the actual wiring/pin map before connecting any hardware.
*/
#include <Arduino.h>
#define FLOW_PIN 14
#define FLOAT_PIN 35
#define ONOFF_PIN 34
#define SOLENOID_PIN 27

// FT-01 calibration: Q (L/min) = a * frequency (Hz) + b
const float CAL_A = 0.02420192f;
const float CAL_B = 0.02599480f;

const unsigned long SAMPLE_MS = 1000;
const unsigned long VALVE_STABILIZATION_MS = 4000;
const unsigned long VALVE_AVERAGING_MS = 5000;
volatile unsigned long pulseCount = 0;
unsigned long previousSample = 0;
unsigned long totalPulses = 0;
float flowLpm = 0.0f, totalVolumeL = 0.0f;
bool valvePrompt = false, measuring = false;
int selectedPosition = 0;
unsigned long stabilizationStart = 0, averagingStart = 0;
float flowSum = 0.0f; unsigned long flowSamples = 0;
String commandBuffer;

void IRAM_ATTR flowPulse() { pulseCount++; }

void printCalibration() {
  Serial.println(F("FT-01 calibration (Q = a*f + b):"));
  Serial.print(F("a = "));
  Serial.println(CAL_A, 8);
  Serial.print(F("b = "));
  Serial.println(CAL_B, 8);
}

void printStatus() {
  Serial.println(F("=== BALL VALVE TEST STATUS ==="));
  Serial.print(F("Valve test active: ")); Serial.println(measuring ? F("YES") : F("NO"));
  Serial.print(F("Selected opening: ")); Serial.print(selectedPosition); Serial.println(F("%"));
  Serial.print(F("Frequency Hz: ")); Serial.println((float)totalPulses / (SAMPLE_MS / 1000.0f), 3);
  Serial.print(F("Flow L/min: ")); Serial.println(flowLpm, 3);
  Serial.print(F("FT-derived total volume L: ")); Serial.println(totalVolumeL, 4);
  printCalibration();
}
void printHelp() {
  Serial.println(F("Commands: VALVE, 25, 50, 75, 100, STATUS, REPORT, RESET, STOP, HELP"));
  Serial.println(F("Set the physical valve position first; this sketch does not actuate a manual valve."));
}
void beginPosition(int pos) {
  if (!(pos == 25 || pos == 50 || pos == 75 || pos == 100)) {
    Serial.println(F("Invalid valve position. Use 25, 50, 75, or 100."));
    return;
  }
  selectedPosition = pos;
  valvePrompt = false;
  measuring = false;
  flowSum = 0; flowSamples = 0;
  stabilizationStart = millis();
  Serial.print(F("BALL VALVE POSITION = ")); Serial.print(pos); Serial.println(F("%"));
  Serial.println(F("Stabilizing flow for 4 seconds; keep valve/pump arrangement unchanged."));
}
void handleCommand(String cmd) {
  cmd.trim(); cmd.toUpperCase();
  if (cmd == "HELP") printHelp();
  else if (cmd == "VALVE") {
    valvePrompt = true;
    Serial.println(F("BALL VALVE FLOW MEASUREMENT"));
    Serial.println(F("Physically set valve opening, then enter 25, 50, 75, or 100."));
  } else if (cmd == "25" || cmd == "50" || cmd == "75" || cmd == "100") {
    if (!valvePrompt) Serial.println(F("Type VALVE first, then enter the physical valve position."));
    else beginPosition(cmd.toInt());
  } else if (cmd == "STATUS" || cmd == "REPORT") printStatus();
  else if (cmd == "RESET") { noInterrupts(); pulseCount=0; interrupts(); totalPulses=0; totalVolumeL=0; Serial.println(F("TOTALS RESET")); }
  else if (cmd == "STOP") { measuring=false; valvePrompt=false; Serial.println(F("VALVE MEASUREMENT STOPPED; pump outputs were not controlled by this sketch.")); }
  else if (cmd.length()) Serial.println(F("Unknown command. Type HELP."));
}
void setup() {
  Serial.begin(115200);
  pinMode(FLOW_PIN, INPUT);
  pinMode(FLOAT_PIN, INPUT);
  pinMode(ONOFF_PIN, INPUT);
  pinMode(SOLENOID_PIN, OUTPUT);
  digitalWrite(SOLENOID_PIN, LOW); // verify this is the safe closed state for your hardware
  attachInterrupt(digitalPinToInterrupt(FLOW_PIN), flowPulse, RISING);
  previousSample=millis();
  Serial.println(F("LAM Stage 2 - Standalone Ball-Valve Flow Logger"));
  Serial.println(F("Pump control intentionally not implemented in this sketch."));
  printCalibration();
  printHelp();
}
void loop() {
  while (Serial.available()) {
    char c=(char)Serial.read();
    if (c=='\n' || c=='\r') { if (commandBuffer.length()) { handleCommand(commandBuffer); commandBuffer=""; } }
    else if (commandBuffer.length()<80) commandBuffer += c;
  }
  unsigned long nowMs=millis();
  if (nowMs-previousSample >= SAMPLE_MS) {
    unsigned long elapsed=nowMs-previousSample; previousSample=nowMs;
    noInterrupts(); unsigned long pulses=pulseCount; pulseCount=0; interrupts();
    totalPulses += pulses;
    float frequency=(elapsed>0) ? pulses*1000.0f/elapsed : 0;
    flowLpm=CAL_A*frequency+CAL_B;
    if (flowLpm<0) flowLpm=0;
    totalVolumeL += flowLpm*((float)elapsed/60000.0f);
    Serial.print(F("DATA,")); Serial.print(nowMs); Serial.print(F(",VALVE,"));
    Serial.print(pulses); Serial.print(','); Serial.print(frequency,3); Serial.print(',');
    Serial.print(flowLpm,4); Serial.print(','); Serial.print(totalVolumeL,5); Serial.print(',');
    Serial.print(digitalRead(FLOAT_PIN)==HIGH ? 1:0); Serial.print(F(",0,0\n"));
  }
  if (!valvePrompt && selectedPosition>0 && !measuring && nowMs-stabilizationStart >= VALVE_STABILIZATION_MS) {
    measuring=true; averagingStart=nowMs; flowSum=0; flowSamples=0;
    Serial.println(F("Flow stabilization complete; averaging for 5 seconds."));
  }
  if (measuring) {
    flowSum += flowLpm; flowSamples++;
    if (nowMs-averagingStart >= VALVE_AVERAGING_MS) {
      float avg=flowSamples ? flowSum/flowSamples : 0;
      Serial.print(F("Stable flow = ")); Serial.print(avg,3); Serial.println(F(" L/min"));
      Serial.print(F("VALVE_RESULT,")); Serial.print(selectedPosition); Serial.print(','); Serial.print(avg,4); Serial.print(','); Serial.println(flowSamples);
      measuring=false; selectedPosition=0;
      Serial.println(F("Type VALVE for the next position."));
    }
  }
}
