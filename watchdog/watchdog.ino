
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

#define COMMAND_STOP                            0x00
#define COMMAND_FIND_LIGHT                      0x01
#define COMMAND_NAVIGATE                        0x02

//constants
#define ANGLE_SENSITIVITY           10

#define SPEED_DEFAULT               125
#define SPEED_FIND_LIGHT_DIRECTION  100
#define SPEED_TURN                  100
#define SPEED_MINIMAL               100

#define DISTANCE_TOO_CLOSE          20
#define DISTANCE_OBSTACLE_DETECTED  50

#define DELAY_NANO                  100
#define DELAY_MICRO                 500
#define DELAY_DEFAULT               1000

#define SMOOTHING_COEFFICIENT       0.8f
#define PRECISION_RANDOM            100.0f

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

//sensors output
float  gyroY, gyroX, gyroZ;
uint16_t obstacleProximity=0;
uint8_t groundFlag=0;
uint16_t lightLevelLeft = 0;
uint16_t lightLevelRight = 0;
int16_t currentSpeedL = 0;
int16_t currentSpeedR = 0;
int16_t currentMotorPowerL = 0;
int16_t currentMotorPowerR = 0;
long stuckStartedTimestamp = 0;

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
  Serial.println("0 - stop");
  Serial.println("1 - find light direction");
  Serial.println("2 - navigate");
  
  while (!hasNewCommand()) {
    //Wait for user input
  }

  processNewCommand();

  delay(DELAY_DEFAULT);
}

boolean hasNewCommand() {
  return Serial.available() > 0;
}

boolean processNewCommand() {
  int command = Serial.parseInt();
  
  Serial.print("Received command: ");
  Serial.println(command);

  switch(command) {
    case COMMAND_FIND_LIGHT:
      findLightDirection();
      break;
    case COMMAND_NAVIGATE:
      navigate();
      break;
    case COMMAND_STOP:
      Stop();
      break;
    default:
      if (command >= SPEED_TURN) {
        Forward(command);
        do {
          isStuck();
          delay(DELAY_MICRO);
        } while (!hasNewCommand());
      }
  }
}

/**
 * Navigation
 */
