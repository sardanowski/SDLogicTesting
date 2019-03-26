//#include <TinyWireM.h>
//#include <USI_TWI_Master.h>
#include <Arduino.h>
#include <Wire.h>
//#include <menu.h>
//#include <menuIO/adafruitGfxOut.h>
#include "Adafruit_MCP23017.h"
#include "Adafruit_ILI9341.h"
#include "SPI.h"
#include "Adafruit_GFX.h"
#include "Print.h"
#include <PinChangeInterruptBoards.h>


//display pins
#define TFT_DC 9
#define TFT_CS 10
#define TFT_MOSI 11
#define TFT_CLK 13

//Pin expander pins
#define SDA A4
#define SCL A5
//dispay declaration
Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC);
//Pin expander declaration
Adafruit_MCP23017 mcp;

byte gateType = 8; //default gate to AND
//void TTLinputPins(byte gateType = 8); //default the gates to AND
//void CMOSinputPins(byte gateType = 8);
//Pushbutton pin assignments
const int OK = 8;
const int DOWN = 6;
const int UP = 5;
const int BACK = 4;
const int RESET = 3;
const int LOWBAT = 15;
const int ZIFOFF = 14;


//UI Global variables
int highlighted = 0;

byte invert = 0;                   //used to tell if it is a NOT gate (not used yet)
byte numberGates = invert ? 6 : 4; //Used for the number of gates being tested
byte inPin[16];                    //array of input PINS to use
byte outPin[16];                   //array of output PINS to use

//Input/output pins for different gates.
//DO NOT REMOVE OR CHANGE
byte tempIN1[] = {2, 4, 6, 9, 11, 13};        //NOT GATES  //DOUBLE CHECK THESE GATES!@!@!@!@!@!@!@!@!@!@!@!@!@!@
byte tempOUT1[] = {3, 5, 7, 8, 10, 12};

byte tempIN2[] = {4, 7, 8, 11};               //TTL NOR  //fixed
byte tempOUT2[] = {2, 3, 5, 6, 9, 10, 12, 13};

byte tempIN3[] = {2, 5, 10, 13};               //REST OF TTL //fixed
byte tempOUT3[] = {3, 4, 6, 7, 8, 9, 11, 12};

byte tempIN4[] = {4, 5, 10, 11};               //CMOS GATES  //fixed
byte tempOUT4[] = {2, 3, 6, 7, 8, 9, 12, 13};


void setup() {
  //put your setup code here, to run once:
  //initialize SPI AND I2C

  //I2C
  Serial.begin(9600);
  //TinyWireM.begin();
  Wire.begin();
  mcp.begin();              //using default address of 0 for MCP_23017
  tft.begin();
  //Pushbuttons set to inputs
  pinMode(OK, INPUT);
  pinMode(DOWN, INPUT);
  pinMode(BACK, INPUT);
  pinMode(ZIFOFF, OUTPUT);

  //Pushbuttons are set to an active low trigger
  digitalWrite(OK, HIGH);
  digitalWrite(DOWN, HIGH);
  digitalWrite(BACK, HIGH);
  digitalWrite(UP, HIGH);
  digitalWrite(RESET, HIGH);

  //Turns off power to the ZIF Socket until testing begins
  digitalWrite(ZIFOFF, LOW);

  //Low battery indicator
  digitalWrite(LOWBAT, LOW);

  highlight(highlighted);
}

void loop() {
  // put your main code here, to run repeatedly:
  //Display main menu

  if (digitalRead(DOWN) == LOW) {
    highlighted++;
    if (highlighted > 9)
      highlighted = 0;
    highlight(highlighted);
  }
  if (digitalRead(UP) == LOW) {
    highlighted--;
    if (highlighted < 0)
      highlighted = 9;
    highlight(highlighted);
  }
  if (digitalRead(OK) == LOW)
    testScreen(highlighted);

}
bool check(byte ar[], int n)
//checking if all the elements in the array are the same
//fault checking
{
  bool flag = 1;

  for (byte i = 0; i < n - 1; i++)
  {
    if (ar[i] != ar[i + 1])
      flag = 0;
  }

  return flag;
}

