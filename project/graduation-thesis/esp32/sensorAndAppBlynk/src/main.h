#ifndef MAIN_H
#define MAIN_H

#include <stdio.h>
#include <Arduino.h>

#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>
#include <SharpGP2Y10.h>
#include <FirebaseESP32.h>

#define BLYNK_TEMPLATE_ID "TMPLpBdYLvHV"
#define BLYNK_DEVICE_NAME "NCKH BASIC 1"
#define BLYNK_AUTH_TOKEN "dPQne8M3jZdkfnskX2lSF8UMFIq9bIpB"

#define FIREBASE_HOST "https://tt-iot-utc-default-rtdb.firebaseio.com/"
#define FIREBASE_AUTH "I8dY6yv4GrCOAASGRfkmOy1nlM4BOrxCT5jCHXWM"

enum StatusSafe
{
    danger = 0,
    safe
};

typedef struct Sensor
{
    int pin;
    unsigned int valueSet, valueRead, valueUse;
    bool statusSafe;
};

// Sensors Sensor;

void configSensor(Sensor *sensor, int pin, unsigned int value_set);
void initPin(Sensor *co, Sensor *dust, Sensor *sound);
void readAllSensor(Sensor *co, Sensor *dust, Sensor *sound);
void runBlynks(Sensor co, Sensor dust, Sensor sound);
void controllDisplay(Sensor *co, Sensor *dust, Sensor *sound);
void checkSafe(Sensor *co, Sensor *dust, Sensor *sound);
void controllLedBoard(Sensor co, Sensor dust, Sensor sound);
void controllLedApp(Sensor co, Sensor dust, Sensor sound);
void writeLcdBasic(Sensor co);
void writeLcdAdvanced(Sensor co, Sensor dust, Sensor sound);
unsigned int *readASensor(Sensor sensor, unsigned int lowerLimit, unsigned int upperLimit);
void sendDataToFirebase(FirebaseData firedata, Sensor co, Sensor dust, Sensor sound);
void senDataSetupToFirebase(FirebaseData firedata, Sensor co, Sensor dust, Sensor sound);

#endif