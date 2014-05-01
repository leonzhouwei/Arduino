/*
  UDPSendReceive.pde:
 This sketch receives UDP message strings, prints them to the serial port
 and sends an "acknowledge" string back to the sender
 
 A Processing sketch is included at the end of file that can be used to send 
 and received messages for testing with a computer.
 
 created 21 Aug 2010
 by Michael Margolis
 
 This code is in the public domain.
 */


#include <SPI.h>         // needed for Arduino versions later than 0018
#include <Ethernet.h>
#include <EthernetUdp.h>         // UDP library from: bjoern@cs.stanford.edu 12/30/2008


// Enter a MAC address and IP address for your controller below.
// The IP address will be dependent on your local network:
byte mac[] = {  
  0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
IPAddress ip(192, 168, 1, 177);

unsigned int localPort = 8888;      // local port to listen on

// buffers for receiving and sending data
char packetBuffer[UDP_TX_PACKET_MAX_SIZE]; //buffer to hold incoming packet,
char  ReplyBuffer[] = "acknowledged";       // a string to send back

// An EthernetUDP instance to let us send and receive packets over UDP
EthernetUDP Udp;

void setup() {
  // set the output pins
  pinMode(3, OUTPUT);
  // start the Ethernet and UDP:
  Ethernet.begin(mac,ip);
  Udp.begin(localPort);
  // 
  Serial.begin(9600);
}

void loop() {
  // if there's data available, read a packet
  int packetSize = Udp.parsePacket();
  if(packetSize)
  {
    Serial.print("Received packet of size ");
    Serial.println(packetSize);
    Serial.print("From ");
    IPAddress remote = Udp.remoteIP();
    for (int i =0; i < 4; i++)
    {
      Serial.print(remote[i], DEC);
      if (i < 3)
      {
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

    // LED control
    controlLight(packetBuffer, length(packetBuffer, UDP_TX_PACKET_MAX_SIZE));
  }
}

void controlLight(char msg[], int len) {
  Serial.println("oops");
  Serial.println(len);
  int index = -1;
  for (int i = 0; i < len; ++i) {
    if (msg[i] == ':') {
      index = i;
      break;
    }
  }
  Serial.println(index);
  if (index > 0 && (len - 1) > index) {
    int number = msg[0] - '0';
    number = number % 10;
    for (int i = 1; i < index; ++i) {
      int j = msg[i] - '0';
      j = j % 10;
      number = number * 10 + j;
    }
    Serial.println(number);
    if (number > 1 && number < 10) {
      if (msg[len-1] == '0') {
        Serial.println("off");
        digitalWrite(number, LOW);
      } else if (msg[len-1] == '1') {
        Serial.println("on");
        digitalWrite(number, HIGH);
      } else {
        // no operations
      }
    }
  }
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


/*
  Processing sketch to run with this example
 =====================================================
 
 // Processing UDP example to send and receive string data from Arduino 
 // press any key to send the "Hello Arduino" message
 
 
 import hypermedia.net.*;
 
 UDP udp;  // define the UDP object
 
 
 void setup() {
 udp = new UDP( this, 6000 );  // create a new datagram connection on port 6000
 //udp.log( true ); 		// <-- printout the connection activity
 udp.listen( true );           // and wait for incoming message  
 }
 
 void draw()
 {
 }
 
 void keyPressed() {
 String ip       = "192.168.1.177";	// the remote IP address
 int port        = 8888;		// the destination port
 
 udp.send("Hello World", ip, port );   // the message to send
 
 }
 
 void receive( byte[] data ) { 			// <-- default handler
 //void receive( byte[] data, String ip, int port ) {	// <-- extended handler
 
 for(int i=0; i < data.length; i++) 
 print(char(data[i]));  
 println();   
 }
 */


