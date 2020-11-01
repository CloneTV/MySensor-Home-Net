/* 
 * MYSBootloader 1.3.0-rc.1
 * OTA RF24 bootloader for MySensors: https://www.mysensors.org
 * Based on MySensors library 2.2
 * Developed and maintained by tekka 2018
 */

#ifndef MYSBootloader_H
#define MYSBootloader_H

extern MyMessage _outMsg,_inMsg;
extern uint8_t _configuredParentID;
extern uint8_t _save_MCUSR;

#define TIMEOUT_MS			(2000)		// careful when changing
#define MAX_FIRMWARE_REQUEST_RESEND	(3)

#define SIGNING_PRESENTATION_VERSION_1	(1)
#define SIGNING_PRESENTATION_VALUE	(0)	// no signing

#define AUTO			(0xFFu)
#define NODE_SENSOR_ID		(0xFFu)
#define DISTANCE_INVALID	(0xFFu)
#define BROADCAST_ADDRESS	(0xFFu)
#define GATEWAY_ADDRESS		(0x00u)
#define DEBUG_NODE_ID		(0xFEu)

#define BL_CMD_MAGIC			(0xDA7Au)
#define BL_CMD_CLEAR_EEPROM		(0x01u)
#define BL_CMD_NODE_ID			(0x02u)
#define BL_CMD_PARENTNODE_ID	(0x03u)
#define EEPROM_ERASED_BYTE		(0xFFu)

// Pseudo command used by processRX. Use out of range values
#define I_FIND_CONFIGURED_PARENT_RESPONSE_PSEUDO_COMMAND	(0xFE)
#define I_NO_ANSWER_PSEUDO_COMMAND				(0xFF)

static uint8_t byteAtoi(const char *str, uint8_t len)
{
	// Initialize result
    uint8_t result = 0;
    // Iterate through all characters of input string and update result
	while(len--) {
        result = result * 10 + *str++ - '0';
	}
    return result;
}

void updateEepromNodeFirmwareConfig(uint8_t *eepromNodeFirmwareConfig) {
	for(uint8_t i = 0; i < sizeof(nodeFirmwareConfig_t); i++) {
		eeprom_update_byte((uint8_t*)(EEPROM_FIRMWARE_TYPE_ADDRESS+i), *eepromNodeFirmwareConfig++);
	}
}

static void _buildMessageProto(const uint8_t type, const uint8_t version_length, const uint8_t command_ack_payload) {
	_outMsg.sender = _eepromNodeConfig.nodeId;
	_outMsg.last = _eepromNodeConfig.nodeId;
	_outMsg.sensor = NODE_SENSOR_ID;
	_outMsg.type = type;
	_outMsg.command_ack_payload = command_ack_payload;
	_outMsg.version_length = version_length;
}

#define MSG_SIGN	(0)
#define ReqACK		(0)

#define _buildMessage(__command, __type, __payload_type, __length) _buildMessageProto(__type,( (__length << 3) | (MSG_SIGN << 2) | (PROTOCOL_VERSION & 3) ),( (__payload_type << 5) | (ReqACK << 3) | (__command & 7) ) )
#define _setMessageDestination(__dest) (_outMsg.destination = __dest)

inline static bool sendMessage(void) {
	//watchdogReset();
	return writeMessage(_eepromNodeConfig.parentNodeId, _outMsg.array, HEADER_SIZE + mGetLength(_outMsg) );
}

static uint8_t processRX(void) {
	// No answer as default return
	uint8_t result = I_NO_ANSWER_PSEUDO_COMMAND;
	if ( _dataAvailable() ) {
		// discard faulty transmissions
		if(readMessage(_inMsg.array)!=mGetLength(_inMsg) + HEADER_SIZE) {
			return result;
		}
		if (_inMsg.destination == _eepromNodeConfig.nodeId) {
			result = _inMsg.type;
			// parse internal find parent response only if configured parent not already found or configured parent not defined
			if ( (mGetCommand(_inMsg) == C_INTERNAL) && (_inMsg.type == I_FIND_PARENT_RESPONSE) &&
			     ( (_eepromNodeConfig.parentNodeId != _configuredParentID) || (_configuredParentID == AUTO) ) ) {
				if (_inMsg.sender == _configuredParentID ) {
					// Configured parent found, save it 
					_eepromNodeConfig.parentNodeId = _configuredParentID;
					// Stop find parent waiting loop by answering using the pseudo command
					result = I_FIND_CONFIGURED_PARENT_RESPONSE_PSEUDO_COMMAND;
				}
				// Payload value is sender distance to Gateway. Increase it to get our distance
				else if ( _inMsg.payload.bValue++ < _eepromNodeConfig.distance) {
					// got new routing info, update settings
					_eepromNodeConfig.distance = _inMsg.payload.bValue;
					_eepromNodeConfig.parentNodeId = _inMsg.sender;
				}
			} 
		}
	}
	return result;
}

