/*******************************************************************************
  UART Bootloader Source File

  File Name:
    bootloader.c

  Summary:
    This file contains source code necessary to execute UART bootloader.

  Description:
    This file contains source code necessary to execute UART bootloader.
    It implements bootloader protocol which uses UART peripheral to download
    application firmware into internal flash from HOST-PC.
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

#include "definitions.h"
#include <device.h>

#define FLASH_START             (0x9d000000UL)
#define FLASH_LENGTH            (0x100000UL)
#define PAGE_SIZE               (512UL)
#define ERASE_BLOCK_SIZE        (4096UL)
#define PAGES_IN_ERASE_BLOCK    (ERASE_BLOCK_SIZE / PAGE_SIZE)

#define BOOTLOADER_SIZE         8192

#define APP_START_ADDRESS       (PA_TO_KVA0(0x1d000000UL))

#define GUARD_OFFSET            0
#define CMD_OFFSET              2
#define ADDR_OFFSET             0
#define SIZE_OFFSET             1
#define DATA_OFFSET             1
#define CRC_OFFSET              0

#define BATCH_ID                0
#define CODE_ID                 1

#define CMD_SIZE                1
#define GUARD_SIZE              4
#define SIZE_SIZE               4
#define OFFSET_SIZE             4
#define CRC_SIZE                4
#define HEADER_SIZE             (GUARD_SIZE + SIZE_SIZE)
#define DATA_SIZE               ERASE_BLOCK_SIZE

#define WORDS(x)                ((int)((x) / sizeof(uint32_t)))

#define OFFSET_ALIGN_MASK       (~ERASE_BLOCK_SIZE + 1)
#define SIZE_ALIGN_MASK         (~PAGE_SIZE + 1)

#define BTL_GUARD               (0x5048434DUL)

/* Compare Value to achieve a 100Ms Delay */
#define TIMER_COMPARE_VALUE     (CORE_TIMER_FREQUENCY / 10)

enum
{
    BL_CMD_UNLOCK       = 0xb5,
    BL_CMD_DATA         = 0xb6,
    BL_CMD_VERIFY       = 0xb7,
    BL_CMD_READ_BATCHID = 0xb2,
    BL_CMD_RESET        = 0xb8,
};

enum
{
    BL_RESP_OK          = 0xbb,
    BL_RESP_ERROR       = 0xbc,
//    BL_RESP_ERROR1       = 0x56,
//    BL_RESP_ERROR2       = 0x57,
    BL_RESP_INVALID     = 0xbc,
    BL_RESP_CRC_OK      = 0xba,
    BL_RESP_CRC_FAIL    = 0xbc,
};

static uint32_t CACHE_ALIGN input_buffer[WORDS(OFFSET_SIZE + DATA_SIZE)];

static uint32_t CACHE_ALIGN flash_data[WORDS(DATA_SIZE)];

static uint32_t flash_addr          = 0;

static uint32_t unlock_begin        = 0;
static uint32_t unlock_end          = 0;

static uint32_t  input_command       = 0;

static bool     packet_received     = false;
static bool     flash_data_ready    = false;
static uint8_t sdata[1];


/* Function to Send the final response for reset command and trigger a reset */
static void trigger_Reset(void)
{
//    bool sstat=false;
    sdata[0]=BL_RESP_OK;
    CANbus_write_1(0xbb, 1, sdata);
//    UART1_WriteByte(BL_RESP_OK);

//    while(sstat == false);
  
    //uncomment this while(1) loop later, used only for testing
//    while(1)
//    {
//        led_GPIO_RE15_Toggle();
//        CORETIMER_DelayMs(2000);
//    }


    /* Perform system unlock sequence */ 
    SYSKEY = 0x00000000;
    SYSKEY = 0xAA996655;
    SYSKEY = 0x556699AA;

    RSWRSTSET = _RSWRST_SWRST_MASK;
    (void)RSWRST;
}

/* Function to Generate CRC by reading the firmware programmed into internal flash */
static uint32_t crc_generate(void)
{
    uint32_t   i, j, value;
    uint32_t   crc_tab[256];
    uint32_t   size    = unlock_end - unlock_begin;
    uint32_t   crc     = 0xffffffff;
    uint8_t    data;

    for (i = 0; i < 256; i++)
    {
        value = i;

        for (j = 0; j < 8; j++)
        {
            if (value & 1)
            {
                value = (value >> 1) ^ 0xEDB88320;
            }
            else
            {
                value >>= 1;
            }
        }
        crc_tab[i] = value;
    }

    for (i = 0; i < size; i++)
    {
        data = *(uint8_t *)KVA0_TO_KVA1((unlock_begin + i));

        crc = crc_tab[(crc ^ data) & 0xff] ^ (crc >> 8);
    }
    return crc;
}

