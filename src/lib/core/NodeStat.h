#if !defined(__MY_SENSOR_STATISTIC_H)
#define __MY_SENSOR_STATISTIC_H 1

#include <Arduino.h>
#include <MySensors.h>

#if defined(MY_GATEWAY_ESP8266)
#  include <ESP.h>
#  include <ESP8266WiFi.h>
#endif

class NodeStat {
#   define POLL_TIMEOUT 1000U

    private:
        int8_t temp = 0;
        uint8_t volt = 0U;
        int16_t rssi = 0;
        uint16_t illum = 0U;
        uint16_t cntj = POLL_TIMEOUT;
        NodeLight const *light;

        bool chipTemperature() {
#           if defined(ENABLE_LIVE_SENSOR_TEMP)
            int8_t t_ = hwCPUTemperature();
            if (t_ == -127)
              return false;
            return ((temp != t_) ? __extension__ ({temp = t_; true;}) : false);
#           else
            return false;
#           endif
        }
        void radioQuality() {
#           if defined(ENABLE_LIVE_SENSOR_RSSI)
#             if defined(MY_GATEWAY_ESP8266)
                int16_t r_ = static_cast<int16_t>(WiFi.RSSI());
#             else
                int16_t r_ = transportGetSendingRSSI();
#             endif
            
            r_ = map(r_, -85, -40, 0, 100);
            r_ = ((r_ < 0) ? 0 : ((r_ > 100) ? 100 : r_));
            if (rssi != r_)
              rssi = r_;
#           endif
        }
        void chipVoltage() {
#           if defined(ENABLE_LIVE_SENSOR_VOLT)
#             if defined(MY_GATEWAY_ESP8266)
                int16_t v_ = ESP.getVcc();
#             else
                int16_t v_ = analogRead(INTERNAL_LIVE_VOLT_PIN) / 10;
#             endif

            v_ = ((v_ < 0) ? 0 : ((v_ > 100) ? 100 : v_));
            if (volt != v_)
              volt = static_cast<uint8_t>(v_);
#           endif
        }
        bool sensorIlluminate() {
          (void) light->read();
          illum = light->getVal();
          //illum = analogRead(INTERNAL_LIVE_ILLUMINATION_PIN);
          return true; // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

#         if defined(ENABLE_LIVE_SENSOR_ILLUMINATION)
          uint16_t i_ = ((2500 / (
            (analogRead(INTERNAL_LIVE_ILLUMINATION_PIN) * 0.0048828125)
            - 500)) / 10);
          
          if (static_cast<int16_t>(i_) < 0)
            return false;
          if (i_ == illum)
            return false;
          illum = i_;
          return true;

#         else
          return false;
#         endif
        }

    public:
        NodeStat() {
        }
        NodeStat(NodeLight const *l) {
          light = l;
        }
        void init(uint16_t) {}
        void init() {
#           if defined(ENABLE_LIVE_SENSOR_VOLT)
#             if defined(MY_GATEWAY_ESP8266)
                ADC_MODE(ADC_VCC);
#             else
                analogReference(INTERNAL);
#             endif
#           endif
        }
        bool presentation() {
        }
        void data(uint16_t & cnt) {
#           if (!defined(ENABLE_LIVE_SENSOR_TEMP) && (!defined(ENABLE_LIVE_SENSOR_ILLUMINATION)))
            return;
#           else

              if ((++cntj < POLL_TIMEOUT) || (!isTransportReady()))
                return;
              cntj = 0U;

#             if defined(ENABLE_LIVE_SENSOR_TEMP)
              if (chipTemperature())
                reportMsg(INTERNAL_LIVE_TEMP, V_TEMP, temp);
#             endif
#           endif
        }
        bool data(const MyMessage & msg) {

          /*
          PRINTF("-- STAT PRE: msg.sensor: %u, type: %u\n", (uint16_t) msg.sensor, (uint16_t) msg.type);
          */

          switch (msg.getType()) {
            case V_VAR1:
            case V_VAR5:
            case I_BATTERY_LEVEL:
            case I_SIGNAL_REPORT_REQUEST: {
              if ((msg.sensor < static_cast<uint8_t>(INTERNAL_LIVE_ILLUMINATION)) ||
                  (msg.sensor > static_cast<uint8_t>(INTERNAL_LIVE_RSSI)))
                return false;
              break;
            }
            default:
              return false;
          }

          /*
          PRINTF("-- STAT msg.sensor: %u, type: %u\n", (uint16_t) msg.sensor, (uint16_t) msg.type);
          */

          switch (msg.getType()) {
            case I_BATTERY_LEVEL: {
              //PRINTLN("-- STAT request I_BATTERY_LEVEL\n");
              chipVoltage();
              sendBatteryLevel(volt, false);
              break;
            }
            case I_SIGNAL_REPORT_REQUEST: {
              //PRINTLN("-- STAT request I_SIGNAL_REPORT_REQUEST\n");
              radioQuality();
              _sendRoute(build(_msgTmp, GATEWAY_ADDRESS, msg.sensor, C_SET, I_SIGNAL_REPORT_RESPONSE, false).set(rssi));
              break;
            }
            case V_VAR1: {
              //PRINTLN("-- STAT request V_VAR1\n");
              radioQuality();
              _sendRoute(build(_msgTmp, GATEWAY_ADDRESS, 7, C_SET, V_TEXT, false).set("xyi"));
              break;
            }
            case V_VAR5: {
              //PRINTLN("-- STAT request V_VAR5\n");
              radioQuality();
              MY_CRITICAL_SECTION {
                char *buff = new char[18]{};
                (void) snprintf(buff, 17, "rssi: %d", rssi);
                _sendRoute(build(_msgTmp, GATEWAY_ADDRESS, 7, C_SET, V_VAR5, false).set(buff));
                delete [] buff;
              }
              break;
            }
          }
          return true;
        }
};

