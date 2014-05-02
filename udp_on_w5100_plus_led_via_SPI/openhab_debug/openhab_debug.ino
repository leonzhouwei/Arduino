#include <SPI.h>         // needed for Arduino versions later than 0018
#include <Ethernet.h>
#include <EthernetUdp.h>         // UDP library from: bjoern@cs.stanford.edu 12/30/2008
#include <OneWire.h>
#include <DallasTemperature.h>

// Enter a MAC address and IP address for your controller below.
// The IP address will be dependent on your local network:
byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
IPAddress ip(192, 168, 1, 177);
unsigned int localPort = 8888;      // local port to listen on

// buffers for receiving and sending data
char packetBuffer[UDP_TX_PACKET_MAX_SIZE]; //buffer to hold incoming packet,
char  ReplyBuffer[15] = "acknowledged";       // a string to send back

// An EthernetUDP instance to let us send and receive packets over UDP
EthernetUDP Udp;

// Data wire is plugged into port 9 on the Arduino
#define ONE_WIRE_BUS 9

// Setup a oneWire instance to communicate with any OneWire devices (not just Maxim/Dallas temperature ICs)
OneWire oneWire(ONE_WIRE_BUS);

// Pass our oneWire reference to Dallas Temperature. 
DallasTemperature sensors(&oneWire);

void setup() {
  // set the output pins
  pinMode(3, OUTPUT);
  pinMode(4, OUTPUT);
  pinMode(5, OUTPUT);
  pinMode(6, OUTPUT);
  pinMode(7, OUTPUT);
  pinMode(8, OUTPUT);
  // start the Ethernet and UDP:
  Ethernet.begin(mac,ip);
  Udp.begin(localPort);
  // 
  Serial.begin(9600);
  // Start up the Dallas Temperature library
  sensors.begin();
}

void loop() {
  // if there's data available, read a packet
  int packetSize = Udp.parsePacket();
  if(packetSize) {
    Serial.print("Received packet of size ");
    Serial.println(packetSize);
    Serial.print("From ");
    IPAddress remote = Udp.remoteIP();
    for (int i =0; i < 4; i++) {
      Serial.print(remote[i], DEC);
      if (i < 3) {
        Serial.print(".");
      }
    }
    Serial.print(", port ");
    Serial.println(Udp.remotePort());

    // read the packet into packetBufffer
    zero(packetBuffer, UDP_TX_PACKET_MAX_SIZE);
    Udp.read(packetBuffer, UDP_TX_PACKET_MAX_SIZE);
    Serial.println("Contents:");
    Serial.println(packetBuffer);

    // handle the request
    handle(packetBuffer, length(packetBuffer, UDP_TX_PACKET_MAX_SIZE));
  }
}

void handle(char msg[], int len) {
  Serial.println("oops: in handle()");
  // extract the target pin number
  int pinNum = extractPinNumber(msg, len);
  Serial.print("target pin number is: ");
  Serial.println(pinNum);
  
  // temperature query service
  if (pinNum == ONE_WIRE_BUS) {
    Udp.beginPacket(Udp.remoteIP(), Udp.remotePort());
    fillWithTempC(ReplyBuffer);
    Udp.write(ReplyBuffer);
    Udp.endPacket();
    return;
  }
  
  // light control service
  if (msg[len-1] == '0') {
    Serial.println("off");
    digitalWrite(pinNum, LOW);
  } else if (msg[len-1] == '1') {
    Serial.println("on");
    digitalWrite(pinNum, HIGH);
  } else {
    // no operations
  }
  
  // illegal target pin number, no operations
}

int length(char array[], int size) {
  int i = 0;
  for (; i < size; ++i) {
    if (array[i] == '\0') {
      break;
    }
  }
  return i;  
}

int zero(char array[], int size) {
  for (int i = 0; i < size; ++i) {
    array[i] = '\0';
  }
}

void fillWithTempC(char dtostrfbuffer[]) { 
  // call sensors.requestTemperatures() to issue a global temperature 
  // request to all devices on the bus
  Serial.print("Requesting temperatures...");
  sensors.requestTemperatures(); // Send the command to get temperatures
  Serial.println("DONE");
  //
  Serial.print("Temperature for the device 1 (index 0) is: ");
  float tempC = sensors.getTempCByIndex(0);
  Serial.println(tempC); 
  //
  dtostrf(tempC, 8, 2, dtostrfbuffer);
  Serial.print("dtostrf: ");
  Serial.println(dtostrfbuffer);
}

int extractPinNumber(char msg[], int len) {
  int pinNum = -1;
  // calculate the end index of the pin number
  int endIndex = -1;
  for (int i = 0; i < len; ++i) {
    if (msg[i] == ':') {
      endIndex = i - 1;
      break;
    }
  }
  if (endIndex == -1) {
    endIndex = len - 1;
  }
  // get the pin number
  pinNum = msg[0] - '0';
  pinNum = pinNum % 10;
  for (int i = 1; i <= endIndex; ++i) {
    int j = msg[i] - '0';
    j = j % 10;
    pinNum = pinNum * 10 + j;
  }
  return pinNum;
}

