
#include <Arduino.h>
//#include <avr/wdt.h>
#include <MeAuriga.h>
//#include "MeEEPROM.h"
#include <Wire.h>
#include <SoftwareSerial.h>

/** FOLLOW THE LIGHT **/

#define POWER_PORT                           A4
#define BUZZER_PORT                          45
#define RGBLED_PORT                          44

//#define COMMAND_STOP                            0x00
//#define COMMAND_FIND_LIGHT                      0x01
//#define COMMAND_NAVIGATE                        0x02

//constants
#define ANGLE_SENSITIVITY           5

#define SPEED_DEFAULT               100
#define SPEED_FIND_LIGHT_DIRECTION  75
#define SPEED_TURN                  75
#define SPEED_MINIMAL               75

#define DISTANCE_TOO_CLOSE          20
#define DISTANCE_OBSTACLE_DETECTED  50
#define DISTANCE_SENSITIVITY        2

#define DELAY_NANO                  100
#define DELAY_MICRO                 500
#define DELAY_DEFAULT               1000
#define DELAY_MACRO                 10000

#define SMOOTHING_COEFFICIENT       0.8f
#define PRECISION_RANDOM            100.0f
#define RATIO_WHEELS_STUCK          0.33f

//sensors
MeUltrasonicSensor *us;  //PORT_10
MeGyro gyro(1,0x69);      //On Board external gyro sensor
MeLineFollower line(PORT_9);
MeEncoderOnBoard Encoder_1(SLOT1);
MeEncoderOnBoard Encoder_2(SLOT2);
MeRGBLed led;
MeBuzzer buzzer;
MeLightSensor lightsensor_12(12);
MeLightSensor lightsensor_11(11);

//variables
long wheelStuckTime = 0;
long distanceStuckTime = 0;
long lastCommandTimeStamp = 0;
uint16_t previousObstacleProximity = 0;

//TODO: detect stuck state
//TODO: conditional turn (depends on what track is stuck)
//TODO: random turn on obstacle avoidance
//TODO: tilt and stuck check when going backwards
//TODO: change direction if it goes backwards too long

void loop()
{
  waitForCommand();
}

/**
 * Commands processing section
 */

void waitForCommand() {
  Serial.println("Please enter command: "); //Prompt User for input
  Serial.println("findLightDirection, navigate, stop, forward, backward, left, right, backwardRight, backwardLeft, forwardRight, forwardLeft");

//  uint16_t obstacleProximity = getObstacleProximity();
  
  while (!hasNewCommand()) {
    //Wait for user input
//    if (!equalsWithinRange(obstacleProximity, getObstacleProximity(), DISTANCE_SENSITIVITY)) {
//      Serial.println(getObstacleProximity());
//    }
//
//    obstacleProximity = getObstacleProximity();
//
//    delay(DELAY_MICRO);
  }

  processNewCommand();
}

boolean hasNewCommand() {
  return Serial.available() > 0;
}

