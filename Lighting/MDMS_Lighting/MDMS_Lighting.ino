
#define MY_PARENT_NODE_ID AUTO
#define MY_NODE_ID AUTO
//#define MY_REPEATER_FEATURE // Enable repeater functionality for this node

// Flash options
#define MY_OTA_FIRMWARE_FEATURE
#define MY_OTA_FLASH_SS 7
#define MY_OTA_FLASH_JDECID 0x2020

//#define MY_SIGNING_SOFT
//#define MY_SIGNING_SOFT_RANDOMSEED_PIN 7
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

#define LIGHT_PIN                3    // Arduino Digital I/O pin number for first relay
#define LIGHT_ID                 1    // State in eprom & MySensor node sensor
#define RELAY_ON                 1    // GPIO value to write to turn on attached relay
#define RELAY_OFF                0    // GPIO value to write to turn off attached relay

#define TEMP_PIN                A3    // Termistor pin
#define TEMP_ID                  2    // Sensor temp ID
#define TEMP_Average             5    // Number of samples to average temperature 

#define CURRENT_PIN             A6    // ACS712 pin
#define CURRENT_ID               3    // Sensor current ID
#define CURRENT_VperAmp      0.185    // 0.185=5A, 0.100=20A, 0.66=30A
#define CURRENT_ADC_BUFF_SIZE   16    // Number of samples to average current
#define CURRENT_MTS              1    // Measure Time Slot for current (in mS)
#define CURRENT_MTS_AMOUNT     100    // Number MTS  

#define NODESYSTEM_ID          254

unsigned long SEND_FREQUENCY = 900000; // Minimum time between send (in milliseconds). We don't wnat to spam the gateway. 

unsigned long currentTime;
uint8_t       LightState;

#define INT_MIN -32767
#define INT_MAX 32767

float   CURRENT_VAL;
int     CURRENT_ADC_MAX;
int     CURRENT_ADC_MIN;
int     CURRENT_ADC_MAX_BUFF[CURRENT_ADC_BUFF_SIZE];
int     CURRENT_ADC_MIN_BUFF[CURRENT_ADC_BUFF_SIZE];
int     CURRENT_ADC_CNT;
int     CURRENT_ADC_BUFF_INDEX=0;

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

  //== Temp
  pinMode(TEMP_PIN, INPUT);

  //== Current
  pinMode(CURRENT_PIN, INPUT);  
  pinMode(12, INPUT);

  // Make sure that ATSHA204 is not floating
//  pinMode(ATSHA204_PIN, INPUT);
//  digitalWrite(ATSHA204_PIN, HIGH); 

  wdt_enable(WDTO_8S);    
}

void presentation()  
{   
  // Node info
  sendSketchInfo("Lighting", "1.2");
  // Light
  present(LIGHT_ID, S_LIGHT, "Light relay");
  // Temp
  present(TEMP_ID, S_TEMP, "Key temperature");
  // Current
  present(CURRENT_ID, S_MULTIMETER, "Source current");
}

void loop() 
{  
  float fval;
  wdt_reset();
  if ((millis() - currentTime) > CURRENT_MTS)
   {
    //=== Current
    int CURRENT_ADC = analogRead(CURRENT_PIN);
    if (CURRENT_ADC > CURRENT_ADC_MAX) CURRENT_ADC_MAX = CURRENT_ADC;
    if (CURRENT_ADC < CURRENT_ADC_MIN) CURRENT_ADC_MIN = CURRENT_ADC;    
    CURRENT_ADC_CNT++;
    if (CURRENT_ADC_CNT > CURRENT_MTS_AMOUNT)
     {
      CURRENT_ADC_MAX_BUFF[CURRENT_ADC_BUFF_INDEX] = CURRENT_ADC_MAX;
      CURRENT_ADC_MIN_BUFF[CURRENT_ADC_BUFF_INDEX] = CURRENT_ADC_MIN;
      CURRENT_ADC_CNT = 0;
      CURRENT_ADC_MIN = CURRENT_ADC_MAX;
      CURRENT_ADC_MAX = 0;
      CURRENT_ADC_BUFF_INDEX++;
      if (CURRENT_ADC_BUFF_INDEX>=CURRENT_ADC_BUFF_SIZE) 
       { 
        CURRENT_ADC_BUFF_INDEX =0;
        CURRENT_ADC_MAX=0;
        CURRENT_ADC_MIN=0;
        for(uint8_t i=0;i<CURRENT_ADC_BUFF_SIZE;i++) CURRENT_ADC_MAX += CURRENT_ADC_MAX_BUFF[i];  
        CURRENT_ADC_MAX = CURRENT_ADC_MAX/CURRENT_ADC_BUFF_SIZE;   
        for(uint8_t i=0;i<CURRENT_ADC_BUFF_SIZE;i++) CURRENT_ADC_MIN += CURRENT_ADC_MIN_BUFF[i]; 
        CURRENT_ADC_MIN = CURRENT_ADC_MIN/CURRENT_ADC_BUFF_SIZE;
        CURRENT_VAL = (((CURRENT_ADC_MAX - CURRENT_ADC_MIN)*0.707*0.00322)/2)/CURRENT_VperAmp;
       }
     }  
   }
}

float ReadTmp(){
  int i;
  float Temp_ADC;
  int amostra[TEMP_Average];

  for (i=0; i< TEMP_Average; i++) {
    amostra[i] = analogRead(TEMP_PIN);    
    delay(10);
  }
  Temp_ADC = 0;
  for (i=0; i< TEMP_Average; i++) {
    Temp_ADC += amostra[i];
  }
  Temp_ADC /= TEMP_Average;
  
  //Calculate temperature using the Beta Factor equation
  float temperatura;

   temperatura = -36.9*log(Temp_ADC)+271.4;
  return temperatura;
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
       case LIGHT_ID:
           SendData(Dest, LIGHT_ID, V_LIGHT, LightState, false);
           break;
           
       case TEMP_ID:
          switch (message.type) {
            case V_TEMP:
              SendData(Dest, TEMP_ID, V_TEMP, ReadTmp(), 1, false);
              break;
            default: break;  
          }
          break;
           
       case CURRENT_ID:
           switch (message.type) {

            // Raw data
            case V_CURRENT:     
             SendData(Dest, CURRENT_ID, V_CURRENT, CURRENT_VAL, 2, false);
            break;
            default: break;  
           }
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
   // TODO Send presentation to button nodes
   // TODO Command button key
}


