#include <Arduino.h>
#include <Wire.h>
#include "Adafruit_MCP23017.h"
#include "Adafruit_ILI9341.h"
#include "SPI.h"
#include "Adafruit_GFX.h"
#include "Print.h"
#include <PinChangeInterruptBoards.h>


//display pins
#define TFT_DC 9
#define TFT_CS 10

//dispay declaration
Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC);
//Pin expander declaration
Adafruit_MCP23017 mcp;

byte gateType; //Gate Type defined by
//AND = 8
//OR = 14
//NAND = 7
//NOR = 1
//Inverter = 0

//Pushbutton pin assignments
const int OK = 8;
const int DOWN = 6;
const int UP = 5;
const int BACK = 4;
const int RESET = 3;
const int LOWBAT = 15;
const int ZIFOFF = 14;
const int EXPANDER = 16;


//UI Global variables
int highlighted = 0;

byte invert = 0;                   //used to tell if it is a NOT gate
byte numberGates = invert ? 6 : 4; //Used for the number of gates being tested
byte inPin[16];                    //array of input PINS to use
byte outPin[16];                   //array of output PINS to use

//Input/output pins for different gates.
//DO NOT REMOVE OR CHANGE
byte NOTIN[] = {2, 4, 6, 10, 12, 14};        //NOT GATES Pin Assignement
byte NOTOUT[] = {3, 5, 7, 9, 11, 13};

byte TTLNORIN[] = {4, 7, 9, 12};               //TTL NOR  Pin Assignement
byte TTLNOROUT[] = {2, 3, 5, 6, 10, 11, 13, 14};

byte TTLIN[] = {2, 5, 11, 14};               //REST OF TTL Pin Assignement
byte TTLOUT[] = {3, 4, 6, 7, 9, 10, 12, 13};

byte CMOSIN[] = {4, 5, 11, 12};               //CMOS GATES  Pin Assignement
byte CMOSOUT[] = {2, 3, 6, 7, 9, 10, 13, 14};

void setup()
//Initializing I2C, SPI, and Pins on Pro Trinket
{
  Serial.begin(9600);
  Wire.begin();
  mcp.begin();              //using default address of 0 for MCP_23017
  tft.begin();

  //Pushbuttons set to inputs
  pinMode(OK, INPUT);
  pinMode(DOWN, INPUT);
  pinMode(BACK, INPUT);
  pinMode(RESET, INPUT);
  pinMode(UP, INPUT);

  //Pushbuttons are set to an active low trigger
  digitalWrite(OK, HIGH);
  digitalWrite(DOWN, HIGH);
  digitalWrite(BACK, HIGH);
  digitalWrite(UP, HIGH);
  digitalWrite(RESET, HIGH);

  //Relay control is an output
  pinMode(ZIFOFF, OUTPUT);
  pinMode(EXPANDER, OUTPUT);

  //Turns off power to the ZIF Socket until testing begins
  digitalWrite(ZIFOFF, LOW);
  digitalWrite(EXPANDER, HIGH);

  //Low battery indicator set to input
  pinMode(LOWBAT, INPUT);

  //Low battery indicator
  digitalWrite(LOWBAT, LOW);
  tft.setCursor(0, 0);
  tft.setTextSize(3);
  tft.fillScreen(ILI9341_BLACK);
  highlight(highlighted);
}

void loop()
//This is the main loop used to navigate the menu with the push buttons
{

  while (1) {
    if (digitalRead(DOWN) == LOW) {  //When down pressed highlight the next gate, loop around if at bottom
      delay(100);
      highlighted++;
      if (highlighted > 9)
        highlighted = 0;
      highlight(highlighted);
    }
    if (digitalRead(UP) == LOW) {    //When Up pressed highlight the previous gate, loop around if at top
      delay(100);
      highlighted--;
      if (highlighted < 0)
        highlighted = 9;
      highlight(highlighted);
    }
    if (digitalRead(OK) == LOW) {     //When okay is pressed, move to testScreen menu and loop
      delay(100);
      testScreen(highlighted);
    }
    if (batteryCheck()) {
      highlight(highlighted);
    }
  }
}

