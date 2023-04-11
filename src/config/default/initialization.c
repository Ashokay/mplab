/*******************************************************************************
  System Initialization File

  File Name:
    initialization.c

  Summary:
    This file contains source code necessary to initialize the system.

  Description:
    This file contains source code necessary to initialize the system.  It
    implements the "SYS_Initialize" function, defines the configuration bits,
    and allocates any necessary global system resources,
 *******************************************************************************/

// DOM-IGNORE-BEGIN
/*******************************************************************************
* Copyright (C) 2019 Microchip Technology Inc. and its subsidiaries.
*
* Subject to your compliance with these terms, you may use Microchip software
* and any derivatives exclusively with Microchip products. It is your
* responsibility to comply with third party license terms applicable to your
* use of third party software (including open source software) that may
* accompany Microchip software.
*
* THIS SOFTWARE IS SUPPLIED BY MICROCHIP "AS IS". NO WARRANTIES, WHETHER
* EXPRESS, IMPLIED OR STATUTORY, APPLY TO THIS SOFTWARE, INCLUDING ANY IMPLIED
* WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY, AND FITNESS FOR A
* PARTICULAR PURPOSE.
*
* IN NO EVENT WILL MICROCHIP BE LIABLE FOR ANY INDIRECT, SPECIAL, PUNITIVE,
* INCIDENTAL OR CONSEQUENTIAL LOSS, DAMAGE, COST OR EXPENSE OF ANY KIND
* WHATSOEVER RELATED TO THE SOFTWARE, HOWEVER CAUSED, EVEN IF MICROCHIP HAS
* BEEN ADVISED OF THE POSSIBILITY OR THE DAMAGES ARE FORESEEABLE. TO THE
* FULLEST EXTENT ALLOWED BY LAW, MICROCHIP'S TOTAL LIABILITY ON ALL CLAIMS IN
* ANY WAY RELATED TO THIS SOFTWARE WILL NOT EXCEED THE AMOUNT OF FEES, IF ANY,
* THAT YOU HAVE PAID DIRECTLY TO MICROCHIP FOR THIS SOFTWARE.
 *******************************************************************************/
// DOM-IGNORE-END

// *****************************************************************************
// *****************************************************************************
// Section: Included Files
// *****************************************************************************
// *****************************************************************************
#include "definitions.h"
//#include "peripheral/eeprom/plib_eeprom.h"
//#include "services/data/flash/eeprom_nvm/eeprom_nvm.h"

#include "device.h"

/*
 * Latest Macros (currently using this)
 */

//status with 3byte response:   For ESP to know the 3 byte status of PIC32 operating in BL mode
#define ST_STATUS                       (0xb0)
#define RESP_STATUS_BL                  (0x01)  //To be sent by BL Code for 3 byte response
#define RESP_STATUS_APP                 (0x02)  //To be sent by Application Code for 1 byte response

//PICID (devsn)
#define ST_PICID                        (0xb1)



//OTA possible for PIC32 to OTA new app code
#define ST_OTA                          (0xb4)
//#define CMD_OTA                         (0x10)
//#define ET_OTA                          (0x3F)
//#define RESP_OTA_READY                  (0x01)  //To be sent by BL code
//#define RESP_OTA_NOT_READY              (0x00)   //To be sent by application code


//BATCHID 
#define ReadVrsn                      (0xb2)
//#define CMD_BATCHID                     (0x10)
//#define ET_BATCHID                      (0x4F)
#define SendVrsn                      (0xb3)

#define ST_OK                         (0xbb)
//CODEID 
//#define ST_CODEID                        (0x60)
//#define CMD_CODEID                       (0x10)
//#define ET_CODEID                        (0x6F)

//Error
//#define ST_ERR                          (0xFF)
//#define RESP_ERR_BL                     (0xFF)
//#define ET_ERR                          (0xFF)

//#define BL_UART_TIMEOUT_IN_SEC          (1)

//#define BUFFER_SIZE                     (4)


volatile uint64_t   mydevsn;






void        main_getDevsn(void);


//uint8_t     init_isStartTokenValid(volatile uint8_t st);
//bool        init_isCmdValid(volatile uint8_t st, volatile uint8_t cmd, volatile uint8_t et);
void        init_send3ByteRespPacket0(void);
void        init_send3ByteRespPacket1(void);
//void        init_send3ByteRespPacket2(void);
void        init_sendPicidRespPacket(void);
//void        init_readBatchidPacket(void);
//void        init_readCodeidPacket(void);


// ****************************************************************************
// ****************************************************************************
// Section: Configuration Bits
// ****************************************************************************
// ****************************************************************************

