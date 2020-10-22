
#define MY_NODE_ID 23
#define MY_PARENT_NODE_ID 0
#define MY_PARENT_NODE_IS_STATIC
#define MY_HOSTNAME "Unit-MoistureNode"
#define MY_VERSION "1.5"
///
#define NO_DEBUG 0
#define NO_DEBUG_RADIO 1
#define LED_DEBUG 1          /* Atmega board internal LED Debug blink method */
///
#define ENABLE_SENSOR_SOIL 1
/* PIN_index_1, PIN_index_2, PIN_index_NRF, PIN_index_A (read) */
#define SOIL_SENSOR_PINS A3,A2,8,A1   /* enable Soil sensor */
/* 
    A3 - sensor (1)
    A1 - sensor (2) + R4.7k
    A2 - R4.7k

    A3 --------------< sensor (1)
    A2 --[4.7k] --x--< sensor (2)
    A1 -----------|

*/
