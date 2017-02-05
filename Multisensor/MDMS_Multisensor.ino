// Enable debug prints to serial monitor
#define MY_DEBUG 
#define DEBUG // Show debug to serrial

// Enable and select radio type attached
#define MY_RADIO_NRF24
//#define MY_RADIO_RFM69 

#define MY_NODE_ID AUTO
#define MY_PARENT_NODE_ID AUTO

//#define BMP085_DEBUG 1

#define SensBMP180 // 21% -26%
#define SensSI7021 // 18% - 24%
#define SensBH1750 // 13% - 21%
#define SensDHT // 18% - 22%
#define SensDS18B20 // 21% - 20%

#include <SPI.h>
//#include <MySensors.h>
#include <avr/wdt.h>
#include <EEPROM.h> 

#include <Wire.h>

// BMP180
#ifdef SensBMP180
  #include <Adafruit_BMP085.h>
#endif

// SI7021
#ifdef SensSI7021
  #include "i2c.h"  
  #include "i2c_SI7021.h"
#endif

// BH1750
#ifdef SensBH1750
  #include <BH1750.h>
#endif

// DHT
#ifdef SensDHT
  #include "DHT.h"
#endif

// DS18B20
#ifdef SensDS18B20
  #include <OneWire.h>
  #include <DallasTemperature.h>
#endif

#define SKETCH_NAME "Multi Sensor"
#define SKETCH_MAJOR_VER "1"
#define SKETCH_MINOR_VER "0"

unsigned long SEND_FREQUENCY = 5000; // 900000; // Minimum time between send (in milliseconds). We don't wnat to spam the gateway. 
unsigned long SEND_INITTIME = 2000;

#define POWER_PIN 6
#define INIT_PIN 18
#define LED_PIN 14

#define BMP180_ID 1
#define Si7021_ID 2
#define BH1750_ID 3
#define DHT11_ID  4

#define DS18B20_ID 10

//== BMP180
#ifdef SensBMP180
  Adafruit_BMP085 bmp;
  int32_t BMP180PressLast = 0;
  float BMP180TempLast = 0;

//MyMessage MesBMP180Pres(BMP180_ID, V_PRESSURE);
//MyMessage MesBMP180Temp(BMP180_ID, V_TEMP);
#endif

//== SI7021
#ifdef SensSI7021
  SI7021 si7021;
#endif

//== BH1750
#ifdef SensBH1750
  BH1750 lightMeter(0x23);
#endif

//== DHT
#ifdef SensDHT
  #define DHTPIN 5     // what digital pin we're connected to
  #define DHTTYPE DHT11   // DHT 11
  //#define DHTTYPE DHT22   // DHT 22  (AM2302), AM2321
  //#define DHTTYPE DHT21   // DHT 21 (AM2301)
  DHT dht(DHTPIN, DHTTYPE);
#endif
  
//== DS18B20
#ifdef SensDS18B20
  #define ONE_WIRE_BUS 4
  #define MAX_ATTACHED_DS18B20 4
  
  OneWire oneWire(ONE_WIRE_BUS);
  DallasTemperature sensors(&oneWire);

  float lastTemperature[MAX_ATTACHED_DS18B20];
  int numSensors=0;
#endif

void setup() {
  wdt_disable();

  Serial.begin(115200);
  Serial.println("Start");
/*  
  // Init adress from pin
  pinMode(INIT_PIN, OUTPUT); 
  if (analogRead(INIT_PIN) == 0){    
    Serial.println("Init adress");
    
    for (int i=0;i<512;i++) {
      EEPROM.write(i, 0xff);
    } 
  }
*/
  Serial.println("#1");
  
  // Disable the ADC by setting the ADEN bit (bit 7) to zero.
  //ADCSRA = ADCSRA & B01111111;
  
  // Disable the analog comparator by setting the ACD bit
  // (bit 7) to one.
  //ACSR = B10000000; 

  // Setup the buttons
  pinMode(POWER_PIN, OUTPUT);
//  pinMode(INIT_PIN, INPUT); 
  pinMode(LED_PIN, OUTPUT);

  Serial.println("#2");

  // Power to sensors
  digitalWrite(POWER_PIN, LOW);

  Serial.println("#3");

  //=== BMP
  #ifdef SensBMP180
    if (!bmp.begin()){
      Serial.println("Could not find a valid BMP085 sensor, check wiring!");
    }  
  #endif

  //=== si7021
  #ifdef SensSI7021
    if (!si7021.initialize()){
      Serial.println("Sensor missing");
    }
    
  #endif

  //=== BH1750
  #ifdef SensBH1750
    lightMeter.begin(BH1750_CONTINUOUS_HIGH_RES_MODE);
  #endif
  
  //=== DHT  
  #ifdef SensDHT
    dht.begin();
  #endif

  //=== DS18B20
  #ifdef SensDS18B20
    sensors.begin();
    //sensors.setWaitForConversion(false);
    numSensors = sensors.getDeviceCount();
  #endif

  Serial.println("#End start");

  wdt_enable(WDTO_8S); 
}
/*
void presentation() {
  // Send the sketch version information to the gateway and Controller
  sendSketchInfo(SKETCH_NAME, SKETCH_MAJOR_VER"."SKETCH_MINOR_VER);

  // Register sensos
  present(BMP180_ID, S_BARO, "BMP180 sensor");  
  present(Si7021_ID, S_HUM, "Si7021 sensor");
  present(BH1750_ID, S_LIGHT, "BH1750 sensor");
  present(DHT11_ID, S_HUM, "BH1750 sensor");
  present(DS18B20_ID, S_TEMP, "DS18B20 sensor");
}
*/