/*** DEVCFG0 ***/
#pragma config DEBUG =      OFF
#pragma config JTAGEN =     OFF
#pragma config ICESEL =     ICS_PGx1
#pragma config TRCEN =      OFF
#pragma config BOOTISA =    MIPS32
#pragma config FSLEEP =     OFF
#pragma config DBGPER =     PG_ALL
#pragma config SMCLR =      MCLR_NORM
#pragma config SOSCGAIN =   GAIN_2X
#pragma config SOSCBOOST =  ON
#pragma config POSCGAIN =   GAIN_LEVEL_3
#pragma config POSCBOOST =  ON
#pragma config EJTAGBEN =   NORMAL

/*** DEVCFG1 ***/
#pragma config FNOSC =      SPLL
#pragma config DMTINTV =    WIN_127_128
#pragma config FSOSCEN =    OFF
#pragma config IESO =       ON
#pragma config POSCMOD =    OFF
#pragma config OSCIOFNC =   OFF
#pragma config FCKSM =      CSECME
#pragma config WDTPS =      PS1048576
#pragma config WDTSPGM =    STOP
#pragma config FWDTEN =     OFF
#pragma config WINDIS =     NORMAL
#pragma config FWDTWINSZ =  WINSZ_25
#pragma config DMTCNT =     DMT31
#pragma config FDMTEN =     OFF

/*** DEVCFG2 ***/
#pragma config FPLLIDIV =   DIV_1
#pragma config FPLLRNG =    RANGE_5_10_MHZ
#pragma config FPLLICLK =   PLL_FRC
#pragma config FPLLMULT =   MUL_60
#pragma config FPLLODIV =   DIV_4
//#pragma config FPBDIV   =   DIV_2           // used for EProm Enabling
#pragma config VBATBOREN =  ON
#pragma config DSBOREN =    ON
#pragma config DSWDTPS =    DSPS32
#pragma config DSWDTOSC =   LPRC
#pragma config DSWDTEN =    OFF
#pragma config FDSEN =      ON
#pragma config BORSEL =     HIGH
#pragma config UPLLEN =     OFF

/*** DEVCFG3 ***/
#pragma config USERID =     0xffff
#pragma config FUSBIDIO2 =   ON
#pragma config FVBUSIO2 =  ON
#pragma config PGL1WAY =    ON
#pragma config PMDL1WAY =   ON
#pragma config IOL1WAY =    ON
#pragma config FUSBIDIO1 =   ON
#pragma config FVBUSIO1 =  ON
#pragma config PWMLOCK =  OFF

/*** BF1SEQ ***/
#pragma config TSEQ =       0x0
#pragma config CSEQ =       0xffff


#pragma config CP = 0


// *****************************************************************************
// *****************************************************************************
// Section: Driver Initialization Data
// *****************************************************************************
// *****************************************************************************


// *****************************************************************************
// *****************************************************************************
// Section: System Data
// *****************************************************************************
// *****************************************************************************

// *****************************************************************************
// *****************************************************************************
// Section: Library/Stack Initialization Data
// *****************************************************************************
// *****************************************************************************


// *****************************************************************************
// *****************************************************************************
// Section: System Initialization
// *****************************************************************************
// *****************************************************************************



// *****************************************************************************
// *****************************************************************************
// Section: Local initialization functions
// *****************************************************************************
// *****************************************************************************



/*******************************************************************************
  Function:
    void SYS_Initialize ( void *data )

  Summary:
    Initializes the board, services, drivers, application and other modules.

  Remarks:
 */

