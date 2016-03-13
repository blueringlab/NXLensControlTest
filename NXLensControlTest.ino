
// Name  : Usage                        Lens Pin 1 - 8 ( from left)       Arduino
// CLK   : SPI Clock                    (Pin 1)                           D13
// DLC   : SPI Data Camera Input        (Pin 2) Lens --> Camera (MISO)    D12
// DCL   : SPI Data Camera Output       (Pin 3) Lens <-- Camera (MOSI)    D11
// ATT   : Attention from Camera        (Pin 4) Lens <-- Camera           D9
// LDR   : Lens Data Ready              (Pin 5)                           D8
// GND   : Ground                       (Pin 6)                           GND
// VDD   : Logic Power (3.3V)           (Pin 7)                           3V3
// VCC   : Focusing Motor Power (5V)    (Pin 8)                           5V
#include <SPI.h>

const int ldrPin = 8;
const int attPin = 9;
const int dclPin = 11;
const int dlcPin = 12;
const int clkPin = 13;

// buffer 255 bytes
byte dlcBuf[255];
byte dclBuf[255];

void setup() {
   Serial.begin(9600);
  
   pinMode(attPin, OUTPUT); 
   pinMode(ldrPin, INPUT); 

   // set up SPI to communicate to NX Lens
   SPI.begin();
   SPI.setBitOrder(MSBFIRST);
   SPI.setDataMode(SPI_MODE3);  
   SPI.setClockDivider(SPI_CLOCK_DIV64); //16MHz / 64 = 250 kHz
   
   Serial.println("Test Begin.");   

   delay(2);

   // echo command: 03 AA 55
   setDclBufferFromString("03AA55");
   sendCameraData();
   receiveLensData();

   delay(100);

   // Set as MF mode: 06 10 08 50 02 00
   setDclBufferFromString("061008500200");
   sendCameraData();
   receiveLensData();
}   

void loop() {
  if (Serial.available() > 0);
}

void sendCameraData() {
    // wait until LDR is HIGH. Don't send data while LDR is low.
    waitLdrStateWhile(LOW);

    debugMsg("DCL", dclBuf);
    
    for(int i=0; i < dclBuf[0]; i++) 
      sendCommand(dclBuf[i]);
}

void receiveLensData() {
    // wait until LDR is LOW. Don't create CLK when LDR becomes low.
    waitLdrStateWhile(HIGH);

    debugMsg("DCL", dclBuf);
    
    // receive one byte. It is length of Lens Response data.
    dlcBuf[0] = sendCommand(0xFF);

    // receive rest of data
    for(int i = 1; i < dlcBuf[0]; i++) 
      dlcBuf[i] = sendCommand(0xFF);

    debugMsg("DLC", dlcBuf);
}

byte sendCommand(byte Cmd) {
  byte Ack = SPI.transfer(Cmd);
  delayMicroseconds(22);
  return(Ack);
}

unsigned int debugMsg(char *note, unsigned char *buff) {
  Serial.print(note);
  Serial.print(": ");
  for(int i = 0; i < buff[0]; i++) {
    Serial.print(buff[i],HEX);
    Serial.print(" ");
  }
  Serial.println();
  return(0);
}

void setDclBufferFromString(String cmdString) {
  int index = 0;
  
  for(int i = 0; i < cmdString.length(); i+=2) {
    dclBuf[index] = hexCharToByte(cmdString.charAt(i));
    dclBuf[index] <<= 4;
    dclBuf[index] |= hexCharToByte(cmdString.charAt(i+1));
    index ++;
  }
}

byte hexCharToByte(char hex) {
    if (hex >= '0' && hex <= '9') return hex - '0';
    if (hex >= 'A' && hex <= 'F') return (hex - 'A')+10;
    if (hex >= 'a' && hex <= 'f') return (hex - 'a')+10;
    return 0;
}

void waitLdrStateWhile(int state) {
    // wait until LDR is LOW
    int ldrState = digitalRead(ldrPin);
    while(ldrState == state) {
      delay(1);
      ldrState = digitalRead(ldrPin);
    }
}

