
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
#define US_PORT                              PORT_6

#define COMMAND_LED                             "led"
#define COMMAND_BEEP                            "beep"
#define COMMAND_GET_TEMPERATURE                 "getTemperature"
#define COMMAND_GET_SOUND_LEVEL                 "getSoundLevel"
#define COMMAND_GET_LIGHT_LEVEL                 "getLightLevel"
#define COMMAND_SET_TEMPERATURE_THRESHOLD       "setTemperatureThreshold"
#define COMMAND_SET_SOUND_LEVEL_THRESHOLD       "setSoundLevelThreshold"
#define COMMAND_SET_LIGHT_LEVEL_THRESHOLD       "setLightLevelThreshold"

#define COMMAND_STOP                            "stop"
#define COMMAND_FIND_LIGHT                      "findLightDirection"
#define COMMAND_NAVIGATE                        "navigate"
#define COMMAND_LEFT                            "left"
#define COMMAND_RIGHT                           "right"
#define COMMAND_FORWARD                         "forward"
#define COMMAND_BACKWARD                        "backward"
#define COMMAND_FORWARD_LEFT                    "forwardLeft"
#define COMMAND_FORWARD_RIGHT                   "forwardRight"
#define COMMAND_BACKWARD_LEFT                   "backwardLeft"
#define COMMAND_BACKWARD_RIGHT                  "backwardRight"

#define EVENT_TILT                              "onTilt"
#define EVENT_OBSTACLE                          "onObstacle"
#define EVENT_FREE_WAY                          "onObstacleFree"
#define EVENT_PROXIMITY                         "onProximity"
#define EVENT_FOUND_LIGHT_DIRECTION             "onFoundLightDirection"
#define EVENT_NOT_MOVING                        "onNotMoving"
#define EVENT_STUCK                             "onStuck"
#define EVENT_PROXIMITY                         "onProximity"
#define EVENT_TEMPERATURE                       "onTemperature"
#define EVENT_SOUND_LEVEL                       "onSoundLevel"
#define EVENT_LIGHT_LEVEL                       "onLightLevel"

//constants
#define ANGLE_SENSITIVITY           5
#define TEMPERATURE_THRESHOLD       1.00f
#define SOUND_LEVEL_THRESHOLD       250.00f
#define LIGHT_LEVEL_THRESHOLD       300

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
MeOnBoardTemp temperature_onboard(PORT_13);
MeSoundSensor soundsensor_14(14);

//variables
long wheelStuckTime = 0;
long distanceStuckTime = 0;
uint16_t previousObstacleProximity = 0;

String lastCommand = "";
long lastCommandTimeStamp = 0;

float lastTemperature = -TEMPERATURE_THRESHOLD;
float lastSoundLevel = -SOUND_LEVEL_THRESHOLD;
uint16_t lastLightLevel = -LIGHT_LEVEL_THRESHOLD;

float temperatureThreshold = TEMPERATURE_THRESHOLD;
float soundLevelThreshold = SOUND_LEVEL_THRESHOLD;
uint16_t lightLevelThreshold = LIGHT_LEVEL_THRESHOLD;

long lastSensorUpdateTimeStamp = 0;

//TODO: conditional turn (depends on what track is stuck)
//TODO: not random turn on obstacle avoidance
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
  processInterruptingCommand();
  
  while (!isInterrupted()) {
    ;
  }

  processInterruptingCommand();
}

boolean isInterrupted() {
  runBackgroundProcesses();
  
  if (hasNewCommand()) {
    lastCommand = Serial.readString();
    Serial.print("Received command: ");
    Serial.println(lastCommand);
    lastCommandTimeStamp = millis();
    return !processNonInterruptingCommand();
  }

  return false;
}

boolean hasNewCommand() {
  return Serial.available() > 0;
}

boolean isCommand(String command) {
  return lastCommand.startsWith(command);
}

boolean processNonInterruptingCommand() {
  boolean isProcessed = false;
  
  if (isCommand(COMMAND_LED)) {
    int index = getCommandParamValueInt(lastCommand, "index");
    int r = getCommandParamValueInt(lastCommand, "r");
    int g = getCommandParamValueInt(lastCommand, "g");
    int b = getCommandParamValueInt(lastCommand, "b");

    if (index > -1) {
      led.setColorAt(index,r,g,b);
    } else {
      led.setColor(r,g,b);
    }
    led.show();

    isProcessed = true;
  } else if (isCommand(COMMAND_BEEP)) {

    int frequency = getCommandParamValueInt(lastCommand, "frequency");
    int duration = getCommandParamValueInt(lastCommand, "duration");
    
    buzzer.tone(frequency, duration);

    isProcessed = true;
  } else if (isCommand(COMMAND_GET_TEMPERATURE)) {

    lastTemperature = -temperatureThreshold;
    checkTemperature();

    isProcessed = true;
  } else if (isCommand(COMMAND_GET_SOUND_LEVEL)) {

    lastSoundLevel = -soundLevelThreshold;
    checkSoundLevel();

    isProcessed = true;
  } else if (isCommand(COMMAND_GET_LIGHT_LEVEL)) {

    lastLightLevel = -lightLevelThreshold;
    checkLightLevel();

    isProcessed = true;
  } else if (isCommand(COMMAND_SET_TEMPERATURE_THRESHOLD)) {
    temperatureThreshold = getCommandParamValueInt(lastCommand, "threshold");

    isProcessed = true;
  } else if (isCommand(COMMAND_SET_SOUND_LEVEL_THRESHOLD)) {
    soundLevelThreshold = getCommandParamValueInt(lastCommand, "threshold");

    isProcessed = true;
  } else if (isCommand(COMMAND_SET_LIGHT_LEVEL_THRESHOLD)) {
    lightLevelThreshold = getCommandParamValueInt(lastCommand, "threshold");

    isProcessed = true;
  }

  if (isProcessed) {
    lastCommand = "";
  }

  return isProcessed;
}

