#ifndef GLOBAL_DECLS_H
#define GLOBAL_DECLS_H
#include "app_cfg.h"
#include "includes.h"
static            CPU_STK      AppTaskStart_Stk[APP_TASK_START_STK_SIZE];
static            OS_TCB       AppTaskStart_TCB;
static            OS_TCB      WriteSPIDataTask_TCB;
static            CPU_STK     WriteSPI_Stk[APP_TASK_WRITE_SPI_STK_SIZE];
static            CPU_STK     LCDClearScreen_Stk[APP_TASK_LCD_CLR_SCR_STK_SIZE];
static            OS_TCB      LCDClearScreenTask_TCB;
static            OS_SEM      QFull;
static            OS_SEM      MemFull;

  
static  void      AppTaskStart(void *p_arg);
static  void      StartLCDTask(void *p_arg);
static  void      WriteSPIDataTask(void *p_arg);
static  void      LCDClearScreenTask(void *p_arg);
static OS_MEM MyPartition;
void critical_error_handler(OS_ERR);
static void configurePins();
void * getMessage();
void sendMessage(void *);
void * getMemBlock();
void putMemBlock(void *);

// LCD commmands
#define DISON       0xaf
#define DISOFF      0xae
#define DISNOR      0xa6
#define DISINV      0xa7
#define SLPIN       0x95
#define SLPOUT      0x94
#define COMSCN      0xbb
#define DISCTL      0xca
#define PASET       0x75
#define CASET       0x15
#define DATCTL      0xbc
#define RGBSET8     0xce
#define RAMWR       0x5c
#define RAMRD       0x5d
#define PTLIN       0xa8
#define PTLOUT      0xa9
#define RMWIN       0xe0
#define RMWOUT      0xee
#define ASCSET      0xaa
#define SCSTART     0xab
#define OSCON       0xd1
#define OSCOFF      0xd2
#define PWRCTR      0x20
#define VOLCTR      0x81
#define VOLUP       0xd6
#define VOLDOWN     0xd7
#define TMPGRD      0x82
#define EPCTIN      0xcd
#define EPCOUT      0xcc
#define EPMWR       0xfc
#define EPMRD       0xfd
#define EPSRRD1     0x7c
#define EPSRRD2     0x7d
#define NOP         0x25

#define ENDPAGE     132
#define ENDCOL      130
#define ENDPIXEL    131

#define BLACK       0x00
#define WHITE       0xFF

#define LCD_CS_PIN      GPIO_Pin_4
#define LCD_RESET_PIN   GPIO_Pin_13
#define LCD_SPI_DIO_PIN     GPIO_Pin_12
#define LCD_SPI_SCK_PIN     GPIO_Pin_10
#define LCD_CS_PORT     GPIOA
#define LCD_SPI_PORT    GPIOC
#define ERR_LED     1
#define APP_START_LED 2
#define NUM_MEM_BLOCKS  APP_TASK_WRITE_SPI_Q_SIZE
#define SPI_DATA_WIDTH 9

#define GLCD_SPI_MOSI_H() LCD_SPI_PORT->BSRR = LCD_SPI_DIO_PIN
#define GLCD_SPI_MOSI_L() LCD_SPI_PORT->BRR  = LCD_SPI_DIO_PIN
#define GLCD_SPI_CLK_H()  LCD_SPI_PORT->BSRR = LCD_SPI_SCK_PIN
#define GLCD_SPI_CLK_L()  LCD_SPI_PORT->BRR  = LCD_SPI_SCK_PIN

OS_ERR ERR;
typedef enum {COMMAND,DATA} BIT_INDICATOR;
typedef struct {
  BIT_INDICATOR flag;
  CPU_INT32U data;
} LCD_SENTINAL;

#endif