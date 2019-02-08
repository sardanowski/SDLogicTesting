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
int gateType;


void setup() {
  //put your setup code here, to run once:
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
  test(gateNumber); //testing this many gates && need to use this as output
}

bool test(int gateNumber) {
  int out1, out2, testresults;
  int testNumber; //number to multiply to truth table values to get unique test results
  int gate = gateNumber;
  bool passFail;
  for (gate; gate > 0; gate--) {
    testNumber = 8;
//    if(!invert){ //testing Inverter more complicated
    for (out1 = 1; out1 >= 0; out1--) {
      for (out2 = 1; out2 >= 0; out1--) {
        testresults = testresults + (check_Gate(out1, out2,0,1) * testNumber); //probably need to send gate pin numbers.
        testNumber / 2;
      }
    }
//    }else{
//      testNumber/4;
//      for (out2 = 1; out2 >= 0; out1--) {
//        testresults = testresults + (check_Gate(out1, out2) * testNumber); //probably need to send gate pin numbers.
//        testNumber / 2;
//      }
//    }
    passFail = testresults==gateType? true: false;
    
    if(passFail == false)
      break;
  }
  return passFail;
}

int check_Gate(int output1, int output2, int outpin1, int outpin2) 
//checking what the output is when giving inputs
{
  int x, y;
  mcp.digitalWrite(outpin1, output1);
  mcp.digitalWrite(outpin2, output2);
  
  //check for shorts here complicated
  
  delay(5); // Make sure the signal has time to propogate through the gate.
  x = mcp.digitalRead(2);

  return x;
}

void resetPins() {
  for (int x = 0; x < 16; x++) {
    mcp.pinMode(x, OUTPUT);
  }
  gateNumber = invert ? 6 : 4;
  return;
}

void TTLinputPins(int gatevalue) 
//used to set the input pins for the gates
{
  invert = 0;
  resetPins();
  switch (gatevalue) {
    case 0://NOT
      gateType = 0;
      invert = 1;

      break;
    case 1://NOR
      gateType = 1;
      break;
    case 6: //XOR
      gateType = 6;

      break;
    case 7: //NAND
      gateType = 7;

      break;
    case 8: //AND GATE
      gateType = 8;

      break;
    case 14://OR GATE
      gateType = 14;

      break;
    default:
      Serial.println("Default gate");
  }
  gateNumber = invert ? 6 : 4;
  return;
}

void CMOSinputPins(int gatevalue)
//used to set the input pins for the gates
{
  invert = 0;
  resetPins();
  switch (gatevalue) {
    case 0://NOT
      gateType = 0;
      invert = 1;

      break;
    case 1://NOR
      gateType = 1;
      break;
    case 6: //XOR
      gateType = 6;

      break;
    case 7: //NAND
      gateType = 7;

      break;
    case 8: //AND GATE
      gateType = 8;

      break;
    case 14://OR GATE
      gateType = 14;

      break;
    default:
      Serial.println("Default gate");
  }
  return;
}
