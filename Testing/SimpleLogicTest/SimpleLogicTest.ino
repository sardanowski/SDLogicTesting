int gatevalue; //Declare all of the variables
int inputPin = 5;
int Output1Pin = 4;
int Output2Pin = 3;
int gateLED = 8;
int led00 = 12;
int led01 = 11;
int led10 = 10;
int led11 = 9;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  pinMode(Output1Pin, OUTPUT);
  pinMode(Output2Pin, OUTPUT);
  pinMode(inputPin, INPUT);
  pinMode(gateLED, OUTPUT);
  pinMode(led00, OUTPUT);
  pinMode(led01, OUTPUT);
  pinMode(led10, OUTPUT);
  pinMode(led11, OUTPUT);
  gatevalue = 0;
}

void loop() {
  gatevalue = gatevalue + (check_Gate(false, false) * 8); //When an input is HIGH it will add a value to the gatevalue
  gatevalue = gatevalue + (check_Gate(false, true) * 4); //and each gate will have a different gatevalue.
  gatevalue = gatevalue + (check_Gate(true, false) * 2);
  gatevalue = gatevalue + (check_Gate(true, true) * 1);

  switch (gatevalue) {
    case 1:
      //When gatevalue is 1 it is an AND gate
      Serial.write("This is an AND gate");
      digitalWrite(gateLED, LOW); //if gate is in, light is off
      digitalWrite(led00, LOW); // AND gate truth table outputs are 0, 0, 0, 1 which is how the LED’s are set up
      digitalWrite(led01, LOW);
      digitalWrite(led10, LOW);
      digitalWrite(led11, HIGH);
      break;
    case 7:
      //When gatevalue is 7 it is an OR gate
      Serial.print("This is an OR gate");
      digitalWrite(gateLED, LOW); //if gate is in, light is off
      digitalWrite(led00, LOW); //Truth table values
      digitalWrite(led01, HIGH);
      digitalWrite(led10, HIGH);
      digitalWrite(led11, HIGH);
      break;
    case 8:
      //When gatevalue is 8 it is an NOR gate
      Serial.write("This is a NOR gate");
      digitalWrite(gateLED, LOW); //if gate is in, light is off
      digitalWrite(led00, HIGH); //Truth table values
      digitalWrite(led01, LOW);
      digitalWrite(led10, LOW);
      digitalWrite(led11, LOW);
      break;
    case 14:
      //When gatevalue is 14 it is an NAND gate
      Serial.write("This is a NAND gate");
      digitalWrite(gateLED, LOW); //if gate is in, light is off
      digitalWrite(led00, HIGH); //Truth table values
      digitalWrite(led01, HIGH);
      digitalWrite(led10, HIGH);
      digitalWrite(led11, LOW);
      break;
    default:
      Serial.write("ERROR: Gate Not Present.");
      digitalWrite(gateLED, HIGH); //if gate is not in, light is on
      digitalWrite(led00, LOW);
      digitalWrite(led01, LOW);
      digitalWrite(led10, LOW);
      digitalWrite(led11, LOW);
  }
  gatevalue = 0;
  delay(1000);
}
int check_Gate(int output1, int output2) {
  int x;
  digitalWrite(Output1Pin, output1);
  digitalWrite(Output2Pin, output2);
  delay(5); // Make sure the signal has time to propogate through the gate.
  x = digitalRead(inputPin);

  return x;
}
