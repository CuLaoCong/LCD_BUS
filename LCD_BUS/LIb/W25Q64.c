/*
 * W25Q64.c
 *
 *  Created on: July 16, 2025
 *  		Author: LongSoCuTe   
 */
/*
	64Mbit = 8MB = 8 * 1024 * 1024 byte = 8,388,608 byte
	Moi trang (page): 256 byte
	Moi khoi (sector): 4KB (gom 16 pages)
	Moi block lon: 64KB (gom 16 sectors) co 128 block
*/


#include "W25Q64.h"

extern SPI_HandleTypeDef hspi2;
#define W25Q_SPI hspi2
#define numBLOCK 128      //tong so block cua 64Mb flash

/*---------- Tao 2 ham dieu khien chan CSW ------- */

void CS_LOW()
{	
	HAL_GPIO_WritePin(CSW_GPIO_Port, CSW_Pin, GPIO_PIN_RESET);
}

void CS_HIGH()
{ 
	HAL_GPIO_WritePin(CSW_GPIO_Port, CSW_Pin, GPIO_PIN_SET);
}
/*-----------Ham bo tro-------------- */

void W25Q_Delay(uint32_t time)
{
	HAL_Delay(time);
}

void SPI_Write(uint8_t *data, uint8_t len)
{
	HAL_SPI_Transmit(&W25Q_SPI, data, len, 2000);
	
}
void SPI_Read(uint8_t *data, uint32_t len)
{
	HAL_SPI_Receive(&W25Q_SPI, data, len, 5000);

}
/*-------------- Cac Ham W25xx   ------------------ */
/*###################################################*/

//Doc trang thai ghi
uint8_t W25Q_ReadStatus(void)
{
    uint8_t cmd = 0x05;
    uint8_t status;
    CS_LOW();
    SPI_Write(&cmd, 1);
    SPI_Read(&status, 1);
    CS_HIGH();
    return status;
}
//Ham cho ghi du lieu xong
void W25Q_WaitBusy(void)
{
    while (W25Q_ReadStatus() & 0x01) {
        HAL_Delay(1); // doi chip flash hoàn tat
    }
}
void W25Q_Reset(void)
{
		uint8_t tData[2];
		tData[0] = 0x66;  //Enable Reset
		tData[1] = 0x99;	//Reset  
		CS_LOW();
		HAL_SPI_Transmit(&W25Q_SPI, tData, 2, 1000);
		CS_HIGH();
		HAL_Delay(100);
}


uint32_t W25Q_ReadID(void) // Doc ID cua W25Qxx 
{
		uint8_t tData = 0x9F;  //Read JEDEC ID ma nay cua nha phat hanh 
		uint8_t rData[3];
		CS_LOW();
		HAL_SPI_Transmit(&W25Q_SPI, &tData, 1 , 1000);
		HAL_SPI_Receive(&W25Q_SPI, rData, 3 ,3000);
		CS_HIGH();
		return ((rData[0] << 16) | (rData[1] << 8) | rData[2]); // nhu stack dich bit cao
}
/*----------- Ham doc du lieu trong vung nho ------------------ */

// Ham doc du lieu tu 1 vung nho flash 
void W25Q_Read(uint32_t startPage, uint8_t offset, uint32_t size, uint8_t *rData)
{	
	
		//off set la vi tri Byte bat dau doc trong 1 page (off set < 256 vi 1 trang co 256Byte)
		uint8_t tData[5];
		uint32_t memAddr = (startPage*256) + offset;    //Page0 0-255 Page 1 = 1 + 256 + 4
		
		// dia chi cua W25Qxx se la 24bit, dau tien phai doc dia chi roi gui cho W25Qxx xac dinh vung nho
		tData[0] = 0x03;  							 //Enable Read
		tData[1] = (memAddr>>16)&0xFF;   //MSB cua dia chi bo nho (lay byte cao nhat cua bo nho)   
		tData[2] = (memAddr>>8)&0xFF; 	 //VD memAddr = 0x123456   tData[1]  = 0x12  tData[2] = 0x34 tData[3] = 0x56
		tData[3] = (memAddr)&0xFF; 			 //LSB cua dia chi bo nho
		
		CS_LOW();
		SPI_Write(tData, 4); 						 //Gui lenh va dia chi de doc du lieu 
		SPI_Read(rData, size);					 //Doc du lieu trong dia chi da gui
		CS_HIGH();
}


