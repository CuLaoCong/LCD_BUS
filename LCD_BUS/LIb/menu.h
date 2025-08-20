#ifndef __MENU_H__
#define __MENU_H__

#include <stdarg.h>
#include <string.h>
#include "stdio.h"
#include <stdbool.h>
#include "main.h"
#include "stm32f1xx_hal.h"
extern UART_HandleTypeDef huart1;


#define TUYEN_SIZE       6         // 5 ký tu + '\0'  vd tuyen 1: 001R3'\n'
#define TUYEN_TONG      99
#define MAX_TUYEN 			20  
//danh  sách tuyen dùng cho thiet bi này
extern uint8_t list[];
#define TONG_TUYEN_SD  sizeof(list) / sizeof(list[0])


/* ---------------------- Menu structures ---------------------- */
typedef struct MenuItem MenuItem;
typedef void (*MenuAction)(const char*ma);

/* ------------- Ham thuc hien lenh ------------- */
void mPrint(const char* format, ...);
void Task_SendMa(const char* ma);
void TaskA(const char*ma);

struct MenuItem {
    const char      *label;   // Chu hien thi len man hinh
    const MenuItem  *child;   // con tro de tro tiep toi -> Menu (NULL neu k di tiep)
		const char 			*ma;			// ma cua tung tuyen	
    MenuAction       action;  // su kien xay ra neu bam nut (NULL neu di tiep nhu tren)
};
/* Cac lop cua giao dien */
extern const MenuItem menu1[], menu2[], menu3[];


/* ---------------- state ---------------- */
#define MAX_DEPTH 5
//#define MAX_VISIBLE_LINES 11
static const MenuItem *menuStack[MAX_DEPTH];
static uint8_t depth = 0;												// so luong trong stack			
static const MenuItem *curMenu = menu1;					// xac dinh xem menu nao
static uint8_t curIdx  = 0;                     // xac dinh vi tri cua dong danh dau (mau xanh a)
static uint8_t inMenu = 0;
static uint8_t firstVisibleIdx = 1;             // xac dinh hien thi dong nao trong menu
static uint8_t lastIdx = 1;

//
#define HEADER_HEIGHT    30
#define SCREEN_HEIGHT    320
#define MAX_VISIBLE_LINES 9  //Toi da cac tuyen hien trong khung hinh

void Build_Menu2_From_Flash(const uint8_t *list, uint8_t count);
void Read_MaTo_Flash(void);
void Write_MaToFlash(void);
void LCD_Start(void);
void LCD_Menu(void);
void Menu_HandleButtons(void);

#endif