boolean navigate() {

  do {
    
    readSensors();

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
  Serial.print(gyroX);
  Serial.print(" gyroY=");
  Serial.print(gyroY);
  Serial.print(" groundFlag=");
  Serial.println(groundFlag);  
  
  moveWithTurn(-SPEED_DEFAULT, getRandomDirection()); 
  
  do {        
    readSensors();
    
    delay(DELAY_NANO);
    
  } while (tiltAlert());
  
  delay(DELAY_MICRO);

  resetStuckTimer();

  Serial.print("Tilt alert is fixed gyroX=");
  Serial.print(gyroX);
  Serial.print(" gyroY=");
  Serial.print(gyroY);
  Serial.print(" groundFlag=");
  Serial.println(groundFlag); 
}

/**
 * Obstacle is too close, trying to find free direction
 */
void modeObstacleIsTooClose() {
  Serial.print("Too close! turning ");
  Serial.println(obstacleProximity);
  
  if (getRandomDirection() < 1) {
    Right(SPEED_DEFAULT);
  } else {
    Left(SPEED_DEFAULT);
  }
  
  do {
    readSensors();
    
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
  Serial.print("Obstacle detected. Avoiding. ");
  Serial.println(obstacleProximity);
  
  moveWithTurn(SPEED_DEFAULT, getRandomDirection());

  do {
    readSensors();
    
    delay(DELAY_NANO);

    if (isStuck()) {
      modeStuck();

      return;
    }

    if (obstacleProximity < DISTANCE_TOO_CLOSE || tiltAlert()) {
      return;
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
  if (isStuck()) {

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
boolean findLightDirection() {

  readSensors();

  uint16_t lightLevel = getLightLevel();
  uint16_t maxLightLevel = lightLevel;
  int16_t maxLightLevelDirection = gyroZ;
  float currentDirection = gyroZ;
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
      readSensors();
      lightLevel = getLightLevel();
      
      if (maxLightLevel < getLightLevel()) {
        maxLightLevel = getLightLevel();
        maxLightLevelDirection = gyroZ;
      }

      dAngle += abs(normalizeAngle(currentDirection - gyroZ));
      currentDirection = gyroZ;

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
  int16_t currentAngle = gyro.getAngle(3);

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
    currentAngle = gyro.getAngle(3);
  } while(!equalsWithinRange(angle, currentAngle, ANGLE_SENSITIVITY));
  
  Stop();
  
  Serial.print("Completed turn. angle=");
  Serial.print(angle);
  Serial.print(" currentAngle=");
  Serial.println(currentAngle);
  
  delay(DELAY_DEFAULT);
  
  return true;
}

/********* UTILITIES **************/

/**
 * 1 - forward
 * > 1 - goes left
 * < 1 - goes right
 */
float getMoveDirection() {
  if (currentSpeedL < 0 && currentSpeedR > 0) {
    return abs(currentSpeedL) / abs(currentSpeedR);
  } else if (currentSpeedL > 0 && currentSpeedR < 0) {
    return abs(currentSpeedR) / abs(currentSpeedL);
  } else {
    return 1;
  }
}

float getRandomDirection() {
  float r = random(1, PRECISION_RANDOM);
  return r/(PRECISION_RANDOM/2);
}

float getOppositeDirection() {
  return 1/getMoveDirection();
}

void resetStuckTimer() {
  stuckStartedTimestamp = 0;
}

boolean isStuck() {
  
  boolean result = false;

  Encoder_1.updateSpeed();
  Encoder_2.updateSpeed();
  currentSpeedL = Encoder_1.getCurrentSpeed();
  currentSpeedR = Encoder_2.getCurrentSpeed();  
  currentMotorPowerL = Encoder_1.getCurPwm();
  currentMotorPowerR = Encoder_2.getCurPwm();

  if (abs(currentMotorPowerL) < SPEED_MINIMAL && abs(currentMotorPowerR) < SPEED_MINIMAL) {

  } else if (abs(currentMotorPowerL) > SPEED_MINIMAL && currentMotorPowerR < SPEED_MINIMAL) {
    result = ((float)currentSpeedL)/((float)currentMotorPowerL) < 0.5;
  } else if (abs(currentMotorPowerR) > SPEED_MINIMAL && currentMotorPowerL < SPEED_MINIMAL) {
    result = ((float)currentSpeedR)/((float)currentMotorPowerR) < 0.5;
  } else {
    result = sqrt(((float)currentSpeedR)/((float)currentMotorPowerR) * ((float)currentSpeedL)/((float)currentMotorPowerL)) < 0.5;
  }

  if (result) {
    if (stuckStartedTimestamp == 0) {
      stuckStartedTimestamp = millis();
    }
  } else {
    stuckStartedTimestamp = 0;
  }

  result = result && stuckStartedTimestamp > 0 && (millis() > (stuckStartedTimestamp + DELAY_DEFAULT));

  if (result) {
    Serial.print("Stuck! ");
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


boolean tiltAlert() {
  return abs(gyroX) > 15 || abs(gyroY) > 15 || groundFlag == 0;
}

void readSensors() {
  //reading sensors
  gyroX = gyro.getAngle(1); //vertical axis
  gyroY = gyro.getAngle(2); //horizontal axis
  gyroZ = gyro.getAngle(3); //direction
  obstacleProximity = us->distanceCm(); //distance to front obstacle
  groundFlag = line.readSensors();
  lightLevelLeft = lightsensor_12.read(); //light left
  lightLevelRight = lightsensor_11.read(); //light right

  //Serial.println(obstacleProximity);
}


/**
 * > 1 - more light from left
 * < 1 - more light from right
 */
float getLightBalance() {
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
  return sqrt(lightLevelLeft*lightLevelRight);
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

/**
 * value1 equals value2  with +/- range
 */
boolean equalsWithinRange(double value1, double value2, double range) {
  return (value1 < value2+range) && (value1 > value2-range);
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
  us = new MeUltrasonicSensor(PORT_10);
  led.setpin(RGBLED_PORT);
  buzzer.setpin(BUZZER_PORT);
  led.setColor(0,0,0,0);
  led.show();
  buzzer.tone(1000,100); 
  buzzer.noTone();

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
}