/* Function to receive application firmware via UART/USART */
static void input_task(void)
{
    static uint32_t ptr             = 0;
    static uint32_t size            = 0;
    static bool     header_received = false;
    bool rstat=false;
    uint8_t         *byte_buf       = (uint8_t *)&input_buffer[0];
//    uint8_t         input_data[8]={0,0,0,0,0,0,0,0};
    uint32_t r_id=0;
    uint8_t rlength;
    uint8_t rdata[8];
    uint8_t k=0;
   
    
//    int k,m=0;
    

    if (packet_received == true)
    {
        return;
    }
    rstat=CANbus_read_1(&r_id, &rlength, rdata);
   
   if(rstat==false)
    {
         return;
    
    }
      

    /* Check if 100 ms have elapsed */
//    if (CORETIMER_CompareHasExpired())
//    {
//        header_received = false;
//    }

    if (header_received == false)
    {
        for(k=0;k<rlength;k++)
        {
        byte_buf[ptr++] = rdata[k];
        }


        if (ptr == HEADER_SIZE)
        {
            if (input_buffer[GUARD_OFFSET] != BTL_GUARD)
            {
//                UART1_Write((uint8_t*)byte_buf,5);
                sdata[0]=BL_RESP_ERROR;
                CANbus_write_1(BL_RESP_ERROR, 1, sdata);
                UART1_Write((uint8_t*)sdata,1);
//                 led_GPIO_RE15_Clear();
//                UART1_WriteByte(BL_RESP_ERROR);
                ptr=0;
            }
            else
            {
                size            = input_buffer[SIZE_OFFSET];
                input_command   = r_id;
                header_received = true;
            }

            ptr = 0;
        }
    }
    else if (header_received == true)
    {
        if (ptr < size)
        {
           for(k=0;k<rlength;k++)
        {
                byte_buf[ptr++] = rdata[k];
        }

      
        }

        if (ptr == size)
        {
            ptr = 0;
            size = 0;
            packet_received = true;
            header_received = false;
//            led_GPIO_RE15_Set();
        }
    }

//    CORETIMER_Start();
}

