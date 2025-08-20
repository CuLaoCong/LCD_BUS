#include "menu.h"
#include "ili9341.h"
#include "W25Q64.h"
  
#define TUYEN_DB_COUNT (sizeof(tuyen_db)/sizeof(tuyen_db[0]))

#define TUYEN_FLASH_PAGE   0
#define TUYEN_FLASH_BASE_ADDR  (TUYEN_FLASH_PAGE * 256)
#define TUYEN_SIZE             6
#define TUYEN_TOTAL_COUNT      99
typedef struct {
    uint8_t so;
    char ten[16];
} TuyenDB;

TuyenDB tuyen_db[] = {
    { 80, "TUYEN GBTT" },
    { 81, "TUYEN NTT" }
};

/* ------------- Chong rung nut bam ------------- */
#define IS_PRESSED(port,pin)  (HAL_GPIO_ReadPin((port),(pin))==GPIO_PIN_RESET)

static void wait_release(GPIO_TypeDef *port, uint16_t pin)
{
    while(IS_PRESSED(port,pin));
}


const MenuItem menu3[] = {
    //{ "Khong biet ghi gi3", NULL, NULL, NULL },
    //{ "Luot di",  NULL, NULL, TaskA },
    //{ "Luot ve",  NULL, NULL, TaskA },
    { NULL, NULL, NULL, NULL } // terminator
};     
MenuItem menu3_dynamic[4];      // 0: header, 1: di, 2: ve, 3: NULL
static char MaDi[16];
static char MaVe[16];
static char label_tuyen[32];  

/* ----------------- Lop 2 (7 items) ----------------- */
const MenuItem menu2[] = {							
    { NULL, NULL, NULL, NULL }
};
MenuItem menu2_dynamic[MAX_TUYEN + 1];  // +1 cho NULL ket thúc
char labels[MAX_TUYEN][16];
char mas[MAX_TUYEN][6];
/* ----------------- Level 1 ----------------- */
const MenuItem menu1[] = {
    { "1.CHON TUYEN", menu2_dynamic, NULL, NULL },
    { "2.XE VE GARA", NULL,  "0XVGR", Task_SendMa  },
    { "3.XE HUY DONG", NULL,  "0XVGR", Task_SendMa  },
    { NULL, NULL, NULL, NULL }
};
/* ----------------------------- Dynamic menu for menu3 ----------------------------- */




void mPrint(const char* format, ...)
{
    char buff[128];  // vùng d?m in
    va_list args;
    va_start(args, format);
    int len = vsnprintf(buff, sizeof(buff) - 3, format, args); // -3 d? ch?a ch? \r\n\0
    va_end(args);

    if (len > 0 && len < sizeof(buff) - 3)
    {
        buff[len++] = '\r';   // Thêm \r
        buff[len++] = '\n';   // Thêm \n
        buff[len] = '\0';     // K?t thúc chu?i

        HAL_UART_Transmit(&huart1, (uint8_t*)buff, len, 1000);
    }
}
void SendRaw(const char* s)
{
    HAL_UART_Transmit(&huart1, (uint8_t*)s, strlen(s), HAL_MAX_DELAY);
}
void Task_SendMa(const char* ma)
{
    if (ma) {
        SendRaw(ma);  // Không dùng mPrint n?a
    }
}


//man hinh khi khoi dong lcd
void LCD_Start(){
	
	ILI9341_FillScreen(ILI9341_WHITE);
	//HAL_Delay(50);
	ILI9341_DrawImageFromFlash(27, 185, 187, 25, 50); 
	ILI9341_DrawImageFromFlash(70, 70, 100, 95, 100);
	//ILI9341_DrawImage(0,20,240,240,busmap_logo);
	ILI9341_WriteString(18, 280, "MENU", Font_16x26, ILI9341_BLACK, ILI9341_WHITE);
	//ILI9341_WriteString(186, 294, "ESC", Font_16x24, ILI9341_WHITE, ILI9341_BLACK);
}

//Menu 
void LCD_Menu(){
	ILI9341_WriteString(18, 280, "OK", Font_16x26, ILI9341_BLACK, ILI9341_WHITE);
	ILI9341_WriteString(182, 280, "ESC", Font_16x26, ILI9341_BLACK, ILI9341_WHITE);
}
uint8_t Menu_CountItems(const MenuItem *menu) {
    uint8_t count = 0;
    while (menu[count].label) ++count;
    return count;
}

