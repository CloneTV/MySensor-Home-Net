
/*
 * not default C++ Class virtual Interface 
 * about this: https://stdcpp.ru/proposals/d07ada73-ed1b-4f25-8033-5e4f079ade55
*/

#include <Arduino.h>

template<class T>
class SensorInterface {
    private:
        T const * pclass;

    public:
        SensorInterface(T const * cls) {
            ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
                pclass = cls;
            }
        }
        ~SensorInterface() {
            delete pclass;
        }
        void init() { pclass->init(); }
        void init(uint16_t & v) { pclass->init(v); }
        void data(uint16_t & c) { pclass->data(c); }
        bool data(const MyMessage & m) { return pclass->data(m); }
        bool presentation() { return pclass->presentation(); }
};