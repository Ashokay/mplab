#include <stddef.h>                     // Defines NULL
#include <stdbool.h>                    // Defines true
#include <stdlib.h>                     // Defines EXIT_FAILURE
#include "definitions.h"                // SYS function prototypes
#include <stdio.h>                      // SYS function prototypes
#include "device.h"
#include "stdint.h"
#include "CANbus_Driver_z.h"
#include <sys/kmem.h>


void CANbus_init_1( void )
{
   
    STB_GPIO_RE14_Clear();//Enable CABbus transreciever

}

bool CANbus_write_1(uint32_t Sadr, uint8_t Sdata_L, uint8_t * Sdata)
{
    uint8_t fifoNum;//transmit fifo number used in harmony
     fifoNum=0;
    CAN_TX_RX_MSG_BUFFER *txMessage = NULL;
    uint8_t count = 0;
    bool status = false;

    if ((*(volatile uint32_t *)(&C1FIFOINT0 + (fifoNum * 0x10)) & _C1FIFOINT0_TXNFULLIF_MASK) == _C1FIFOINT0_TXNFULLIF_MASK)
    {
        txMessage = (CAN_TX_RX_MSG_BUFFER *)PA_TO_KVA1(*(volatile uint32_t *)(&C1FIFOUA0 + (fifoNum * 0x10)));

            txMessage->msgSID = Sadr;
            txMessage->msgEID = 0;
            txMessage->msgEID |= Sdata_L;

            while(count < Sdata_L)
            {
                txMessage->msgData[count++] = *Sdata++;
            }
        /* Request the transmit */
        *(volatile uint32_t *)(&C1FIFOCON0SET + (fifoNum * 0x10)) = _C1FIFOCON0_UINC_MASK;
        *(volatile uint32_t *)(&C1FIFOCON0SET + (fifoNum * 0x10)) = _C1FIFOCON0_TXREQ_MASK;

        status = true;
    }
    return status;

}
    

bool CANbus_read_1(uint32_t *msg_id, uint8_t *length, uint8_t *Rdata)
{ 
    uint8_t fifoNum;//receiver fifo number used in harmony
    fifoNum=1;
    CAN_TX_RX_MSG_BUFFER *rxMessage = NULL;
    uint8_t count = 0;
    bool status = false;

    /* Check if there is a message available in FIFO */
    if ((*(volatile uint32_t *)(&C1FIFOINT0 + (fifoNum * 0x10)) & _C1FIFOINT0_RXNEMPTYIF_MASK) == _C1FIFOINT0_RXNEMPTYIF_MASK)
    {
        /* Get a pointer to RX message buffer */
        rxMessage = (CAN_TX_RX_MSG_BUFFER *)PA_TO_KVA1(*(volatile uint32_t *)(&C1FIFOUA0 + (fifoNum * 0x10)));

        *msg_id = rxMessage->msgSID & 0x7FF;
        *length = rxMessage->msgEID & 0xF;
        
        /* Copy the data into the payload */
        while (count < *length)
        {
           *Rdata++ = rxMessage->msgData[count++];
        }
        /* Message processing is done, update the message buffer pointer. */
        *(volatile uint32_t *)(&C1FIFOCON0SET + (fifoNum * 0x10)) = _C1FIFOCON0_UINC_MASK;

        /* Message is processed successfully, so return true */
        status = true;
    }   
    return status;
}
        
    

/*******************************************************************************
 End of File
*/