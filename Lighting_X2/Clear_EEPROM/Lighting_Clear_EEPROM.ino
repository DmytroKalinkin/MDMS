 
#define MY_PARENT_NODE_ID AUTO
#define MY_NODE_ID 9
//#define MY_REPEATER_FEATURE // Enable repeater functionality for this node

// Flash options
#define MY_OTA_FIRMWARE_FEATURE
#define MY_OTA_FLASH_SS 8
#define MY_OTA_FLASH_JDECID 0x2020

#define MY_SIGNING_ATSHA204
#define MY_SIGNING_ATSHA204_PIN 16 // A2

//opcodes
#define WREN  6
#define WRDI  4
#define RDSR  5
#define WRSR  1
#define READ  3
#define WRITE 2

// Enable debug prints to serial monitor
#define MY_DEBUG 
//#define DEBUG

// Enable and select radio type attached
#define MY_RADIO_NRF24
//#define MY_RADIO_RFM69

#include <SPI.h>
#include <MySensors.h>

#define SPIFLASH_BLOCKERASE_32K   0xD8

#define RELAY_1_PIN              14    // Arduino Digital I/O pin number for first relay
#define RELAY_2_PIN              15    // Arduino Digital I/O pin number for first relay

#define LED_PIN                  16    // Arduino Digital I/O pin number for first relay
#define BUTTON_PIN                4    // Arduino Digital I/O pin number for first relay

#define RELAY_ON                 1    // GPIO value to write to turn on attached relay
#define RELAY_OFF                0    // GPIO value to write to turn off attached relay

#define RELAY_1_ID               1    // State in eprom & MySensor node sensor
#define RELAY_2_ID               2    // State in eprom & MySensor node sensor

uint8_t       Relay_1_State, Relay_2_State;

#define INT_MIN -32767
#define INT_MAX 32767

int adr=0; 


void setup() { 
   
  Serial.begin(115200);

  pinMode(LED_PIN, OUTPUT);   
  pinMode(BUTTON_PIN, INPUT);
  pinMode(MY_OTA_FLASH_SS, OUTPUT);   
  digitalWrite(BUTTON_PIN, HIGH);   

  delay(100); 
  digitalWrite(MY_OTA_FLASH_SS,LOW);
  SPI.transfer(0x06); //
  digitalWrite(MY_OTA_FLASH_SS,HIGH); //release chip, signal end transfer
  
  delay(100); 
  digitalWrite(MY_OTA_FLASH_SS,LOW);
  SPI.transfer(0xC7); //
  digitalWrite(MY_OTA_FLASH_SS,HIGH); //release chip, signal end transfer
  delay(5000); 
}

byte read_eeprom(int EEPROM_address)
{
  //READ EEPROM
  int data;
  digitalWrite(MY_OTA_FLASH_SS,LOW);
  SPI.transfer(READ); //transmit read opcode
  SPI.transfer((char)(0));   //send MSByte address first
  SPI.transfer((char)(EEPROM_address>>8));   //send MSByte address first
  SPI.transfer((char)(EEPROM_address));      //send LSByte address
  data = SPI.transfer(0xFF); //get data byte
  digitalWrite(MY_OTA_FLASH_SS,HIGH); //release chip, signal end transfer
  return data;
}


void loop() 
{  
  Serial.print(read_eeprom(adr), HEX);
  Serial.print("----");
  Serial.print(adr, DEC);  
  Serial.print('\n');
  adr++;
  delay(200);
}


