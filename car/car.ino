#include <Smartcar.h>
#include <BluetoothSerial.h>
#include <VL53L0X.h>
#include <Wire.h>

//Ultrasonic sensors 
const int MAX_DISTANCE = 300;
const int trigPinFront = 19; //D19
const int echoPinFront = 5; //D5
const int trigPinRight = 33; //D33
const int echoPinRight = 18; //D18

//Constants 
const int STOP_DIST = 15; //this distance is in centimiters for the front sensor
const int RIGHT_DIST = 30; // this distance is in cm and are for the sensor on the right side
const int LEFT_DIST = 300; //this distance is in millimiters for the right side sensors

//Constructors to control the motors  
BrushedMotor leftMotor(smartcarlib::pins::v2::leftMotorPins);
BrushedMotor rightMotor(smartcarlib::pins::v2::rightMotorPins);
DifferentialControl control (leftMotor, rightMotor);

//Gyroscope 
const int GYRO_OFFSET = 22;
GY50 gyroscope(GYRO_OFFSET);

//Sensors 
SR04 front(trigPinFront, echoPinFront, MAX_DISTANCE);
SR04 right(trigPinRight, echoPinRight, MAX_DISTANCE);
VL53L0X left;

//Bluetooth
BluetoothSerial SerialBT;

//Odometer 
const auto pulsesPerMeter = 600;
DirectionlessOdometer leftOdometer( smartcarlib::pins::v2::leftOdometerPin, []() { leftOdometer.update(); }, pulsesPerMeter);
DirectionlessOdometer rightOdometer( smartcarlib::pins::v2::rightOdometerPin, []() {rightOdometer.update(); },pulsesPerMeter);

//Smartcar constructor 
SmartCar car(control, gyroscope, leftOdometer, rightOdometer);

//-------------------------------Set Up and Loop----------------------------------------------------//

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  SerialBT.begin("Smartcar"); //Name of the BT in the car
  pinMode(LED_BUILTIN, OUTPUT);
  SerialBT.register_callback(callback);
  Wire.begin();

  left.setTimeout(500);
  if (!left.init()){        //This checks if micro Lidar sensor is initialized and keeps in this state til it is.
    while(1){}
    }
  left.startContinuous(); 
}

void loop() {  
  // put your main code here, to run repeatedly:
  obstacleAvoidance();
}

//-------------------------------Set Up and Loop----------------------------------------------------//

//Method that detects if the smartcar is connected to bluetooth
void callback(esp_spp_cb_event_t event, esp_spp_cb_param_t *param) {
  if (event == ESP_SPP_SRV_OPEN_EVT) { //If the smartcar has a bluetooth connection
    digitalWrite(LED_BUILTIN, HIGH); // turn the LED on (HIGH is the voltage level)
  }

  if (event == ESP_SPP_CLOSE_EVT ) { //If the smartcar does not have a bluetooth connection
    stop();    
    
    while (true){
      digitalWrite(LED_BUILTIN, HIGH);   // turn the LED on (HIGH is the voltage level)
      delay(1000);                       // wait for a second
      digitalWrite(LED_BUILTIN, LOW);    // turn the LED off by making the voltage LOW
      delay(1000);                       // wait for a second
    }
  }
}

//Main method that calls other methods, also avoids obsticles
void obstacleAvoidance() {
  int frontDistance = front.getDistance();
  int leftDistance = left.readRangeContinuousMillimeters();
  int rightDistance = right.getDistance();

  if (frontDistance <= STOP_DIST && frontDistance > 0){ //stop when distance is less than 15 cm.
    stop();
    driveAroundObsticle();
  } else {
    handleInput();
  }
}

