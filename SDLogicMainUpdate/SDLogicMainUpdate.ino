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

byte gateType = 8; //default gate to AND
//void TTLinputPins(byte gateType = 8); //default the gates to AND
//void CMOSinputPins(byte gateType = 8);

byte invert = 0;                   //used to tell if it is a NOT gate (not used yet)
byte numberGates = invert ? 6 : 4; //Used for the number of gates being tested
byte inPin[16];                    //array of input PINS to use
byte outPin[16];                   //array of output PINS to use

//Input/output pins for different gates.
//DO NOT REMOVE OR CHANGE
byte tempIN1[] = {2, 4, 6, 9, 11, 13};        //NOT GATES  //fixed
byte tempOUT1[] = {3, 5, 7, 8, 9, 10, 12};

byte tempIN2[] = {4, 7, 8, 11};               //TTL NOR  //fixed
byte tempOUT2[] = {2, 3, 5, 6, 9, 10, 12, 13};

byte tempIN3[] = {2, 5, 10, 13};               //REST OF TTL //fixed
byte tempOUT3[] = {3, 4, 6, 7, 8, 9, 11, 12};

byte tempIN4[] = {4, 5, 10, 11};               //CMOS GATES  //
byte tempOUT4[] = {2, 3, 6, 7, 8, 9, 12, 13};


void setup() {
  //put your setup code here, to run once:
  //DISPLAY()
  //DEFINE Default pinouts
  //initialize SPI AND I2C

  //I2C
  Serial.begin(9600);
  //TinyWireM.begin();
  Wire.begin();
  mcp.begin();              //using default address of 0 for MCP_23017
  tft.begin();
}

void loop() {
  // put your main code here, to run repeatedly:
  tft.fillScreen(ILI9341_WHITE);
  tft.setTextSize(5);
  tft.setTextColor(ILI9341_BLACK);  //Display stuff
  tft.setCursor(0, 0);
  tft.println("start");

  gateType = 14;             //CHANGE THIS TO CHANGE GATE TYPE
  CMOSinputPins(gateType);   //selecting gate type here

  delay(1000);
  bool result = test(numberGates); //testing this many gates && need to use this as output
  outputResult(result);            //outputting to display

  tft.fillScreen(ILI9341_WHITE);
  tft.setTextSize(5);
  tft.setCursor(0, 0);   //Display stuff
  tft.println("DONE");
  delay(1000);
}

void outputResult(bool result) 
//outputting the pass fail
{

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

bool test(byte numberGates) {
  byte out1, out2, testresults = 0;  //outx are loop index's, testresults is for checking return values
  byte multNum = 1;                  //number to multiply to truth table values to get unique test results
  byte gate;                         //gate number we are on(loop index)
  bool passFail;                     //pass or fail bool
  byte pinout1, pinout2, pinIn;      //pins to output on and input from

  for (gate = 0; gate < numberGates; gate++) {//looping through the number of gates to test
    pinout1 = outPin[gate * 2];               //setting output pin 1
    pinout2 = outPin[gate * 2 + 1];           //setting output pin 2
    pinIn = inPin[gate];                      //seeting input pin
    multNum = 1;                              //resetting variable after each gate itteration
    testresults = 0;                          //resetting variable after each gate itteration

    //    if(!invert){ //testing Inverter more complicated
<<<<<<< HEAD
    for (out1 = 0; out1 <= 1; out1++) {
      for (out2 = 0; out2 <= 1; out2++) {
        byte checkValue = check_Gate(out1, out2, pinout1, pinout2, pinIn);
        testresults = testresults + checkValue * multNum; //probably need to send gate pin numbers.

        tft.fillScreen(ILI9341_WHITE);
        tft.setCursor(0, 0);
        tft.setTextSize(4);
        tft.fillScreen(ILI9341_BLUE);
        tft.setTextColor(ILI9341_BLACK); //code to look at testing results
        tft.println(testresults);
        tft.println(multNum);
        tft.println(checkValue);
        tft.println(pinout1);
        tft.println(pinout2);
        tft.println(pinIn);
        delay(1000);

        multNum = multNum * 2;
=======
    for (out1 = 0; out1 <= 1; out1++) {                                    //looping through binary outputs
      for (out2 = 0; out2 <= 1; out2++) {                                  //looping through binary outputs
        byte checkValue = check_Gate(out1, out2, pinout1, pinout2, pinIn); //outputting binary values and reading input pin
        testresults = testresults + checkValue * multNum;                  //calculation each gates unique number

//        tft.fillScreen(ILI9341_WHITE);
//        tft.setCursor(0, 0);
//        tft.setTextSize(4);
//        tft.setTextColor(ILI9341_BLACK); //code to look at testing results
//        tft.println(testresults);
//        tft.println(multNum);     //testing outputs
//        tft.println(checkValue);
//        tft.println(pinout1);
//        tft.println(pinout2);
//        tft.println(pinIn);
//        delay(500);

        multNum = multNum * 2;                                            //double the num to generate unique test values.
>>>>>>> da2156a27fb773cf53fb8152e29e92185c3267d0

      }
    }
    //    }else{
    //      multNum/4;
    //      for (out2 = 1; out2 >= 0; out1--) {
    //        testresults = testresults + (check_Gate(out1, out2) * multNum); //probably need to send gate pin numbers.
    //        multNum / 2;
    //      }
    //    }
    passFail = (testresults == gateType) ? true : false;                 //check pass or fail

    if (passFail == false) {                                             //return if fail, else continue
      return passFail;
    }
  }
  return passFail;
}

byte check_Gate(byte output1, byte output2, byte outpin1, byte outpin2, byte input1)
//checking what the output is when giving inputs
{
  mcp.digitalWrite(outpin1, output1);                                   //writing output1 to outpin1
  delay(100);
  mcp.digitalWrite(outpin2, output2);                                   //writing output2 to outpin2

  //check for shorts here complicated

  delay(100);                                                           // Make sure the signal has time to propogate through the gate.
  //tft.println("Check Gate Value is :");
  //return 
  byte x = mcp.digitalRead(input1);                                     //reading from the input pin

  tft.fillScreen(ILI9341_WHITE);
  tft.setCursor(0, 0);
  tft.setTextSize(4);
  tft.setTextColor(ILI9341_BLACK); //code to look at testing results
  tft.println(output1);
  tft.println(output2);
  tft.println(x);
  delay(500);
  //  tft.println(x);

  return x;
}

void resetPins() 
//resetting all pins to output and adding an internal pull up
{
  for (byte x = 0; x < 16; x++) {
    mcp.pinMode(x, OUTPUT);
    mcp.pullUp(x,HIGH);
    inPin[x] = 0;
    outPin[x] = 0;
  }
  return;
}

void TTLinputPins(byte gatevalue)
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

  numberGates = invert ? 6 : 4;
  return;
}

void CMOSinputPins(byte gatevalue)
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
  numberGates = invert ? 6 : 4;
  return;
}

// Function to copy 'len' elements from 'src' to 'dst'
void copy(byte* src, byte* dst, int len) {
  memcpy(dst, src, sizeof(src[0])*len);
}