void increment(byte* A, byte len)
//used to increment array for fault checking
{
  boolean carry = true;
  for (int i = (len - 1); i >= 0; i--) {
    if (carry) {
      if (A[i] == 0) {
        A[i] = 1;
        carry = false;
      }
      else {
        A[i] = 0;
        carry = true;
      }
    }
  }
  return;
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
    pinIn = inPin[gate];                      //seeting input pin
    multNum = 1;                              //resetting variable after each gate itteration
    testresults = 0;                          //resetting variable after each gate itteration

    if (!invert) { //testing Inverter more complicated
      pinout1 = outPin[gate * 2];               //setting output pin 1
      pinout2 = outPin[gate * 2 + 1];           //setting output pin 2
      for (out1 = 0; out1 <= 1; out1++) {                                    //looping through binary outputs
        for (out2 = 0; out2 <= 1; out2++) {                                  //looping through binary outputs
          byte checkValue = check_Gate(out1, out2, pinout1, pinout2, pinIn); //outputting binary values and reading input pin
          testresults = testresults + checkValue * multNum;                  //calculation each gates unique number

                  tft.fillScreen(ILI9341_WHITE);
                  tft.setCursor(0, 0);
                  tft.setTextSize(4);
                  tft.setTextColor(ILI9341_BLACK); //code to look at testing results
                  tft.println(testresults);
                  tft.println(multNum);     //testing outputs
                  tft.println(checkValue);
                  tft.println(pinout1);
                  tft.println(pinout2);
                  tft.println(pinIn);
                  delay(500);

          multNum = multNum * 2;                                            //double the num to generate unique test values.

        }
      }
    } else {
      pinout1 = outPin[gate];               //setting output pin 1
      for (out2 = 0; out2 <= 1; out1++) {
        byte checkValue = check_Invert(out1, pinout1, pinIn);      //outputting binary values and reading input pin
        testresults = testresults + checkValue * multNum;                  //calculation each gates unique number
        multNum * 2;
      }
      testresults = testresults == 1 ? 0 : -1;
    }
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
  mcp.digitalWrite(outpin2, output2);                                   //writing output2 to outpin2

  byte x[4];
  //check for shorts here complicated

  byte values[2] = {0, 0};
  byte len = 2;
  byte sendval = 0;
  byte gate = 0;
  byte inc = 0;
  for (inc = 0; inc < sizeof(x); inc++) {
    for (gate = 0; gate < numberGates * 2; gate++) { //looping through the number of gates to test
      byte pinOut = outPin[gate];

      if (pinOut == outpin1 || pinOut == outpin2) continue;

      mcp.digitalWrite(pinOut, values[sendval++]);                                   //writing output1 to outpin1
      if (sendval > 1) sendval = 0;
    }
    delay(5);                                                           // Make sure the signal has time to propogate through the gate.
    x[inc] = mcp.digitalRead(input1);                               //reading from the input pin
    increment(&values[0], len);
    //                      tft.fillScreen(ILI9341_WHITE);
    //                  tft.setCursor(0, 0);
    //                  tft.setTextSize(4);
    //                  tft.setTextColor(ILI9341_BLACK); //code to look at testing results
    //                  tft.println(values[0]);
    //                  tft.println(values[1]);     //testing outputs
    //                  tft.println(sizeof(x));
    //                  tft.println(inc);
  }
  byte retval = check(x, sizeof(x)) ? x[0] : -100;

  return retval;
}