//Ve menu 
static void TFT_DrawMenu(const MenuItem *menu, uint8_t hilite)
{
    ILI9341_FillScreen(ILI9341_WHITE);

    FontDef font = Font_16x26;
    uint16_t lineHeight = font.height + 4;

    for (uint8_t line = 0, i = firstVisibleIdx; line < MAX_VISIBLE_LINES && menu[i].label; ++line, ++i)
    {
        uint16_t y = 10 + line * lineHeight;

        if (i == hilite) {
            ILI9341_FillRectangle(0, y - 2, ILI9341_WIDTH, lineHeight, ILI9341_CYAN);
            ILI9341_WriteString(10, y, menu[i].label, font, ILI9341_BLACK, ILI9341_CYAN);
        } else {
            ILI9341_WriteString(10, y, menu[i].label, font, ILI9341_BLACK, ILI9341_WHITE);
        }
				LCD_Menu();
    }
}
//Cai nay dung de len xuong k phai fillscreen man minh
void TFT_UpdateLine(const MenuItem *menu, uint8_t idx, uint8_t highlight)
{
    if (!menu[idx].label) return;

    FontDef font = Font_16x26;
    uint16_t lineHeight = font.height + 4;
    uint8_t line = idx - firstVisibleIdx;
    uint16_t y = 10 + line * lineHeight;

    if (highlight) {
        ILI9341_FillRectangle(0, y - 2, ILI9341_WIDTH, lineHeight, ILI9341_CYAN);
        ILI9341_WriteString(10, y, menu[idx].label, font, ILI9341_BLACK, ILI9341_CYAN);
    } else {
        ILI9341_FillRectangle(0, y - 2, ILI9341_WIDTH, lineHeight, ILI9341_WHITE);
        ILI9341_WriteString(10, y, menu[idx].label, font, ILI9341_BLACK, ILI9341_WHITE);
    }
}

//Dung cho menu 3 vi co header
void TFT_DrawMenu3(const MenuItem *menu, uint8_t hilite)
{
    ILI9341_FillScreen(ILI9341_WHITE);
    FontDef font = Font_16x26;
    uint16_t lineHeight = font.height + 4;

    for (uint8_t i = 0, line = 0; menu[i].label; ++i, ++line) {
        uint16_t y = 10 + line * lineHeight;

        if (i == hilite && i != 0) {  // ?? Không highlight dòng 0
            ILI9341_FillRectangle(0, y - 2, ILI9341_WIDTH, lineHeight, ILI9341_CYAN);
            ILI9341_WriteString(10, y, menu[i].label, font, ILI9341_BLACK, ILI9341_CYAN);
        } else {
            ILI9341_WriteString(10, y, menu[i].label, font, ILI9341_BLACK, ILI9341_WHITE);
        }
    }
}


// Hàm thay the chuoi `from` ? cuoi `str` bang chuoi `to`  -->>> copy gpt
void replaceAtEnd(char *str, const char *from, const char *to) {
    size_t len_str = strlen(str);
    size_t len_from = strlen(from);
    size_t len_to = strlen(to);

    if (len_str >= len_from && strcmp(str + len_str - len_from, from) == 0) {
        strcpy(str + len_str - len_from, to);
    }
}

/* ---------------- Xu ly cac nut bam ---------------- */


