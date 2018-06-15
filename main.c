#include "const_data.h"
#include "global_decls.h"
#include <stdlib.h>
#include "includes.h"
#include <intrinsics.h> // for __no_operation() intrinsic to IAR compiler

void critical_error_handler(OS_ERR err)
{  
   ERR=err;
   BSP_IntDisAll();
   OSSchedLock(&err); // lock down system
   while (DEF_ON)
     BSP_LED_On(ERR_LED);
}

int main()
{
  OS_ERR err;
  
  OSInit(&err);                                               /* Init uC/OS-III.                                      */

  OSTaskCreate((OS_TCB     *)&AppTaskStart_TCB,              /* Create the start task  */                              
                 (CPU_CHAR   *)"App Task Start",
                 (OS_TASK_PTR )AppTaskStart,
                 (void       *)0,
                 (OS_PRIO     )APP_TASK_START_PRIO,
                 (CPU_STK    *)&AppTaskStart_Stk[0],
                 (CPU_STK_SIZE)APP_TASK_START_STK_SIZE / 10,
                 (CPU_STK_SIZE)APP_TASK_START_STK_SIZE,
                 (OS_MSG_QTY  )0,
                 (OS_TICK     )0,
                 (void       *)0,
                 (OS_OPT      )(OS_OPT_TASK_STK_CHK | OS_OPT_TASK_STK_CLR),
                 (OS_ERR     *)&err);
                  if (err != OS_ERR_NONE)
                     critical_error_handler(err);
                  
  OSStart(&err);  /* Start multitasking (i.e. give control to uC/OS-III). */
  
  return 0;
}

