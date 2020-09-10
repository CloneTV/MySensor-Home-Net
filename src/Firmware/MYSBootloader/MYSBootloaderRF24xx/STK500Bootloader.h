/* 
 * MYSBootloader 1.3.0-rc.1
 * OTA RF24 bootloader for MySensors: https://www.mysensors.org
 * Based on MySensors library 2.2
 * Developed and maintained by tekka 2018
 *
 *
 * STK500 optiboot code: 
 * - Copyright 2013-2015 by Bill Westfield
 * - modified from optiboot: https://github.com/Optiboot/optiboot
 */

#ifndef STK500Bootloader_H
#define STK500Bootloader_H

#include "stk500.h"

extern void updateEepromNodeFirmwareConfig(uint8_t *eepromNodeFirmwareConfig);

static void verifySpace(void) {
	if (getch() != CRC_EOP) {
		// no space (0x20) received, reboot
		watchdogConfig(WATCHDOG_16MS);
		while(1){};
	}
	putch(STK_INSYNC);
}

static void getNch(uint8_t count) {
	do {
		(void)getch(); 
	} while (--count);
	verifySpace();
}

static void STK500Bootloader(void) {
	uint8_t ch;
	uint16_t address = 0;	// not necessary, uses 4 bytes
	uint8_t  length;
	uint16_t FW_len = 0;
	bool exit_signal = false;
	
	initUART();
	
	// loop
	while(!exit_signal) {
		watchdogReset();
		// get character from UART
		ch = getch();
		
		if (ch == STK_GET_PARAMETER) {
			ch = getch();
			verifySpace();
			if (ch == 0x82) {
				putch(MYSBOOTLOADER_MINVER);
			} 
			else if (ch == 0x81) {
				putch(MYSBOOTLOADER_MAJVER + 100);	// use out-of-sequence versioning to prevent collisions, see optiboot source
			} 
			else putch(0x03);
		} 
		else if (ch == STK_SET_DEVICE) {
			getNch(20);
		} 
		else if (ch == STK_SET_DEVICE_EXT) {
			getNch(5);
		} 
		else if (ch == STK_LOAD_ADDRESS) {
			// LOAD ADDRESS, convert from word address to byte address
			address = ( getch() | getch() << 8) * 2;			
			verifySpace();
		} 
		else if (ch == STK_UNIVERSAL) {
			// UNIVERSAL command is ignored
			getNch(4);
			putch(0x00);
		} 
		else if (ch == STK_PROG_PAGE) {
			(void)getch();
			length = getch();
			(void)getch();
			FW_len += length;
			for (uint8_t i= 0; i < length; i+=2) {
				uint16_t data = getch() | ((getch() << 8));
				writeTemporaryBuffer(address + i,data);
			}
			programPage(address);
			// Read command terminator, start reply
			verifySpace();		
		} 
		else if (ch == STK_READ_PAGE) {
			// only flash
			(void)getch();
			length = getch();
			(void)getch();
			verifySpace();
			do {
				// read a flash byte and increment the address
				__asm__ ("lpm %0,Z+\n" : "=r" (ch), "=z" (address): "1" (address));
				putch(ch);
			} while (--length);
		} 
		else if (ch == STK_READ_SIGN) {
			// READ SIGN - return what Avrdude wants to hear
			verifySpace();
			putch(SIGNATURE_0);
			putch(SIGNATURE_1);
			putch(SIGNATURE_2);	
		} 
		else if (ch == STK_LEAVE_PROGMODE) {
			// version and type = 0xFFFF for STK bootloader, since this info is not submitted
			nodeFirmwareConfig_t _eepromNodeFirmwareConfig;
			_eepromNodeFirmwareConfig.type_command.type = 0xFFFF;
			_eepromNodeFirmwareConfig.version_data.version = 0xFFFF;
			// compute FW blocks and FW CRC
			_eepromNodeFirmwareConfig.blocks = FW_len / FIRMWARE_BLOCK_SIZE;
			_eepromNodeFirmwareConfig.crc = calcCRCrom(FW_len);
			// write FW settings to eeprom
			updateEepromNodeFirmwareConfig((uint8_t*)&_eepromNodeFirmwareConfig);
			exit_signal = true;
			verifySpace();
		} 
		else {
			verifySpace();
		}
		putch(STK_OK);
	}
}

#endif // STK500Bootloader_H