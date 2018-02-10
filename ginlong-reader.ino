
#include <SPI.h>

#include <pins_arduino.h>

//display drivers
#include <LiquidCrystal.h>

//wifi headers
#include <ESP8266WiFiMulti.h>
#include <WiFiUdp.h>
#include <WiFiClient.h>
#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>

//constants
//Screen
#define   LCDRS   D2
#define   LCDEN   D3
#define   LCD4   D4
#define   LCD5  D5
#define   LCD6  D6
#define   LCD7  D7

#define TICKTOCK D1

#define POWERSW D0

// turn the power relay on if we are making more than this amount of power.
const int MINIMUM_POWER = 2000;

const unsigned long TEMPERATURE_INTERVAL = 1000;
const unsigned long PACKET_TIMEOUT = 1000 * 60 * 10; // 10 minutes

const float CALIBRATION_CONSTANT = 98.0;

// WiFi parameters to be configured
const char* ssid = "porcini";
const char* password = "#Barghest#";
IPAddress me(192, 168, 1, 5);
IPAddress gateway(192, 168, 1, 1); // set gateway to match your network
IPAddress subnet(255, 255, 255, 0); // set subnet mask to match your network
float setPoint = 37.5;

//WiFiServer server;
WiFiUDP udp;
const int MAX_SIZE = 500;
char packetBuffer[MAX_SIZE];
LiquidCrystal lcd(LCDRS, LCDEN, LCD4, LCD5, LCD6, LCD7);

unsigned long lastPacket;
unsigned long lastTemperature;



void setup() {
  //initialize the serial port
  Serial.begin(9600);
  Serial.setDebugOutput(true);
  while (!Serial);
  Serial.println("Serial line configured");

  lcd.begin(16, 2);

  pinMode(POWERSW, OUTPUT);
  pinMode(TICKTOCK, OUTPUT);
  powerOff();


  //wifi init
  WiFi.persistent(false);
  WiFi.mode(WIFI_OFF);
  WiFi.mode(WIFI_STA);


  lcd.clear();

  lcd.print("Connecting");
  lcd.cursor();
  lcd.blink();
  // Connect to WiFi
  WiFi.config(me, gateway, subnet);
  WiFi.begin(ssid, password);

  // while wifi not connected yet, print '.'
  // then after it connected, get out of the loop
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  lcd.clear();
  lcd.print("WiFi connected");
  lcd.noCursor();
  lcd.noBlink();
  // Print the IP address
  lcd.setCursor(0, 1);
  lcd.print(WiFi.localIP());
  //server.begin(2300);
  udp.begin(3030);


  lastPacket = millis();
  lastTemperature=lastPacket;
}


int watts = 0;
float temp = 0;

char next = 4;

void loop() {

  unsigned long now = millis();

  if ( now < lastPacket) {
    // timer rolled over
    lastPacket = now;
  } else if ( (now - lastPacket) > PACKET_TIMEOUT ) {
    watts = 0; // haven't heard from it.
    Serial.print("Timed out for packet");
    lcd.clear();
    lastPacket = now; // only once do this
    printWatts();
    tickTock(nowk);
    decideToHeat();
  }

  if ( now < lastTemperature) {
    // timer rolled over
    lastTemperature = now;
  } else if ( (now - lastTemperature) > TEMPERATURE_INTERVAL) {
    temp = measureTemperature(now);
    decideToHeat();
    lastTemperature = now;
    next |= 1;
  }

  int packetSize = udp.parsePacket();
  if (packetSize) {
    lastPacket=now;
    Serial.print("Received packet of size ");
    Serial.println(packetSize);
    Serial.print("From ");
    IPAddress remoteIp = udp.remoteIP();
    Serial.print(remoteIp);
    Serial.print(", port ");
    Serial.println(udp.remotePort());
    // read the packet into packetBufffer
    udp.read(packetBuffer, packetSize);

    //dumpPacket(packetSize);
    if ( packetBuffer[3] == 0xB1) {
      return; // not the packet we want
    }
    int x1 = packetBuffer[59] & 0xff;
    int x2 = packetBuffer[60] & 0xff;
    watts = (x1 * 256) + x2;
    printWatts();
    decideToHeat();
  } else {
    // not busy, can do slow lcd stuff w/o borking net link
    switch ( next) {
      case 0: break;
      case 1: tickTock(now);
    next = 0;
        break;
     
        case 4:lcd.clear();
        next=0;
        break;
        default:
        break;
    }
  }
}


void tickTock(unsigned long now) {
  if ( (now % 2) == 0) {
  digitalWrite(TICKTOCK, HIGH);
  } else {
  digitalWrite(TICKTOCK, LOW); 
  }

}
void dumpPacket(int packetSize) {
  Serial.println("Contents:");
  for ( int i = 0; i < packetSize; i++) {
    Serial.print((int)packetBuffer[i], HEX);
    Serial.print(" ");
  }
  Serial.println();
}

void decideToHeat() {
  if ( watts > MINIMUM_POWER && temp < setPoint) {
    powerOn();
  } else {
    powerOff();
  }
}



void printWatts() {
  lcd.setCursor(0, 1);
  lcd.print("watts:" );
  lcd.print(watts);
  lcd.print("   "); // erase line
  Serial.println(watts);
  Serial.println("watts:");
}

float measureTemperature(unsigned long now) {
  int tempI = analogRead(A0);
  lcd.setCursor(0, 0);
  float temp = (((tempI / 1024.0) * 1000l ) - 273.4) - CALIBRATION_CONSTANT;
  lcd.print("temp:" );
  lcd.print(temp, 1);
  lcd.print(" ");
  return temp;

}

int on = 0;

void powerOn() {
  if (on == 1) return;
  on = 1;
  Serial.println(" Power on");
  digitalWrite(POWERSW, HIGH); // have power

}
void powerOff() {
  if ( on == 0 ) return;
  on = 0;
  digitalWrite(POWERSW, LOW); //not so much
  Serial.println(" Power off");
}