//Gets input from bluetooth and translate to commands for the car 
void handleInput() { //handle serial input (String!!)
  if (SerialBT.available()) { 
    String input;
    while (SerialBT.available()) { 
      input = SerialBT.readStringUntil('\n');   
    }; //read till last character
    
    if (input.startsWith("v")) {
      int throttle = input.substring(1).toInt();
      forward(throttle);
    }

    if (input.startsWith("b")) {
      int throttle = input.substring(1).toInt();
      reverse(throttle);
    }

    if (input.startsWith("t")) {
      int throttle = input.substring(1).toInt();
      turn(throttle);
    }  
  }
}

//-----------------------------------For testing------------------------------//

const int TURN_SPEED1 = 30; //Turn speed for turning on the spot [Only for testing!!!]
const int RIGHT_DIST1 = 15; // this distance is in cm and are for the sensor on the right side
const int LEFT_DIST1 = 150; //this distance is in millimiters for the right side sensors
const float TURN_SPEED2 = 30; //Turn speed for turning on the spot [Only for testing!!!]
const long lenghtOfSensor = 14;

bool atObsticle = false;
bool clearToTheRight = true; 
bool clearToTheLeft = true; 

void driveAroundObsticle(){
  checkFront();
  checkLeftSide();
  checkRightSide();
  
 if (clearToTheRight) {
    rotateOnSpot(90, TURN_SPEED1);
    goDistance(lenghtOfSensor, TURN_SPEED2);

    stop();
    /*
    leftOdometer.update(); 
    rightOdometer.update(); 
    
    while (left.readRangeContinuousMillimeters() <= LEFT_DIST1 && left.readRangeContinuousMillimeters() > 0 ) {
      forward(TURN_SPEED1);
    }
    stop();
    rotateOnSpot(-90, TURN_SPEED1);

    int lenghtOfDrive = lenghtOfSensor + ((leftOdometer.getDistance() + rightOdometer.getDistance()) / 2);

    while (left.readRangeContinuousMillimeters() <= LEFT_DIST1 && left.readRangeContinuousMillimeters() > 0 ) {
      forward(TURN_SPEED1);
    }
    stop();
    rotateOnSpot(-90, TURN_SPEED1);

    goDistance(lenghtOfDrive, TURN_SPEED2);
    rotateOnSpot(90, TURN_SPEED1);

  */
  } else if (clearToTheLeft){
    rotateOnSpot(-90, TURN_SPEED1);
    goDistance(lenghtOfSensor, TURN_SPEED2);

    stop(); 

    /*
    leftOdometer.update(); 
    rightOdometer.update(); 

    while (right.getDistance() <= RIGHT_DIST1 && right.getDistance() > 0 ) {
     forward(TURN_SPEED1);
    }
    stop();
    rotateOnSpot(90, TURN_SPEED1);

    int lenghtOfDrive =  lenghtOfSensor + ((leftOdometer.getDistance() + rightOdometer.getDistance()) / 2);
    
    while (right.getDistance() <= RIGHT_DIST1 && right.getDistance() > 0 ) {
      forward(TURN_SPEED1);
    }
    stop();
    rotateOnSpot(90, TURN_SPEED1);

    goDistance(lenghtOfDrive, TURN_SPEED2);
    rotateOnSpot(-90, TURN_SPEED1);
    */

  
  } else {
    rotateOnSpot(180, TURN_SPEED1);
  } 
  
  atObsticle = false;
  clearToTheLeft = true; 
  clearToTheRight = true; 
}

void checkFront(){
  int frontDistance = front.getDistance();
  if (frontDistance <= STOP_DIST && frontDistance > 0){ //stop when distance is less than 15 cm.
    atObsticle = true; 
  }else {
    atObsticle = false;
   }
}

void checkLeftSide(){
  int leftDistance = left.readRangeContinuousMillimeters();
  if(leftDistance <= LEFT_DIST1 && leftDistance > 0 ){
    clearToTheLeft = false;
  }else {
    clearToTheLeft = true;
   }
}

void checkRightSide(){
  int rightDistance = right.getDistance();
  if(rightDistance <= RIGHT_DIST1 && rightDistance > 0){
    clearToTheRight = false;
  }else {
    clearToTheRight = true;
   }
}
