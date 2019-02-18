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
int gateType = 7;
void TTLinputPins(gateType); //default the gates to AND
void CMOSinputPins(gateType);

int invert = 0;
int gateNumber = invert ? 6 : 4; //Used for the number of gates being tested
int inPin[16];
int outPin[16];


int tempIN1[] = {1, 3, 5, 10, 12, 14};
int tempOUT1[] = {0, 2, 4, 11, 13, 15};

int tempIN2[] = {0, 3, 12, 15};
int tempOUT2[] = {1, 2, 4, 5, 10, 11, 13, 14};

int tempIN3[] = {2, 5, 10, 13};
int tempOUT3[] = {0, 1, 3, 4, 11, 12, 14, 15};

int tempIN4[] = {2, 3, 12, 13};
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
  TTLinputPins();

}

void loop() {
  // put your main code here, to run repeatedly:
  tft.fillScreen(ILI9341_WHITE);
  tft.setTextSize(5);
  tft.setTextColor(ILI9341_BLACK);
  tft.setCursor(0, 0);
  tft.printf("start\n");
  delay(1000);
  tft.fillScreen(ILI9341_YELLOW);
  delay(50);
  test(gateNumber); //testing this many gates && need to use this as output
  delay(100);
  tft.fillScreen(ILI9341_WHITE);
  tft.setTextSize(5);
  tft.setCursor(0, 0);
  tft.println("DONE");
  delay(10);
}

bool test(int gateNumber) {
  int out1, out2, testresults;
  int testNumber = 8; //number to multiply to truth table values to get unique test results
  int gate;
  bool passFail = true;
  int pinout1, pinout2, pinIn;
  tft.setCursor(0, 0);
 // tft.println("a");
  for (gate = 0; gate < gateNumber; gate++) {
    pinout1 = outPin[gate * 2];
    pinout2 = outPin[gate * 2 + 1];
    pinIn = inPin[gate];
    //tft.setCursor(0,10);
 //   tft.println("d");
    //    if(!invert){ //testing Inverter more complicated
    for (out1 = 1; out1 >= 0; out1--) {
      for (out2 = 1; out2 >= 0; out2--) {
        testresults = testresults + (check_Gate(out1, out2, pinout1, pinout2, pinIn) * testNumber); //probably need to send gate pin numbers.
        testNumber = testNumber / 2;
        //tft.println(testresults);
      }
      tft.fillScreen(ILI9341_YELLOW);
      tft.setCursor(0, 0);
      //tft.print("b");
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
      tft.setCursor(0, 0);
      tft.fillScreen(ILI9341_RED);
      tft.setTextSize(4);
      tft.setTextColor(ILI9341_BLACK);
      tft.println("FAIL");
      delay(1000);
      break;
    }
  }
  if (passFail == true) {
    tft.setCursor(0, 0);
    tft.fillScreen(ILI9341_GREEN);
    tft.setTextSize(4);
    tft.setTextColor(ILI9341_BLACK);
    tft.println("PASS!!!");
    delay(1000);
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
      mcp.pinMode(1, INPUT);
      mcp.pinMode(3, INPUT);
      mcp.pinMode(5, INPUT);
      mcp.pinMode(10, INPUT);
      mcp.pinMode(12, INPUT);
      mcp.pinMode(14, INPUT);
      break;

    case 1://NOR
      gateType = 1;
      copy(tempIN2, inPin, sizeof(tempIN2));
      copy(tempOUT2, outPin, sizeof(tempOUT2));
      mcp.pinMode(0, INPUT);
      mcp.pinMode(3, INPUT);
      mcp.pinMode(12, INPUT);
      mcp.pinMode(15, INPUT);
      break;

    case 7: //NAND
      gateType = 7;
      copy(tempIN3, inPin, sizeof(tempIN3));
      copy(tempOUT3, outPin, sizeof(tempOUT3));
      mcp.pinMode(2, INPUT);
      mcp.pinMode(5, INPUT);
      mcp.pinMode(10, INPUT);
      mcp.pinMode(13, INPUT);
      break;

    case 8: //AND GATE
      gateType = 8;
      copy(tempIN3, inPin, sizeof(tempIN3));
      copy(tempOUT3, outPin, sizeof(tempOUT3));
      mcp.pinMode(2, INPUT);
      mcp.pinMode(5, INPUT);
      mcp.pinMode(10, INPUT);
      mcp.pinMode(13, INPUT);
      break;

    case 14://OR GATE
      gateType = 14;
      copy(tempIN3, inPin, sizeof(tempIN3));
      copy(tempOUT3, outPin, sizeof(tempOUT3));
      mcp.pinMode(2, INPUT);
      mcp.pinMode(5, INPUT);
      mcp.pinMode(10, INPUT);
      mcp.pinMode(13, INPUT);
      break;

    default:
      Serial.println("No Gate Selected");
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
      mcp.pinMode(1, INPUT);
      mcp.pinMode(3, INPUT);
      mcp.pinMode(5, INPUT);
      mcp.pinMode(10, INPUT);
      mcp.pinMode(12, INPUT);
      mcp.pinMode(14, INPUT);
      break;

    case 1://NOR
      gateType = 1;
      copy(tempIN4, inPin, sizeof(tempIN4));
      copy(tempOUT4, outPin, sizeof(tempOUT4));
      mcp.pinMode(2, INPUT);
      mcp.pinMode(3, INPUT);
      mcp.pinMode(12, INPUT);
      mcp.pinMode(13, INPUT);
      break;

    case 7: //NAND
      gateType = 7;
      copy(tempIN4, inPin, sizeof(tempIN4));
      copy(tempOUT4, outPin, sizeof(tempOUT4));
      mcp.pinMode(2, INPUT);
      mcp.pinMode(3, INPUT);
      mcp.pinMode(12, INPUT);
      mcp.pinMode(13, INPUT);
      break;

    case 8: //AND GATE
      gateType = 8;
      copy(tempIN4, inPin, sizeof(tempIN4));
      copy(tempOUT4, outPin, sizeof(tempOUT4));
      mcp.pinMode(2, INPUT);
      mcp.pinMode(3, INPUT);
      mcp.pinMode(12, INPUT);
      mcp.pinMode(13, INPUT);
      break;

    case 14://OR GATE
      gateType = 14;
      copy(tempIN4, inPin, sizeof(tempIN4));
      copy(tempOUT4, outPin, sizeof(tempOUT4));
      mcp.pinMode(2, INPUT);
      mcp.pinMode(3, INPUT);
      mcp.pinMode(12, INPUT);
      mcp.pinMode(13, INPUT);
      break;

    default:
      Serial.println("No Gate Selected");
  }
  gateNumber = invert ? 6 : 4;
  return;
}

// Function to copy 'len' elements from 'src' to 'dst'
void copy(int* src, int* dst, int len) {
  memcpy(dst, src, sizeof(src[0])*len);
}