boolean processNewCommand() {
  String command = Serial.readString();
  
  Serial.print("Received command: ");
  Serial.println(command);
  lastCommandTimeStamp = millis();

  if (command.startsWith("findLightDirection")) {
    commandFindLightDirection();
  } else if (command.startsWith("navigate")) {
    commandNavigate();
  } else if (command.startsWith("stop")) {
    Stop();
  } else if (command.startsWith("backwardRight")) {
    BackwardAndTurnRight(SPEED_DEFAULT);
    executeAndStopUntilNewCommandWithDelay();
  } else if (command.startsWith("backwardLeft")) {
    BackwardAndTurnLeft(SPEED_DEFAULT);
    executeAndStopUntilNewCommandWithDelay();
  } else if (command.startsWith("forwardRight")) {
    TurnRight(SPEED_DEFAULT);
    executeAndStopUntilNewCommandWithDelay();
  } else if (command.startsWith("forwardLeft")) {
    TurnLeft(SPEED_DEFAULT);
    executeAndStopUntilNewCommandWithDelay();
  } else if (command.startsWith("forward")) {
    Forward(SPEED_DEFAULT);
    executeAndStopUntilNewCommandWithDelay();
  } else if (command.startsWith("backward")) {
    Backward(SPEED_DEFAULT);
    executeAndStopUntilNewCommandWithDelay();
  } else if (command.startsWith("left")) {
    Left(SPEED_DEFAULT);
    executeAndStopUntilNewCommandWithDelay();
  } else if (command.startsWith("right")) {
    Right(SPEED_DEFAULT);
    executeAndStopUntilNewCommandWithDelay();
  } else if (command.startsWith("led")) {
    int index = getCommandParamValueInt(command, "index");
    int r = getCommandParamValueInt(command, "r");
    int g = getCommandParamValueInt(command, "g");
    int b = getCommandParamValueInt(command, "b");
    Serial.println(index);
    Serial.println(r);
    Serial.println(g);
    Serial.println(b);

    if (index > -1) {
      led.setColorAt(index,r,g,b);
    } else {
      led.setColor(r,g,b);
    }
    led.show();
  } else if (command.startsWith("beep")) {

    int frequency = getCommandParamValueInt(command, "frequency");
    int duration = getCommandParamValueInt(command, "duration");
    Serial.println(frequency);
    Serial.println(duration);
    
    buzzer.tone(frequency, duration);
  }
}

void executeAndStopUntilNewCommandWithDelay() {
    do {
      if (millis() > lastCommandTimeStamp + DELAY_MACRO) {
        Stop(); //we hit delay
        return;
      }
    } while (!hasNewCommand());  
}

/**
 * Navigation
 */
boolean commandNavigate() {

  do {

    uint16_t obstacleProximity = getObstacleProximity();

    if (tiltAlert()) {
      
      modeOnTilt();
      
    } else if (obstacleProximity < DISTANCE_TOO_CLOSE) {

      modeObstacleIsTooClose();
      
    } else if (obstacleProximity < DISTANCE_OBSTACLE_DETECTED) {
      
      modeAvoidObstacle();
      
    } else {

      modeContinueNavigation();

    }

    delay(DELAY_NANO);
    
  } while (!hasNewCommand());

  Serial.println("Detected new command");

  Stop();

  delay(DELAY_DEFAULT);
}

/**
 * Critical angle or don't see the ground - move backward
 */
void modeOnTilt() {
  Serial.print("Tilt alert!!! gyroX=");
  Serial.print(getGyroX());
  Serial.print(" gyroY=");
  Serial.print(getGyroY());
  Serial.print(" groundFlag=");
  Serial.println(getGroundFlag());  

  Backward(SPEED_DEFAULT);
  
  delay(DELAY_MICRO);

  if (getRandomDirection() < 1) {
    BackwardAndTurnRight(SPEED_DEFAULT);
  } else {
    BackwardAndTurnLeft(SPEED_DEFAULT);
  }  
  
  do {     
       
    delay(DELAY_NANO);

    if (hasNewCommand()) {
      break;
    }
    
  } while (tiltAlert());
  
  delay(DELAY_DEFAULT);

  resetStuckTimer();

  Serial.print("Tilt alert is fixed gyroX=");
  Serial.print(getGyroX());
  Serial.print(" gyroY=");
  Serial.print(getGyroY());
  Serial.print(" groundFlag=");
  Serial.println(getGroundFlag()); 
}

/**
 * Obstacle is too close, trying to find free direction
 */
void modeObstacleIsTooClose() {
  uint16_t obstacleProximity = getObstacleProximity();
  
  Serial.print("Too close! turning ");
  Serial.println(obstacleProximity);

  Backward(SPEED_DEFAULT);
  
  delay(DELAY_MICRO);
  
  if (getRandomDirection() < 1) {
    Right(SPEED_DEFAULT);
  } else {
    Left(SPEED_DEFAULT);
  }
 
  do {
    obstacleProximity = getObstacleProximity();

    if (distanceIsNotChanging()) {
      modeStuck();
      return;
    }

    if (hasNewCommand()) {
      break;
    }
    
    delay(DELAY_NANO);
    
  } while (obstacleProximity < DISTANCE_OBSTACLE_DETECTED);

  Serial.print("Found obstacle free direction ");
  Serial.println(obstacleProximity);

  delay(DELAY_MICRO);

  resetStuckTimer();
}

