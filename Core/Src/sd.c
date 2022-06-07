/*
 * sd.c
 *
 *  Created on: Jun 1, 2022
 *      Author: user
 */

#include "sd.h"
#define SPI_TIMEOUT 100
/**
 * helper variables
 */
/**
 * structure describing command frame
 */
typedef struct {
	uint8_t cmd_index;
	uint32_t argument;
	uint8_t crc;
} CMD_Frame;
extern int32_t timer1, timer2;
/**
 * helper funcrions
 */
/**
 * @brief this function sends cmd to SDC
 * @param handle to SD struct
 * @param cmd to send
 * @return response from cd
 */
static uint8_t SD_SendCmd(SD_HandleTypeDef *hsd, uint8_t cmd, uint32_t arg);
static void SD_Select(SD_HandleTypeDef *hsd);
static void SD_Deselect(SD_HandleTypeDef *hsd);
static void SPI_TxByte(SD_HandleTypeDef *hsd, uint8_t data);
static void SPI_TxBuffer(SD_HandleTypeDef *hsd, void *buffer, uint16_t len);
static uint8_t SPI_RxByte(SD_HandleTypeDef *hsd);
static void SPI_RxBytePtr(SD_HandleTypeDef *hsd, uint8_t *buff);
static uint8_t SD_ReadyWait(SD_HandleTypeDef *hsd, uint32_t timeout);

/**
 * main functions
 */
/**
 * @brief function which initilaizes sd card i SPI mode.
 * @param handle to sd structure in which sd is described.
 */
void SD_Init(SD_HandleTypeDef *hsd) {
	//DONE?
	CMD_Frame tmp_frame;
	uint32_t cnt = 0x1fff;
	uint8_t ocr[4], n;
	//reset card
	SD_Deselect(hsd);
	for (int i = 0; i < 12; ++i) {
		SPI_TxByte(hsd, 0xff);
	}
	SD_Select(hsd);
	tmp_frame.argument = 0;
	tmp_frame.cmd_index = CMD0;
	tmp_frame.crc = 0x95;
	SPI_TxBuffer(hsd, &tmp_frame, sizeof(tmp_frame));
	while ((SPI_RxByte(hsd) != 0x01) && cnt) {
		cnt--;
	}
	SD_Deselect(hsd);
	SPI_TxByte(hsd, 0XFF);
	hsd->state = SD_POWERED;
	//init
	SD_Select(hsd);
	uint8_t type = 0;
	if (SD_SendCmd(hsd, CMD0, 0) == 1) {
		/* timeout 1 sec */
		timer1 = 1000;

		/* SDC V2+ accept CMD8 command, http://elm-chan.org/docs/mmc/mmc_e.html */
		if (SD_SendCmd(hsd, CMD8, 0x1AA) == 1) {
			/* operation condition register */
			for (n = 0; n < 4; n++) {
				ocr[n] = SPI_RxByte(hsd);
			}

			/* voltage range 2.7-3.6V */
			if (ocr[2] == 0x01 && ocr[3] == 0xAA) {
				/* ACMD41 with HCS bit */
				do {
					if (SD_SendCmd(hsd, CMD55, 0) <= 1
							&& SD_SendCmd(hsd, CMD41, 1UL << 30) == 0)
						break;
				} while (timer1);

				/* READ_OCR */
				if (timer1 && SD_SendCmd(hsd, CMD58, 0) == 0) {
					/* Check CCS bit */
					for (n = 0; n < 4; n++) {
						ocr[n] = SPI_RxByte(hsd);
					}

					/* SDv2 (HC or SC) */
					type = (ocr[0] & 0x40) ? CT_SD2 | CT_BLOCK : CT_SD2;
				}
			}
		} else {
			/* SDC V1 or MMC */
			type = (SD_SendCmd(hsd, CMD55, 0) <= 1
					&& SD_SendCmd(hsd, CMD41, 0) <= 1) ?
			CT_SD1 :
															CT_MMC;

			do {
				if (type == CT_SD1) {
					if (SD_SendCmd(hsd, CMD55, 0) <= 1
							&& SD_SendCmd(hsd, CMD41, 0) == 0)
						break; /* ACMD41 */
				} else {
					if (SD_SendCmd(hsd, CMD1, 0) == 0)
						break; /* CMD1 */
				}

			} while (timer1);

			/* SET_BLOCKLEN */
			if (!timer1 || SD_SendCmd(hsd, CMD16, 512) != 0)
				type = 0;
		}
	}

	hsd->CardType = type;

	/* Idle */
	SD_Deselect(hsd);
	SPI_TxByte(hsd, 0xff);

	/* Clear STA_NOINIT */
	if (type) {
		hsd->state = SD_INITIALIZED;
	} else {
		/* Initialization failed */
		SD_DeInit(hsd);
		hsd->state = SD_ERROR;
	}

}
/**
 * @brief function which deinitilaizes sd card .
 * @param handle to sd structure in which sd is described.
 */