#endif


//#define MY_DEBUG
#define MY_RADIO_NRF24
#define MY_NODE_ID 3
#define MY_RF24_PA_LEVEL RF24_PA_MAX

#include <MySensors.h>
#include <DallasTemperature.h>
#include <OneWire.h>

typedef struct BatteryStatus {
  float voltage;
  int level;
} BatteryStatus;

typedef struct Temperatures {
  float top;
  float bottom;
} Temperatures;

const byte TOP_SENSOR_ID = 1;
const byte BOTTOM_SENSOR_ID = 2;
const byte BATTERY_VOLTAGE_SENSOR_ID = 3;
const uint32_t SLEEP_TIME = 600000UL;
const bool DEBUG = false;
const bool DO_SEARCH_DEVICES = false;
const float VBAT_FULL = 2.8;
const float VBAT_EMPTY = 2.0;

BatteryStatus batteryStatus;
BatteryStatus currentBatteryStatus;
Temperatures temperatures;
Temperatures currentTemperatures;

// define oneWire communication  
OneWire oneWire(3);
DallasTemperature sensors(&oneWire); 
DeviceAddress topThermometer = {0x28,0xFF,0xF8,0x5,0x1A,0x4,0x0,0x4A};
DeviceAddress bottomThermometer = {0x28,0xFF,0x71,0xF,0x12,0x4,0x0,0x4C};

// define mysensors messages
MyMessage tempTopMsg(TOP_SENSOR_ID, V_TEMP);
MyMessage tempBottomMsg(BOTTOM_SENSOR_ID, V_TEMP);
MyMessage vbatMsg(BATTERY_VOLTAGE_SENSOR_ID, V_VOLTAGE);

void getBatteryStatus(BatteryStatus *batteryStatus) {
  int val = analogRead(A0);
  float voltage  = val * 0.00325458871;
  int level;
  if (voltage > VBAT_FULL) {
    level = 100;
  } else if (voltage < VBAT_EMPTY) {
    level = 0;
  } else {
    level = (int) ((voltage - VBAT_EMPTY) * 100.0 / (VBAT_FULL - VBAT_EMPTY));
  }
  if (DEBUG) {
    Serial.print("getBatteryStatus: ");
    Serial.print("val=");
    Serial.print(val);
    Serial.print(", level=");
    Serial.print(level);
    Serial.print("%, ");
    Serial.print("voltage=");
    Serial.print(voltage);
    Serial.print("V");
    Serial.println();
  }
  batteryStatus->voltage = voltage;
  batteryStatus->level = level;
}

void sendBatteryStatus(BatteryStatus *batteryStatus) {
  sendBatteryLevel(batteryStatus->level);
  vbatMsg.set(batteryStatus->voltage, 2);
  send(vbatMsg);
}

bool isTemperatureValid(float temp) {
  return temp != -127.00 && temp != 85.00;
}

bool getTemperatures(Temperatures *temperatures) {
  sensors.requestTemperatures();
  int16_t conversionTime = sensors.millisToWaitForConversion(sensors.getResolution());
  sleep(conversionTime);

  temperatures->top = sensors.getTempC(topThermometer);
  temperatures->bottom = sensors.getTempC(bottomThermometer);

  if (DEBUG) {
    Serial.print("getTemperatures: ");
    Serial.print("top=");
    Serial.print(temperatures->top);
    Serial.print(", bottom=");
    Serial.print(temperatures->bottom);
    Serial.println();
  }

  return isTemperatureValid(temperatures->top) && isTemperatureValid(temperatures->bottom);
}

void sendTemperatures(Temperatures *temperatures) {
  tempTopMsg.set(temperatures->top, 1);
  tempBottomMsg.set(temperatures->bottom, 1);
  send(tempTopMsg);
  send(tempBottomMsg);
}

void searchDevices() {
  Serial.print("Locating devices...");
  int deviceCount = sensors.getDeviceCount();
  Serial.print("found ");
  Serial.print(deviceCount, DEC);
  Serial.println(" devices.");
  for (int deviceIndex = 0; deviceIndex < deviceCount; deviceIndex++) {
    DeviceAddress addr;
    if (sensors.getAddress(addr, deviceIndex)) {
      Serial.print("Device ");
      Serial.print(deviceIndex, DEC);
      Serial.print(" = {");
      for (uint8_t i = 0; i < 8; i++)
      {
          Serial.print("0x");
          Serial.print(addr[i], HEX);
          if (i + 1 < 8) {
            Serial.print(",");
          }
      }
      Serial.print("}");
      Serial.println();
    } else {
      Serial.print("Unable to find address for Device ");
      Serial.println(deviceIndex, DEC);
    }
  }
}

void before() {
  sensors.begin();

  if (DO_SEARCH_DEVICES) {
    searchDevices();
  }
}

voi