//Ham FastRead la Update cua Read dung de doc nhanh du lieu (toc do SPI > 25MHz)
void W25Q_FastRead(uint32_t startPage, uint8_t offset, uint32_t size, uint8_t *rData)
{	
	
		
		uint8_t tData[5];
		uint32_t memAddr = (startPage*256) + offset;    //Page0 0-255 Page 1 = 1 + 256 + 4
		
		// doc 24bit 
		tData[0] = 0x0B;  //Enable Fast Read
		tData[1] = (memAddr>>16)&0xFF;   //MSB cua dia chi bo nho (lay byte cao nhat cua bo nho)   
		tData[2] = (memAddr>>8)&0xFF; 	 //VD memAddr = 0x123456   tData[1]  = 0x12  tData[2] = 0x34 tData[3] = 0x56
		tData[3] = (memAddr)&0xFF; 			 //LSB cua dia chi bo nho
		tData[4] = 0; 									 //Dummy byte la 1 bit trong de tao delay (bat buoc voi FastRead)
	
		CS_LOW();
		SPI_Write(tData, 5); 		
		SPI_Read(rData, size);					 //Doc du lieu
		CS_HIGH();
}

/*-------------- Ham ghi du lieu/xoa du lieu   ------------------ */


//Bat che do ghi (06h)
void write_enable(void)
{
		uint8_t tData = 0x06;            // enable write
		CS_LOW();
		SPI_Write(&tData, 1);
		CS_HIGH();
		W25Q_Delay(5); 									 // delay 5ms
}

//Tat che do ghi (04h)
void write_disable(void)
{
		uint8_t tData = 0x04;            // disable write
		CS_LOW();
		SPI_Write(&tData, 1);
		CS_HIGH();
		W25Q_Delay(5); 									 // delay 5ms
}

//Ham nay dung khi offset > 266
uint32_t bytestowrite(uint32_t size, uint16_t offset)
{
		if((size+offset)<256) return size;
		else return 256 - offset;
}
uint32_t bytestomodify(uint32_t size, uint16_t offset)
{
		if((size+offset)<4096) return size;
		else return 4096 - offset;
}




//Ham xoa 1 khoi (sector)
void W25Q_Erase_Sector(uint16_t numsector)
{
		uint8_t tData[5];
		uint32_t memAddr = numsector*16*256;    //1 sector = 16 pages x 256 byte	
		write_enable();
		
		tData[0] = 0x20; 								 //Erase sector
		tData[1] = (memAddr>>16)&0xFF;   //MSB cua dia chi bo nho (lay byte cao nhat cua bo nho)   
		tData[2] = (memAddr>>8)&0xFF; 	 //VD memAddr = 0x123456   tData[1]  = 0x12  tData[2] = 0x34 tData[3] = 0x56
		tData[3] = (memAddr)&0xFF; 			 //LSB cua dia chi bo nho
		
		CS_LOW();
		SPI_Write(tData, 4);
		CS_HIGH();
}

//Ham ghi du lieu vao 1 Trang (256byte)
void W25Q_Write_Clean(uint32_t page, uint16_t offset, uint32_t size, uint8_t *data)
{
	
		//Tinh toan so trang va khoi de xoa truoc khi ghi
		//Vi chi xoa theo khoi duoc nen can tinh ra khoi
		uint8_t tData[266];
		uint32_t startPage = page;
		uint32_t endPage = startPage + ((size + offset-1)/256);    // tinh ra endpage
		uint32_t numPages = endPage - startPage + 1; 							 // lo luong page
		
		uint16_t startSector = startPage/16;
		uint16_t endSector = endPage/16;
		uint16_t numSectors = endSector - startSector + 1;
		//xoa truoc khi ghi
		for(uint16_t i=0; i<numPages; i++ )
		{
			W25Q_Erase_Sector(startSector++);	
			W25Q_WaitBusy();			//Xoa bo nho theo Sector
		}
		
		uint32_t dataPosition = 0;
		
		//Write data
		for(uint32_t i=0; i<numPages; i++)
		{
			uint32_t memAddr = (startPage*256) + offset;						//tinh dia chi vung nho
			uint16_t bytesremaining = bytestowrite(size, offset);		//tinh so byte ghi trong page
			uint32_t indx = 0;
			
			write_enable();
			
			tData[0] = 0x02; 								 //page program
			tData[1] = (memAddr>>16)&0xFF;   //MSB cua dia chi bo nho (lay byte cao nhat cua bo nho)   
			tData[2] = (memAddr>>8)&0xFF; 	 //VD memAddr = 0x123456   tData[1]  = 0x12  tData[2] = 0x34 tData[3] = 0x56
			tData[3] = (memAddr)&0xFF; 			 //LSB cua dia chi bo nho
			indx = 4;												 //Day la so phan tu mang da gui (24byte dia chi)
			
			//tinh so phan tu mang de gui (gom data + 4 phan dia chi)
			uint16_t bytestosend = bytesremaining + indx;
			
			for(uint16_t i=0; i<bytesremaining; i++)
			{
				tData[indx++] = data[i+dataPosition];    //ghi du lieu tu mang data vao mang tdata 
			}
			
			
			// gui lenh va data
			if(bytestosend > 250)
			{
				CS_LOW();
				SPI_Write(tData, 100);				//gui dia chi va data ghi vao W25
				SPI_Write(tData + 100, bytestosend - 100);				//gui dia chi va data ghi vao W25
				CS_HIGH();
			}
			else
			{
				// gui lenh va data
				CS_LOW();
				SPI_Write(tData, bytestosend);							//gui dia chi va data ghi vao W25
				CS_HIGH();
			}
			
			
		
			//W25Q_WaitBusy();
			//luu du lieu cho lan sau
			startPage++;
			offset = 0;
			size = size - bytesremaining;
			dataPosition = dataPosition + bytesremaining;
			
			W25Q_Delay(5);
			write_disable();
		}

}
//Ham nay de ghi theo sector tuong tu cai ham ghi vao page
void W25Q_Write(uint32_t page, uint16_t offset, uint32_t size, uint8_t *data)
{
	
		uint16_t startSector = page/16;
		uint16_t endSector = (page + ((size + offset-1)/256))/16;
		uint16_t numSectors = endSector - startSector + 1;
		
		uint8_t previousData[4096];
		uint32_t sectorOffset = ((page%16)*256) + offset;    //tinh offset cua sector
		uint32_t dataindx = 0;
	
		for(uint16_t i=0; i<numSectors; i++ )
		{
			uint32_t startPage = startSector*16;
			W25Q_FastRead(startPage, 0, 4096, previousData);
			
			uint32_t bytesRemaining = bytestomodify(size, sectorOffset);
			for(uint16_t j=0; j<bytesRemaining; j++)
			{
				previousData[j+sectorOffset] = data[j+dataindx];
			}
			
			W25Q_Write_Clean(startPage, 0 , 4096, previousData);
			//W25Q_WaitBusy();
			startSector++;
			sectorOffset = 0;
			dataindx = dataindx + bytesRemaining;
			size = size - bytesRemaining;
		}
}