/**
 * Approaching obstacle, lets change direction
 */
void modeAvoidObstacle() {
  uint16_t obstacleProximity = getObstacleProximity();
  
  Serial.print("Obstacle detected. Avoiding. ");
  Serial.println(obstacleProximity);
  
  //moveWithTurn(SPEED_DEFAULT, getRandomDirection());

  if (getRandomDirection() < 1) {
    TurnRight(SPEED_DEFAULT);
  } else {
    TurnLeft(SPEED_DEFAULT);
  }

  do {
    obstacleProximity = getObstacleProximity();
    
    delay(DELAY_NANO);

    if (areWheelsStuck() || distanceIsNotChanging()) {
      modeStuck();
      return;
    }

    if (obstacleProximity < DISTANCE_TOO_CLOSE || tiltAlert()) {
      return;
    }

    if (hasNewCommand()) {
      break;
    }
    
  } while (obstacleProximity < DISTANCE_OBSTACLE_DETECTED);

  Serial.print("Found obstacle free direction ");
  Serial.println(obstacleProximity);

  delay(DELAY_MICRO);

  resetStuckTimer();  
}

/**
 * No obstacles detected - continue navigation
 */
void modeContinueNavigation() {
  if (areWheelsStuck() || distanceIsNotChanging()) {

    modeStuck();

  } else {
    
    Forward(SPEED_DEFAULT);
  }  
}

/**
 * it's stuck. Moving backward
 */
void modeStuck() {
  Backward(SPEED_DEFAULT);
  
  delay(DELAY_DEFAULT);
  
  if (getRandomDirection() < 1) {
    BackwardAndTurnRight(SPEED_DEFAULT);
  } else {
    BackwardAndTurnLeft(SPEED_DEFAULT);
  }
  
  delay(DELAY_DEFAULT*2);

  resetStuckTimer(); 
}

/**
 * Turn to identify maximum light intensity
 */
boolean commandFindLightDirection() {

  uint16_t lightLevel = getLightLevel();
  uint16_t maxLightLevel = lightLevel;
  int16_t gyroZ = getGyroZ();
  int16_t maxLightLevelDirection = gyroZ;
  int16_t currentDirection = maxLightLevelDirection;
  float lightBalance = getLightBalance();
  float dAngle = 0;

  Serial.print("Searching for light direction. lightBalance=");
  Serial.print(lightBalance);
  Serial.print(" lightLevel=");
  Serial.println(lightLevel);

  if (lightBalance < 1) {
    Right(SPEED_DEFAULT);
  } else {
    Left(SPEED_DEFAULT);
  } 
  
  do {
      lightLevel = getLightLevel();
      gyroZ = getGyroZ();
      
      if (maxLightLevel < lightLevel) {
        maxLightLevel = lightLevel;
        maxLightLevelDirection = gyroZ;
      }

      dAngle += abs(normalizeAngle(currentDirection - gyroZ));
      currentDirection = gyroZ;

      if (hasNewCommand()) {
        break;
      }

      delay(DELAY_NANO);
  } while (dAngle < 360);

  Stop();

  Serial.print("Found light direction. direction=");
  Serial.print(maxLightLevelDirection);
  Serial.print(" lightLevel=");
  Serial.println(maxLightLevel);

  delay(DELAY_DEFAULT);

  turnToAngle(SPEED_DEFAULT, maxLightLevelDirection);
}

/**
 * Turn to angle
 */