byte check_Invert(byte output1, byte outpin1, byte input1)
//checking what the output is when giving inputs
{
  mcp.digitalWrite(outpin1, output1);                                   //writing output1 to outpin1

  byte x[2];
  //check for shorts here complicated
  byte values = 0;
  byte gate = 0;
  byte inc = 0;
  for (inc = 0; inc < sizeof(x); inc++) {
    for (gate = 0; gate < numberGates; gate++) {//looping through the number of gates to test
      byte pinOut = outPin[gate];

      if (pinOut == outpin1) continue;

      mcp.digitalWrite(pinOut, values);                                   //writing output1 to outpin1

    }
    delay(10);                                                           // Make sure the signal has time to propogate through the gate.
    x[inc] = mcp.digitalRead(input1);                               //reading from the input pin
    values++;
  }

  byte retval = check(x, sizeof(x)) ? x[0] : -100;

  return retval;
}

void resetPins()
//resetting all pins to output and adding an internal pull up
{
  for (byte x = 0; x < 16; x++) {
    mcp.pinMode(x, OUTPUT);
    mcp.pullUp(x, HIGH);
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

//Function to highlight text for UI
void highlight(int x) {
  tft.setCursor(0, 0);
  tft.setTextSize(2);
  tft.fillScreen(ILI9341_BLACK);
  if (x == 0) tft.setTextColor(ILI9341_YELLOW);
  else tft.setTextColor(ILI9341_WHITE);
  tft.println("TTL AND Gate");
  if (x == 1) tft.setTextColor(ILI9341_YELLOW);
  else if (x == 0) tft.setTextColor(ILI9341_WHITE);
  tft.println("TTL OR Gate");
  if (x == 2) tft.setTextColor(ILI9341_YELLOW);
  else if (x == 1) tft.setTextColor(ILI9341_WHITE);
  tft.println("TTL NAND Gate");
  if (x == 3) tft.setTextColor(ILI9341_YELLOW);
  else if (x == 2) tft.setTextColor(ILI9341_WHITE);
  tft.println("TTL NOR Gate");
  if (x == 4) tft.setTextColor(ILI9341_YELLOW);
  else if (x == 3) tft.setTextColor(ILI9341_WHITE);
  tft.println("TTL Inverter");
  if (x == 5) tft.setTextColor(ILI9341_YELLOW);
  else if (x == 4) tft.setTextColor(ILI9341_WHITE);
  tft.println("CMOS AND Gate");
  if (x == 6) tft.setTextColor(ILI9341_YELLOW);
  else if (x == 5) tft.setTextColor(ILI9341_WHITE);
  tft.println("CMOS OR Gate");
  if (x == 7) tft.setTextColor(ILI9341_YELLOW);
  else if (x == 6) tft.setTextColor(ILI9341_WHITE);
  tft.println("CMOS NAND Gate");
  if (x == 8) tft.setTextColor(ILI9341_YELLOW);
  else if (x == 7) tft.setTextColor(ILI9341_WHITE);
  tft.println("CMOS NOR Gate");
  if (x == 9) tft.setTextColor(ILI9341_YELLOW);
  else if (x == 8) tft.setTextColor(ILI9341_WHITE);
  tft.println("CMOS Inverter");
  /*switch (x) {
    //TTL And highlighted
    case 0:
      tft.fillScreen(ILI9341_BLACK);
      tft.setTextColor(ILI9341_YELLOW);
      tft.println("TTL AND Gate");
      tft.setTextColor(ILI9341_WHITE);
      tft.println("TTL OR Gate");
      tft.println("TTL NAND Gate");
      tft.println("TTL NOR Gate");
      tft.println("TTL Inverter");
      tft.println("CMOS AND Gate");
      tft.println("CMOS OR Gate");
      tft.println("CMOS NAND Gate");
      tft.println("CMOS NOR Gate");
      tft.println("CMOS Inverter");
      break;

    //TTL OR Highlighted
    case 1:
      tft.fillScreen(ILI9341_BLACK);
      tft.setTextColor(ILI9341_WHITE);
      tft.println("TTL AND Gate");
      tft.setTextColor(ILI9341_YELLOW);
      tft.println("TTL OR Gate");
      tft.setTextColor(ILI9341_WHITE);
      tft.println("TTL NAND Gate");
      tft.println("TTL NOR Gate");
      tft.println("TTL Inverter");
      tft.println("CMOS AND Gate");
      tft.println("CMOS OR Gate");
      tft.println("CMOS NAND Gate");
      tft.println("CMOS NOR Gate");
      tft.println("CMOS Inverter");
      break;

    //TTL NAND Highlighted
    case 2:
      tft.fillScreen(ILI9341_BLACK);
      tft.setTextColor(ILI9341_WHITE);
      tft.println("TTL AND Gate");
      tft.println("TTL OR Gate");
      tft.setTextColor(ILI9341_YELLOW);
      tft.println("TTL NAND Gate");
      tft.setTextColor(ILI9341_WHITE);
      tft.println("TTL NOR Gate");
      tft.println("TTL Inverter");
      tft.println("CMOS AND Gate");
      tft.println("CMOS OR Gate");
      tft.println("CMOS NAND Gate");
      tft.println("CMOS NOR Gate");
      tft.println("CMOS Inverter");
      break;

    //TTL NOR HIghlighted
    case 3:
      tft.fillScreen(ILI9341_BLACK);
      tft.setTextColor(ILI9341_WHITE);
      tft.println("TTL AND Gate");
      tft.println("TTL OR Gate");
      tft.println("TTL NAND Gate");
      tft.setTextColor(ILI9341_YELLOW);
      tft.println("TTL NOR Gate");
      tft.setTextColor(ILI9341_WHITE);
      tft.println("TTL Inverter");
      tft.println("CMOS AND Gate");
      tft.println("CMOS OR Gate");
      tft.println("CMOS NAND Gate");
      tft.println("CMOS NOR Gate");
      tft.println("CMOS Inverter");
      break;

    //TTL NOT
    case 4:
      tft.fillScreen(ILI9341_BLACK);
      tft.setTextColor(ILI9341_WHITE);
      tft.println("TTL AND Gate");
      tft.println("TTL OR Gate");
      tft.println("TTL NAND Gate");
      tft.println("TTL NOR Gate");
      tft.setTextColor(ILI9341_YELLOW);
      tft.println("TTL Inverter");
      tft.setTextColor(ILI9341_WHITE);
      tft.println("CMOS AND Gate");
      tft.println("CMOS OR Gate");
      tft.println("CMOS NAND Gate");
      tft.println("CMOS NOR Gate");
      tft.println("CMOS Inverter");
      break;

    //CMOS AND
    case 5:
      tft.fillScreen(ILI9341_BLACK);
      tft.setTextColor(ILI9341_WHITE);
      tft.println("TTL AND Gate");
      tft.println("TTL OR Gate");
      tft.println("TTL NAND Gate");
      tft.println("TTL NOR Gate");
      tft.println("TTL Inverter");
      tft.setTextColor(ILI9341_YELLOW);
      tft.println("CMOS AND Gate");
      tft.setTextColor(ILI9341_WHITE);
      tft.println("CMOS OR Gate");
      tft.println("CMOS NAND Gate");
      tft.println("CMOS NOR Gate");
      tft.println("CMOS Inverter");
      break;

    //CMOS OR
    case 6:
      tft.fillScreen(ILI9341_BLACK);
      tft.setTextColor(ILI9341_WHITE);
      tft.println("TTL AND Gate");
      tft.println("TTL OR Gate");
      tft.println("TTL NAND Gate");
      tft.println("TTL NOR Gate");
      tft.println("TTL Inverter");
      tft.println("CMOS AND Gate");
      tft.setTextColor(ILI9341_YELLOW);
      tft.println("CMOS OR Gate");
      tft.setTextColor(ILI9341_WHITE);
      tft.println("CMOS NAND Gate");
      tft.println("CMOS NOR Gate");
      tft.println("CMOS Inverter");
      break;

    //CMOS NAND
    case 7:
      tft.fillScreen(ILI9341_BLACK);
      tft.setTextColor(ILI9341_WHITE);
      tft.println("TTL AND Gate");
      tft.println("TTL OR Gate");
      tft.println("TTL NAND Gate");
      tft.println("TTL NOR Gate");
      tft.println("TTL Inverter");
      tft.println("CMOS AND Gate");
      tft.println("CMOS OR Gate");
      tft.setTextColor(ILI9341_YELLOW);
      tft.println("CMOS NAND Gate");
      tft.setTextColor(ILI9341_WHITE);
      tft.println("CMOS NOR Gate");
      tft.println("CMOS Inverter");
      break;

    //CMOS NOR
    case 8:
      tft.fillScreen(ILI9341_BLACK);
      tft.setTextColor(ILI9341_WHITE);
      tft.println("TTL AND Gate");
      tft.println("TTL OR Gate");
      tft.println("TTL NAND Gate");
      tft.println("TTL NOR Gate");
      tft.println("TTL Inverter");
      tft.println("CMOS AND Gate");
      tft.println("CMOS OR Gate");
      tft.println("CMOS NAND Gate");
      tft.setTextColor(ILI9341_YELLOW);
      tft.println("CMOS NOR Gate");
      tft.setTextColor(ILI9341_WHITE);
      tft.println("CMOS Inverter");
      break;

    //CMOS NOT
    case 9:
      tft.fillScreen(ILI9341_BLACK);
      tft.setTextColor(ILI9341_WHITE);
      tft.println("TTL AND Gate");
      tft.println("TTL OR Gate");
      tft.println("TTL NAND Gate");
      tft.println("TTL NOR Gate");
      tft.println("TTL Inverter");
      tft.println("CMOS AND Gate");
      tft.println("CMOS OR Gate");
      tft.println("CMOS NAND Gate");
      tft.println("CMOS NOR Gate");
      tft.setTextColor(ILI9341_YELLOW);
      tft.println("CMOS Inverter");
      tft.setTextColor(ILI9341_WHITE);
      break;
    }*/
}


//Testing screen
//Will display device being tested, number of passes, fails, and previous result
void testScreen(int testNum) {
  int leave = 0;
  int passed;
  int failed;
  bool prev;
  tft.setCursor(0, 0);
  tft.fillScreen(ILI9341_BLACK);
  tft.setTextSize(2);
  //runs the proper testing procedure and display
  switch (testNum) {
    case 0:
      passed = 0;
      failed = 0;
      tft.println("Press OK to test TTL And Gate");
      // delay(5000);  //allow for button debouncing
      while (1) {
        if (digitalRead(OK) == LOW) {
          tft.fillScreen(ILI9341_BLACK);
          gateType = 8;
          digitalWrite(ZIFOFF, HIGH);
          delay(1000);
          TTLinputPins(gateType);
          delay(10);
          prev = test(numberGates);
          //prev = test(8);
          outputResult(prev);
          if (prev == false)
            failed++;
          if (prev == true)
            passed++;
          tft.setTextColor(ILI9341_WHITE);
          tft.fillScreen(ILI9341_BLACK);
          tft.setCursor(0, 0);
          tft.setTextSize(2);
          tft.printf("Passed: %d\n", passed);
          tft.printf("Failed: %d\n", failed);
          if (prev == false)
            tft.println("Previous: Failed");
          if (prev == true)
            tft.println("Previous: Passed");
          digitalWrite(ZIFOFF, LOW);
          while (1) {
            if (digitalRead(OK) == LOW)
              break;
            if (digitalRead(BACK) == LOW) {
              leave = 1;
              break;
            }
          }
        }
        if (leave == 1) {
          leave = 0;
          highlight(highlighted);
          break;
        }
      }
      break;

    case 1:
      failed = 0;
      passed = 0;
      tft.println("Press OK to test TTL Or Gate");
      while (1) {
        if (digitalRead(OK) == LOW) {
          tft.fillScreen(ILI9341_BLACK);
          //prev = test(8);
          if (prev == 0)
            failed++;
          if (prev == 1)
            passed++;
          tft.setCursor(0, 0);
          tft.setTextSize(2);
          tft.printf("Passed: %d\n", passed);
          tft.printf("Failed: %d\n", failed);
          if (prev == 0)
            tft.println("Previous: Failed");
          if (prev == 1)
            tft.println("Previous: Passed");
          while (1) {
            if (digitalRead(OK) == LOW)
              break;
            if (digitalRead(BACK) == LOW) {
              leave = 1;
              break;
            }
          }
        }
        if (leave == 1) {
          leave = 0;
          highlight(highlighted);
          break;
        }
      }
      break;

    case 2:

      tft.println("Press OK to test TTL NAND Gate");
      failed = 0;
      passed = 0;
      while (1) {
        if (digitalRead(OK) == LOW) {
          tft.fillScreen(ILI9341_BLACK);
          //prev = test(8);
          if (prev == 0)
            failed++;
          if (prev == 1)
            passed++;
          tft.setCursor(0, 0);
          tft.setTextSize(2);
          tft.printf("Passed: %d\n", passed);
          tft.printf("Failed: %d\n", failed);
          if (prev == 0)
            tft.println("Previous: Failed");
          if (prev == 1)
            tft.println("Previous: Passed");
          while (1) {
            if (digitalRead(OK) == LOW)
              break;
            if (digitalRead(BACK) == LOW) {
              leave = 1;
              break;
            }
          }
        }
        if (leave == 1) {
          leave = 0;
          highlight(highlighted);
          break;
        }
      }
      break;

    case 3:

      tft.println("Press OK to test TTL NOR Gate");
      failed = 0;
      passed = 0;
      while (1) {
        if (digitalRead(OK) == LOW) {
          tft.fillScreen(ILI9341_BLACK);
          //prev = test(8);
          if (prev == 0)
            failed++;
          if (prev == 1)
            passed++;
          tft.setCursor(0, 0);
          tft.setTextSize(2);
          tft.printf("Passed: %d\n", passed);
          tft.printf("Failed: %d\n", failed);
          if (prev == 0)
            tft.println("Previous: Failed");
          if (prev == 1)
            tft.println("Previous: Passed");
          while (1) {
            if (digitalRead(OK) == LOW)
              break;
            if (digitalRead(BACK) == LOW) {
              leave = 1;
              break;
            }
          }
        }
        if (leave == 1) {
          leave = 0;
          highlight(highlighted);
          break;
        }
      }
      break;

    case 4:

      tft.println("Press OK to test TTL Inverter");
      failed = 0;
      passed = 0;
      while (1) {
        if (digitalRead(OK) == LOW) {
          tft.fillScreen(ILI9341_BLACK);
          //prev = test(8);
          if (prev == 0)
            failed++;
          if (prev == 1)
            passed++;
          tft.setCursor(0, 0);
          tft.setTextSize(2);
          tft.printf("Passed: %d\n", passed);
          tft.printf("Failed: %d\n", failed);
          if (prev == 0)
            tft.println("Previous: Failed");
          if (prev == 1)
            tft.println("Previous: Passed");
          while (1) {
            if (digitalRead(OK) == LOW)
              break;
            if (digitalRead(BACK) == LOW) {
              leave = 1;
              break;
            }
          }
        }
        if (leave == 1) {
          leave = 0;
          highlight(highlighted);
          break;
        }
      }
      break;

    case 5:

      tft.println("Press OK to test CMOS AND Gate");
      failed = 0;
      passed = 0;
      while (1) {
        if (digitalRead(OK) == LOW) {
          tft.fillScreen(ILI9341_BLACK);
          //prev = test(8);
          if (prev == 0)
            failed++;
          if (prev == 1)
            passed++;
          tft.setCursor(0, 0);
          tft.setTextSize(2);
          tft.printf("Passed: %d\n", passed);
          tft.printf("Failed: %d\n", failed);
          if (prev == 0)
            tft.println("Previous: Failed");
          if (prev == 1)
            tft.println("Previous: Passed");
          while (1) {
            if (digitalRead(OK) == LOW)
              break;
            if (digitalRead(BACK) == LOW) {
              leave = 1;
              break;
            }
          }
        }
        if (leave == 1) {
          leave = 0;
          highlight(highlighted);
          break;
        }
      }
      break;

    case 6:

      tft.println("Press OK to test CMOS OR Gate");
      failed = 0;
      passed = 0;
      while (1) {
        if (digitalRead(OK) == LOW) {
          tft.fillScreen(ILI9341_BLACK);
          //prev = test(8);
          if (prev == 0)
            failed++;
          if (prev == 1)
            passed++;
          tft.setCursor(0, 0);
          tft.setTextSize(2);
          tft.printf("Passed: %d\n", passed);
          tft.printf("Failed: %d\n", failed);
          if (prev == 0)
            tft.println("Previous: Failed");
          if (prev == 1)
            tft.println("Previous: Passed");
          while (1) {
            if (digitalRead(OK) == LOW)
              break;
            if (digitalRead(BACK) == LOW) {
              leave = 1;
              break;
            }
          }
        }
        if (leave == 1) {
          leave = 0;
          highlight(highlighted);
          break;
        }
      }
      break;

    case 7:

      tft.println("Press OK to test CMOS NAND Gate");
      failed = 0;
      passed = 0;
      while (1) {
        if (digitalRead(OK) == LOW) {
          tft.fillScreen(ILI9341_BLACK);
          //prev = test(8);
          if (prev == 0)
            failed++;
          if (prev == 1)
            passed++;
          tft.setCursor(0, 0);
          tft.setTextSize(2);
          tft.printf("Passed: %d\n", passed);
          tft.printf("Failed: %d\n", failed);
          if (prev == 0)
            tft.println("Previous: Failed");
          if (prev == 1)
            tft.println("Previous: Passed");
          while (1) {
            if (digitalRead(OK) == LOW)
              break;
            if (digitalRead(BACK) == LOW) {
              leave = 1;
              break;
            }
          }
        }
        if (leave == 1) {
          leave = 0;
          highlight(highlighted);
          break;
        }
      }
      break;

    case 8:

      tft.println("Press OK to test CMOS NOR Gate");
      failed = 0;
      passed = 0;
      while (1) {
        if (digitalRead(OK) == LOW) {
          tft.fillScreen(ILI9341_BLACK);
          //prev = test(8);
          if (prev == 0)
            failed++;
          if (prev == 1)
            passed++;
          tft.setCursor(0, 0);
          tft.setTextSize(2);
          tft.printf("Passed: %d\n", passed);
          tft.printf("Failed: %d\n", failed);
          if (prev == 0)
            tft.println("Previous: Failed");
          if (prev == 1)
            tft.println("Previous: Passed");
          while (1) {
            if (digitalRead(OK) == LOW)
              break;
            if (digitalRead(BACK) == LOW) {
              leave = 1;
              break;
            }
          }
        }
        if (leave == 1) {
          leave = 0;
          highlight(highlighted);
          break;
        }
      }
      break;

    case 9:

      tft.println("Press OK to test CMOS Inverter");
      failed = 0;
      passed = 0;
      while (1) {
        if (digitalRead(OK) == LOW) {
          tft.fillScreen(ILI9341_BLACK);
          //prev = test(8);
          if (prev == 0)
            failed++;
          if (prev == 1)
            passed++;
          tft.setCursor(0, 0);
          tft.setTextSize(2);
          tft.printf("Passed: %d\n", passed);
          tft.printf("Failed: %d\n", failed);
          if (prev == 0)
            tft.println("Previous: Failed");
          if (prev == 1)
            tft.println("Previous: Passed");
          while (1) {
            if (digitalRead(OK) == LOW)
              break;
            if (digitalRead(BACK) == LOW) {
              leave = 1;
              break;
            }
          }
        }
        if (leave == 1) {
          leave = 0;
          highlight(highlighted);
          break;
        }
      }
      break;
  }
}
