/*
 * sd.h
 *
 *  Created on: May 17, 2022
 *      Author: user
 */

#ifndef INC_SD_H_
#define INC_SD_H_
#include <inttypes.h>
#include "stm32f4xx_hal.h"
#define SD_DATA_BLOCK 512
typedef struct {
	uint8_t cmd_index;
	uint32_t argument;
	uint8_t crc;
}CMD_Frame;
//SD commands
#define CDM0 0
#define CMD1 1
#define ACMD41 41
#define CMD8 8
#define CMD9 9
#define CMD10 10
#define CMD12 12
#define CMD16 16
#define CMD17 17
#define CMD18 18
#define CMD23 23
#define ACMD23 23
#define CMD24 24
#define CMD25 25
#define CMD55 55
#define CMD58 58

//abbreviations for above commands
#define GO_IDDLE_STATE CMD0
#define SET_OP_COND CMD1
#define APP_SEND_OP_COND ACMD41
#define SEND_IF_COND CMD8
#define SEND_CSD CMD9
#define SEND_CID CMD10
#define STOP_TRANSMISSION CMD12
#define SET_BLOCKLEN CMD16
#define READ_SINGLE_BLOCK CMD17
#define READ_MULTIPLE_BLOCK CMD18
#define SET_BLOCK_COUNT CMD23
#define SET_WR_BLOCK_ERASE_COUNT ACMD23
#define WRITE_BLOCK CMD24
#define WRITE_MULTIPLE_BLOCK CMD25
#define APP_CMD CMD55
#define READ_OCR CMD58
/*
 * @brief function which initilaizes sd card i SPI mode
 * @param handle to SPI structure on which card is connected
 */
void SD_Init(SPI_HandleTypeDef* hspi,GPIO_TypeDef*,uint8_t CS_Pin);
#endif /* INC_SD_H_ */
