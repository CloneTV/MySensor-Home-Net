#if !defined(MY_DEBUG_DISPLAY_H_)
#define MY_DEBUG_DISPLAY_H_

typedef void (*prn_cb)(const char*);

#  if defined(MY_DEBUG_DISPLAY)
typedef MyDebugDisplay<NUBRR1H, NUBRR1L, NUCSR1A, NUCSR1B, NUDR1, 
  RXEN1, TXEN1, NRXCIE1, UDRE1, U2X1, 128> _MyDisplay;

_MyDisplay MyDisplay;

template <typename T>
class MyDebugDisplay : public Print {
    private:
        bool isEnable = false;
        T cls;
    
    public:
        MyDebugDisplay() {
            cls = new T();
        }
        ~MyDebugDisplay() {
            delete cls;
        }
        operator bool() {
            return isEnable;
        }
        void available(bool b) {
            isEnable = b;
        }
        void display(const char *s) {
            if (!isEnable)
                return;
        }
        // virtual size_t Print::write(uint8_t)
        size_t write(uint8_t val) {
            Serial.write(val);
            return 1U;
        }
        void flush() {

        }
};

#  endif
#endif



#if defined(MY_DEBUG_DISPLAY)
#  if defined(MY_DEBUGDEVICE)
#    undef MY_DEBUGDEVICE
#  endif
#  define MY_DEBUGDEVICE display
#endif


