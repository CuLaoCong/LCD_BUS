/*
 * W25Q64.c
 *
 *  Created on: July 16, 2025
 *  		Author: LongSoCuTe   
 */

#ifndef __W25Q64_H
#define __W25Q64_H
#include "main.h"

void W25Q_Reset(void);
uint32_t W25Q_ReadID(void);   //Ham doc 

void W25Q_Read(uint32_t startPage, uint8_t offset, uint32_t size, uint8_t *rData);
void W25Q_FastRead(uint32_t startPage, uint8_t offset, uint32_t size, uint8_t *rData);
void W25Q_Erase_Sector(uint16_t numsector);

void W25Q_Write_Clean(uint32_t page, uint16_t offset, uint32_t size, uint8_t *data);
void W25Q_Write(uint32_t page, uint16_t offset, uint32_t size, uint8_t *data);
void W25Q_WaitBusy(void);

uint8_t W25Q_Read_Byte (uint32_t Addr);
void W25Q_Write_Byte (uint32_t Addr, uint8_t data);
void W25Q_WriteImageToFlash(uint32_t addr, const uint8_t *data, uint32_t size);
void W25Q_Write_32B(uint32_t page, uint16_t offset, uint32_t size, uint32_t *data);
void W25Q_Read_32B(uint32_t page, uint16_t offset, uint32_t size, uint32_t *data);


void W25Q_Write_SectorSafe(uint32_t addr, uint8_t *data, uint32_t size);
void W25Q_ChipErase(void);
void W25Q_Write_SectorSafe_WithOffset(uint32_t startSector, uint16_t offset, uint8_t *data, uint32_t size);



#endif  /* __W25Q64_H */