///////////////////////////////////////////////////////////////////////////////



uint8_t W25Q_Read_Byte (uint32_t Addr)
{
	uint8_t tData[5];
	uint8_t rData;

	tData[0] = 0x03; 							 //enable sector
	tData[1] = (Addr>>16)&0xFF;    //MSB cua dia chi bo nho (lay byte cao nhat cua bo nho)   
	tData[2] = (Addr>>8)&0xFF; 	   //VD memAddr = 0x123456   tData[1]  = 0x12  tData[2] = 0x34 tData[3] = 0x56
	tData[3] = (Addr)&0xFF; 			 //LSB cua dia chi bo nho

	CS_LOW();
	SPI_Write(tData,4);
	SPI_Read(&rData, 1);
	CS_HIGH();
	return rData;


}
void W25Q_Write_Byte (uint32_t Addr, uint8_t data)
{
	uint8_t tData[6];
	uint8_t indx;

	tData[0] = 0x02; 							 //enable sector
	tData[1] = (Addr>>16)&0xFF;    //MSB cua dia chi bo nho (lay byte cao nhat cua bo nho)   
	tData[2] = (Addr>>8)&0xFF; 	   //VD memAddr = 0x123456   tData[1]  = 0x12  tData[2] = 0x34 tData[3] = 0x56
	tData[3] = (Addr)&0xFF; 			 //LSB cua dia chi bo nho
	tData[4] = data;
	
	if(W25Q_Read_Byte(Addr) == 0xFF)
	{
		write_enable();
		CS_LOW();
		SPI_Write(tData,4);
		SPI_Write(tData, indx);
		CS_HIGH();
		W25Q_Delay(5);
	}
}

void W25Q_Write_32B(uint32_t page, uint16_t offset, uint32_t size, uint32_t *data)
{
	uint8_t data8[size*4];
	uint32_t indx = 0;
	for(uint32_t i = 0; i < size; i++)
	{
			data8[indx++] = data[i] & 0xFF;
			data8[indx++] = (data[i]>>8) & 0xFF;
			data8[indx++] = (data[i]>>16) & 0xFF;
			data8[indx++] = (data[i]>>24) & 0xFF;
	
	}
	W25Q_Write(page, offset, indx, data8);

}

void W25Q_Read_32B(uint32_t page, uint16_t offset, uint32_t size, uint32_t *data)
{
	uint8_t data8[size*4];
	uint32_t indx = 0;
	
	W25Q_FastRead(page, offset, size*4, data8);
	for(uint32_t i=0; i<size; i++)
	{
	//	data[i] = (data8[indx++]) | (data8[indx++] << 8) | (data[i] << 16) | (data[i] << 24);
	}
}
///////////////////////////////////////////////////////////////////////////////


// Xoa toan bo flash
void W25Q_ChipErase(void)
{
    uint8_t cmd = 0xC7;  // 
    write_enable();
    CS_LOW();
    SPI_Write(&cmd, 1);
    CS_HIGH();
    W25Q_WaitBusy(); // 
}