boolean processInterruptingCommand() {
  boolean isProcessed = false;

  if (isCommand(COMMAND_FIND_LIGHT)) {
    lastCommand = "";
    commandFindLightDirection();
    isProcessed = true;
  } else if (isCommand(COMMAND_NAVIGATE)) {
    lastCommand = "";
    commandNavigate();
    isProcessed = true;
  } else if (isCommand(COMMAND_STOP)) {
    lastCommand = "";
    Stop();
    isProcessed = true;
  } else if (isCommand(COMMAND_BACKWARD_RIGHT)) {
    lastCommand = "";
    BackwardAndTurnRight(SPEED_DEFAULT);
    executeAndStopUntilNewCommandWithDelay();
    isProcessed = true;
  } else if (isCommand(COMMAND_BACKWARD_LEFT)) {
    lastCommand = "";
    BackwardAndTurnLeft(SPEED_DEFAULT);
    executeAndStopUntilNewCommandWithDelay();
    isProcessed = true;
  } else if (isCommand(COMMAND_FORWARD_RIGHT)) {
    lastCommand = "";
    TurnRight(SPEED_DEFAULT);
    executeAndStopUntilNewCommandWithDelay();
    isProcessed = true;
  } else if (isCommand(COMMAND_FORWARD_LEFT)) {
    lastCommand = "";
    TurnLeft(SPEED_DEFAULT);
    executeAndStopUntilNewCommandWithDelay();
    isProcessed = true;
  } else if (isCommand(COMMAND_FORWARD)) {
    lastCommand = "";
    Forward(SPEED_DEFAULT);
    executeAndStopUntilNewCommandWithDelay();
    isProcessed = true;
  } else if (isCommand(COMMAND_BACKWARD)) {
    lastCommand = "";
    Backward(SPEED_DEFAULT);
    executeAndStopUntilNewCommandWithDelay();
    isProcessed = true;
  } else if (isCommand(COMMAND_LEFT)) {
    lastCommand = "";
    Left(SPEED_DEFAULT);
    executeAndStopUntilNewCommandWithDelay();
    isProcessed = true;
  } else if (isCommand(COMMAND_RIGHT)) {
    lastCommand = "";
    Right(SPEED_DEFAULT);
    executeAndStopUntilNewCommandWithDelay();
    isProcessed = true;
  }

  return isProcessed;
}

void executeAndStopUntilNewCommandWithDelay() {
  do {
    if (millis() > lastCommandTimeStamp + DELAY_MACRO) {
      Stop(); //we hit delay
      return;
    }
  } while (!isInterrupted());
}

void runBackgroundProcesses() {
    checkTemperature();
    checkSoundLevel();
    checkLightLevel();
}

void checkTemperature() {
  float currentTemperature = getTemperature();
  if (!equalsWithinRange(lastTemperature, currentTemperature, TEMPERATURE_THRESHOLD)) {
    lastTemperature = currentTemperature;
    Serial.println(generateEventJson(EVENT_TEMPERATURE, "temperature", currentTemperature));
  }
}

void checkSoundLevel() {
  float currentSoundLevel = getSoundLevel();
  if (!equalsWithinRange(lastSoundLevel, currentSoundLevel, SOUND_LEVEL_THRESHOLD)) {
    lastSoundLevel = currentSoundLevel;   
    Serial.println(generateEventJson(EVENT_SOUND_LEVEL, "soundLevel", currentSoundLevel));
  }
}

void checkLightLevel() {
  uint16_t currentLightLevel = getLightLevel();
  if (!equalsWithinRange(lastLightLevel, currentLightLevel, LIGHT_LEVEL_THRESHOLD)) {
    lastLightLevel = currentLightLevel;
    Serial.println(generateEventJson(EVENT_LIGHT_LEVEL, "lightLevel", currentLightLevel));   
  }
}

String generateEventJson(String eventName, String propertyName, float propertyValue) {
  String json = "{ \"event\":\"";
  json += eventName;
  json += "\",\"name\":\"";
  json += propertyName;
  json += "\",\"value\":\"";
  json += propertyValue;
  json += "\"}";
  return json;
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
    
  } while (!isInterrupted());
}

/**
 * Critical angle or don't see the ground - move backward
 */