void Menu_HandleButtons(void)
{	

	//Ban dau chua bam nut gi se o menu chinh 
	if (!inMenu)
    {
        if(IS_PRESSED(OK_GPIO_Port, OK_Pin))
        {
            HAL_Delay(50);
            if(IS_PRESSED(OK_GPIO_Port, OK_Pin))
            {
                wait_release(OK_GPIO_Port, OK_Pin);
                inMenu = 1;
                curMenu = menu1;
                curIdx = 0;
								firstVisibleIdx = 0;
								//TFT_DrawHeader(curMenu[0].label);
								TFT_DrawMenu(curMenu, curIdx);
            }
        }
        return;
    }
    /* ---- OK ---- */
     if(IS_PRESSED(OK_GPIO_Port, OK_Pin))
    {
        HAL_Delay(50);
        if(IS_PRESSED(OK_GPIO_Port, OK_Pin))
        {
            wait_release(OK_GPIO_Port, OK_Pin);
            const MenuItem *item = &curMenu[curIdx];
						//kiem tra trang thai tiep
            if(item->child)
            {
                if(depth < MAX_DEPTH)
                {
                    menuStack[depth++] = curMenu;
											
                    if(item->child == menu3) // task 3 xu ly hoi cang
                    {		
											
											//copy ten tuyen de luu vao bien toan cuc
											snprintf(label_tuyen, sizeof(label_tuyen), "%s", item->label);

											     // Tao mã di và mã ve										
												if (item->ma) {
														snprintf(MaDi, sizeof(MaDi), "%s", item->ma);
														snprintf(MaVe, sizeof(MaVe), "%s", item->ma);

														// Thay R3 ? 01 và 02
														replaceAtEnd(MaDi, "R3", "01");
														replaceAtEnd(MaVe, "R3", "02");
												}

												//tao menu 3
                        menu3_dynamic[0] = (MenuItem){ label_tuyen, NULL, NULL, NULL };
                        menu3_dynamic[1] = (MenuItem){ "CHIEU DI", NULL, MaDi, Task_SendMa };
                        menu3_dynamic[2] = (MenuItem){ "CHIEU VE", NULL, MaVe, Task_SendMa };
                        menu3_dynamic[3] = (MenuItem){ NULL, NULL, NULL, NULL };

                        curMenu = menu3_dynamic;
												curIdx = 1;              // Không chon dòng 0
												firstVisibleIdx = 0;     // Nhung van ve tu dòng 0
												TFT_DrawMenu3(curMenu, curIdx);  // ?? Goi hàm riêng
												
                    }
                    else
                    {		//khong phai lop 3 thi cu hien thi ra thoi
                        curMenu = item->child;
                        //curIdx = 1;
												//firstVisibleIdx = 1;
												//TFT_DrawHeader(curMenu[0].label);
                        //TFT_DrawMenu(curMenu, curIdx);
												curIdx = 0;
												firstVisibleIdx = 0;
												//TFT_DrawHeader(curMenu[0].label);
                        TFT_DrawMenu(curMenu, curIdx);
                }
                    } 	
            }
           if(item->action != NULL)
            {
                item->action(item->ma); // hien tai không goi Task
            }
        }
    }

    /* ---- ESC ---- */
    if(IS_PRESSED(ESC_GPIO_Port, ESC_Pin))
    {
        HAL_Delay(50);
        if(IS_PRESSED(ESC_GPIO_Port, ESC_Pin))
        {
            wait_release(ESC_GPIO_Port, ESC_Pin);
            if(depth)
            {		// chon menu trong stack
                curMenu = menuStack[--depth];
                curIdx  = 0;  // dong 0 
								firstVisibleIdx = 0;
								//TFT_DrawHeader(curMenu[0].label);
                TFT_DrawMenu(curMenu, curIdx);
            }
            else
            {
                /*Quay tro ve man hinh chinh khi chua lam gi */
              inMenu = 0;       /* QUAN TR?NG: dánh dau dã thoát menu */
							curIdx  = 0;
							firstVisibleIdx = 1;
							LCD_Start();
            }
        }
    }

    /* ---- UP / DOWN ---- */
      // UP
    if (IS_PRESSED(UP_GPIO_Port, UP_Pin))
    {
           HAL_Delay(50);
    if (IS_PRESSED(UP_GPIO_Port, UP_Pin)) {
        wait_release(UP_GPIO_Port, UP_Pin);
        lastIdx = curIdx;
				//Xu ly menu2 la cai chon tuyen
        if (curMenu == menu2_dynamic) {
            uint8_t count = Menu_CountItems(curMenu);

            if (curIdx == 0) {
                curIdx = count - 1;
                firstVisibleIdx = (curIdx / MAX_VISIBLE_LINES) * MAX_VISIBLE_LINES;
                TFT_DrawMenu(curMenu, curIdx);
            } else {
                --curIdx;

                if (curIdx < firstVisibleIdx) {
                    firstVisibleIdx -= MAX_VISIBLE_LINES;
                    TFT_DrawMenu(curMenu, curIdx);
                } else {
                    TFT_UpdateLine(curMenu, lastIdx, 0);
                    TFT_UpdateLine(curMenu, curIdx, 1);
                }
            }
           }
						//Xu ly menu 3
						else if (curMenu == menu3_dynamic) {
            lastIdx = curIdx;
            curIdx = (curIdx == 1) ? 2 : 1;
            TFT_UpdateLine(curMenu, lastIdx, 0);
            TFT_UpdateLine(curMenu, curIdx, 1);
						}
            else
            {
                if (curIdx == 0) {
                    while (curMenu[curIdx + 1].label) ++curIdx;
                    firstVisibleIdx = (curIdx >= MAX_VISIBLE_LINES) ? curIdx - MAX_VISIBLE_LINES + 1 : 0;
                    TFT_DrawMenu(curMenu, curIdx);
                } else {
                    --curIdx;
                    if (curIdx < firstVisibleIdx) {
                        firstVisibleIdx = curIdx;
                        TFT_DrawMenu(curMenu, curIdx);
                    } else {
                        TFT_UpdateLine(curMenu, lastIdx, 0);
                        TFT_UpdateLine(curMenu, curIdx, 1);
                    }
                }
            }
        }
    }

    // DOWN
    if (IS_PRESSED(DOWN_GPIO_Port, DOWN_Pin))
    {
            HAL_Delay(50);
    if (IS_PRESSED(DOWN_GPIO_Port, DOWN_Pin)) {
        wait_release(DOWN_GPIO_Port, DOWN_Pin);
        lastIdx = curIdx;

        if (curMenu == menu2_dynamic) {
            uint8_t count = Menu_CountItems(curMenu);

            if (!curMenu[curIdx + 1].label) {
                curIdx = 0;
                firstVisibleIdx = 0;
                TFT_DrawMenu(curMenu, curIdx);
            } else {
                ++curIdx;

                if (curIdx >= firstVisibleIdx + MAX_VISIBLE_LINES) {
                    firstVisibleIdx += MAX_VISIBLE_LINES;
                    TFT_DrawMenu(curMenu, curIdx);
                } else {
                    TFT_UpdateLine(curMenu, lastIdx, 0);
                    TFT_UpdateLine(curMenu, curIdx, 1);
                }
            }
				}else if (curMenu == menu3_dynamic) {
            lastIdx = curIdx;
            curIdx = (curIdx == 2) ? 1 : 2;
            TFT_UpdateLine(curMenu, lastIdx, 0);
            TFT_UpdateLine(curMenu, curIdx, 1);
        }
            else
            {
                if (!curMenu[curIdx + 1].label) {
                    curIdx = 0;
                    firstVisibleIdx = 0;
                    TFT_DrawMenu(curMenu, curIdx);
                } else {
                    ++curIdx;
                    if (curIdx >= firstVisibleIdx + MAX_VISIBLE_LINES) {
                        ++firstVisibleIdx;
                        TFT_DrawMenu(curMenu, curIdx);
                    } else {
                        TFT_UpdateLine(curMenu, lastIdx, 0);
                        TFT_UpdateLine(curMenu, curIdx, 1);
                    }
                }
            }
        }
    }
}
void Write_MaToFlash(void){
		char all_ma[TUYEN_TONG * TUYEN_SIZE];
		for (uint8_t i = 0; i < TUYEN_TONG; i++) {
        snprintf(&all_ma[i * TUYEN_SIZE], TUYEN_SIZE, "%03dR3", i + 1);
    }
		 W25Q_Write_Clean(1000, 0, sizeof(all_ma), (uint8_t*)all_ma);
};