boolean turnToAngle(uint8_t moveSpeed, int16_t angle) {
  angle = normalizeAngle(angle);
  int16_t currentAngle = getGyroZ();

  Serial.print("Started turn. angle=");
  Serial.print(angle);
  Serial.print(" currentAngle=");
  Serial.println(currentAngle);

  if (normalizeAngle(angle-currentAngle) > 0) {
    Right(moveSpeed);
  } else {
    Left(moveSpeed);
  }
  
  do {
    currentAngle = getGyroZ();

    if (hasNewCommand()) {
      break;
    }
    
  } while(!equalsWithinRange(angle, currentAngle, ANGLE_SENSITIVITY));
  
  Stop();
  
  Serial.print("Completed turn. angle=");
  Serial.print(angle);
  Serial.print(" currentAngle=");
  Serial.println(currentAngle);
  
  delay(DELAY_DEFAULT);
  
  return true;
}

/********* SENSORS **************/

boolean tiltAlert() {
  return abs(getGyroX()) > 15 || abs(getGyroY()) > 15 || getGroundFlag() == 0;
}

float getGyroX() {
  return gyro.getAngle(1); //vertical axis
}

float getGyroY() {
  return gyro.getAngle(2); //horizontal axis
}

float getGyroZ() {
  return gyro.getAngle(3); //direction
}

uint16_t getObstacleProximity() {
  return us->distanceCm();
}

uint8_t getGroundFlag() {
  return line.readSensors();
}

uint16_t getLightLevelLeft() {
  return lightsensor_12.read(); //light left
}

uint16_t getLightLevelRight() {
  return lightsensor_11.read(); //light right
}

/**
 * > 1 - more light from left
 * < 1 - more light from right
 */
float getLightBalance() {
  uint16_t lightLevelLeft = getLightLevelLeft();
  uint16_t lightLevelRight = getLightLevelRight();
  
  if (lightLevelRight == lightLevelLeft) {
    return 1;
  } else if(lightLevelRight == 0) {
    return 2;
  } else if (lightLevelLeft == 0) {
    return 0;
  }
  return (float)lightLevelLeft/(float)lightLevelRight;
}

float getLightLevel() {
  return sqrt(getLightLevelLeft()*getLightLevelRight());
}

/**
 * convert angle to -180 / +180 range
 */
float normalizeAngle(float angle) {
  if (angle > 180) {
    angle = angle-360;
  } else if (angle < -180) {
    angle = angle+360;
  }

  return angle;
}

/********* UTILITIES **************/

float getRandomDirection() {
  float r = random(1, PRECISION_RANDOM);
  return r/(PRECISION_RANDOM/2);
}

void resetStuckTimer() {
  wheelStuckTime = 0;
}

boolean distanceIsNotChanging() {
  boolean result = false;

  uint16_t obstacleProximity = getObstacleProximity();

  result = equalsWithinRange(obstacleProximity, previousObstacleProximity, DISTANCE_SENSITIVITY);

  previousObstacleProximity = obstacleProximity;

  if (result && obstacleProximity < 400) {
    if (distanceStuckTime == 0) {
      distanceStuckTime = millis();
    }
  } else {
    distanceStuckTime = 0;
  }

  result = result && distanceStuckTime > 0 && (millis() > (distanceStuckTime + DELAY_DEFAULT*2));

  if (result) {
    Serial.print("Stuck (distance)! ");
    Serial.print(" previousObstacleProximity=");
    Serial.print(previousObstacleProximity);
    Serial.print(" obstacleProximity=");
    Serial.println(obstacleProximity);
  }

  return result;
}