void SYS_Initialize ( void* data )
{
    /* Start out with interrupts disabled before configuring any modules */
    __builtin_disable_interrupts();

    CLK_Initialize();

    /* Configure CP0.K0 for optimal performance (cached instruction pre-fetch) */
    __builtin_mtc0(16, 0,(__builtin_mfc0(16, 0) | 0x3));

    /* Configure Wait States and Prefetch */
    CHECONbits.PFMWS = 3;
    CHECONbits.PREFEN = 0;
    
    GPIO_Initialize();
    
    CAN1_Initialize();
    UART1_Initialize();
    
    EEPROM_Initialize();
    CORETIMER_Initialize();
//	UART1_Initialize();
   
    main_getDevsn();
    
    
    CANbus_init_1();
    
    
    //EPROM ===================================================================//
//    int status = DataEEIntit();
     
//    STB_GPIO_RE14_Clear();
    //CAN receiver=============================================================//
    STB_GPIO_RE14_Clear();
    
    
    //CAN transmit=============================================================//
//    uint32_t t_id=0x123;
//    uint8_t SLength;
//    ------------------------//  use this ------------------//
//    uint8_t sdata[1];
//    
//    //To process commands from ESP
//    volatile uint8_t espRequestPacket[3];
//    volatile uint8_t i;
//    volatile uint8_t j;
    volatile uint8_t exit_doWhile_flag = 0;
    volatile uint8_t run_bootloader_flag = 0;
//     uint32_t BatchID=0;
//     uint32_t CodeID=0;
     uint32_t rCodeID=0;
     uint32_t rBatchID=0;
     
    
    int Timeout_counter=0;
    
    //To ensure timeout and start application if no data is received over UART
//    volatile uint32_t timer_timeout_in_ms = 0;
//    volatile uint32_t timer_startCount = 0;
//    volatile uint32_t timer_endCount = 0;
//     volatile uint32_t Timeout_counter;
//    i=0;
//    espRequestPacket[0] = 0;
//    espRequestPacket[1] = 0;
//    espRequestPacket[2] = 0;
     led_GPIO_RE15_Set();
    
    
    
//    timer_timeout_in_ms = (1*1000);//replace 1 to BL_UART_TIMEOUT_IN_SEC
//    timer_endCount = (CORE_TIMER_FREQUENCY/1000)*timer_timeout_in_ms;
//    timer_startCount = _CP0_GET_COUNT();
//    
//    i=0;
//    j=0;
    exit_doWhile_flag = 0;
    run_bootloader_flag = 0;
    
    do
    {
       
       
        bool rstat=0;
        uint32_t id;
        uint8_t rLength; 
        uint8_t rdata[8]={0,0,0,0,0,0,0,0};
        rstat=CANbus_read_1(&id, &rLength, rdata);
        if(rstat == true)
        {
//            UART1_Write((uint8_t*)rdata,5);
//            espRequestPacket[0] = rdata[0];
//            led_GPIO_RE15_Set();
//             
//        {
                    switch(id)
                    {
                        case ST_STATUS: 
                            //Send 3 byte response that PIC32 is in BL mode             
                            init_send3ByteRespPacket0();
                            
                            exit_doWhile_flag = 0;
                            break;
                            
                        case ST_PICID:  
                            //Send response as devsn of Pic32 as PICID
                            init_sendPicidRespPacket();
                            CORETIMER_DelayMs(2000);
                            exit_doWhile_flag = 0;
                            break;
                            
//                        case ReadVrsn:  
//                            //Send response as devsn of Pic32 as PICID
//                           
//                             BatchID |= (uint32_t)rdata[0]<<24;
//                             BatchID |= (uint32_t)rdata[1]<<16;
//                             BatchID |= (uint32_t)rdata[2]<<8;
//                             BatchID |= (uint32_t)rdata[3];
//                             
//                             bool bstat=0;
//                             bstat = EEPROM_WordWrite(0xBD000BCC, BatchID);
//                             if(bstat==true)
//                             {
////                               uint8_t sdata3[3];
////                               sdata3[0]=ST_BATCHID;
////                               sdata3[1]=CMD_BATCHID;
////                               sdata3[2]=ET_BATCHID;
////                               CANbus_write_1(0x40, 1, sdata3);
////                               UART1_Write((uint8_t*)sdata3,3);
//                             }
//                             
//                             CodeID |= (uint32_t)rdata[4]<<24;
//                             CodeID |= (uint32_t)rdata[5]<<16;
//                             CodeID |= (uint32_t)rdata[6]<<8;
//                             CodeID |= (uint32_t)rdata[7];
//                             
//                             bool cstat=0;
//                             cstat = EEPROM_WordWrite(0xBD000BD0, CodeID);
//                             if(cstat==true)
//                             {
//                              uint8_t sdata4[1];
//                              sdata4[0]=ST_OK;
////                              sdata4[1]=CMD_CODEID;
////                              sdata4[2]=ET_CODEID;
//                              CANbus_write_1(ReadVrsn, 1, sdata4);
////                              UART1_Write((uint8_t*)sdata4,3);
//                             }
//                            
////                                 CANbus_write_1(ReadVrsn, 1, sdata4);
////                             init_readBatchidPacket();
//                            exit_doWhile_flag = 0;
//                            break;
                            
                            
                        case SendVrsn:
                            
//                             bool mstat=0;
//                             mstat=EEPROM_WordRead( 0xBD000BCC, &rBatchID );
//                             if(mstat==true)
//                             {
//                                 
////            
////                              UART1_Write((uint8_t*)m,4);
//                             
//                             }
                             EEPROM_WordRead( 0xBD000BCC, &rBatchID );
                             bool m1stat=0;
                             m1stat=EEPROM_WordRead( 0xBD000BD0, &rCodeID );
                             if(m1stat==true)
                             {
//                                
////            
////                              UART1_Write((uint8_t*)m,4);
//                             
                             }
                             uint8_t m[8];
                                  m[0]=(rBatchID >> 24) & 0xFF;
                                  m[1]=(rBatchID >> 16) & 0xFF;
                                  m[2]=(rBatchID >> 8) & 0xFF;
                                  m[3]= rBatchID & 0xFF;
//                                  CANbus_write_1(0x123, 4, m);

                                  m[4]=(rCodeID >> 24) & 0xFF;
                                  m[5]=(rCodeID >> 16) & 0xFF;
                                  m[6]=(rCodeID >> 8) & 0xFF;
                                  m[7]= rCodeID & 0xFF;
                                  CANbus_write_1(SendVrsn, 8, m);
                                  
//                                  EEPROM_PageErase(0xBD000BCC);
//                                  EEPROM_PageErase(0xBD000BD0);
                             
                            
                            
                            
                             exit_doWhile_flag = 0;
                            break;
                            
                        case ST_OTA:    
                            //Send response PIC32 BL can do ota
                            
                            if(rdata[0]==0x30)
                            {
                            //exit doWhile loop to start OTA
                             init_send3ByteRespPacket1();
                            exit_doWhile_flag = 1;
                            run_bootloader_flag = 1;
                            }
                            else
                            {
                                exit_doWhile_flag = 0;
                            
                            }
                            //In case of application code, it will run automatically when no uart bytes are rx within timeout.
                            break;
                            
                        default:    
                            //send error response
//                            init_send3ByteRespPacket2();
                            
                            exit_doWhile_flag = 0;
                            break;
                    }
//                }
//                else    
//                {
//                    //send error response
//                    init_send3ByteRespPacket2();
//                    exit_doWhile_flag = 0;
//                }
//                
//                
//                i=0;j=0;
//                espRequestPacket[0] = 0;
//                timer_startCount = _CP0_GET_COUNT();
//          
        }
        else if(Timeout_counter >= 40000000)
        {
            
//                 led_GPIO_RE15_Clear();
                Timeout_counter=0;
                exit_doWhile_flag = 1;    //uncomment this line for final use
        }
//            
//            if((_CP0_GET_COUNT()-timer_startCount)>timer_endCount)
//            {
//                exit_doWhile_flag = 1;    //uncomment this line for final use
//            }
        Timeout_counter++;
//        
    }while(exit_doWhile_flag != 1);
    
    
    
    
    if(!run_bootloader_flag)    
    {
        led_GPIO_RE15_Clear();
        //run application if run_bootloader_flag is clear
        run_Application();
    }      
    
    //run bootloader if run_bootloader_flag is set 
   
    NVM_Initialize();

    EVIC_Initialize();
}