void modeOnTilt() {
  
  Serial.print(EVENT_TILT);
  Serial.print(" x=");
  Serial.print(getGyroX());
  Serial.print(" y=");
  Serial.print(getGyroY());
  Serial.print(" ground=");
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

    if (isInterrupted()) {
      break;
    }
    
  } while (tiltAlert());
  
  delay(DELAY_DEFAULT);

  resetStuckTimer();

//  Serial.print("Tilt alert is fixed gyroX=");
//  Serial.print(getGyroX());
//  Serial.print(" gyroY=");
//  Serial.print(getGyroY());
//  Serial.print(" groundFlag=");
//  Serial.println(getGroundFlag()); 
}

/**
 * Obstacle is too close, trying to find free direction
 */
void modeObstacleIsTooClose() {
  uint16_t obstacleProximity = getObstacleProximity();
  
  Serial.print(EVENT_OBSTACLE);
  Serial.print(" distance=");
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

    if (isInterrupted()) {
      break;
    }
    
    delay(DELAY_NANO);
    
  } while (obstacleProximity < DISTANCE_OBSTACLE_DETECTED);

  Serial.print(EVENT_FREE_WAY);
  Serial.print(" distance=");
  Serial.println(obstacleProximity);

  delay(DELAY_MICRO);

  resetStuckTimer();
}

/**
 * Approaching obstacle, lets change direction
 */
void modeAvoidObstacle() {
  uint16_t obstacleProximity = getObstacleProximity();

  Serial.print(EVENT_PROXIMITY);
  Serial.print(" distance=");
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

    if (isInterrupted()) {
      break;
    }
    
  } while (obstacleProximity < DISTANCE_OBSTACLE_DETECTED);

  Serial.print(EVENT_FREE_WAY);
  Serial.print(" distance=");
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

//  Serial.print("Searching for light direction. lightBalance=");
//  Serial.print(lightBalance);
//  Serial.print(" lightLevel=");
//  Serial.println(lightLevel);

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

      if (isInterrupted()) {
        break;
      }

      delay(DELAY_NANO);
  } while (dAngle < 360);

  Stop();

  Serial.print(EVENT_FOUND_LIGHT_DIRECTION);
  Serial.print(" direction=");
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

//  Serial.print("Started turn. angle=");
//  Serial.print(angle);
//  Serial.print(" currentAngle=");
//  Serial.println(currentAngle);

  if (normalizeAngle(angle-currentAngle) > 0) {
    Right(moveSpeed);
  } else {
    Left(moveSpeed);
  }
  
  do {
    currentAngle = getGyroZ();

    if (isInterrupted()) {
      break;
    }
    
  } while(!equalsWithinRange(angle, currentAngle, ANGLE_SENSITIVITY));
  
  Stop();
  
//  Serial.print("Completed turn. angle=");
//  Serial.print(angle);
//  Serial.print(" currentAngle=");
//  Serial.println(currentAngle);
  
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

float getTemperature() {
  return temperature_onboard.readValue();
}

float getSoundLevel() {
    return soundsensor_14.strength();
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

    Serial.print(EVENT_NOT_MOVING);
    Serial.print(" previousDistance=");
    Serial.print(previousObstacleProximity);
    Serial.print(" distance=");
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

    Serial.print(EVENT_STUCK);
    Serial.print(" motorL=");
    Serial.print(currentMotorPowerL);
    Serial.print(" speedL");
    Serial.print(currentSpeedL);
    Serial.print(" motorR=");
    Serial.print(currentMotorPowerR);
    Serial.print(" speedR=");
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

const int16_t TEMPERATURENOMINAL     = 25;    //Nominl temperature depicted on the datasheet
const int16_t SERIESRESISTOR         = 10000; // Value of the series resistor
const int16_t BCOEFFICIENT           = 3380;  // Beta value for our thermistor(3350-3399)
const int16_t TERMISTORNOMINAL       = 10000; // Nominal temperature value for the thermistor

/**
 * \par Function
 *    calculate_temp
 * \par Description
 *    This function is used to convert the temperature.
 * \param[in]
 *    In_temp - Analog values from sensor.
 * \par Output
 *    None
 * \return
 *    the temperature in degrees Celsius
 * \par Others
 *    None
 */
float calculate_temp(int16_t In_temp)
{
  float media;
  float temperatura;
  media = (float)In_temp;
  // Convert the thermal stress value to resistance
  media = 1023.0 / media - 1;
  media = SERIESRESISTOR / media;
  //Calculate temperature using the Beta Factor equation

  temperatura = media / TERMISTORNOMINAL;              // (R/Ro)
  temperatura = log(temperatura); // ln(R/Ro)
  temperatura /= BCOEFFICIENT;                         // 1/B * ln(R/Ro)
  temperatura += 1.0 / (TEMPERATURENOMINAL + 273.15);  // + (1/To)
  temperatura = 1.0 / temperatura;                     // Invert the value
  temperatura -= 273.15;                               // Convert it to Celsius
  return temperatura;
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
  us = new MeUltrasonicSensor(US_PORT);
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

  //demo();
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

