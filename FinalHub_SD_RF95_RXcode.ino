// David Durkin
// Adafruit RH95HCW RX code for the ethanol vapor sensor HUB
// Started March 14th, 2019
// Version 1.1 (March 16th, 2019)
//

#include <SPI.h>
#include <SD.h>
#include <RH_RF95.h>
#include <RHReliableDatagram.h>

/************ Radio Setup ***************/

// Frequency, must match TX's freq!
#define RF95_FREQ 915.0

#if defined (__AVR_ATmega2560__)  // Arduino Mega Board
#define RFM95_CS      43  //
#define RFM95_RST     4  // "A"
#define RFM95_INT     2  // 
#endif

File myFile; //for the SD card Reader
int pinCS = 53 ; //Chip select allows us to write and read from two modules at the same time

// Singleton instance of the radio driver
RH_RF95 rf95(RFM95_CS, RFM95_INT);

int16_t packetnum = 0;  // packet counter, we increment per xmission

void setup()
{
  Serial.begin(115200);
  //while (!Serial) { delay(1); } // wait until serial console is open, remove if not tethered to computer


  pinMode(RFM95_RST, OUTPUT);
  digitalWrite(pinCS, HIGH); //when pins are high you cannot write to them;
  digitalWrite(RFM95_RST, HIGH);
  
  delay(500);
  digitalWrite(pinCS,LOW);
  if (SD.begin())
  {
    Serial.println("SD card is ready to use.");
  } else
  {
    Serial.println("SD card initialization failed");
    return;
  }
  delay(500);
  digitalWrite(pinCS, HIGH);


  Serial.println("HUB powered on and...");
  Serial.println();

  // manual reset
  digitalWrite(RFM95_RST, LOW);
  delay(10);
  digitalWrite(RFM95_RST, HIGH);
  delay(10);

  if (!rf95.init()) {
    Serial.println("RFM95 radio init failed");
    while (1);
  }
  Serial.println("RFM95 initialized and ready to go");
  // Defaults after init are 434.0MHz, modulation GFSK_Rb250Fd250, +13dbM (for low power module)
  // No encryption
  if (!rf95.setFrequency(RF95_FREQ)) {
    Serial.println("setFrequency failed");
  }

  // If you are using a high power RF95 eg RFM95HW, you *must* set a Tx power with the
  // ishighpowermodule flag set like this:
  rf95.setTxPower(23, false);  // range from 14-20 for power, 2nd arg must be true for 95HCW

  Serial.print("RFM95 radio @");  Serial.print((int)RF95_FREQ);  Serial.println(" MHz");
}


// Dont put this on the stack:
uint8_t data[] = "Acknowledge";
// Dont put this on the stack:
uint8_t buf[RH_RF95_MAX_MESSAGE_LEN];

void loop() {
  if (rf95.available())
  {
    // Wait for a message addressed to us from the client
    uint8_t buf[RH_RF95_MAX_MESSAGE_LEN];
    uint8_t len = sizeof(buf);

    if (rf95.recv(buf, &len)) {
      buf[len] = 0; // zero out remaining string

      Serial.print("Got packet from node");
      Serial.print(" [RSSI :");
      Serial.print(rf95.lastRssi());
      Serial.print("] : ");
      Serial.println((char*)buf);

      // write to sd card by setting 53 high and setting CS for sd card low to write to, execute write function
      // then set 53 low again and CS for SD high. All mosi and miso and SCK are the sam
      uint8_t data[] = "Acknowledge from HUB";
      rf95.send(data, sizeof(data));
      rf95.waitPacketSent();
      Serial.println("Sent a reply");

      
      digitalWrite(RFM95_CS, HIGH);
      digitalWrite(pinCS, LOW);

      myFile = SD.open("test.txt", FILE_WRITE);
      
      if (myFile) {
        Serial.println("Writing to file...");
        // Write to file
        myFile.println((char*)buf);
        myFile.close(); // close the file
        Serial.println("Done.");
      }
      // if the file didn't open, print an error:
      else {
        Serial.println("error opening test.txt");
      }
      delay(1000);
      
      digitalWrite(RFM95_CS, LOW);
      digitalWrite(pinCS, HIGH);

    }
    // Send a reply back to the originator client
    else {
      Serial.println("Sending failed (no ack)");
    }
  }
}
