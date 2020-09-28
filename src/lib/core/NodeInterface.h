
/*
 * not default C++ Class virtual Interface 
 * about this: https://stdcpp.ru/proposals/d07ada73-ed1b-4f25-8033-5e4f079ade55
*/

#include <Arduino.h>

template<class T>
class SensorInterface {
    public:
        bool init() {
            return static_cast<T*>(this)->go_init();
        }
        void data(uint16_t & c) {
            static_cast<T*>(this)->go_data(c);
        }
        bool data(const MyMessage & m) {
            return static_cast<T*>(this)->go_data(m);
        }
        bool presentation() {
            return static_cast<T*>(this)->go_presentation();
        }
};