boolean areWheelsStuck() {
  
  boolean result = false;

  Encoder_1.updateSpeed();
  Encoder_2.updateSpeed();
  int16_t currentSpeedL = Encoder_1.getCurrentSpeed();
  int16_t currentSpeedR = Encoder_2.getCurrentSpeed();  
  int16_t currentMotorPowerL = Encoder_1.getCurPwm();
  int16_t currentMotorPowerR = Encoder_2.getCurPwm();

  if (abs(currentMotorPowerL) < SPEED_MINIMAL && abs(currentMotorPowerR) < SPEED_MINIMAL) {

  } else if (abs(currentMotorPowerL) > SPEED_MINIMAL && currentMotorPowerR < SPEED_MINIMAL) {
    result = ((float)currentSpeedL)/((float)currentMotorPowerL) < RATIO_WHEELS_STUCK;
  } else if (abs(currentMotorPowerR) > SPEED_MINIMAL && currentMotorPowerL < SPEED_MINIMAL) {
    result = ((float)currentSpeedR)/((float)currentMotorPowerR) < RATIO_WHEELS_STUCK;
  } else {
    result = sqrt(((float)currentSpeedR)/((float)currentMotorPowerR) * ((float)currentSpeedL)/((float)currentMotorPowerL)) < RATIO_WHEELS_STUCK;
  }

  if (result) {
    if (wheelStuckTime == 0) {
      wheelStuckTime = millis();
    }
  } else {
    wheelStuckTime = 0;
  }

  result = result && wheelStuckTime > 0 && (millis() > (wheelStuckTime + DELAY_DEFAULT));

  if (result) {
    Serial.print("Stuck! (wheels) ");
    Serial.print(" ");
    Serial.print(currentMotorPowerL);
    Serial.print("/");
    Serial.print(currentSpeedL);
    Serial.print(" ");
    Serial.print(currentMotorPowerR);
    Serial.print("/");
    Serial.println(currentSpeedR);
  }

  return result;
}

/**
 * value1 equals value2  with +/- range
 */
boolean equalsWithinRange(double value1, double value2, double range) {
  return (value1 < value2+range) && (value1 > value2-range);
}

// splitting a string and return the part nr index split by separator
String getStringSegmentByIndex(String data, char separator, int index) {
    int stringData = 0;        //variable to count data part nr 
    String dataPart = "";      //variable to hole the return text

    for(int i = 0; i<data.length()-1; i++) {    //Walk through the text one letter at a time
        if(data[i]==separator) {
            //Count the number of times separator character appears in the text
            stringData++;
        } else if(stringData==index) {
            //get the text when separator is the rignt one
            dataPart.concat(data[i]);
        } else if(stringData>index) {
            //return text and stop if the next separator appears - to save CPU-time
            return dataPart;
            break;
        }
    }
    //return text if this is the last part
    return dataPart;
}

String getCommandParamValue(String command, String paramName) {
  String param = paramName + "=";
  String paramValue = "";
  
  if (command.indexOf(paramName) > -1) {
    command = command.substring(command.indexOf(paramName) + param.length());
    int paramEndIndex = command.length();
    if (command.indexOf(" ") > -1) {
      paramEndIndex = command.indexOf(" ");
    }
    paramValue = command.substring(0, paramEndIndex);
  }

  return paramValue;
}

int getCommandParamValueInt(String command, String paramName) {
  String paramValue = getCommandParamValue(command, paramName);
  return (paramValue.length() > 0) ? paramValue.toInt() : -1;
}

/********* MOVEMENT RELATED **********/

void Stop(void)
{
  Encoder_1.setMotorPwm(0);
  Encoder_2.setMotorPwm(0);
}

void moveWithTurn(double moveSpeed, double _direction)
{
  if (_direction == 0) {
    _direction = 1;
  }
  
  if (_direction > 1) {
    moveSpeed = moveSpeed/_direction;
  } else {
    moveSpeed = moveSpeed*_direction;
  }
  Encoder_1.setMotorPwm(-moveSpeed*_direction);
  Encoder_2.setMotorPwm(moveSpeed/_direction);
}

void Left(uint8_t moveSpeed)
{
  Encoder_1.setMotorPwm(-moveSpeed);
  Encoder_2.setMotorPwm(-moveSpeed);
}

void Right(uint8_t moveSpeed)
{
  Encoder_1.setMotorPwm(moveSpeed);
  Encoder_2.setMotorPwm(moveSpeed);
}

/**
 * \par Function
 *    Forward
 * \par Description
 *    This function use to control the car kit go forward.
 * \param[in]
 *    None
 * \par Output
 *    None
 * \return
 *    None
 * \par Others
 *    None
 */
