//#include <TinyWireM.h>
//#include <USI_TWI_Master.h>
#include <Wire.h>
#include "Adafruit_MCP23017.h"
#include "Adafruit_ILI9341.h"
#include "SPI.h"
#include "Adafruit_GFX.h"
#include "Adafruit_ILI9341.h"
#include "Print.h"

//display pins
#define TFT_DC 9
#define TFT_CS 10
#define TFT_MOSI 11
#define TFT_CLK 13

//Pin expander pins
#define SDA A4
#define SCL A5

Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC);
Adafruit_MCP23017 mcp;
int gateType = 8; //default gate to AND
//void TTLinputPins(8); //default the gates to AND
//void CMOSinputPins(8);

int invert = 0;
int gateNumber = invert ? 6 : 4; //Used for the number of gates being tested
int inPin[16];
int outPin[16];

//Input/output pins for different gates.
//DO NOT REMOVE OR CHANGE
int tempIN1[] = {1, 3, 5, 10, 12, 14};        //NOT GATES
int tempOUT1[] = {0, 2, 4, 11, 13, 15};

int tempIN2[] = {0, 3, 12, 15};               //TTL NOR
int tempOUT2[] = {1, 2, 4, 5, 10, 11, 13, 14};

int tempIN3[] = {2, 5, 10, 13};               //REST OF TTL
int tempOUT3[] = {0, 1, 3, 4, 11, 12, 14, 15};

int tempIN4[] = {2, 3, 12, 13};               //CMOS GATES
int tempOUT4[] = {0, 1, 4, 5, 10, 11, 14, 15};


void setup() {
  //put your setup code here, to run once:
  //DISPLAY()
  //DEFINE Default pinouts
  //initialize SPI AND I2C

  //I2C
  Serial.begin(9600);
  mcp.begin();
  tft.begin();
}

void loop() {
  // put your main code here, to run repeatedly:
  tft.fillScreen(ILI9341_WHITE);
  tft.setTextSize(5);
  tft.setTextColor(ILI9341_BLACK);
  tft.setCursor(0, 0);
  tft.println("START");
  gateType = 8; //CHANGE THIS TO CHANGE GATE TYPE
  TTLinputPins(gateType); //selecting gate type here
  delay(1000);
  bool result = test(gateNumber); //testing this many gates && need to use this as output
  outputResult(result);

  tft.fillScreen(ILI9341_WHITE);
  tft.setTextSize(5);
  tft.setCursor(0, 0);
  tft.println("DONE");
  delay(1000);
}

void outputResult(bool result) {

  if (result == false) {
    tft.setCursor(0, 0);
    tft.fillScreen(ILI9341_RED);
    tft.setTextSize(4);
    tft.setTextColor(ILI9341_BLACK);
    tft.println("FAIL");
  }
  else {
    tft.setCursor(0, 0);
    tft.fillScreen(ILI9341_GREEN);
    tft.setTextSize(4);
    tft.setTextColor(ILI9341_BLACK);
    tft.println("PASS!!!");
  }
  delay(1000);
}

bool test(int gateNumber) {
  int out1, out2, testresults = 0;
  int testNumber = 8; //number to multiply to truth table values to get unique test results
  int gate;
  bool passFail = true;
  int pinout1, pinout2, pinIn;

  for (gate = 0; gate < gateNumber; gate++) {
    pinout1 = outPin[gate * 2];
    pinout2 = outPin[gate * 2 + 1];
    pinIn = inPin[gate];

    //    if(!invert){ //testing Inverter more complicated
    for (out1 = 1; out1 >= 0; out1--) {
      for (out2 = 1; out2 >= 0; out2--) {
        testresults = testresults + (check_Gate(out1, out2, pinout1, pinout2, pinIn) * testNumber); //probably need to send gate pin numbers.
        testNumber = testNumber / 2;

        tft.setCursor(0, 0);
        tft.setTextSize(4);
        tft.setTextColor(ILI9341_BLACK); //code to look at testing results
        tft.println(testresults);
        delay(1000);

      }
    }
    //    }else{
    //      testNumber/4;
    //      for (out2 = 1; out2 >= 0; out1--) {
    //        testresults = testresults + (check_Gate(out1, out2) * testNumber); //probably need to send gate pin numbers.
    //        testNumber / 2;
    //      }
    //    }
    passFail = (testresults == gateType) ? true : false;

    if (passFail == false) {
      return passFail;
    }
  }
  return passFail;
}

int check_Gate(int output1, int output2, int outpin1, int outpin2, int input1)
//checking what the output is when giving inputs
{
  int x;
  mcp.digitalWrite(outpin1, output1);
  mcp.digitalWrite(outpin2, output2);

  //check for shorts here complicated

  delay(5); // Make sure the signal has time to propogate through the gate.
  x = mcp.digitalRead(input1);

  return x;
}

