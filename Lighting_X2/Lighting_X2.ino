 
#define MY_PARENT_NODE_ID AUTO
#define MY_NODE_ID 9
//#define MY_REPEATER_FEATURE // Enable repeater functionality for this node

// Flash options
#define MY_OTA_FIRMWARE_FEATURE
#define MY_OTA_FLASH_SS 8
#define MY_OTA_FLASH_JDECID 0x2020

#define MY_SIGNING_ATSHA204
#define MY_SIGNING_ATSHA204_PIN 16 // A2


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

void setup() { 
  uint8_t val;
  
  wdt_disable();

  Serial.begin(115200);

  //== Relay_1
  Relay_1_State = loadState(RELAY_1_ID)?RELAY_ON:RELAY_OFF;
  pinMode(RELAY_1_PIN, Relay_1_State);   

  //== Relay_2
  Relay_2_State = loadState(RELAY_2_ID)?RELAY_ON:RELAY_OFF;
  pinMode(RELAY_2_PIN, Relay_2_State);  
  
  pinMode(LED_PIN, OUTPUT);   
  pinMode(BUTTON_PIN, INPUT);
  digitalWrite(BUTTON_PIN, HIGH);   
  wdt_enable(WDTO_8S);    
}

void presentation()  
{   
  // Node info
  sendSketchInfo("Lighting X2", "3.0");
  // Relay_1
  present(RELAY_1_ID, S_BINARY, "Light X2 CH1");
  // Relay_1
  present(RELAY_2_ID, S_BINARY, "Light X2 CH2");
   
}

void loop() 
{  
  float fval;
  wdt_reset();
  if (digitalRead(BUTTON_PIN) == 0) digitalWrite(LED_PIN, LOW);  
   else digitalWrite(LED_PIN, HIGH);  
}


bool SendData(uint8_t Dest, uint8_t Sensor, uint8_t Type, const char* Value, bool Ack) {
  MyMessage answerMsg;
  mSetCommand(answerMsg, C_SET);
  answerMsg.setSensor(Sensor);
  answerMsg.setDestination(Dest);
  answerMsg.setType(Type);
  return send(answerMsg.set(Value), Ack); 
}

bool SendData(uint8_t Dest, uint8_t Sensor, uint8_t Type, int Value, bool Ack){
  MyMessage answerMsg;
  mSetCommand(answerMsg, C_SET);
  answerMsg.setSensor(Sensor);
  answerMsg.setDestination(Dest);
  answerMsg.setType(Type);
  if (Value == INT_MIN)
    return send(answerMsg.set(""), Ack);
  else
    return send(answerMsg.set(Value), Ack);
}

bool SendData(uint8_t Dest, uint8_t Sensor, uint8_t Type, float Value, uint8_t Decimals, bool Ack){
  MyMessage answerMsg;
  mSetCommand(answerMsg, C_SET);
  answerMsg.setSensor(Sensor); 
  answerMsg.setDestination(Dest);
  answerMsg.setType(Type);
  if (Value == NAN)
    return send(answerMsg.set(""), Ack);
  else
    return send(answerMsg.set(Value, Decimals), Ack);
}



void receive(const MyMessage &message) {
  uint8_t Dest = message.sender;
  // Request
  if (mGetCommand(message) == C_REQ) {
     switch (message.sensor) {
       case RELAY_1_ID:
           SendData(Dest, RELAY_1_ID, V_STATUS, Relay_1_State, false);
           break;
           
       case RELAY_2_ID:
           SendData(Dest, RELAY_2_ID, V_STATUS, Relay_2_State, false);
           break;
    }
    return ;
  }
  
  // Relay_1 state
  if ((message.type==V_STATUS) && (message.sensor == RELAY_1_ID)) {
     // Change relay state     
     Relay_1_State = message.getBool()?RELAY_ON:RELAY_OFF;
     pinMode(RELAY_1_PIN, Relay_1_State);   
     // Store state in eeprom
     saveState(RELAY_1_ID, message.getBool());
  } 
  
  // Relay_2 state
  if ((message.type==V_STATUS) && (message.sensor == RELAY_2_ID)) {
     // Change relay state     
     Relay_2_State = message.getBool()?RELAY_ON:RELAY_OFF;
     pinMode(RELAY_2_PIN, Relay_2_State);   
     // Store state in eeprom
     saveState(RELAY_2_ID, message.getBool());
  } 

   // TODO Send presentation to button nodes
   // TODO Command button key
}


