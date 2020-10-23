#if !defined(__MY_SENSOR_I2C_WEATHER_H)
#define __MY_SENSOR_I2C_WEATHER_H 1

#  if defined(ENABLE_I2C_SENSOR_TEMP)
#  if defined(ENABLE_LIVE_SENSOR_TEMP)
#    pragma message "WARNING - do not include 'NodeLiveTemp.h', sensor ID intersect!"
#  endif

/* ------- WEATHER BMP180/280 I2C ------- */

#  if defined(POLL_WAIT_SECONDS)
#    undef POLL_WAIT_SECONDS
#  endif
#  define POLL_WAIT_SECONDS 90U // 30U
#  define IDX_Staus     0
#  define IDX_Change    1

static const PROGMEM char *str_forecast[] = {
    "stable",
    "sunny",
    "cloudy",
    "unstable",
    "thunderstorm",
    "unknown"
};

#define USEIIC 1 // I2C Default Interface
#include <Arduino.h>
#include <Adafruit_BMP280.h>

class NodeI2CWeather : public SensorInterface<NodeI2CWeather> {

    public:
        enum FORECAST {
            STABLE = 0,     
            SUNNY = 1,
            CLOUDY = 2,
            UNSTABLE = 3,
            THUNDERSTORM = 4,
            UNKNOWN = 5
        };

    private:
        bool isAction[2] = {
            false,
            true
        };
        uint8_t cntf = 4U;
        float *p_values = nullptr;
        float a = 0, t = 0, p = 0, p_raw = 0;
        NodeI2CWeather::FORECAST forecast = NodeI2CWeather::FORECAST::STABLE;
        Adafruit_BMP280 *bmp;
                             
        uint8_t forecastCalc() {
          /* approximately every 5 minutes */
          if (++cntf < 4U)
            return 0xff;

          cntf = 0U;
          const float alt_ = static_cast<float>(BMP280_ALTITUDE_INT),
                      shift1_ = 288.15,
                      shift2_ = 0.0065,
                      shift3_ = 0.1903,
                      shift4_ = 5.255;

          for (uint16_t i = 11; i > 0U; i--)
            p_values[i] = p_values[(i - 1)];

          float altm_ = ((shift1_ / shift2_) * (1 - pow((p_raw / alt_), shift3_)));
          p_values[0] = alt_ * pow((1 - ((shift2_ * (altm_ - a)) / shift1_)), shift4_);

          if ((p_values[0] - p_values[1]) < -30) {
            forecast = NodeI2CWeather::FORECAST::THUNDERSTORM;
          } else {
            float pd0_ = p_values[0] - p_values[11];
            if (pd0_ > 100)
                forecast = NodeI2CWeather::FORECAST::STABLE;
            else if ((pd0_ <= 100) && (pd0_ >= 20))
                forecast = NodeI2CWeather::FORECAST::CLOUDY;
            else if (pd0_ < -200)
                forecast = NodeI2CWeather::FORECAST::THUNDERSTORM;
            else if ((pd0_ >= -200) && (pd0_ <= -20))
                forecast = NodeI2CWeather::FORECAST::UNSTABLE;
            else if ((pd0_ < 20) && (pd0_ > -20)) {
                if (t > 15U)
                    forecast = NodeI2CWeather::FORECAST::SUNNY;
                else
                    forecast = NodeI2CWeather::FORECAST::CLOUDY;
            } else {
                forecast = NodeI2CWeather::FORECAST::UNKNOWN;
            }
          }
          return static_cast<uint8_t>(forecast);
        }
        uint8_t getTempId() {
            return static_cast<uint8_t>(INTERNAL_LIVE_TEMP);
        }
        uint8_t getBaroId() {
            return static_cast<uint8_t>(INTERNAL_LIVE_BARO);
        }
        const char * getForecastString(uint8_t & r) {
            if (r < __NELE(str_forecast))
                return str_forecast[r];
            return str_forecast[(__NELE(str_forecast) - 1U)];
        }

    public:
        NodeI2CWeather() {
            bmp = new Adafruit_BMP280();
            p_values = new float[12]{};
        }
        ~NodeI2CWeather() {
            delete bmp;
            delete [] p_values;
        }
        bool getReady() {
            return isAction[IDX_Staus];
        }
        float getPressure() {
            return p;
        }
        float getTemperature() {
            return t;
        }
        float getAltitude() {
            return a;
        }
        NodeI2CWeather::FORECAST getForecast() {
            return forecast;
        }
        
        bool go_init() {
            bmp->setSampling(
                Adafruit_BMP280::MODE_NORMAL,
                Adafruit_BMP280::SAMPLING_X16,
                Adafruit_BMP280::SAMPLING_X16,
                Adafruit_BMP280::FILTER_OFF,
                Adafruit_BMP280::STANDBY_MS_4000);
            isAction[IDX_Staus] = bmp->begin(static_cast<uint16_t>(BMP280_ADDRESS));
            return isAction[IDX_Staus];
        }
        bool go_presentation() {
          
            uint8_t tid = getTempId(),
                    bid = getBaroId();
            if (!presentSend(tid, S_TEMP))
              return false;
            if (!presentSend(tid, V_TEMP))
              return false;
            if (!presentSend(bid, S_BARO))
              return false;
            if (!presentSend(bid, V_PRESSURE))
              return false;
            if (!presentSend(bid, V_FORECAST))
              return false;
            return true;
        }
        void go_data(uint16_t & cnt) {

            if (((cnt % POLL_WAIT_SECONDS) == 0) || (isAction[IDX_Change])) {

                p_raw = ((bmp->readPressure() + bmp->readPressure()) / 2);
                float t_ = bmp->readTemperature(),
                      p_ = (p_raw / 100.0F / 1.333);
                  
                if ((t != t_) || (isAction[IDX_Change])) {
                    t = t_;
                    reportMsg(
                        getTempId(),
                        V_TEMP,
                        t,
                        1U,
                        false
                    );
                }
                if ((p != p_) || (isAction[IDX_Change])) {
                    p = p_;
                    reportMsg(
                       getBaroId(),
                       V_PRESSURE,
                       p,
                       1U,
                       false
                    );
                    ///
                    const float alt_ = static_cast<float>(BMP280_ALTITUDE_DEFAULT);
                    a = ((bmp->readAltitude(alt_) + bmp->readAltitude(alt_)) / 2);
                    uint8_t r_ = forecastCalc();
                    if (r_ != 0xff)
                        reportMsg(
                            getBaroId(),
                            V_FORECAST,
                            getForecastString(r_),
                            false
                        );
                }
                if (isAction[IDX_Change])
                    isAction[IDX_Change] = false;
            }
        }
        bool go_data(const MyMessage & msg) {
           
            switch (msg.getType()) {
                case V_TEMP: {
                    if (msg.sensor != getTempId())
                        return false;
                    break;
                }
                case V_FORECAST:
                case V_PRESSURE: {
                    if (msg.sensor != getBaroId())
                        return false;
                    break;
                }
                default:
                    return false;
            }
            isAction[IDX_Change] = true;
            return true;
        }
};

#  undef POLL_WAIT_SECONDS
#  undef IDX_Staus
#  undef IDX_Change
#  endif
#endif
