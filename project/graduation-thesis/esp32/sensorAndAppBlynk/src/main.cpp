#include "main.h"
#include "var.h"

char auth[] = BLYNK_AUTH_TOKEN;

// Led on board
#define ledSafeBoard 23   // pin 12 + of Led
#define ledDangerBoard 22 // pin 13 + of Led

// Config led on App Blynk
WidgetLED COAppLedSafe(V2);
WidgetLED COAppLedDanger(V3);

WidgetLED SOUNDAppLedSafe(V12);
WidgetLED SOUNDAppLedDanger(V13);

WidgetLED DUSTAppLedSafe(V22);
WidgetLED DUSTAppLedDanger(V23);

WidgetLCD LCDs(V100);

SharpGP2Y10 dustSensor(39, 38);

FirebaseData fbdata;


void setup()
{
  Sensor co; 
  Sensor dust;
  Sensor sound;

  Serial.begin(115200);

  Blynk.begin(auth, ssid, pass);
  Serial.println("Connect wifi " + String(ssid) + " success.");

  initPin(&co, &dust, &sound);

  Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);

  senDataSetupToFirebase(fbdata, co, dust, sound);

  lastTime = millis();
}

void loop()
{
  Sensor co; 
  Sensor dust;
  Sensor sound;

  runBlynks(co, dust, sound);

  readAllSensor(&co, &dust, &sound);

  controllDisplay(&co, &dust, &sound);

  sendDataToFirebase(fbdata, co, dust, sound);
}

void configSensor(Sensor *sensor, int pin, unsigned int value_set)
{
  sensor->pin = pin;
  sensor->valueSet = value_set;
  sensor->statusSafe = safe;
  sensor->valueRead = 0;
  sensor->valueUse = 0;
}

void initPin(Sensor *co, Sensor *dust, Sensor *sound)
{
  configSensor(co, 34, 500);
  configSensor(dust, 39, 1000);
  configSensor(sound, 36, 2000);

  // Input
  pinMode(co->pin, INPUT);
  pinMode(dust->pin, INPUT);
  pinMode(sound->pin, INPUT);

  // Output
  pinMode(ledSafeBoard, OUTPUT);
  pinMode(ledDangerBoard, OUTPUT);
  digitalWrite(ledSafeBoard, LOW);
  digitalWrite(ledDangerBoard, LOW);
}

void readAllSensor(Sensor *co, Sensor *dust, Sensor *sound)
{
  unsigned int *result;
  if (millis() - lastTime >= outTime)
  {
    result = readASensor(*co, 200, 2000);
    co->valueUse = *result;

    result = readASensor(*dust, 1000, 3000);
    dust->valueUse = *result;

    result = readASensor(*sound, 3000, 4000);
    sound->valueUse = *result;
    
    lastTime = millis();
  }
}

void runBlynks(Sensor co, Sensor dust, Sensor sound)
{
  Blynk.run();

  // Transmit data from Esp32 to App Blynk
  Blynk.virtualWrite(V1, co.valueUse);
  Blynk.virtualWrite(V11, sound.valueUse);
  Blynk.virtualWrite(V21, dust.valueUse);
}

void controllDisplay(Sensor *co, Sensor *dust, Sensor *sound)
{
  checkSafe(co, dust, sound);

  controllLedBoard(*co, *dust, *sound);
  controllLedApp(*co, *dust, *sound);
  writeLcdAdvanced(*co, *dust, *sound);
}

void checkSafe(Sensor *co, Sensor *dust, Sensor *sound)
{
  if (co->valueUse <= co->valueSet)
    co->statusSafe = safe;
  else
    co->statusSafe = danger;

  if (dust->valueUse <= dust->valueSet)
    dust->statusSafe = safe;
  else
    dust->statusSafe = danger;

  if (sound->valueUse <= sound->valueSet)
    sound->statusSafe = safe;
  else
    sound->statusSafe = danger;
}

void controllLedBoard(Sensor co, Sensor dust, Sensor sound)
{
  if (co.statusSafe && dust.statusSafe && sound.statusSafe)
  {
    digitalWrite(ledSafeBoard, HIGH);
    digitalWrite(ledDangerBoard, LOW);
  }
  else
  {
    digitalWrite(ledSafeBoard, LOW);
    digitalWrite(ledDangerBoard, HIGH);
  }
}

void controllLedApp(Sensor co, Sensor dust, Sensor sound)
{
  if (co.statusSafe == safe)
  {
    COAppLedSafe.on();
    COAppLedDanger.off();
  }
  else
  {
    COAppLedSafe.off();
    COAppLedDanger.on();
  }

  if (sound.statusSafe)
  {
    SOUNDAppLedSafe.on();
    SOUNDAppLedDanger.off();
  }
  else
  {
    SOUNDAppLedSafe.off();
    SOUNDAppLedDanger.on();
  }

  if (dust.statusSafe)
  {
    DUSTAppLedSafe.on();
    DUSTAppLedDanger.off();
  }
  else
  {
    DUSTAppLedSafe.off();
    DUSTAppLedDanger.on();
  }
}

// No support for web blynk
void writeLcdBasic(Sensor co)
{
  /*
  Ba - Basic
  transmit data up lcd in app blynk
  lcd on app must setup basic mode
  */

  Blynk.virtualWrite(V100, "CO_Value: " + String(co.valueUse));
}

void writeLcdAdvanced(Sensor co, Sensor dust, Sensor sound)
{
  /*
  Ad - Advanced
  lcd on app must setup advanced mode
  */

  static int counter;

  LCDs.print(0, 0, "environment:");

  if (co.statusSafe && dust.statusSafe && sound.statusSafe)
  {
    LCDs.print(5, 1, " safe ");
    counter++;
  }
  else
  {
    LCDs.print(5, 1, "danger");
  }

  if (counter % 4 == 0)
  {
    LCDs.clear();
  }
}

unsigned int *readASensor(Sensor sensor, unsigned int lowerLimit, unsigned int upperLimit)
{
  static unsigned int result = 0;

  sensor.valueRead = analogRead(sensor.pin);
  sensor.valueUse = map(sensor.valueRead, minAnalog, maxAnalog, lowerLimit, upperLimit);

  Serial.println("\n\nCO Gas value basic: " + String(sensor.valueRead) + "  CO Gas value use: " + String(sensor.valueUse));

  result = sensor.valueUse;
  return (&result);
}


void sendDataToFirebase(FirebaseData firedata, Sensor co, Sensor dust, Sensor sound)
{
  // Value use
  Firebase.setInt(firedata, "Node1/Co/Value", co.valueUse);
  Firebase.setInt(firedata, "Node1/Dust/Value", dust.valueUse);
  Firebase.setInt(firedata, "Node1/Sound/Value", sound.valueUse);

  // Status current
  Firebase.setInt(firedata, "Node1/Co/Safe", co.statusSafe);
  Firebase.setInt(firedata, "Node1/Dust/Safe", dust.statusSafe);
  Firebase.setInt(firedata, "Node1/Sound/Safe", sound.statusSafe);
}

/*
  Run only once when start turn on power MCU
*/
void senDataSetupToFirebase(FirebaseData firedata, Sensor co, Sensor dust, Sensor sound)
{
  Firebase.setInt(firedata, "Node1/Co/Set", co.valueSet);
  Firebase.setInt(firedata, "Node1/Dust/Set", dust.valueSet);
  Firebase.setInt(firedata, "Node1/Sound/Set", sound.valueSet);
}