void Forward(uint8_t moveSpeed)
{
  Encoder_1.setMotorPwm(-moveSpeed);
  Encoder_2.setMotorPwm(moveSpeed);
}

/**
 * \par Function
 *    Backward
 * \par Description
 *    This function use to control the car kit go backward.
 * \param[in]
 *    None
 * \par Output
 *    None
 * \return
 *    None
 * \par Others
 *    None
 */
void Backward(uint8_t moveSpeed)
{
  Encoder_1.setMotorPwm(moveSpeed);
  Encoder_2.setMotorPwm(-moveSpeed);
}

/**
 * \par Function
 *    BackwardAndTurnLeft
 * \par Description
 *    This function use to control the car kit go backward and turn left.
 * \param[in]
 *    None
 * \par Output
 *    None
 * \return
 *    None
 * \par Others
 *    None
 */
void BackwardAndTurnLeft(uint8_t moveSpeed)
{
  Encoder_1.setMotorPwm(moveSpeed/4);
  Encoder_2.setMotorPwm(-moveSpeed);
}

/**
 * \par Function
 *    BackwardAndTurnRight
 * \par Description
 *    This function use to control the car kit go backward and turn right.
 * \param[in]
 *    None
 * \par Output
 *    None
 * \return
 *    None
 * \par Others
 *    None
 */
void BackwardAndTurnRight(uint8_t moveSpeed)
{
  Encoder_1.setMotorPwm(moveSpeed);
  Encoder_2.setMotorPwm(-moveSpeed/4);
}

/**
 * \par Function
 *    TurnLeft
 * \par Description
 *    This function use to control the car kit go backward and turn left.
 * \param[in]
 *    None
 * \par Output
 *    None
 * \return
 *    None
 * \par Others
 *    None
 */
void TurnLeft(uint8_t moveSpeed)
{
  Encoder_1.setMotorPwm(-moveSpeed);
  Encoder_2.setMotorPwm(moveSpeed/2);
}

/**
 * \par Function
 *    TurnRight
 * \par Description
 *    This function use to control the car kit go backward and turn right.
 * \param[in]
 *    None
 * \par Output
 *    None
 * \return
 *    None
 * \par Others
 *    None
 */
void TurnRight(uint8_t moveSpeed)
{
  Encoder_1.setMotorPwm(-moveSpeed/2);
  Encoder_2.setMotorPwm(moveSpeed);
}

/**
 * \par Function
 *    isr_process_encoder1
 * \par Description
 *    This function use to process the interrupt of encoder1 drvicer on board,
 *    used to calculate the number of pulses.
 * \param[in]
 *    None
 * \par Output
 *    The number of pulses on encoder1 driver
 * \return
 *    None
 * \par Others
 *    None
 */
void isr_process_encoder1(void)
{
  if(digitalRead(Encoder_1.getPortB()) == 0)
  {
    Encoder_1.pulsePosMinus();
  }
  else
  {
    Encoder_1.pulsePosPlus();
  }
}

/**
 * \par Function
 *    isr_process_encoder2
 * \par Description
 *    This function use to process the interrupt of encoder2 drvicer on board,
 *    used to calculate the number of pulses.
 * \param[in]
 *    None
 * \par Output
 *    The number of pulses on encoder2 driver
 * \return
 *    None
 * \par Others
 *    None
 */
void isr_process_encoder2(void)
{
  if(digitalRead(Encoder_2.getPortB()) == 0)
  {
    Encoder_2.pulsePosMinus();
  }
  else
  {
    Encoder_2.pulsePosPlus();
  }
}

/****** SETUP *******/