uint8_t RxData[TUYEN_TONG * TUYEN_SIZE];
void Read_MaTo_Flash(void){

		W25Q_FastRead(1000,0,TUYEN_TONG * TUYEN_SIZE, RxData);

};


void Build_Menu2_From_Flash(const uint8_t *list, uint8_t count)
{
    for (uint8_t i = 0; i < count; i++) 
    {
        uint8_t tuyen = list[i];
        if (tuyen == 0 || tuyen > TUYEN_TOTAL_COUNT) continue;

        // Tính dia chi byte trong Flash
        uint32_t byteAddr = TUYEN_FLASH_BASE_ADDR + (tuyen - 1) * TUYEN_SIZE;
        uint32_t page = byteAddr / 256;
        uint16_t page_offset = byteAddr % 256;

        // Ðoc mã tuyen tu flash
        W25Q_Read(page, page_offset, TUYEN_SIZE - 1, (uint8_t*)mas[i]);
        mas[i][TUYEN_SIZE - 1] = '\0'; 

        // Tao nhãn hien thi
        if (tuyen == 80) {
            snprintf(labels[i], sizeof(labels[i]), "%d. TUYEN GBTT", i + 1);
        }
        else if (tuyen == 81) {
            snprintf(labels[i], sizeof(labels[i]), "%d. TUYEN NTT", i + 1);
        }
				else if (tuyen == 82) {
            snprintf(labels[i], sizeof(labels[i]), "%d. TUYEN 96TT", i + 1);
        }
        else {
            snprintf(labels[i], sizeof(labels[i]), "%d. TUYEN %d", i + 1, tuyen);
        }

        // Gán vào menu dong
        menu2_dynamic[i] = (MenuItem){ labels[i], menu3, mas[i], Task_SendMa };
    }

    // Ket thúc menu
    menu2_dynamic[count] = (MenuItem){ NULL, NULL, NULL, NULL };
}