void init_send3ByteRespPacket0(void)
{
    
    uint8_t sdata0[1];
    sdata0[0]=RESP_STATUS_BL;

    CANbus_write_1(ST_STATUS, 1, sdata0); 
    UART1_Write((uint8_t*)sdata0,1);
}
void init_send3ByteRespPacket1(void)
{
    
    uint8_t sdata1[1];
    sdata1[0]=ST_OTA;
    CANbus_write_1(ST_OTA, 1, sdata1);
    UART1_Write((uint8_t*)sdata1,1);
}
//void init_send3ByteRespPacket2(void)
//{
//    
//    uint8_t sdata2[1];
////    init_send3ByteRespPacket(ST_ERR,RESP_ERR_BL,ET_ERR); 
//    sdata2[0]=ST_ERR;
//   
//    CANbus_write_1(0xFF, 1, sdata2);                       
//}


void init_sendPicidRespPacket(void)
{
    unsigned char *d = (unsigned char *)&mydevsn;

    uint8_t MyDevPicId[8];
//    sdata_picid_st[0]=ST_PICID;
    MyDevPicId[0]=*d++;
    MyDevPicId[1]=*d++;
    MyDevPicId[2]=*d++;
    MyDevPicId[3]=*d++;
    MyDevPicId[4]=*d++;
    MyDevPicId[5]=*d++;
    MyDevPicId[6]=*d++;
    MyDevPicId[7]=*d++;
//    sdata_picid_et[0]=ET_PICID;
   
    
//    CANbus_write_1(0x20,1,sdata_picid_st);
    CORETIMER_DelayUs(90);
    CANbus_write_1(ST_PICID,8,MyDevPicId);
//    CORETIMER_DelayUs(90);
//    CANbus_write_1(0x123,1,sdata_picid_et);
//    UART1_Write((uint8_t*)sdata_picid_st,1);
//     CORETIMER_DelayUs(90);
//    UART1_Write((uint8_t*)MyDevPicId,8);
//     CORETIMER_DelayUs(90);
//    UART1_Write((uint8_t*)sdata_picid_et,1);
    

//    CANbus_write(0x123, 2, sdata_picid_et); 
   
}


void main_getDevsn(void)
{
    
    volatile uint32_t mydevsnL;
    volatile uint32_t mydevsnH;
    
    mydevsn     = 0;
    mydevsnL    = DEVSN0;
    mydevsnH    = DEVSN1;
    mydevsn     = mydevsnH; 
    mydevsn     = (mydevsn << 32);
    mydevsn     = (mydevsn + mydevsnL);
}
