#include <Arduino.h>
#include <ArduinoJson.h>
#include <WiFi.h>
#include <SPIFFS.h>
#include <Update.h>
#include "namedMesh.h"
#include <Wire.h>
#include "SparkFun_VL53L1X.h"

#define SENS_NAME "S02"
#define MESH_SSID       "rede_mesh_i9"
#define MESH_PASSWORD   "deumaoito"
#define MESH_PORT       5555
#define SDA 8
#define SCL 9
#define SPEED 400000

SFEVL53L1X distanceSensor;
Scheduler userScheduler;
namedMesh mesh;
int distanceMM = 0;

String nodeName = SENS_NAME; // Name needs to be unique

Task taskSendMessage( TASK_SECOND*0.5, TASK_FOREVER, []() {
    String msg = String(distanceMM);
    String to = "central_i9";
    mesh.sendSingle(to, msg); 
});

void setup() {
  // put your setup code here, to run once:
  // Serial.begin(9600);
  Wire.setPins(SDA,SCL);
  Wire.setClock(400000);
  Wire.begin();

  if (distanceSensor.begin(Wire) != 0) //Begin returns 0 on a good init
  {
    //Serial.println("Sensor failed to begin. Please check wiring. Freezing...");
    while (1);
  }
  //Serial.println("Sensor online!");
   mesh.setDebugMsgTypes(ERROR | DEBUG | CONNECTION);  // set before init() so that you can see startup messages

  mesh.init(MESH_SSID, MESH_PASSWORD, &userScheduler, MESH_PORT);

  mesh.setName(nodeName); // This needs to be an unique name! 

  mesh.onReceive([](String &from, String &msg) {
    //Serial.printf("Received message by name from: %s, %s\n", from.c_str(), msg.c_str());
  });

  mesh.onChangedConnections([]() {
    //Serial.printf("Changed connection\n");
  });

  userScheduler.addTask(taskSendMessage);
  taskSendMessage.enable();
}

void loop() {
  // put your main code here, to run repeatedly:
  distanceSensor.startRanging();
  while (!distanceSensor.checkForDataReady())
  {
    delay(1);
  }
  distanceMM = distanceSensor.getDistance();
  distanceSensor.clearInterrupt();
  distanceSensor.stopRanging();
  mesh.update();
}