void setup()
{
  delay(5);
  Serial.begin(115200);
  delay(5);
  attachInterrupt(Encoder_1.getIntNum(), isr_process_encoder1, RISING);
  attachInterrupt(Encoder_2.getIntNum(), isr_process_encoder2, RISING);
  us = new MeUltrasonicSensor(PORT_6);
  led.setpin(RGBLED_PORT);
  buzzer.setpin(BUZZER_PORT);
  led.setColor(0,0,0,0);
  led.show();
  //buzzer.tone(1000,100); 
  //buzzer.noTone();

  // enable the watchdog
  delay(5);
  gyro.begin();
  delay(5);
  pinMode(13,OUTPUT);

  //Set Pwm 8KHz
  TCCR1A = _BV(WGM10);
  TCCR1B = _BV(CS11) | _BV(WGM12);

  TCCR2A = _BV(WGM21) | _BV(WGM20);
  TCCR2B = _BV(CS21);

  Encoder_1.setPulse(9);
  Encoder_2.setPulse(9);
  Encoder_1.setRatio(39.267);
  Encoder_2.setRatio(39.267);
  Encoder_1.setPosPid(1.8,0,1.2);
  Encoder_2.setPosPid(1.8,0,1.2);
  Encoder_1.setSpeedPid(0.18,0,0);
  Encoder_2.setSpeedPid(0.18,0,0);
  Encoder_1.setMotionMode(DIRECT_MODE);
  Encoder_2.setMotionMode(DIRECT_MODE);

  demo();
}

void demo(void)
{
  uint8_t R_Bright = 0;
  for(uint8_t i = 0;i < 40;i++)
  {
    led.setColor(0,R_Bright,R_Bright,R_Bright);
    led.show();
    R_Bright += 1;
    delay(12);
    
  }

  R_Bright = 40;
  for(uint8_t i = 0;i < 40;i++)
  {
    led.setColor(0,R_Bright,R_Bright,R_Bright);
    led.show();
    R_Bright -= 1;
    delay(12);
    
  }

  buzzer.tone(988, 125);   //NOTE_B5
  led.setColor(0,20,0,0);
  led.show();
  delay(200);
  

  led.setColor(0,20,5,0);
  led.show();
  delay(200);

  led.setColor(0,15,15,0);
  led.show();
  delay(200);

  led.setColor(0,0,20,0);
  led.show();
  delay(200);
  

  led.setColor(0,0,0,20);
  led.show();
  delay(200);

  led.setColor(0,10,0,20);
  led.show();
  delay(200);

  led.setColor(0,20,0,20);
  led.show();
  delay(200);
  

  led.setColor(0,0,0,0);
  buzzer.tone(1976, 125);  //NOTE_B6
  led.setColor(12,20,10,20);
  led.setColor(1,20,10,20);
  led.setColor(2,20,10,20);
  led.show();
  delay(375);
  
  buzzer.tone(1976, 125);  //NOTE_B6
  led.setColor(3,20,20,0);
  led.setColor(4,20,20,0);
  led.setColor(5,20,20,0);
  led.show();
  delay(375);
  
  buzzer.tone(1976, 125);  //NOTE_B69
  led.setColor(6,0,10,20);
  led.setColor(7,0,10,20);
  led.setColor(8,0,10,20);
  led.show();
  delay(500);
  
  buzzer.tone(1976, 125);  //NOTE_B69
  led.setColor(9,10,0,0);
  led.setColor(10,10,0,0);
  led.setColor(11,10,0,0);
  led.show();
  delay(500);
  

  led.setColor(0,0,0,0);
  for(uint8_t i=0;i<4;i++)
  {
    led.setColor(12,20,10,20);
    led.setColor(1,20,10,20);
    led.setColor(2,20,10,20);
    
    led.setColor(3,20,20,0);
    led.setColor(4,20,20,0);
    led.setColor(5,20,20,0);

    led.setColor(6,0,10,20);
    led.setColor(7,0,10,20);
    led.setColor(8,0,10,20);

    led.setColor(9,10,0,0);
    led.setColor(10,10,0,0);
    led.setColor(11,10,0,0);
    led.show();
    delay(2);
    buzzer.tone(2349, 250);  //NOTE_D7
    led.setColor(0,0,0,0);
    led.show();
    delay(100);
    
  }

  buzzer.tone(262, 250);   //NOTE_D5
  buzzer.tone(294, 250);   //NOTE_E5
  buzzer.tone(330, 250);   //NOTE_C5
  led.setColor(0,0,0,0);
  led.show();
}