static void configurePins()
{
   GPIO_InitTypeDef GPIO_struct;
   
   BSP_PeriphEn(BSP_PERIPH_ID_IOPA);
   BSP_PeriphEn(BSP_PERIPH_ID_IOPC);
  
   // CS PIN select
   GPIO_struct.GPIO_Pin=LCD_CS_PIN;
   GPIO_struct.GPIO_Speed=GPIO_Speed_50MHz;
   GPIO_struct.GPIO_Mode=GPIO_Mode_Out_PP;
   GPIO_Init(LCD_CS_PORT,&GPIO_struct);
   GPIO_struct.GPIO_Pin=LCD_SPI_SCK_PIN;
   GPIO_struct.GPIO_Speed=GPIO_Speed_50MHz;
   GPIO_struct.GPIO_Mode=GPIO_Mode_Out_PP;
   GPIO_Init(LCD_SPI_PORT,&GPIO_struct);
   GPIO_struct.GPIO_Pin=LCD_SPI_DIO_PIN;
   GPIO_struct.GPIO_Speed=GPIO_Speed_50MHz;
   GPIO_struct.GPIO_Mode=GPIO_Mode_Out_PP;
   GPIO_Init(LCD_SPI_PORT,&GPIO_struct);
   GPIO_struct.GPIO_Pin=LCD_RESET_PIN;
   GPIO_struct.GPIO_Speed=GPIO_Speed_50MHz;
   GPIO_struct.GPIO_Mode=GPIO_Mode_Out_PP;
   GPIO_Init(LCD_SPI_PORT,&GPIO_struct);
}
static void StartLCDTask(void *p_arg)
{
   // assertion:CS Low to enable chip
   void * p_storage=(void *)malloc(NUM_MEM_BLOCKS*(sizeof(LCD_SENTINAL)+sizeof(void*)));

   OS_ERR err;
    
    /* Create Task */
    OSTaskCreate((OS_TCB     *)&WriteSPIDataTask_TCB,                                       
                 (CPU_CHAR   *)"Write SPI Data Task",
                 (OS_TASK_PTR )WriteSPIDataTask,
                 (void       *)0,
                 (OS_PRIO     )APP_TASK_WRITE_SPI_PRIO,
                 (CPU_STK    *)&WriteSPI_Stk[0],
                 (CPU_STK_SIZE)APP_TASK_WRITE_SPI_STK_SIZE / 10,
                 (CPU_STK_SIZE)APP_TASK_WRITE_SPI_STK_SIZE,
                 (OS_MSG_QTY  )APP_TASK_WRITE_SPI_Q_SIZE, // setup internal message queue
                 (OS_TICK     )0,
                 (void       *)0,
                 (OS_OPT      )(OS_OPT_TASK_STK_CHK | OS_OPT_TASK_STK_CLR),
                 (OS_ERR     *)&err);
   if (err != OS_ERR_NONE)
      critical_error_handler(err);
                  
   // allocate storage that will remain past the end of this stack
   if (p_storage != (void *)0)
     OSMemCreate(&MyPartition,
                 "Data Sentinal Partition",
                 p_storage,
                 NUM_MEM_BLOCKS,
                 sizeof(LCD_SENTINAL)+sizeof(void*),
                 &err);
   else
     err=OS_ERR_HEAP_EXHAUSTED;
    if (err != OS_ERR_NONE)
      critical_error_handler(err);
  
   
   //enable chip
   GPIO_WriteBit(LCD_CS_PORT,LCD_CS_PIN,Bit_RESET);
   GLCD_SPI_CLK_H();   // CLOCK HIGH
   GPIO_WriteBit(LCD_CS_PORT,LCD_CS_PIN,Bit_RESET);
   
   // RESET LCD
   GPIO_WriteBit(LCD_SPI_PORT,LCD_RESET_PIN,Bit_RESET);
   
    // 100 ms // resolution is mutiples of tick rate
    OSTimeDlyHMSM((CPU_INT16U) 0,
                  (CPU_INT16U) 0,
                  (CPU_INT16U) 0,
                  (CPU_INT32U) 100,
                  (OS_OPT    ) OS_OPT_TIME_HMSM_STRICT,
                  (OS_ERR   *)&err);   
     GPIO_WriteBit(GPIOC,LCD_RESET_PIN,Bit_SET);
     
     // 100 ms // resolution is mutiples of tick rate
    OSTimeDlyHMSM((CPU_INT16U) 0,
                  (CPU_INT16U) 0,
                  (CPU_INT16U) 0,
                  (CPU_INT32U) 100,
                  (OS_OPT    ) OS_OPT_TIME_HMSM_STRICT,
                  (OS_ERR   *)&err);
 
    // post configuration data from table
    for (CPU_INT08U i=0; i < sizeof(LCD_Configure_Data)/sizeof(LCD_Configure_Data[0]); i ++)
    {  
       LCD_SENTINAL * data_to_send=(LCD_SENTINAL*)getMemBlock();
        *data_to_send=LCD_Configure_Data[i];
        
          // post data to task message queue
          OSTaskQPost(&WriteSPIDataTask_TCB,(void *)data_to_send,(OS_MSG_SIZE)sizeof(*data_to_send),
                OS_OPT_POST_FIFO|OS_OPT_POST_NO_SCHED,&err);
          if (err != OS_ERR_NONE)
          {  
             putMemBlock((void * )data_to_send);
             critical_error_handler(err);
          }
      
    }
  
     OSTimeDlyHMSM(0,0,0,250,OS_OPT_TIME_HMSM_STRICT,&err);//delay 250 milliseconds
                                                           // to stabilize power supply
    
    LCD_SENTINAL * dis_on=(LCD_SENTINAL*)getMemBlock();
    dis_on->flag=COMMAND;
    dis_on->data=DISON; // turn on display
    OSTaskQPost(&WriteSPIDataTask_TCB,(void *)dis_on,(OS_MSG_SIZE)sizeof(LCD_SENTINAL),
                OS_OPT_POST_FIFO|OS_OPT_POST_NO_SCHED,&err);
    if (err != OS_ERR_NONE)
    {  
       putMemBlock((void * )dis_on);
       critical_error_handler(err);
    }
    
    OSTaskSemPost(&LCDClearScreenTask_TCB,OS_OPT_POST_NONE,&err); // clear screen
    if (err != OS_ERR_NONE)
       critical_error_handler(err);
    OSTaskDel((OS_TCB *)0,&err); // one-off task-calls scheduler
    
}
static  void  AppTaskStart (void *p_arg)
{
    CPU_INT32U  cnts;
    OS_ERR      err;
    
    /* task information */
    CPU_STK     StartLCD_Stk[APP_TASK_START_LCD_STK_SIZE];
    OS_TCB      StartLCDTask_TCB;
        
   
    BSP_Init();                                                       /* Initialize BSP functions                          */
    CPU_Init();                                                       /* Initialize the uC/CPU services                    */

                                                                          
    cnts          = BSP_CPU_ClkFreq() / (CPU_INT32U)OSCfg_TickRate_Hz;/* Determine nbr SysTick increments                  */
    OS_CPU_SysTickInit(cnts);                                         /* Init uC/OS periodic time src (SysTick).           */

#if OS_CFG_STAT_TASK_EN > 0u
    OSStatTaskCPUUsageInit(&err);                                     /* Compute CPU capacity with no task running         */
#endif
    configurePins();   // configure necessary I/O pins
 //   configureSPI();    // configure SPI that will use pins
    
   /* create global kernel objects */
   OSSemCreate(&QFull,"Q Full Semaphore",APP_TASK_WRITE_SPI_Q_SIZE,&err);
    if (err != OS_ERR_NONE)
      critical_error_handler(err);
   OSSemCreate(&MemFull,"Memory Full Semaphore",NUM_MEM_BLOCKS,&err);
   if (err != OS_ERR_NONE)
      critical_error_handler(err);
   
   /*Create task*/ 
   OSTaskCreate((OS_TCB     *)&StartLCDTask_TCB,                                       
                 (CPU_CHAR   *)"Start LCD Task",
                 (OS_TASK_PTR )StartLCDTask,
                 (void       *)0,
                 (OS_PRIO     )APP_TASK_START_LCD_PRIO,
                 (CPU_STK    *)&StartLCD_Stk[0],
                 (CPU_STK_SIZE)APP_TASK_START_LCD_STK_SIZE / 10,
                 (CPU_STK_SIZE)APP_TASK_START_LCD_STK_SIZE,
                 (OS_MSG_QTY  )0,
                 (OS_TICK     )0,
                 (void       *)0,
                 (OS_OPT      )(OS_OPT_TASK_STK_CHK | OS_OPT_TASK_STK_CLR),
                 (OS_ERR     *)&err);
                  if (err != OS_ERR_NONE)
                     critical_error_handler(err);
                  
                  
                  // create LCD Clear Screen task
                 OSTaskCreate((OS_TCB     *)&LCDClearScreenTask_TCB,                                       
                 (CPU_CHAR   *)"LCDClearScreen",
                 (OS_TASK_PTR )LCDClearScreenTask,
                 (void       *)0,
                 (OS_PRIO     )APP_TASK_LCD_CLR_SCR_PRIO,
                 (CPU_STK    *)&LCDClearScreen_Stk[0],
                 (CPU_STK_SIZE)APP_TASK_LCD_CLR_SCR_STK_SIZE / 10,
                 (CPU_STK_SIZE)APP_TASK_LCD_CLR_SCR_STK_SIZE,
                 (OS_MSG_QTY  )0,
                 (OS_TICK     )0,
                 (void       *)0,
                 (OS_OPT      )(OS_OPT_TASK_STK_CHK | OS_OPT_TASK_STK_CLR),
                 (OS_ERR     *)&err);
                   if (err != OS_ERR_NONE)
                     critical_error_handler(err);
                   
     BSP_LED_Off(0);

    while (DEF_ON) {                                                  /* Task body, always written as an infinite loop.    */
        BSP_LED_Toggle(APP_START_LED);
        OSTimeDlyHMSM(0, 0, 0, 500,
                      OS_OPT_TIME_HMSM_STRICT,
                      &err);
    }
}
static void LCDClearScreenTask(void *p_arg)
{
    CPU_TS ts;
    OS_ERR err;
    const LCD_SENTINAL lcd_set_scr_dims[]={{.flag=COMMAND,.data=PASET},
    {.flag=DATA, .data=0},
    {.flag=DATA, .data=ENDPIXEL},
    {.flag=COMMAND, .data=CASET},
    {.flag=DATA, .data=0},
    {.flag=DATA, .data=ENDPIXEL},
    {.flag=COMMAND, .data=RAMWR}};
   
    LCD_SENTINAL lcd_pixel_set[]={{.flag=COMMAND,.data=PASET},
    {.flag=DATA, .data=65},
    {.flag=DATA, .data=ENDPIXEL},
    {.flag=COMMAND,.data=CASET},
    {.flag=DATA, .data=65},
    {.flag=DATA, .data=ENDPIXEL/2},
    {.flag=COMMAND, .data=RAMWR}};
    
    while (DEF_ON)
    {  
        // wait for task to be signaled
        OSTaskSemPend(0,OS_OPT_PEND_BLOCKING,&ts,&err);
        
        if (err == OS_ERR_NONE)
        {
           for (CPU_INT08U i=0; i < sizeof(lcd_set_scr_dims)/sizeof(lcd_set_scr_dims[0]); i++)
           {
              LCD_SENTINAL* lcd_info=(LCD_SENTINAL *)getMemBlock();
              *lcd_info=lcd_set_scr_dims[i];
              
              // send message
              sendMessage((void *)lcd_info);
              
              
           }
           LCD_SENTINAL* lcd_color;
           for (CPU_INT32U i=0;i < ((ENDPIXEL+1)*(ENDPIXEL+1)/2+1); i++)
           {
              lcd_color=(LCD_SENTINAL*)getMemBlock();
              lcd_color->flag=DATA;
              lcd_color->data=BLACK;
              
                // send message
              sendMessage((void *)lcd_color);          
           }
        }
        else
           critical_error_handler(err);

        // draw white box
        for (CPU_INT08U i=0; i < sizeof(lcd_pixel_set)/sizeof(lcd_pixel_set[0]); i++)
           {
              LCD_SENTINAL* lcd_info=(LCD_SENTINAL *)getMemBlock();
              *lcd_info=lcd_pixel_set[i];
              
              // send message
              sendMessage((void *)lcd_info);
           }
           LCD_SENTINAL* lcd_color;
           for (CPU_INT32U i=0;i < ((ENDPIXEL-65+1)*(ENDPIXEL-65+1)/2+1); i++)
           {
              lcd_color=(LCD_SENTINAL*)getMemBlock();
            
              lcd_color->flag=DATA;
              lcd_color->data=WHITE;
               
                // send message
              sendMessage((void *)lcd_color);
           }
    }
}    
static void WriteSPIDataTask(void *p_arg)
{
    LCD_SENTINAL * lcd_data;
       
    // pend on message from internal message queue
    // message contains structure with enum indicating COMMAND or DATA
    // function pointer to function such as void send9thbit(flag)
    // where flag indicates command or data
  
    // pend on data to send 
    while (DEF_ON)
    {
           lcd_data=(LCD_SENTINAL*)getMessage();
           if (lcd_data->flag == DATA) // set 9th bit high to indicate LCD data coming
               lcd_data->data|= 1UL << (SPI_DATA_WIDTH-1);
           for (CPU_INT32U mask = 1UL << (SPI_DATA_WIDTH-1); mask; mask>>= 1)
           {
              // Clock Low
              GLCD_SPI_CLK_L();
              // Set Data
              if (mask & lcd_data->data)
              {
                 GLCD_SPI_MOSI_H();
              }
              else
              {
                 GLCD_SPI_MOSI_L();
              }
             // Clock High
             __no_operation();
             __no_operation();
             __no_operation();         
             GLCD_SPI_CLK_H();
             __no_operation();
             __no_operation();
             __no_operation();
           }
        putMemBlock((void * )lcd_data);
    }
} 
void * getMessage()
{
   OS_ERR err;
   CPU_TS ts;
   OS_MSG_SIZE msg_size;
   
   void*ptr=OSTaskQPend(0,OS_OPT_PEND_BLOCKING,&msg_size,&ts,&err);
   if (err != OS_ERR_NONE)
      critical_error_handler(err);
    
    OSSemPost(&QFull,OS_OPT_POST_1,&err);  // signal room in message queue
    if (err != OS_ERR_NONE)
      critical_error_handler(err);

    return ptr;
}