void SD_DeInit(SD_HandleTypeDef *hsd) {
	//DONE?
	//stop transmission
	SD_Select(hsd);
	SD_SendCmd(hsd, CMD12, 0);
	SD_Deselect(hsd);
	SPI_TxByte(hsd, 0xff);
	//clear status
	hsd->state = SD_UNINITIALIZED;

}
/**
 * @brief this function reads size of connected sd card in multiply of 512 bytes.
 * @param handle to sd structure in which sd is described.
 * @return number of blocks available in connected sdc
 */
size_t SD_Get_Size(SD_HandleTypeDef *hsd) {
	//TODO
	return 0;
}

/**
 * @brief This function reads a single block of 512B.
 * @param handle to sd structure in which sd is described.
 * @param buffer where data has to be written.
 * @param block address to read.
 * @return true if operation was successful, false otherwise.
 */
int SD_Read_Block(SD_HandleTypeDef *hsd, uint8_t *datablock, uint32_t address) {
	//DONE?
	SD_Select(hsd);
	if (SD_SendCmd(hsd,CMD17, address)) {
		SD_Deselect(hsd);
		SPI_RxByte(hsd);
		return 0;
	} else {
		uint8_t token;
		uint16_t len=512;
		/* timeout 200ms */
		timer1 = 200;

		/* loop until receive a response or timeout */
		do {
			token = SPI_RxByte(hsd);
		} while ((token == 0xFF) && timer1);

		/* invalid response */
		if (token != 0xFE)
			return 0;

		/* receive data */
		do {
			SPI_RxBytePtr(hsd, datablock++);
		} while (len--);

		/* discard CRC */
		SPI_RxByte(hsd);
		SPI_RxByte(hsd);

		SD_Deselect(hsd);
		SPI_RxByte(hsd);
		SPI_RxByte(hsd);
		return 1;
	}

}
/**
 * @brief This function reads a single block of 512B to SD card.
 * @param handle to sd structure in which sd is described.
 * @param buffer of data which has to be written.
 * @param block address where to write.
 * @return true if operation was successful, false otherwise.
 */
int SD_Write_Block(SD_HandleTypeDef *hsd, void *datablock, uint32_t address) {
	//DONE?
	SD_Select(hsd);
	if (SD_SendCmd(hsd, CMD24, address)) {
		SD_Deselect(hsd);
		SPI_RxByte(hsd);
		return 0;
	} else {
		uint8_t resp;
		uint8_t i = 0;

		/* wait SD ready */
		if (SD_ReadyWait(hsd, 1000) != 0xFF)
			return 0;

		/* transmit token */
		SPI_TxByte(hsd, 0xfe);

		/* if it's not STOP token, transmit data */

		SPI_TxBuffer(hsd, (uint8_t*) datablock, 512);

		/* discard CRC */
		SPI_RxByte(hsd);
		SPI_RxByte(hsd);

		/* receive response */
		while (i <= 64) {
			resp = SPI_RxByte(hsd);

			/* transmit 0x05 accepted */
			if ((resp & 0x1F) == 0x05)
				break;
			i++;
		}

		/* recv buffer clear */
		while (SPI_RxByte(hsd) == 0)
			;
		SD_Deselect(hsd);
		SPI_RxByte(hsd);
		SPI_RxByte(hsd);
		/* transmit 0x05 accepted */
		if ((resp & 0x1F) == 0x05)
			return 1;
		return 0;
	}

	return 0;
}

