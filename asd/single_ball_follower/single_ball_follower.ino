#include <Ultrasonic.h>

/*
This code is based on the examples at http://forum.arduino.cc/index.php?topic=396450
*/


// Example 5 - parsing text and numbers with start and end markers in the stream

#include "helper.h"
#include "ISAMobile.h"
#include <AutoPID.h>
#define M_PI 3.14159265358979323846


#define TEMP_READ_DELAY 800

//pid settings and gains
#define OUTPUT_MIN -5
#define OUTPUT_MAX 5
#define KP 0.12
#define KI 0.0003
#define KD 0

double input, setPoint, outputVal=0;

AutoPID myPID(&input, &setPoint, &outputVal, OUTPUT_MIN, OUTPUT_MAX, KP, KI, KD);

unsigned long lastTempUpdate;

const byte numChars = 32;
char receivedChars[numChars];
char tempChars[numChars];        // temporary array for use when parsing

dataPacket packet;


boolean newData = false;

void SetPowerLevel(EngineSelector side, int level)
{
  level = constrain(level, -255, 255);

  if (side == EngineSelector::Left) {
    if (level > 0) {
      // do przodu
      digitalWrite(LEFT_IN1, false);
      digitalWrite(LEFT_IN2, true);
      analogWrite(LEFT_PWM, level);
    } else if (level < 0) {
      // do tyłu
      digitalWrite(LEFT_IN1, true);
      digitalWrite(LEFT_IN2, false);
      analogWrite(LEFT_PWM, -level);
    } else {
      // stop (soft)
      digitalWrite(LEFT_IN1, true);
      digitalWrite(LEFT_IN2, true);
      analogWrite(LEFT_PWM, 0);
    }
  }
  
  if (side == EngineSelector::Right) {
    if (level > 0) {
      // do przodu
      digitalWrite(RIGHT_IN1, true);
      digitalWrite(RIGHT_IN2, false);
      analogWrite(RIGHT_PWM, level);
    } else if (level < 0) {
      // do tyłu
      digitalWrite(RIGHT_IN1, false);
      digitalWrite(RIGHT_IN2, true);
      analogWrite(RIGHT_PWM, -level);
    } else {
      // stop (soft)
      digitalWrite(RIGHT_IN1, true);
      digitalWrite(RIGHT_IN2, true);
      analogWrite(RIGHT_PWM, 0);
    }
  } 
}


//============

void setup() {
  
      // Czujniki ultradźwiekowe
  for (int i = (int)UltraSoundSensor::__first; i <= (int)UltraSoundSensor::__last; i++)
  {
    pinMode(ultrasound_trigger_pin[i], OUTPUT);
    pinMode(ultrasound_echo_pin[i], INPUT);
    
    digitalWrite(ultrasound_trigger_pin[i], 0);
  }
  
  // Silniki
  pinMode(A_PHASE, OUTPUT);
  pinMode(A_ENABLE, OUTPUT);
  pinMode(B_PHASE, OUTPUT);
  pinMode(B_ENABLE, OUTPUT);
  
  //pinMode(MODE, OUTPUT); -- podłaczone na krótko ze stanem wysokim
  //digitalWrite(MODE, true);  -- podłaczone na krótko ze stanem wysokim
  
  SetPowerLevel(EngineSelector::Left, 0);
  SetPowerLevel(EngineSelector::Right, 0);

  // Wejścia enkoderowe
  pinMode(ENCODER_LEFT, INPUT);
  pinMode(ENCODER_RIGHT, INPUT);
  
  Serial.begin(57600);
  Serial.println("This demo expects 3 pieces of data - text, an integer and a floating point value");
  Serial.println("Enter data in this style <HelloWorld, 12, 24.7>  ");
  Serial.println();
    
}

//============

void loop() {
    recvWithStartEndMarkers();
    if (newData == true) {
        strcpy(tempChars, receivedChars);
            // this temporary copy is necessary to protect the original data
            //   because strtok() used in parseData() replaces the commas with \0
        packet = parseData();
        if (packet.error >= -100 && packet.error <= 100) {
          input = packet.error;
          myPID.run();
  
          float normalSpeed=200;
          float s=20;
  
          if((float)outputVal>0 ){
             SetPowerLevel(EngineSelector::Right, normalSpeed+outputVal*s);
             SetPowerLevel(EngineSelector::Left, normalSpeed-outputVal*s);
          } else if((float)outputVal<0 ){
             SetPowerLevel(EngineSelector::Right, normalSpeed+outputVal*s);
             SetPowerLevel(EngineSelector::Left, normalSpeed-outputVal*s); 
          } else{
             SetPowerLevel(EngineSelector::Right, normalSpeed);
             SetPowerLevel(EngineSelector::Left, normalSpeed);
          }
        } else {
          SetPowerLevel(EngineSelector::Right, 0);
          SetPowerLevel(EngineSelector::Left, 0);
        }        

        showParsedData(packet);
        newData = false;
        
    }
}

//============

void recvWithStartEndMarkers() {
    static boolean recvInProgress = false;
    static byte ndx = 0;
    char startMarker = '<';
    char endMarker = '>';
    char rc;

    while (Serial.available() > 0 && newData == false) {
        rc = Serial.read();

        if (recvInProgress == true) {
            if (rc != endMarker) {
                receivedChars[ndx] = rc;
                ndx++;
                if (ndx >= numChars) {
                    ndx = numChars - 1;
                }
            }
            else {
                receivedChars[ndx] = '\0'; // terminate the string
                recvInProgress = false;
                ndx = 0;
                newData = true;
            }
        }

        else if (rc == startMarker) {
            recvInProgress = true;
        }
    }
}

//============

dataPacket parseData() {      // split the data into its parts

    dataPacket tmpPacket;

    char * strtokIndx; // this is used by strtok() as an index

    strtokIndx = strtok(tempChars,",");      // get the first part - the string
    strcpy(tmpPacket.message, strtokIndx); // copy it to messageFromPC
 
    strtokIndx = strtok(NULL, ","); // this continues where the previous call left off
    tmpPacket.error = atof(strtokIndx);     // convert this part to an integer

    strtokIndx = strtok(NULL, ",");
    tmpPacket.blob_size = atof(strtokIndx);     // convert this part to a float

    return tmpPacket;
}


void showParsedData(dataPacket packet) {
    Serial.print("Message ");
    Serial.println(packet.message);
    Serial.print("Error ");
    Serial.println(packet.error);
    Serial.print("Blob size ");
    Serial.println(packet.blob_size);
}