void sendMessage(void * data)
{
   CPU_TS ts;
   OS_ERR err;
   
   // wait till room in Msg Q
   OSSemPend(&QFull,0,OS_OPT_PEND_BLOCKING,&ts,&err);
    if (err != OS_ERR_NONE)   
       critical_error_handler(err);
   OSTaskQPost(&WriteSPIDataTask_TCB,data,(OS_MSG_SIZE)sizeof(LCD_SENTINAL),
                OS_OPT_POST_FIFO,&err);
    if (err != OS_ERR_NONE)   
       critical_error_handler(err);
}

void * getMemBlock()
{
   void * ptr;
   CPU_TS ts;
   OS_ERR err;
   OSSemPend(&MemFull,0,OS_OPT_PEND_BLOCKING,&ts,&err);
   if (err != OS_ERR_NONE)
      critical_error_handler(err);
   ptr=OSMemGet(&MyPartition,&err);
   if (err != OS_ERR_NONE)
      critical_error_handler(err);
   return ptr;
}

void putMemBlock(void* ptr)
{
   OS_ERR err;
   
   OSMemPut(&MyPartition,ptr,&err);
   if (err!= OS_ERR_NONE)
       critical_error_handler(err);
   OSSemPost(&MemFull,OS_OPT_POST_1,&err); // signal memory is free
   if (err!= OS_ERR_NONE)
       critical_error_handler(err);
}