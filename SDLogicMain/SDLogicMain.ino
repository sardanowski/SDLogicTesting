#include <TinyWireM.h>
#include <USI_TWI_Master.h>
#include <Wire.h>
#include "Adafruit_MCP23017.h"
#include "Adafruit_ILI9341.h"

//display pins
#define TFT_DC 9
#define TFT_CS 10
#define TFT_MOSI 11
#define TFT_CLK 13

//Pin expander pins
#define SDA A4
#define SCL A5

Adafruit_MCP23017 mcp;

void TTLinputPins(int gate = 8); //default the gates to AND
void CMOSinputPins(int gate = 8);

int invert = 0;
int gateNumber = invert ? 6 : 4; //Used for the number of gates being tested
int inPin[14];
int outPin[14];

void setup() {
  // put your setup code here, to run once:
  //DISPLAY()
  //DEFINE Default pinouts
  //initialize SPI AND I2C

  //I2C
  Serial.begin(9600);
  mcp.begin();
  TTLinputPins();

}

void loop() {
  // put your main code here, to run repeatedly:
  test(gateNumber);
}

int test(int gateNumber) {
  int out1, out2, testresults;
  int testNumber = 8; //number to multiply to truth table values to get unique test results
  int gate = gateNumber;
  for (gate; gate > 0; gate--) {
    for (out1 = 1; out1 >= 0; out1--) {
      for (out2 = 1; out2 >= 0; out1--) {
        testresults = testresults + (check_Gate(out1, out2) * testNumber);
        testNumber / 2;
      }
    }
  }
}

int check_Gate(int output1, int output2) {
  int x;
  mcp.digitalWrite(0, output1);
  mcp.digitalWrite(1, output2);
  delay(5); // Make sure the signal has time to propogate through the gate.
  x = mcp.digitalRead(2);

  return x;
}

void resetPins() {
  for (int x = 0; x < 16; x++) {
    mcp.pinMode(x, OUTPUT);
  }
  return;
}

void TTLinputPins(int gatevalue) {
  resetPins();
  switch (gatevalue) {
    case 0://NOT

      break;
    case 1://NOR

      break;
    case 6: //XOR

      break;
    case 7: //NAND

      break;
    case 8: //AND GATE

      break;
    case 14://OR GATE

      break;
    default:
      Serial.println("Default gate");
  }
  return;
}

void CMOSinputPins(int gatevalue) {
  resetPins();
  switch (gatevalue) {
    case 0://NOT

      break;
    case 1://NOR

      break;
    case 6: //XOR

      break;
    case 7: //NAND

      break;
    case 8: //AND GATE

      break;
    case 14://OR GATE

      break;
    default:
      Serial.println("Default gate");
  }
  return;
}
