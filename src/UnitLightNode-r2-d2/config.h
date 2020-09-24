
/// UnitLightNode - RELAY 2, DIMMERS 2 

#define MY_NODE_ID 22
#define MY_PARENT_NODE_ID 0
#define MY_PARENT_NODE_IS_STATIC
#define MY_HOSTNAME "Unit-LightNode-r2-d2"
#define MY_VERSION "1.2"
///
#define NO_DEBUG 1
#define NO_DEBUG_RADIO 1
#define LED_DEBUG 0          /* Atmega board internal LED Debug blink method */
///
#define LIGHT_SENSOR 2       /* number Relays && Buttons */
#define LIGHT_ON_POWER STATE /* enable LIGHT on power ON/OFF/STATE by number or set: ALL */
// OR
//#define LIGHT_SENSOR_BTN 2   /* number Relays && Buttons */
//#define LIGHT_ON_POWER_BTN STATE /* enable LIGHT on power ON/OFF/STATE by number or set: ALL */
///
#define DIMMER_SENSOR 2      /* number Dimmers */
///
#define INTERNAL_LIVE_VOLT_PIN A0  /* Internal power volts */
#define INTERNAL_LIVE_ILLUMINATION_PIN A1 /* Internal illumination photo-resistor */
