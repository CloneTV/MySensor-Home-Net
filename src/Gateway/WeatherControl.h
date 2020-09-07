
#define USEIIC 0

#include <Arduino.h>
#include <Adafruit_BMP280.h>
#include <MySensors.h>
#include "Gateway.h"

const PROGMEM char *str_forecast[] = {
    "stable",
    "sunny",
    "cloudy",
    "unstable",
    "thunderstorm",
    "unknown"
};

struct WeatherControl {
    private:
        uint16_t cntm = 600U, cntf = 4U;
        float a = 0, t = 0, p = 0, p_raw = 0;
        float *p_values = nullptr;
        enum FORECAST {
            STABLE = 0,     
            SUNNY = 1,
            CLOUDY = 2,
            UNSTABLE = 3,
            THUNDERSTORM = 4,
            UNKNOWN = 5
        };
        Adafruit_BMP280 *bmp;
        //
        void sendData(float f, int16_t flag, const uint8_t sensorId, const mysensors_data_t dataType) {
            MyMessage msg(sensorId, dataType);
            send(msg.set(f, flag));
            msg.clear();
        }
        void sendData(const char *s, const uint8_t sensorId, const mysensors_data_t dataType) {
            if (s == nullptr)
              return;
            MyMessage msg(sensorId, dataType);
            send(msg.set(s));
            msg.clear();
        }
        uint8_t forecast() {
          // approximately every 5 minutes
          if (++cntf < 4U)
            return 0xff;

          cntf = 0U;
          const float alt_ = BMP_ALTITUDE_INT,
                      shift1_ = 288.15,
                      shift2_ = 0.0065,
                      shift3_ = 0.1903,
                      shift4_ = 5.255;

          for (uint16_t i = 11; i > 0U; i--)
            p_values[i] = p_values[(i - 1)];

          float altm_ = ((shift1_ / shift2_) * (1 - pow((p_raw / alt_), shift3_)));
          p_values[0] = alt_ * pow((1 - ((shift2_ * (altm_ - a)) / shift1_)), shift4_);

          if ((p_values[0] - p_values[1]) < -30)
            return FORECAST::THUNDERSTORM;
          
          float pd0_ = p_values[0] - p_values[11];
          if (pd0_ > 100)
            return FORECAST::STABLE;
          else if ((pd0_ <= 100) && (pd0_ >= 20))
            return FORECAST::CLOUDY;
          else if (pd0_ < -200)
            return FORECAST::THUNDERSTORM;
          else if ((pd0_ >= -200) && (pd0_ <= -20))
            return FORECAST::UNSTABLE;
          else if ((pd0_ < 20) && (pd0_ > -20)) {
            if (t > 15U)
              return FORECAST::SUNNY;
            else
              return FORECAST::CLOUDY;
          }
          return FORECAST::UNKNOWN;
        }

    public:
        ~WeatherControl() {
            delete bmp;
            delete [] p_values;
        }
        WeatherControl() {
            bmp = new Adafruit_BMP280();
            p_values = new float[12]{};
        }
        bool init(uint16_t addr) {
            bmp->setSampling(
                Adafruit_BMP280::MODE_NORMAL,
                Adafruit_BMP280::SAMPLING_X16,
                Adafruit_BMP280::SAMPLING_X16,
                Adafruit_BMP280::FILTER_OFF,
                Adafruit_BMP280::STANDBY_MS_4000);
            return bmp->begin(addr);
        }
        void data() {
            // approximately every 1 minutes
            if (++cntm < 600U)
                return;

            cntm = 0U;
            p_raw = ((bmp->readPressure() + bmp->readPressure()) / 2);
            float t_ = bmp->readTemperature(),
                  p_ = (p_raw / 100.0F / 1.333);
                  
            if (t != t_) {
                t = t_;
                sendData(t, 1, NODE_SENSOR_WETHER_TEMP, V_TEMP);
            }
            if (p != p_) {
                p = p_;
                sendData(p, 0, NODE_SENSOR_WETHER_BARO, V_PRESSURE);
                ///
                const float alt_ = BMP_ALTITUDE_DEFAULT;
                a = ((bmp->readAltitude(alt_) + bmp->readAltitude(alt_)) / 2);
                uint8_t r_ = forecast();
                if (r_ != 0xff)
                  sendData(str_forecast[r_], NODE_SENSOR_WETHER_BARO, V_FORECAST);
            }
        }
};