void loop() {  
  wdt_reset();
/*
  // Power on
  //digitalWrite(POWER_PIN, LOW);
  //sleep(SEND_INITTIME); 
  //wdt_reset();
*/

  // === BMP180
  #ifdef SensBMP180
    // Press
    int32_t BMP180Press = bmp.readPressure();
  //  if (BMP180Press != BMP180PressLast) {
  //    send(MesBMP180Pres.set(BMP180Press));
      #ifdef DEBUG
         Serial.print("[BMP180] Press:");
         Serial.println(BMP180Press);
      #endif
      BMP180PressLast = BMP180Press;
  //  }
    // Temp
    float BMP180Temp = bmp.readTemperature();
  //  if (BMP180Temp != BMP180TempLast) {
  //    send(MesBMP180Temp.set(BMP180Temp, 2));
      #ifdef DEBUG
         Serial.print("[BMP180] Temp:");
         Serial.println(BMP180Temp);
      #endif
      BMP180TempLast = BMP180Temp;
  //  } 
   #endif

  #ifdef SensSI7021
    static float humi, temp;

    si7021.getHumidity(humi);
    si7021.getTemperature(temp);
    si7021.triggerMeasurement();

    Serial.print("[Si7021] TEMP: ");
    Serial.println(temp);
    Serial.print("[Si7021] HUMI: ");
    Serial.print(humi);
    Serial.println("");
 
  #endif

  //=== BH1750
  #ifdef SensBH1750
    uint16_t lux = lightMeter.readLightLevel();
    Serial.print("[BH1750] Light: ");
    Serial.print(lux);
    Serial.println(" lx");

 #endif
  
  //=== Dht
  #ifdef SensDHT
    
    // Read Hum
    float h = dht.readHumidity();
    if (isnan(h)) {
      Serial.println("Failed to read DHT hum!");
    } else {
      Serial.print("[DHT] Humidity: ");
      Serial.print(h);
      Serial.println(" %\t");
    }
   
    // Read temperature 
    float t = dht.readTemperature();
    if (isnan(t)) {
      Serial.println("Failed to read DHT temp!");
    } else {
      Serial.print("[DHT] Temperature: ");
      Serial.print(t);
      Serial.println(" *C ");  
    }
  #endif

  //=== DS18B20
  #ifdef SensDS18B20
     // Fetch temperatures from Dallas sensors
     sensors.requestTemperatures();
    
     // query conversion time and sleep until conversion completed
     //int16_t conversionTime = sensors.millisToWaitForConversion(sensors.getResolution());
     // sleep() call can be replaced by wait() call if node need to process incoming messages (or if node is repeater)
     //@@@ sleep(conversionTime);
     //delay(conversionTime);     
    
     // Read temperatures and send them to controller 
     for (int i=0; i<numSensors && i<MAX_ATTACHED_DS18B20; i++) {
       // Fetch and round temperature to one decimal
       float temperature = (sensors.getTempCByIndex(i) * 10.) / 10.;
       Serial.print("[DS18B20] Temperature D");
       Serial.print(i);
       Serial.print(":");
       Serial.print(temperature);
       Serial.println(" *C ");
       
     }
  #endif

         Serial.println(" ******************************* ");
      delay(1000);     
/*
  // Power off
  //digitalWrite(POWER_PIN, HIGH);
  //sleep(SEND_FREQUENCY); 
  */  
}