/**
 * @brief this function sends cmd to SDC
 * @param cmd to send
 * @return response from cd
 */
uint8_t SD_SendCmd(SD_HandleTypeDef *hsd, uint8_t cmd, uint32_t arg) {
	//DONE?
	uint8_t crc, res;

	/* wait SD ready */
	if (SD_ReadyWait(hsd, 1000) != 0xFF)
		return 0xFF;

	/* transmit command */
	SPI_TxByte(hsd, cmd); /* Command */
	SPI_TxByte(hsd, (uint8_t) (arg >> 24)); /* Argument[31..24] */
	SPI_TxByte(hsd, (uint8_t) (arg >> 16)); /* Argument[23..16] */
	SPI_TxByte(hsd, (uint8_t) (arg >> 8)); /* Argument[15..8] */
	SPI_TxByte(hsd, (uint8_t) arg); /* Argument[7..0] */

	/* prepare CRC */
	if (cmd == CMD0)
		crc = 0x95; /* CRC for CMD0(0) */
	else if (cmd == CMD8)
		crc = 0x87; /* CRC for CMD8(0x1AA) */
	else
		crc = 1;

	/* transmit CRC */
	SPI_TxByte(hsd, crc);

	/* Skip a stuff byte when STOP_TRANSMISSION */
	if (cmd == CMD12)
		SPI_RxByte(hsd);

	/* receive response */
	uint8_t n = 10;
	do {
		res = SPI_RxByte(hsd);
	} while ((res & 0x80) && --n);

	return res;
}
/*****************************************************************************/
void SD_Select(SD_HandleTypeDef *hsd) {
	HAL_GPIO_WritePin(hsd->CS_Port, hsd->CS_Pin, GPIO_PIN_RESET);
	HAL_Delay(1);
}
void SD_Deselect(SD_HandleTypeDef *hsd) {
	HAL_GPIO_WritePin(hsd->CS_Port, hsd->CS_Pin, GPIO_PIN_SET);
	HAL_Delay(1);
}
static void SPI_TxByte(SD_HandleTypeDef *hsd, uint8_t data) {
	//DONE?
	while (!__HAL_SPI_GET_FLAG(hsd->hspi, SPI_FLAG_TXE))
		;
	HAL_SPI_Transmit(hsd->hspi, &data, 1, SPI_TIMEOUT);
}
static void SPI_TxBuffer(SD_HandleTypeDef *hsd, void *buffer, uint16_t len) {
	//DONE?
	while (!__HAL_SPI_GET_FLAG(hsd->hspi, SPI_FLAG_TXE))
		;
	HAL_SPI_Transmit(hsd->hspi, buffer, len, SPI_TIMEOUT);
}
static uint8_t SPI_RxByte(SD_HandleTypeDef *hsd) {
	//DONE?
	uint8_t dummy, data;
	dummy = 0xFF;

	while (!__HAL_SPI_GET_FLAG(hsd->hspi, SPI_FLAG_TXE))
		;
	HAL_SPI_TransmitReceive(hsd->hspi, &dummy, &data, 1, SPI_TIMEOUT);

	return data;

}
static void SPI_RxBytePtr(SD_HandleTypeDef *hsd, uint8_t *buff) {
	//DONE?
	*buff = SPI_RxByte(hsd);
}
static uint8_t SD_ReadyWait(SD_HandleTypeDef *hsd, uint32_t timeout) {
	//DONE?
	uint8_t res;

	/* timeout 500ms */
	timer2 = timeout;

	/* if SD goes ready, receives 0xFF */
	do {
		res = SPI_RxByte(hsd);
	} while ((res != 0xFF) && (timer2 <= 0));

	return res;

}
