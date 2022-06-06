/*
 * sd.h
 *
 *  Created on: May 17, 2022
 *      Author: Krzysztof Gałowicz
 *
 */

#ifndef INC_SD_H_
#define INC_SD_H_

#include <inttypes.h>
#include "stm32f4xx_hal.h"

/* Disk Status Bits (DSTATUS) */

#define STA_NOINIT		0x01	/* Drive not initialized */
#define STA_NODISK		0x02	/* No medium in the drive */
#define STA_PROTECT		0x04	/* Write protected */
#define SD_DATA_BLOCK 512

/* Definitions for MMC/SDC command */
#define CMD0     (0x40+0)     	/* GO_IDLE_STATE */
#define CMD1     (0x40+1)     	/* SEND_OP_COND */
#define CMD8     (0x40+8)     	/* SEND_IF_COND */
#define CMD9     (0x40+9)     	/* SEND_CSD */
#define CMD10    (0x40+10)    	/* SEND_CID */
#define CMD12    (0x40+12)    	/* STOP_TRANSMISSION */
#define CMD16    (0x40+16)    	/* SET_BLOCKLEN */
#define CMD17    (0x40+17)    	/* READ_SINGLE_BLOCK */
#define CMD18    (0x40+18)    	/* READ_MULTIPLE_BLOCK */
#define CMD23    (0x40+23)    	/* SET_BLOCK_COUNT */
#define CMD24    (0x40+24)    	/* WRITE_BLOCK */
#define CMD25    (0x40+25)    	/* WRITE_MULTIPLE_BLOCK */
#define CMD41    (0x40+41)    	/* SEND_OP_COND (ACMD) */
#define CMD55    (0x40+55)    	/* APP_CMD */
#define CMD58    (0x40+58)    	/* READ_OCR */

/* MMC card type flags (MMC_GET_TYPE) */
#define CT_MMC		0x01		/* MMC ver 3 */
#define CT_SD1		0x02		/* SD ver 1 */
#define CT_SD2		0x04		/* SD ver 2 */
#define CT_SDC		0x06		/* SD */
#define CT_BLOCK	0x08		/* Block addressing */



typedef enum{
	SD_UNINITIALIZED=0,
	SD_POWERED=1,
	SD_INITIALIZED=2,
	SD_ERROR=3
}SD_StateTypeDef;
/**
 * structure describing SD card
 */
typedef struct {
	SPI_HandleTypeDef *hspi;
	GPIO_TypeDef *CS_Port;
	uint16_t CS_Pin;
	GPIO_TypeDef *DET_Port;
	uint16_t DET_Pin;
	GPIO_TypeDef *WP_Port;
	uint16_t WP_Pin;
	size_t size;
	SD_StateTypeDef state;
	uint8_t CardType;

} SD_HandleTypeDef;
/**
 * @brief function which initilaizes sd card i SPI mode.
 * @param handle to sd structure in which sd is described.
 */
void SD_Init(SD_HandleTypeDef *hsd);
/**
 * @brief function which deinitilaizes sd card .
 * @param handle to sd structure in which sd is described.
 */
void SD_DeInit(SD_HandleTypeDef *hsd);
/**
 * @brief this function reads size of connected sd card in multiply of 512 bytes.
 * @param handle to sd structure in which sd is described.
 * @return number of blocks available in connected sdc
 */
size_t SD_Get_Size(SD_HandleTypeDef *hsd);

/**
 * @brief This function reads a single block of 512B.
 * @param handle to sd structure in which sd is described.
 * @param buffer where data has to be written.
 * @param block address to read.
 * @return true if operation was successful, false otherwise.
 */
int SD_Read_Block(SD_HandleTypeDef *hsd, uint8_t *datablock, uint32_t address);
/**
 * @brief This function reads a single block of 512B to SD card.
 * @param handle to sd structure in which sd is described.
 * @param buffer of data which has to be written.
 * @param block address where to write.
 * @return true if operation was successful, false otherwise.
 */
int SD_Write_Block(SD_HandleTypeDef *hsd, void *datablock, uint32_t address);


#endif /* INC_SD_H_ */