void resetPins() {
  for (int x = 0; x < 16; x++) {
    mcp.pinMode(x, OUTPUT);
    inPin[x] = 0;
    outPin[x] = 0;
  }
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
      copy(tempIN1, inPin, sizeof(tempIN1));
      copy(tempOUT1, outPin, sizeof(tempOUT1));
      mcp.pinMode(inPin[0], INPUT);
      mcp.pinMode(inPin[1], INPUT);
      mcp.pinMode(inPin[2], INPUT);
      mcp.pinMode(inPin[3], INPUT);
      mcp.pinMode(inPin[4], INPUT);
      mcp.pinMode(inPin[5], INPUT);
      break;

    case 1://NOR
      gateType = 1;
      copy(tempIN2, inPin, sizeof(tempIN2));
      copy(tempOUT2, outPin, sizeof(tempOUT2));
      mcp.pinMode(inPin[0], INPUT);
      mcp.pinMode(inPin[1], INPUT);
      mcp.pinMode(inPin[2], INPUT);
      mcp.pinMode(inPin[3], INPUT);
      break;

    case 7: //NAND
      gateType = 7;
      copy(tempIN3, inPin, sizeof(tempIN3));
      copy(tempOUT3, outPin, sizeof(tempOUT3));
      mcp.pinMode(inPin[0], INPUT);
      mcp.pinMode(inPin[1], INPUT);
      mcp.pinMode(inPin[2], INPUT);
      mcp.pinMode(inPin[3], INPUT);
      break;

    case 8: //AND GATE
      gateType = 8;
      copy(tempIN3, inPin, sizeof(tempIN3));
      copy(tempOUT3, outPin, sizeof(tempOUT3));
      mcp.pinMode(inPin[0], INPUT);
      mcp.pinMode(inPin[1], INPUT);
      mcp.pinMode(inPin[2], INPUT);
      mcp.pinMode(inPin[3], INPUT);
      break;

    case 14://OR GATE
      gateType = 14;
      copy(tempIN3, inPin, sizeof(tempIN3));
      copy(tempOUT3, outPin, sizeof(tempOUT3));
      mcp.pinMode(inPin[0], INPUT);
      mcp.pinMode(inPin[1], INPUT);
      mcp.pinMode(inPin[2], INPUT);
      mcp.pinMode(inPin[3], INPUT);
      break;

    default:
      tft.println("No Gate Selected");
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
      copy(tempIN1, inPin, sizeof(tempIN1));
      copy(tempOUT1, outPin, sizeof(tempOUT1));
      mcp.pinMode(inPin[0], INPUT);
      mcp.pinMode(inPin[1], INPUT);
      mcp.pinMode(inPin[2], INPUT);
      mcp.pinMode(inPin[3], INPUT);
      mcp.pinMode(inPin[4], INPUT);
      mcp.pinMode(inPin[5], INPUT);
      break;

    case 1://NOR
      gateType = 1;
      copy(tempIN4, inPin, sizeof(tempIN4));
      copy(tempOUT4, outPin, sizeof(tempOUT4));
      mcp.pinMode(inPin[0], INPUT);
      mcp.pinMode(inPin[1], INPUT);
      mcp.pinMode(inPin[2], INPUT);
      mcp.pinMode(inPin[3], INPUT);
      break;

    case 7: //NAND
      gateType = 7;
      copy(tempIN4, inPin, sizeof(tempIN4));
      copy(tempOUT4, outPin, sizeof(tempOUT4));
      mcp.pinMode(inPin[0], INPUT);
      mcp.pinMode(inPin[1], INPUT);
      mcp.pinMode(inPin[2], INPUT);
      mcp.pinMode(inPin[3], INPUT);
      break;

    case 8: //AND GATE
      gateType = 8;
      copy(tempIN4, inPin, sizeof(tempIN4));
      copy(tempOUT4, outPin, sizeof(tempOUT4));
      mcp.pinMode(inPin[0], INPUT);
      mcp.pinMode(inPin[1], INPUT);
      mcp.pinMode(inPin[2], INPUT);
      mcp.pinMode(inPin[3], INPUT);
      break;

    case 14://OR GATE
      gateType = 14;
      copy(tempIN4, inPin, sizeof(tempIN4));
      copy(tempOUT4, outPin, sizeof(tempOUT4));
      mcp.pinMode(inPin[0], INPUT);
      mcp.pinMode(inPin[1], INPUT);
      mcp.pinMode(inPin[2], INPUT);
      mcp.pinMode(inPin[3], INPUT);
      break;

    default:
      tft.println("No Gate Selected");
  }
  gateNumber = invert ? 6 : 4;
  return;
}

// Function to copy 'len' elements from 'src' to 'dst'
void copy(int* src, int* dst, int len) {
  memcpy(dst, src, sizeof(src[0])*len);
}
