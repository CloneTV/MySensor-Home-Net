
ATMELSTUDIO := $(filter-out $@,$(MAKECMDGOALS))

BAUDRATE :=
PROJECT := MYSBootloader
MCU := atmega328p
CLK := 8000000L

# BAUDRATE as 9600 38400 57600
ifeq ($(BAUDRATE),)
	BAUDRATE = 9600
endif

ISP_PORT = com9
ISP_SPEED = $(BAUDRATE)
ISP_PROTOCOL = stk500v2
ISP_MCU = m328p
ISP_LFUSE = FF # F7
ISP_HFUSE = E2 # E2/DA
ISP_EFUSE = FD # 06
ISP_ARGS = -c$(ISP_PROTOCOL) -P$(ISP_PORT) -b$(ISP_SPEED) -p$(ISP_MCU)

BINPATH = $(ATMELSTUDIO)/toolchain/avr8/avr8-gnu-toolchain/bin/
INCPATH = $(ARDUINOULIB)
CFLAGS = -funsigned-char -funsigned-bitfields -DF_CPU=$(CLK) -DBAUD_RATE=$(BAUDRATE) -Os -ffunction-sections -fdata-sections -fpack-struct -fshort-enums -mrelax -Wall -Wextra -Wundef -pedantic -mmcu=$(MCU) -c -std=gnu99 -MD -MP -MF "$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -MT"$(@:%.o=%.o)" 
LDFLAGS = -nostartfiles -Wl,-s -Wl,-static -Wl,-Map="$(OutputFileName).map" -Wl,--start-group -Wl,--end-group -Wl,--gc-sections -mrelax -Wl,-section-start=.text=0x7800 -mmcu=$(MCU)  

all: clean out

clean:
	@$(ATMELSTUDIO)/shellutils/rm -f *.o
	@$(ATMELSTUDIO)/shellutils/rm -f *.elf
	@$(ATMELSTUDIO)/shellutils/rm -f *.hex

$(PROJECT).o: $(PROJECT).c
	"$(BINPATH)avr-gcc" -I$(INCPATH) $(CFLAGS) $< -o $@

$(PROJECT).elf: $(PROJECT).o
	"$(BINPATH)avr-gcc" $(LDFLAGS) -o $@ $< -lm
	
$(PROJECT).hex: $(PROJECT).elf
	"$(BINPATH)avr-objcopy" -O ihex -R .eeprom $< $@ 

out: $(PROJECT).hex
	"$(BINPATH)avr-size" $(PROJECT).elf

copy: $(PROJECT).hex
	copy $(PROJECT).hex "$(COPYTO)/$(PROJECT)_$(ISP_MCU)_$(CLK)_Mhz_$(BAUDRATE).hex"

load: clean out isp

isp: $(PROJECT).hex
	"$(BINPATH)avrdude" $(ISP_ARGS) -e -u -Ulock:w:0x3f:m -qq -Uefuse:w:0x$(ISP_EFUSE):m -Uhfuse:w:0x$(ISP_HFUSE):m -Ulfuse:w:0x$(ISP_LFUSE):m -Ulock:w:0x0f:m
	"$(BINPATH)avrdude" $(ISP_ARGS) -V -q -s -Uflash:w:$(PROJECT).hex
	"$(BINPATH)avrdude" $(ISP_ARGS)