bool batteryCheck()
//Used to check if the battery is low or not
{
  if (digitalRead(LOWBAT) == LOW) {
    tft.fillScreen(ILI9341_RED);
    tft.setCursor(0, 0);
    tft.setTextSize(2);
    tft.println("LOW BATTERY");
    tft.println("PLEASE CHARGE");
    tft.println("TO RESUME TESTING");
    while (digitalRead(LOWBAT) == HIGH) {};
    return true;
  }
  return false;
}
bool check(byte ar[], int n)
//checking if all the elements in the array are the same
//This is used to check the array of results from fault checking
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
//used to increment binary array for fault checking
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
    tft.setCursor(65, 150);
    tft.fillScreen(ILI9341_RED);
    tft.setTextSize(4);
    tft.setTextColor(ILI9341_BLACK);
    tft.println("FAIL");
  }
  else {
    tft.setCursor(65, 150);
    tft.fillScreen(ILI9341_GREEN);
    tft.setTextSize(4);
    tft.setTextColor(ILI9341_BLACK);
    tft.println("PASS");
  }
  delay(1000);
}

bool test(byte numberGates) {
  byte out1, out2, testresults = 0;  //outx are loop index's, testresults is for checking return values
  byte multNum = 1;                  //number to multiply to truth table values to get unique test results
  byte gate;                         //gate number we are on(loop index)
  bool passFail;                     //pass or fail bool
  byte pinout1, pinout2, pinIn;      //pins to output on and input from

  tft.fillScreen(ILI9341_WHITE);
  tft.setCursor(0, 0);
  tft.setTextSize(3);
  tft.setTextColor(ILI9341_BLACK); //code to look at testing results
  tft.println("TESTING\nPLEASE WAIT..");

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
          multNum = multNum * 2;                                            //double the num to generate unique test values.

        }
      }
    } else if (invert) {
      pinout1 = outPin[gate];               //setting output pin 1
      for (out1 = 0; out1 <= 1; out1++) {
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
  }
  zeroPins();
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
    delay(5);                                                           // Make sure the signal has time to propogate through the gate.
    x[inc] = mcp.digitalRead(input1);                               //reading from the input pin
    values++;
  }

  zeroPins();
  byte retval = x[1] == x[0] ? x[0] : -100;

  return retval;
}

void resetPins()
//resetting all pins to output and adding an internal pull up
{
  for (byte x = 0; x < 16; x++) {
    mcp.pinMode(x, OUTPUT);
    mcp.pullUp(x, HIGH);
    mcp.digitalWrite(x, 0);
    inPin[x] = 0;
    outPin[x] = 0;
  }
  return;
}

void zeroPins()
//setting all ouputs to be 0
{
  for (byte x = 0; x < 16; x++) {
    mcp.digitalWrite(x, 0);
  }
  return;
}

