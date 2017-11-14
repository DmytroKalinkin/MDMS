
#define MY_PARENT_NODE_ID AUTO
#define MY_NODE_ID AUTO
#define MY_REPEATER_FEATURE // Enable repeater functionality for this node

// Flash options
#define MY_OTA_FIRMWARE_FEATURE
#define MY_OTA_FLASH_SS 7
#define MY_OTA_FLASH_JDECID 0x2020

//#define MY_SIGNING_SOFT
//#define MY_SIGNING_SOFT_RANDOMSEED_PIN 7
//#define MY_SIGNING_ATSHA204
//#define MY_SIGNING_ATSHA204_PIN 16 // A2


// Enable debug prints to serial monitor
#define MY_DEBUG 
//#define DEBUG

// Enable and select radio type attached
#define MY_RADIO_NRF24
//#define MY_RADIO_RFM69

#include <SPI.h>
#include <MySensors.h>

#define SPIFLASH_BLOCKERASE_32K   0xD8

#define LIGHT_PIN                4    // Arduino Digital I/O pin number for first relay
#define LIGHT_ID                 1    // State in eprom & MySensor node sensor
#define RELAY_ON                 1    // GPIO value to write to turn on attached relay
#define RELAY_OFF                0    // GPIO value to write to turn off attached relay

#define NODESYSTEM_ID          254

uint8_t       LightState;


void setup() { 
  uint8_t val;
  
  wdt_disable();

  Serial.begin(115200);

  //== Light
  // Then set relay pins in output mode
  pinMode(LIGHT_PIN, OUTPUT);   
  // Set relay to last known state (using eeprom storage) 
  LightState = loadState(LIGHT_ID)?RELAY_ON:RELAY_OFF;
  digitalWrite(LIGHT_PIN, LightState);

  wdt_enable(WDTO_8S);    
}

void presentation()  
{   
  // Node info
  sendSketchInfo("MDMSNode Lighting V1.1", "0.1");
  // Light
  present(LIGHT_ID, S_LIGHT, "Light relay");
}

void loop() 
{  
  wdt_reset();
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


void receive(const MyMessage &message) {
  uint8_t Dest = message.sender;
  // Request
  if (mGetCommand(message) == C_REQ) {
     switch (message.sensor) {
       case LIGHT_ID:
           SendData(Dest, LIGHT_ID, V_LIGHT, LightState, false);
           break;
    }
    return ;
  }
  
  // Light state
  if ((message.type==V_LIGHT) && (message.sensor == LIGHT_ID)) {
     // Change relay state     
     LightState = message.getBool()?RELAY_ON:RELAY_OFF;
     digitalWrite(LIGHT_PIN, LightState);
     // Store state in eeprom
     saveState(LIGHT_ID, message.getBool());
  } 

}


