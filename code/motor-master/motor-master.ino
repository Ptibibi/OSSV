// Master code for Arduino Uno for the Saddle Vibrator
// https://www.thingiverse.com/thing:3554455
// Version 1.0 (14.4.19)

#include <Wire.h>

byte inData[10];                                 // incomming data array for data from controller (make larger than you need)

const int  motor0_out = 5; //3,5,9,10,11                          // pin designation for motor 0
const int  motor0_in1 = 4; //3,5,9,10,11                          // pin designation for motor 0
const int  motor0_in2 = 3; //3,5,9,10,11                          // pin designation for motor 0
const int  motor1_out = 9; //3,5,9,10,11                          // pin designation for motor 1
const int  motor1_in3 = 10; //3,5,9,10,11                          // pin designation for motor 1
const int  motor1_in4 = 11; //3,5,9,10,11                          // pin designation for motor 1
const int  LED = 13;                             // LED showing debugging mode

bool  button0 = 0;                               // internal variables for button 0 on controller
bool  button1 = 0;
bool  button1flag = 0;                           // check flag to see if button is being held down, debounce
bool  rampmode = 0;                              // flag for ramping mode on motor 0
bool  flag = 0;                                  // dead man switch for connection, stop motors if no data
bool  debugflag = 0;                             // set 1 for debugflag mode to print serial updates

void setup()
{
  setup_debug();
  setup_pinout();
  setup_remote();
}

void setup_debug() {
  Serial.begin(9600);                            // set serial baud to 9600
  Serial.println("setup_debug");
}

void setup_pinout() {
  // Set pim mode and init state of vibrator motor
  Serial.println("setup_pinout");
  pinMode(motor0_out, OUTPUT);
  pinMode(motor0_in1, OUTPUT);
  pinMode(motor0_in2, OUTPUT);
  digitalWrite(motor0_in1, HIGH);
  digitalWrite(motor0_in2, LOW);
  analogWrite(motor0_out, 0);

  // Set pim mode and init state of rotary motor
  pinMode(motor1_out, OUTPUT);
  pinMode(motor1_in3, OUTPUT);
  pinMode(motor1_in4, OUTPUT);
  digitalWrite(motor1_in3, HIGH);
  digitalWrite(motor1_in4, LOW);
  analogWrite(motor0_out, 0);
}

void setup_remote() {
  Serial.println("setup_remote");
  delay(500);                                    // allow controller to start first
  Wire.begin();                                  // join i2c bus (address optional for master)
}

void loop() {
  get_remote_data();
  check_deadman();
  motors_routines();
  debug_serial();
}

void get_remote_data() {
  flag = 0;                                      // set connection flag to off to show data to stop motors if no data arrives
  Wire.requestFrom(8, 5);                        // request 5 bytes from slave device #8
  while (Wire.available()) {
    for (int i = 0; i <= 4; i++) {
      inData[i] = Wire.read(); - '0';            // read 1 byte from the wire buffer in to "inData[i]"  -'0' is to convert back to int from char  
    }
    button0 = inData[2];                         // check to see if any buttons have been presed
    button1 = inData[3];
    
    if (inData [4] == 1){
      debugflag = 1;                             // enter debug mode
      digitalWrite(LED, HIGH);                   // LED showing debugging mode, HIGH);
    }
    else
    {
      debugflag = 0;                             // exit debug mode
      digitalWrite(LED, LOW);                    // LED showing debugging mode, HIGH);
    }
    
    flag = 1;                                    // set connection flag to on to show data has arrived.
  }
}

void check_deadman() {
  if (flag == 0) {                               // deadman (no connection) switch to stop motors
    for (int i = inData[0]; i == 0; i--) {       // decrease motor 0 and 1 speeds until stopped
      analogWrite(motor0_out, 0);
      delay(10);
    }
    for (int i = inData[1]; i == 0; i--) {
      analogWrite(motor1_out, 0);
      delay(10);
    }
  }
}

void motors_routines() {
  if (flag == 1) {                                // only continue if controller is connected (deadman switch check)

    // ***************** BUTTON 0 ROUTINES *****************

    if (button0 == 1) {                           // process button routine if button0 has been pressed
      button0press();
    }

    // ***************** BUTTON 1 ROUTINES *****************

    if (button1 == 1) {                           
      button1flag = 1;                            // set button flag to make sure it does not continuously run the routine (debounce)
    }

    if ((button1 == 0) && (button1flag == 1)) {   // if button has been released reset button0 flag and process routine
      button1flag = 0;
      if (rampmode == 0) {
        button1press();
      }
      else if (rampmode == 1) {
        rampmode = 0;
      }

    }

    // ****************** MOTOR  ROUTINES ******************

    if ((button0 == 0) && (button1 == 0)) {       // no buttons have been pressed - set motor speed
      if (rampmode == 1) {
        inData[0] = 255;
      }
      analogWrite(motor0_out, inData[0]);           // PWM to output motor 0 port
      delay(10);
      analogWrite(motor1_out, inData[1]);             // PWM to output motor 1 port
    }
  }
}

void button0press() {                             // button0 has been pressed
  inData[0] = 255;                                // set motor0_out speed to 100%
  analogWrite(motor0_out, inData[0]);                 // PWM to output motor 0 port
}

void button1press() {                             // button1 button has been pressed
  rampmode = 1;
  for (int i = inData[0]; i <= 255; i++) {            // slowly ramp motor speed to 100%
    Serial.print(i);
    Serial.println(".");
    analogWrite(motor0_out, i);
    delay(10);
  }
  Serial.println();
}

void debug_serial() {
  if (debugflag == 1) {
    showSerial();
    delay(1000);
  }
  else if (debugflag == 0) {
    delay(100);
  }
}

void showSerial() {
  Serial.print("Masterboard Status: ");
  if (flag == 0) {                                // deadman (no connection) switch to stop motors
    Serial.println("Controller disconnected. (Debugging)");
  }
  else if (flag == 1) {
    Serial.println("Controller connected. (Debugging)");
  }
  Serial.print("Motor 0:");
  Serial.print(inData[0]);
  Serial.print(" / ");
  Serial.print("Motor 1:");
  Serial.print(inData[1]);
  Serial.print(" / ");
  Serial.print("Button 0:");
  Serial.print(button0);
  Serial.print(" / ");
  Serial.print("Button 1:");
  Serial.print(button1);
  Serial.print(" / ");
  Serial.print("Button 1 Flag:");
  Serial.print(button1flag);
  Serial.print(" / ");
  Serial.print("Ramp Mode:");
  Serial.print(rampmode);

  Serial.println();
  Serial.println();
}