void assignInputs()
{
  if (invert)
  {
    mcp.pinMode(inPin[0], INPUT);
    mcp.pinMode(inPin[1], INPUT);
    mcp.pinMode(inPin[2], INPUT);
    mcp.pinMode(inPin[3], INPUT);
    mcp.pinMode(inPin[4], INPUT);
    mcp.pinMode(inPin[5], INPUT);
  }
  else
  {
    mcp.pinMode(inPin[0], INPUT);
    mcp.pinMode(inPin[1], INPUT);
    mcp.pinMode(inPin[2], INPUT);
    mcp.pinMode(inPin[3], INPUT);
  }
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
      copy(NOTIN, inPin, sizeof(NOTIN));
      copy(NOTOUT, outPin, sizeof(NOTOUT));
      assignInputs();
      break;

    case 1://NOR
      gateType = 1;
      copy(TTLNORIN, inPin, sizeof(TTLNORIN));
      copy(TTLNOROUT, outPin, sizeof(TTLNOROUT));
      assignInputs();
      break;

    case 7: //NAND
      gateType = 7;
      copy(TTLIN, inPin, sizeof(TTLIN));
      copy(TTLOUT, outPin, sizeof(TTLOUT));
      assignInputs();
      break;

    case 8: //AND GATE
      gateType = 8;
      copy(TTLIN, inPin, sizeof(TTLIN));
      copy(TTLOUT, outPin, sizeof(TTLOUT));
      assignInputs();
      break;

    case 14://OR GATE
      gateType = 14;
      copy(TTLIN, inPin, sizeof(TTLIN));
      copy(TTLOUT, outPin, sizeof(TTLOUT));
      assignInputs();
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
      copy(NOTIN, inPin, sizeof(NOTIN));
      copy(NOTOUT, outPin, sizeof(NOTOUT));
      assignInputs();
      break;

    case 1://NOR
      gateType = 1;
      copy(CMOSIN, inPin, sizeof(CMOSIN));
      copy(CMOSOUT, outPin, sizeof(CMOSOUT));
      assignInputs();
      break;

    case 7: //NAND
      gateType = 7;
      copy(CMOSIN, inPin, sizeof(CMOSIN));
      copy(CMOSOUT, outPin, sizeof(CMOSOUT));
      assignInputs();
      break;

    case 8: //AND GATE
      gateType = 8;
      copy(CMOSIN, inPin, sizeof(CMOSIN));
      copy(CMOSOUT, outPin, sizeof(CMOSOUT));
      assignInputs();
      break;

    case 14://OR GATE
      gateType = 14;
      copy(CMOSIN, inPin, sizeof(CMOSIN));
      copy(CMOSOUT, outPin, sizeof(CMOSOUT));
      assignInputs();
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
  tft.setTextSize(3);
  if (x == 0) tft.setTextColor(ILI9341_YELLOW);
  else tft.setTextColor(ILI9341_WHITE);
  tft.println("74LS08 TTL AND");
  if (x == 1) tft.setTextColor(ILI9341_YELLOW);
  else if (x == 0) tft.setTextColor(ILI9341_WHITE);
  tft.println("74LS32 TTL OR");
  if (x == 2) tft.setTextColor(ILI9341_YELLOW);
  else if (x == 1) tft.setTextColor(ILI9341_WHITE);
  tft.println("74LS00 TTL NAND");
  if (x == 3) tft.setTextColor(ILI9341_YELLOW);
  else if (x == 2) tft.setTextColor(ILI9341_WHITE);
  tft.println("74LS02 TTL NOR");
  if (x == 4) tft.setTextColor(ILI9341_YELLOW);
  else if (x == 3) tft.setTextColor(ILI9341_WHITE);
  tft.println("74LS04 TTL Inverter");
  if (x == 5) tft.setTextColor(ILI9341_YELLOW);
  else if (x == 4) tft.setTextColor(ILI9341_WHITE);
  tft.println("CD4081 CMOS AND");
  if (x == 6) tft.setTextColor(ILI9341_YELLOW);
  else if (x == 5) tft.setTextColor(ILI9341_WHITE);
  tft.println("CD4071 CMOS OR");
  if (x == 7) tft.setTextColor(ILI9341_YELLOW);
  else if (x == 6) tft.setTextColor(ILI9341_WHITE);
  tft.println("CD4011 CMOS NAND");
  if (x == 8) tft.setTextColor(ILI9341_YELLOW);
  else if (x == 7) tft.setTextColor(ILI9341_WHITE);
  tft.println("CD4001 CMOS NOR");
  if (x == 9) tft.setTextColor(ILI9341_YELLOW);
  else if (x == 8) tft.setTextColor(ILI9341_WHITE);
  tft.println("CD4069 CMOS Inverter");
  tft.setTextColor(ILI9341_WHITE);
  loop();
}

int passed = 0;
int failed = 0;
int reset = 0;
bool prev;

void displayGate(){
  switch (highlighted) {
    case 0:
      tft.println("Press OK to \ntest 74LS08 TTL And Gate");
      break;

    case 1:
      tft.println("Press OK to \ntest 74LS32 TTL OR Gate");
      break;

    case 2://NAND
      tft.println("Press OK to \ntest 74LS00 TTL Nand Gate");
      break;

    case 3://NOR
      tft.println("Press OK to \ntest 74LS02 TTL Nor Gate");
      break;

    case 4://NOT
      tft.println("Press OK to \ntest 74LS04 TTL Inverter");
      break;

    case 5://AND
      tft.println("Press OK to test CD4081 CMOS AND Gate");
      break;

    case 6://OR
      tft.println("Press OK to test CD4071 CMOS OR Gate");
      break;

    case 7://NAND
      tft.println("Press OK to test CD4011 CMOS NAND Gate");
      break;

    case 8://NOR
      tft.println("Press OK to test CD4001 CMOS NOR Gate");
      break;

    case 9://NOT
      tft.println("Press OK to test CD4069 CMOS Inverter");
      break;
  }
}