/* Function to process the received command */
static void command_task(void)
{
    uint32_t i;
    uint32_t rCodeID=0;
    uint32_t rBatchID=0;

    if (BL_CMD_UNLOCK == input_command)
    {
        uint32_t begin  = (input_buffer[ADDR_OFFSET] & OFFSET_ALIGN_MASK);
//         uint8_t m[4];
//            m[0]=(begin >> 24) & 0xFF;
//            m[1]=(begin >> 16) & 0xFF;
//            m[2]=(begin >> 8) & 0xFF;
//            m[3]= begin & 0xFF;
//            
//            UART1_Write((uint8_t*)m,4);

        uint32_t end    = begin + (input_buffer[SIZE_OFFSET] & SIZE_ALIGN_MASK);

        if (end > begin && end <= (FLASH_START + FLASH_LENGTH))
        {
            unlock_begin = begin;
            unlock_end = end;
            sdata[0]=BL_RESP_OK;
            bool tstat=false;
            tstat=CANbus_write_1(BL_RESP_OK, 1, sdata);
            if(tstat==true)
            {
               
//                led_GPIO_RE15_Set();
//               UART1_Write((uint8_t*)sdata,1);
               
//            UART1_WriteByte(BL_RESP_OK);
            }
        }
        else
        {
            unlock_begin = 0;
            unlock_end = 0;
            sdata[0]=BL_RESP_ERROR;
            bool Tstat=false;
//            CANbus_write1(0x123, 1, sdata);
            Tstat=CANbus_write_1(BL_RESP_ERROR, 1, sdata);
            if(Tstat==true)
            {
                
//               UART1_Write((uint8_t*)sdata,1);
            }
//       
        }
    }
    else if (BL_CMD_DATA == input_command)
    {
        flash_addr = (input_buffer[ADDR_OFFSET] & OFFSET_ALIGN_MASK);

        if (unlock_begin <= flash_addr && flash_addr < unlock_end)
        {
            for (i = 0; i < WORDS(DATA_SIZE); i++)
            {
                flash_data[i] = input_buffer[i + DATA_OFFSET];
            }

            flash_data_ready = true;
//            UART1_Write((uint8_t*)sdata,1);
        }
        else
        {
            sdata[0]=BL_RESP_ERROR;
            CANbus_write_1(BL_RESP_ERROR, 1, sdata);
//            UART1_WriteByte(BL_RESP_ERROR);
        }
    }
    else if (BL_CMD_VERIFY == input_command)
    {
        uint32_t crc        = input_buffer[CRC_OFFSET];
        uint32_t crc_gen    = 0;

        crc_gen = crc_generate();

        if (crc == crc_gen)
        {
            sdata[0]=BL_RESP_CRC_OK;
            CANbus_write_1(BL_RESP_CRC_OK, 1, sdata);
          
        }
//            UART1_WriteByte(BL_RESP_CRC_OK);
        else
        {
            sdata[0]=BL_RESP_CRC_FAIL;
            CANbus_write_1(BL_RESP_CRC_FAIL, 1, sdata);
    
        }
//            UART1_WriteByte(BL_RESP_CRC_FAIL);
    }
    else if (BL_CMD_READ_BATCHID == input_command)
    {
       uint32_t BatchID  = input_buffer[BATCH_ID];

        uint32_t CodeID    =  input_buffer[CODE_ID];

                             
                             bool bstat=0;
                             bstat = EEPROM_WordWrite(0xBD000BCC, BatchID);
                             if(bstat==true)
                             {
//                               uint8_t sdata3[3];
//                               sdata3[0]=ST_BATCHID;
//                               sdata3[1]=CMD_BATCHID;
//                               sdata3[2]=ET_BATCHID;
//                               CANbus_write_1(0x40, 1, sdata3);
//                               UART1_Write((uint8_t*)sdata3,3);
                             }
                             
                             bool cstat=0;
                             cstat = EEPROM_WordWrite(0xBD000BD0, CodeID);
                             if(cstat==true)
                             {
//                              uint8_t sdata4[1];
//                              sdata4[0]=0xbb;
////                              sdata4[1]=CMD_CODEID;
////                              sdata4[2]=ET_CODEID;
//                              CANbus_write_1(0xb2, 1, sdata4);
//                              UART1_Write((uint8_t*)sdata4,3);
                             }
                             
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
                                  CANbus_write_1(0xb2, 8, m);

    }
    else if (BL_CMD_RESET == input_command)
    {
        trigger_Reset();
    }
    else
    {
        sdata[0]=BL_RESP_INVALID;
        CANbus_write_1(BL_RESP_INVALID, 1, sdata);
//        UART1_WriteByte(BL_RESP_INVALID);
    }

    packet_received = false;
}

/* Function to program received application firmware data into internal flash */
static void flash_task(void)
{
    uint32_t addr       = flash_addr;
    uint32_t page       = 0;
    uint32_t write_idx  = 0;


    /* Erase the Current sector */
    NVM_PageErase(addr);

    /* Wait for erase to complete */
    while(NVM_IsBusy() == true);

    for (page = 0; page < PAGES_IN_ERASE_BLOCK; page++)
    {
        NVM_RowWrite(&flash_data[write_idx], addr);

        while(NVM_IsBusy() == true);

        addr += PAGE_SIZE;
        write_idx += WORDS(PAGE_SIZE);
    }

    flash_data_ready = false;
    
    sdata[0]=BL_RESP_OK;
    CANbus_write_1(BL_RESP_OK, 1, sdata);
//    UART1_Write((uint8_t*)sdata,1);

}

void run_Application(void)
{
    uint32_t msp            = *(uint32_t *)(APP_START_ADDRESS);

    void (*fptr)(void);

    /* Set default to APP_RESET_ADDRESS */
    fptr = (void (*)(void))APP_START_ADDRESS;
    
    if (msp == 0xffffffff)
    {
        return;
    }

    fptr();
}

bool __WEAK bootloader_Trigger(void)
{
    if(!sw_GPIO_RB4_Get())    /*Check if esp input is low i.e. new bin file not present*/
    {
        /*run application if it is present in the program flash*/
        /*If application is not present in program flash, then continue with bootloader only*/
        return false;
    }
    else
    {
        /*If switch input is high, means new bin file present. So Directly run bootloader only*/
        return true;
    }
    
    /* Function can be overriden with custom implementation */
    return false;
}

void bootloader_Tasks(void)
{
//    CORETIMER_CompareSet(TIMER_COMPARE_VALUE);
    
//    led_GPIO_RE15_Set();

    while (1)
    {
        input_task();

        if (flash_data_ready)
            flash_task();
        else if (packet_received)
            command_task();
    }
}