// process until specific response received, timeout after C_TIMEOUT ms
#if F_CPU==1000000L
	#define COUNTER_START_VAL 0xFFFE / 4	// empirical value
#else
	#define COUNTER_START_VAL 0xFFFE
#endif
#define WAIT_DIVIDER (COUNTER_START_VAL / TIMEOUT_MS)
#if F_CPU>=16000000
	#define GRANULARITY_US (200 / WAIT_DIVIDER)
#elif F_CPU>=8000000
	#define GRANULARITY_US (100 / WAIT_DIVIDER)
#elif F_CPU>=2000000
	#define GRANULARITY_US (25)
#else
	#define GRANULARITY_US (0)
#endif

static bool send_process_type(const uint8_t response_type, uint8_t retries) {	
	bool response = false;
	do {
		(void)sendMessage();
		uint16_t count = COUNTER_START_VAL;
		do {
			watchdogReset();
			// process incoming messages
			response = (processRX() == response_type ) ;
			_delay_us(GRANULARITY_US);
		} while (count-- && !response);
	} while (retries-- && !response);
	return response;
}

static void MySensorsBootloader(void) {
	nodeFirmwareConfig_t _eepromNodeFirmwareConfig;
	uint16_t RequestedBlock = 0u;
	uint16_t _newFirmwareCrc = 0xFFFF;
	// mappings
	requestFirmwareConfig_t *firmwareConfigRequest = (requestFirmwareConfig_t*)_outMsg.payload.data;
	nodeFirmwareConfig_t *firmwareConfigResponse = (nodeFirmwareConfig_t *)_inMsg.payload.data;
	requestFirmwareBlock_t *firmwareRequest = (requestFirmwareBlock_t *)_outMsg.payload.data;
	responseFirmwareBlock_t *firmwareResponse = (responseFirmwareBlock_t *)_inMsg.payload.data;
	SM_BL_STATE BL_STATE = BL_READ_CONFIG;		// initial state
	initSPI();
	// main loop
	while(1) {
		// states
		if(BL_STATE == BL_READ_CONFIG) {	
			#ifdef DEBUG
				DEBUG_PORT = DEBUG_READ_CONFIG;
			#endif	
			eeprom_busy_wait();
			// Read node config from EEPROM, i.e. nodeId, parent nodeId, distance
			eeprom_read_block(&_eepromNodeConfig, (uint8_t*)EEPROM_NODE_ID_ADDRESS, sizeof(nodeConfig_t));
			// Read firmware config from EEPROM, i.e. type, version, CRC, blocks
			eeprom_read_block(&_eepromNodeFirmwareConfig, (uint8_t*)EEPROM_FIRMWARE_TYPE_ADDRESS, sizeof(nodeFirmwareConfig_t));
			// initialize radio
			//if (_eepromNodeConfig.nodeId == AUTO)
				//_eepromNodeConfig.nodeId = 254U;
			if(initRadio()) {
				BL_STATE = BL_FIND_PARENTS;
			}
			else {
				// if radio not initialized, proceed to startup check
				BL_STATE = BL_RUN;
			}					
		} 
		else if(BL_STATE == BL_FIND_PARENTS) {
			#ifdef DEBUG
				DEBUG_PORT = DEBUG_FIND_PARENTS;
			#endif
			// save parent node id for static parent - if static parent not found, use closest node to gateway for communication
			_configuredParentID = _eepromNodeConfig.parentNodeId;
			_eepromNodeConfig.parentNodeId = AUTO;
			_eepromNodeConfig.distance = DISTANCE_INVALID;
			// prepare for I_FIND_PARENTS
			 _setMessageDestination(BROADCAST_ADDRESS);
			 // here we deal with ChinRF24 clones and inverted NO_ACK bit error: do not use the NO_ACK feature, but limit number of retries to 3
			 _writeRegister(SETUP_RETR, 5 << ARD | 3 << ARC);
			 _buildMessage(C_INTERNAL,I_FIND_PARENT_REQUEST, P_BYTE, 1);
			// wait until I_FIND_CONFIGURED_PARENT_RESPONSE_PSEUDO_COMMAND command received => does not exist as real command,
			// therefore process incoming messages until timeout or (configured) parent found
			// force 1 retry in order to ensure first message reception from routers. Work around for NRF24L01 PID problem
			(void)send_process_type(I_FIND_CONFIGURED_PARENT_RESPONSE_PSEUDO_COMMAND,1);
			// from now on, all messages directed to GW
			_setMessageDestination(GATEWAY_ADDRESS);
			// auto retransmit delay 1500us, auto retransmit count 15
			_writeRegister(SETUP_RETR, 5 << ARD | 15 << ARC);
			if ( _eepromNodeConfig.parentNodeId!=AUTO ) {
				BL_STATE = BL_CHECK_ID;
			}
			else {
				BL_STATE = BL_RUN;		// if no reply received (e.g. no parent nodes), proceed to startup
			}
		} 
		else if(BL_STATE == BL_CHECK_ID) {
			#ifdef DEBUG
				DEBUG_PORT = DEBUG_CHECK_ID;
			#endif
			if(_eepromNodeConfig.nodeId==GATEWAY_ADDRESS || _eepromNodeConfig.nodeId==AUTO) {
				_buildMessage(C_INTERNAL,I_ID_REQUEST, P_BYTE, 0);
				if (send_process_type(I_ID_RESPONSE, 3)) {
					#if defined(DEBUG)
						// atoi uses ~50bytes needed for led debug
						const uint8_t newID = DEBUG_NODE_ID;
					#else
						// Less than 40 bytes atoi version
						const uint8_t newID = byteAtoi(_inMsg.payload.data, mGetLength(_inMsg));
					#endif
					eeprom_update_byte((uint8_t*)EEPROM_NODE_ID_ADDRESS, newID);	// save ID in eeprom
					BL_STATE = BL_READ_CONFIG;
				}
			} 
			else {
				// ID is valid
				BL_STATE = BL_CONFIGRUATION;
			}
		}
		else if(BL_STATE == BL_CONFIGRUATION) {
			#ifdef DEBUG
				DEBUG_PORT = DEBUG_CONFIGURATION;
			#endif
			// default state: if no reply received, proceed to startup
			BL_STATE = BL_RUN;
			// singing preferences, inform GW that BL does not require signed messages
			_outMsg.payload.data[0] = SIGNING_PRESENTATION_VERSION_1;
			_outMsg.payload.data[1] = SIGNING_PRESENTATION_VALUE;
			_buildMessage(C_INTERNAL,I_SIGNING_PRESENTATION,P_CUSTOM,2);
			(void)send_process_type(I_SIGNING_PRESENTATION,0);
			// update with current CRC in case of memory corruption or failed transmission
			_eepromNodeFirmwareConfig.crc = calcCRCrom(_eepromNodeFirmwareConfig.blocks*FIRMWARE_BLOCK_SIZE);
			// copy to outMsg mapping	
			(void)memcpy(firmwareConfigRequest,&_eepromNodeFirmwareConfig,sizeof(nodeFirmwareConfig_t));
			// add BL information
			firmwareConfigRequest->BLVersion = MYSBOOTLOADER_VERSION;
			// Send a firmware config request to GW/controller
			_buildMessage(C_STREAM,ST_FIRMWARE_CONFIG_REQUEST,P_CUSTOM,sizeof(requestFirmwareConfig_t));
			if(send_process_type(ST_FIRMWARE_CONFIG_RESPONSE, 3)) {
				#ifdef DEBUG
					DEBUG_PORT = (uint8_t)(DEBUG_CONFIG_RECEIVED);
				#endif

				if (memcmp(&_eepromNodeFirmwareConfig,&_inMsg.payload.data,sizeof(nodeFirmwareConfig_t))) {
					#ifdef BOOTLOADER_COMMANDS
					// BL commands use 86 bytes
					if (!firmwareConfigResponse->blocks && firmwareConfigResponse->crc == BL_CMD_MAGIC) {
						// cmd 0x01 clear eeprom
						if(firmwareConfigResponse->type_command.bl_command == BL_CMD_CLEAR_EEPROM) {
							for(uint16_t i = 0; i < EEPROM_SIZE; i++) eeprom_update_byte((uint8_t *)i, EEPROM_ERASED_BYTE);
						} 
						else { 
							// cmd 0x02 set id
							// cmd 0x03 set parent id
							if(firmwareConfigResponse->type_command.bl_command == BL_CMD_NODE_ID || firmwareConfigResponse->type_command.bl_command == BL_CMD_PARENTNODE_ID) {
								eeprom_update_byte((uint8_t*)(EEPROM_NODE_ID_ADDRESS+firmwareConfigResponse->type_command.bl_command-2), (uint8_t)firmwareConfigResponse->version_data.bl_data);
							}
						}
						BL_STATE = BL_READ_CONFIG;
					} 
					else 
					#endif
					{		
						// update needed, do not allow writing to bootloader section, i.e. max block < 0x7800 / 0x10 = 0x780 = 1920
						if (firmwareConfigResponse->blocks < (BOOTLOADER_START_ADDRESS/FIRMWARE_BLOCK_SIZE)) {
							// save received firmware config
							(void)memcpy(&_eepromNodeFirmwareConfig,_inMsg.payload.data,sizeof(nodeFirmwareConfig_t));
							BL_STATE = BL_INIT_UPDATE;
						}
					}
				}	
			}
		} 
		else if(BL_STATE == BL_INIT_UPDATE) {
			#ifdef DEBUG
				DEBUG_PORT = DEBUG_INIT_UPDATE;
			#endif
			BL_STATE = BL_DO_UPDATE;	// default
			// copy new FW details for update
			_newFirmwareCrc = _eepromNodeFirmwareConfig.crc;
			// invalidate CRC
			_eepromNodeFirmwareConfig.crc = 0xFFFF;
			// update EEPROM, save new FW details in case of faulty transmissions
			updateEepromNodeFirmwareConfig((uint8_t*)&_eepromNodeFirmwareConfig);
			// set FW block to request
			RequestedBlock = _eepromNodeFirmwareConfig.blocks;		
		}
		else if(BL_STATE == BL_DO_UPDATE) {
			#ifdef DEBUG
				DEBUG_PORT = DEBUG_DO_UPDATE;
			#endif
			
			// maps to outMsg
			firmwareRequest->type = _eepromNodeFirmwareConfig.type_command.type;
			firmwareRequest->version = _eepromNodeFirmwareConfig.version_data.version;
			firmwareRequest->block = RequestedBlock - 1;
			// prepare FW block request
			_buildMessage(C_STREAM,ST_FIRMWARE_REQUEST,P_CUSTOM,sizeof(requestFirmwareBlock_t));
			// request FW from controller, load FW counting backwards
			if(send_process_type(ST_FIRMWARE_RESPONSE, MAX_FIRMWARE_REQUEST_RESEND)) {
				if (!memcmp(firmwareRequest,firmwareResponse,sizeof(requestFirmwareBlock_t))) {
					// calculate page offset
					const uint8_t offset = ((RequestedBlock - 1) * FIRMWARE_BLOCK_SIZE) % SPM_PAGESIZE;
					// write to page buffer
					for(uint8_t i = 0; i < FIRMWARE_BLOCK_SIZE; i += 2) {
						writeTemporaryBuffer(offset + i, firmwareResponse->data[i] | firmwareResponse->data[i+1]<<8);
					}
					// program page if full
					if (!offset) {
						programPage( ( (RequestedBlock - 1) * FIRMWARE_BLOCK_SIZE));
					}
					// proceed with next=previous block
					RequestedBlock--;
					if(!RequestedBlock) {
						// if last block requested, proceed with the validation
						_eepromNodeFirmwareConfig.crc = _newFirmwareCrc;					
						// validate FW CRC
						if(calcCRCrom(_eepromNodeFirmwareConfig.blocks * FIRMWARE_BLOCK_SIZE) != _newFirmwareCrc) {
							// received FW is invalid, invalidate CRC
							// new FW length 0 blocks
							_eepromNodeFirmwareConfig.blocks = 0u;
							// CRC will evaluate to 0xFFFF with 0 blocks
							_eepromNodeFirmwareConfig.crc = 0u;
						}
						// write FW settings to EEPROM
						updateEepromNodeFirmwareConfig((uint8_t*)&_eepromNodeFirmwareConfig);
						BL_STATE = BL_READ_CONFIG;
					}
				}
			} 
			else {
				BL_STATE = BL_CONFIGRUATION;
			}
		}
		else if(BL_STATE == BL_RUN) {
			#ifdef DEBUG
				DEBUG_PORT = DEBUG_PREPARE_RUN;
			#endif
			// verify flash CRC with stored CRC in EEPROM
			if(calcCRCrom(_eepromNodeFirmwareConfig.blocks * FIRMWARE_BLOCK_SIZE) == _eepromNodeFirmwareConfig.crc) {
				#ifdef DEBUG
					DEBUG_PORT = DEBUG_RUN;
				#endif
				SPIclose();				
				// watchdog settings
				#ifdef WATCHDOG_ON_SKETCH_START
					watchdogReset();
				#else
					watchdogConfig(WATCHDOG_OFF);
				#endif
				// save the reset flags in the designated register for application retrieval (using .init0), uses 6 bytes
				__asm__ __volatile__ ("mov r2, %0\n" :: "r" (_save_MCUSR));
				// run sketch
				((void(*)()) 0)();
			}
			// crc invalid, start from beginning
			BL_STATE = BL_READ_CONFIG;
		}
	}
}

#endif