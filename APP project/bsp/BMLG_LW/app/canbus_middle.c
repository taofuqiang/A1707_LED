/***************************************************************************************************
*                    (c) Copyright 2008-2018  Syncretize technologies co.,ltd.
*										All Rights Reserved
*
*\File          canbus_middle.c
*\Description   
*\Note          
*\Log           2018.08.29    Ver 1.0    Job
*               创建文件。
***************************************************************************************************/
#include "canbus_middle.h"
#include "protocol_anylise_frame.h"
#include "param.h"
#include "iap_if.h"
#include "flash_if.h"
#include "string.h"

//需要根据各个pcb重新编写的函数如下
//extern u32 CAN_Paramset_Read_Construct(u8* buf , u32 len);
//extern u32 CAN_Automatic_Check_Construct(u8* buf, u32 len);
//extern void CAN_Paramset_Receive_Analyse(u8* buf);


/***************************************************************************************************
*\Function      canbus_middle_send
*\Description   CAN数据发送
*\Parameter     can
*\Parameter     ID
*\Parameter     buf
*\Parameter     len
*\Return        u32
*\Note          
*\Log           2018.08.30    Ver 1.0    Job               
				创建函数。
***************************************************************************************************/
u32 canbus_middle_send(CANBus_type* can, u32 stdID, u8* buf, u32 len)
{
    CanTxMsg tx;
    u32 i = 0;
    u32 num = 0;
    u32 len_left = 0;

    if (!can || len == 0 || !buf)
    {
        return 0;
    }

    num = len / 8;
    len_left = len % 8;

    tx.StdId = stdID;
    tx.IDE = CAN_Id_Standard;
    tx.RTR = CAN_RTR_Data;

    //每次发送8字节
    for (i = 0; i < num; i++)
    {
        tx.DLC = 8;
        memcpy(tx.Data, buf + i * 8, 8);
        if (!stm32_CAN_send(can, &tx))
        {
            return i * 8;
        }
        if (i % 10 == 0)
        {
            rt_thread_delay(1);
        }
    }

    //发送多余的字节
    if (len_left)
    {
        tx.DLC = len_left;
        memcpy(tx.Data, buf + num * 8, len_left);
        if (!stm32_CAN_send(can, &tx))
        {
            return num * 8;
        }
    }

    return len;
}

/***************************************************************************************************
*\Function      canbus_middle_thread_entry
*\Description   
*\Parameter     p
*\Return        void
*\Note          
*\Log           2018.08.30    Ver 1.0    Job               
				创建函数。
***************************************************************************************************/
void canbus_middle_thread_entry(void* p)
{
    CanRxMsg rx;
    u32 delay = 0;
	can_ota_init();

    rt_thread_delay(200);
    stm32_CAN_clear(&CAN);

    while (1)
    {
        if (stm32_CAN_recv(&CAN, &rx))
        {
            delay++;
            if (delay % 10 == 0)
            {
                rt_thread_delay(1);
                delay = 0;
            }
            if (rx.IDE != CAN_Id_Standard || rx.RTR != CAN_RTR_Data)
            {
                continue;
            }
            switch (rx.StdId)
            {
            case 0x500:
                AddDataToBuf(&CAN_BUF500, rx.Data, rx.DLC);
//			{
//				u32  i = 0;
//				rt_kprintf("LEN:%d\n", rx.DLC);
//				for(i = 0; i< rx.DLC;i++)
//				{
//					rt_kprintf("%02X ", rx.Data[i]);
//				}
//				rt_kprintf("\n");
//			}
                break;
			
            default:
                rt_kprintf("StdID[0x%03X] is no support!\n", rx.StdId);
                break;
            }
        }
        else
        {
            delay = 0;
            rt_thread_delay(1);
        } 
    }
}
//USART1中断处理函数  FE 01 XX CS BE
uint8_t UART1_RxFlag = 0;
void USART1_IRQHandler_Callback(void)
{
  u8 temp;
	temp= USART_ReceiveData(USART1);
	static u8 state = State_Frame_Head;
	static u8 length = 0;
	static u8 i = 0;
  static u8 cs = 0;
  
	switch(state)//Frame Analyze 
	{
//Identify Frame Head,Init Timeout and Varies		
		case State_Frame_Head:
		if(temp == UART1_Frame_Head)
		{
			state = State_Frame_Length;
		  length = 0;
		  i = 0;
		  cs = 0;
		}
//		else
//		 printf("** Frame Head Error. ** \n\r");		
		break;
//Get Data Length 			
		case State_Frame_Length:		
		length = temp;
		state = State_Frame_Data;
		break;
//Get Data  		
		case State_Frame_Data:
		USART1_RxBuffer[i] = temp;
		cs ^= temp;
		if(i == length -1)
		{
			i = 0;
		  state = State_Frame_CS;
		}
		else
		  i++;
		break;
//Compare CS		
		case State_Frame_CS:
		if(temp == cs)
		{
		  state = State_Frame_End;
		}
		else
		{
		  state = State_Frame_Head;
      //printf("** Frame CS Error. ** \n\r");			
		}		
		break;
//Compare Frame End		
		case State_Frame_End:
		if(temp == UART1_Frame_End)
		{
		  state = State_Frame_Head;
			UART1_RxFlag = 1;
			//printf("** Frame Transmit successfully. ** \n\r");
		}
		else
		{
		  state = State_Frame_Head;	
//      printf("** Frame End Error. ** \n\r");			
		}
		break;
//Unvalid State :	Break	
		default : break;
		
	}
}