void testingGates() {
  while (1) {
    if (digitalRead(OK) == LOW) {
      batteryCheck();
      digitalWrite(ZIFOFF, HIGH);
      delay(100);
      prev = test(numberGates);
      outputResult(prev);
      if (prev == false)
        failed++;
      if (prev == true)
        passed++;
      tft.setTextColor(ILI9341_WHITE);
      tft.fillScreen(ILI9341_BLACK);
      tft.setCursor(0, 0);
      tft.setTextSize(3);
      tft.printf("Passed: %d\n", passed);
      tft.printf("Failed: %d\n", failed);
      if (prev == false)
        tft.println("Previous:\n Failed");
      if (prev == true)
        tft.println("Previous:\n Passed");
      displayGate();
      digitalWrite(ZIFOFF, LOW);
      while (1) {
        if (digitalRead(OK) == LOW)
          break;
        if (digitalRead(BACK) == LOW) {
          tft.setCursor(0, 0);
          tft.fillScreen(ILI9341_BLACK);
          highlight(highlighted);
          loop();
        }
        if (digitalRead(RESET) == LOW) {
          tft.setCursor(0, 0);
          tft.fillScreen(ILI9341_BLACK);
          tft.setTextSize(3);
          tft.setTextColor(ILI9341_WHITE);
          tft.println("Counts reset");
          reset = 1;
          testScreen(highlighted);
        }
      }
    }
    if (digitalRead(BACK) == LOW) {
      tft.setCursor(0, 0);
      tft.fillScreen(ILI9341_BLACK);
      highlight(highlighted);
      loop();
    }
  }
}

//Testing screen
//Will display device being tested, number of passes, fails, and previous result
void testScreen(int testNum) {
  passed = 0;
  failed = 0;
  if (reset == 0) {
    tft.setCursor(0, 0);
    tft.fillScreen(ILI9341_BLACK);
    tft.setTextSize(3);
  }
  reset = 0;
  //runs the proper testing procedure and display
  switch (testNum) {
    case 0:
      tft.println("Press OK to \ntest TTL And Gate");
      gateType = 8;
      TTLinputPins(gateType);
      testingGates();
      break;

    case 1:
      tft.println("Press OK to \ntest TTL OR Gate");
      gateType = 14;
      TTLinputPins(gateType);
      testingGates();
      break;

    case 2://NAND
      tft.println("Press OK to \ntest TTL Nand Gate");
      gateType = 7;
      TTLinputPins(gateType);
      testingGates();
      break;

    case 3://NOR
      tft.println("Press OK to \ntest TTL Nor Gate");
      gateType = 1;
      TTLinputPins(gateType);
      testingGates();
      break;

    case 4://NOT
      tft.println("Press OK to \ntest TTL Inverter");
      gateType = 0;
      TTLinputPins(gateType);
      testingGates();
      break;

    case 5://AND

      tft.println("Press OK to test CMOS AND Gate");
      gateType = 8;
      CMOSinputPins(gateType);
      testingGates();
      break;

    case 6://OR
      tft.println("Press OK to test CMOS OR Gate");
      gateType = 14;
      CMOSinputPins(gateType);
      testingGates();
      break;

    case 7://NAND
      tft.println("Press OK to test CMOS NAND Gate");
      gateType = 7;
      CMOSinputPins(gateType);
      testingGates();
      break;

    case 8://NOR
      tft.println("Press OK to test CMOS NOR Gate");
      gateType = 1;
      CMOSinputPins(gateType);
      testingGates();
      break;

    case 9://NOT
      tft.println("Press OK to test CMOS Inverter");
      gateType = 0;
      CMOSinputPins(gateType);
      testingGates();
      break;
  }
}
