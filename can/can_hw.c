/**
  ******************** (C) COPYRIGHT 2011 DJI **********************************
  *
  * @Project Name       : BL_WKM2_MAIN.uvproj
  * @File Name          : can_hw.c
  * @Environment        : keil mdk4.12/LPC1765/100M cclock
  * @Author&Date        : 2011-08-31
  * @Version            : 1.10
  ******************************************************************************
  * @Description
  *	    can hardware config
  */

/* Includes ------------------------------------------------------------------*/
#include "../drivers/drivers.h"
#include "can_inc.h"


/* Private define ------------------------------------------------------------*/

/* uart printf for debug */
#define __CAN_DEBUG_UART_PRINT__

/* for debug */
#ifdef __CAN_DEBUG_UART_PRINT__

#define CAN_PRINT(x) uart_printf x;

#elif defined __CAN_DEBUG_VCOM_PRINT__

#define CAN_PRINT(x) VCOM_printf x;

#else
#define CAN_PRINT(x) ;
#endif

/* Private variables ---------------------------------------------------------*/
CAN_MSG_Type       msgRxBuf;

static CPU_INT08U  TxPrio1 = 0;          /* can 动态优先级发送模式下,优先级递增变量 */
static CPU_INT08U  TxPrio2 = 0;


/* Private function declaration ----------------------------------------------*/
static void CAN_SoftReset (LPC_CAN_TypeDef *CANx);
static void CAN_CtrlInit(LPC_CAN_TypeDef *CANx, uint32_t can_btr);
static void CAN_CtrlDeInit(LPC_CAN_TypeDef *CANx);
#if __CAN1_ENABLE__
static void CAN1_ISR(void);
#endif
#if __CAN2_ENABLE__
static void CAN2_ISR(void);
#endif

/* public Function -----------------------------------------------------------*/
void CAN_IRQHandler(void)
{
#if __CAN1_ENABLE__
    CAN1_ISR();
#endif

#if __CAN2_ENABLE__
    CAN2_ISR();
#endif
}

static void CAN_SoftReset (LPC_CAN_TypeDef *CANx)
{
	CANx->MOD = 9;
	CANx->GSR = 0;
	CANx->MOD = 0x08;
}

static void CAN1_ISR(void)
{
	uint32_t status;
	uint32_t TmpStatus;

	status    = LPC_CAN1->ICR;
	TmpStatus = LPC_CANAF->LUTerrAd;

#ifdef __CAN_DEBUG_UART_PRINT__
	if(TmpStatus&0x00001FFC) /* 10:2 */
	{
		CAN_PRINT((0,"AF:%.2x\r\n",TmpStatus));
	}
#endif

   	if(status & ICR_RI_SET)           /* read interrupt state	*/
   	{
	  	CAN_RxINTRoute(LPC_CAN1);
   	}

   	if( status & ( ICR_TI1_SET | ICR_TI2_SET | ICR_TI3_SET) )
   	{
   	  	CAN_TxINTRoute(LPC_CAN1);
   	}

	/* err occur */
	if((status & ICR_ERRSTA_BIT_MASK) != 0)
	{
		if(status&ICR_EI_SET) // error warning interrupt
		{
			// todo error warnning
			CAN_PRINT((0,"error warning.\r\n"));
		}

		if(status&ICR_DOI_SET) // Data overrun interrupt
		{
			// todo dataoverun
// 		    CAN_PRINT((0,"data overrun.\r\n"));
			CAN_SoftReset(LPC_CAN1);
		}

		if(status&ICR_WUI_SET) // wake up interrupt
		{

		}

		if(status&ICR_EPI_SET) // error passive interrupt
		{
			CAN_PRINT((0,"error passive.\r\n"));
		}

		if(status&ICR_ALI_SET) // arbitration lost
		{
			CAN_PRINT((0,"arbitration lost.\r\n"));
		}

		if(status & ICR_BEI_SET)     // BEI 总线错误中断
	   	{
		    //CAN_PRINT((0,"bus error.\r\n"));
	 	  	if( (LPC_CAN1->MOD & 0x01) ) // rm == 1,说明是有busoff导致的can控制器复位
	   	  	{
	        	CAN_PRINT((0,"can reset in int 0x%08x.\r\n",LPC_CAN1->GSR));
		    	LPC_CAN1->GSR =0;
		    	LPC_CAN1->MOD =( (1<<3) );
		    	LPC_CAN1->CMR =0x02;
	      	}

	      	if( LPC_CAN1->GSR & 0x80)  /* bus off */
	      	{
		  		CAN_PRINT((0,"buf off.\r\n"));
				  CAN_PRINT((0,"can reset in int 0x%08x.\r\n",LPC_CAN1->GSR));
	        	CAN_SoftReset(LPC_CAN1);
		  	}
	   	}
	}
}

void CAN2_ISR(void)
{
	uint32_t status;
	uint32_t TmpStatus;

	status = LPC_CAN2->ICR;
    TmpStatus = LPC_CANAF->LUTerrAd;
    
#ifdef __CAN_DEBUG_UART_PRINT__
	if(TmpStatus&0x00001FFC) /* 10:2 */
	{
		CAN_PRINT((0,"AF:%.2x\r\n",TmpStatus));
	}
#endif

   	if(status & ICR_RI_SET)           /* read interrupt state	*/
   	{
	  	CAN_RxINTRoute(LPC_CAN2);
   	}
   	if( status & ( ICR_TI1_SET | ICR_TI2_SET | ICR_TI3_SET))
   	{
   	  	CAN_TxINTRoute(LPC_CAN2);
   	}

	/* err occur*/
	if((status & ICR_ERRSTA_BIT_MASK) != 0)
	{
        if(status&ICR_EI_SET) // error warning interrupt
		{
			// todo error warnning
			CAN_PRINT((0,"error warning.\r\n"));
		}

		if(status&ICR_DOI_SET) // Data overrun interrupt
		{
			// todo dataoverun
// 		    CAN_PRINT((0,"data overrun.\r\n"));
			CAN_SoftReset(LPC_CAN2);
		}

		if(status&ICR_WUI_SET) // wake up interrupt
		{

		}

		if(status&ICR_EPI_SET) // error passive interrupt
		{
			CAN_PRINT((0,"error passive.\r\n"));
		}

		if(status&ICR_ALI_SET) // arbitration lost
		{
			CAN_PRINT((0,"arbitration lost.\r\n"));
		}
        
	   	if(status & ICR_BEI_SET)     // BEI 总线错误中断
	   	{
			//CAN_PRINT((0,"bus error.\r\n"));
	 	  	if( (LPC_CAN2->MOD & 0x01) ) // rm == 1,说明是有busoff导致的can控制器复位
	   	  	{
	        	CAN_PRINT((0,"can reset in int 0x%08x.\r\n",LPC_CAN2->GSR));
		    	LPC_CAN2->GSR =0;
		    	LPC_CAN2->MOD =( (1<<3) );
		    	LPC_CAN2->CMR =0x02;
	      	}

	      	if( LPC_CAN2->GSR & 0x80)
	      	{
		  		CAN_PRINT((0,"buf off.\r\n"));
				CAN_PRINT((0,"can reset in int 0x%08x.\r\n",LPC_CAN2->GSR));
	        	CAN_SoftReset(LPC_CAN2);
		  	}
	   	}
	}
}

void CAN_Init(void)
{
#if __CAN1_ENABLE__
	CAN_CtrlInit(LPC_CAN1,BITRATE_1000K25MHZ);
#endif
#if __CAN2_ENABLE__
    CAN_CtrlInit(LPC_CAN2,BITRATE_1000K25MHZ);
#endif
	CAN_RING_BUF_Init();
	/* 填写验收滤波表格，node表示要接收数据(本站)的站号*/
  CAN_InitAFTable();
  /* 不从UCOS的中断管理函数进入 */
	NVIC_EnableIRQ( CAN_IRQn );
}

static void CAN_CtrlInit(LPC_CAN_TypeDef *CANx, CPU_INT32U can_btr)
{
	if( CANx == LPC_CAN1 ) {
        LPC_SC->PCONP |= 1 << 13;
    #if defined( __CAN1_PIN021_PIN022__ )
		/* p0.21,p0.22 FCNT4 -- RD1,TD1 */
		PINSEL1 &= ~( 3 << 10 );            
		PINSEL1 |=  ( 3 << 10 );			 
		PINSEL1 &= ~( 3 << 12 );            
		PINSEL1 |=  ( 3 << 12 );
    #elif defined( __CAN1_PIN000_PIN001__ )
        /* p0.0,p0.1 FCNT2 -- RD1,TD1 */
		PINSEL0 &= ~( 3 << 0 );            
		PINSEL0 |=  ( 1 << 0 );			 
		PINSEL0 &= ~( 3 << 2 );            
		PINSEL0 |=  ( 1 << 2 );
    #endif
    } else if( CANx == LPC_CAN2 ) {
		LPC_SC->PCONP |= 1 << 14;
    #if defined( __CAN2_PIN004_PIN005__ )
		/* p0.4,p0.5 FCNT3 -- RD2,TD2 */
 		PINSEL0 &= ~( 3 << 8 );            
 		PINSEL0 |=  ( 2 << 8 );			 
 		PINSEL0 &= ~( 3 << 10 );            
 		PINSEL0 |=  ( 2 << 10 );
    #elif defined( __CAN2_PIN207_PIN208__ )
		/* p2.7,p2.8 FCNT2 -- RD2,TD2 */
		PINSEL4 &= ~( 3 << 14 );            
		PINSEL4 |=  ( 1 << 14 );			 
		PINSEL4 &= ~( 3 << 16 );            
		PINSEL4 |=  ( 1 << 16 );
    #endif
	}

	CANx->MOD = 0x09;	    // 复位CAN，并置位TPM
  	CANx->IER = 0x00;	    // Disable Receive Interrupt
  	CANx->GSR = 0x00;	    // Reset error counter when CANxMOD is in reset
  	CANx->BTR = can_btr;	// 设置CAN总线时钟，具体计算见can.h

	// 使能 接收 发送 总线错误
    //CANx->IER |=  IER_RIE | IER_TIE1 | IER_EIE | IER_DOIE | IER_BEIE | IER_TIE2 | IER_TIE3;

	CANx->IER |=  IER_RIE | IER_TIE1 | IER_EIE | IER_BEIE | IER_DOIE | IER_TIE2 | IER_TIE3;

	CANx->MOD =  0x08;      //取消复位状态
}

void CAN_DeInit(void)
{
#if __CAN1_ENABLE__
	CAN_CtrlDeInit(LPC_CAN1);
#endif

#if __CAN2_ENABLE__
    CAN_CtrlDeInit(LPC_CAN2);
#endif
}

static void CAN_CtrlDeInit( LPC_CAN_TypeDef *CANx )
{
	CANx->MOD = 0x09;	    // 复位CAN，并置位TPM
  	CANx->IER = 0x00;	    // Disable all Interrupt
  	CANx->GSR = 0x00;	    // Reset error counter when CANxMOD is in reset
	CANx->MOD = 0x08;       //取消复位状态

	if( CANx == LPC_CAN1 ) {
        LPC_SC->PCONP &= ~( 1u << 13 );
    } else if( CANx == LPC_CAN2 ) {
	    LPC_SC->PCONP &= ~( 1u << 14 );
	}
}

void CAN_TBS1_WRITE(LPC_CAN_TypeDef *pCANx,CPU_INT32U DATA_A,CPU_INT32U DATA_B,CPU_INT32U TFI, CPU_INT32U TID)
{
	pCANx->TFI1 = TFI;
	pCANx->TID1 = TID;
	pCANx->TDA1 = DATA_A;
	pCANx->TDB1 = DATA_B;
}

void CAN_TBS2_WRITE(LPC_CAN_TypeDef *pCANx,CPU_INT32U DATA_A, CPU_INT32U DATA_B, CPU_INT32U TFI, CPU_INT32U TID)
{
	pCANx->TFI2 = TFI;
	pCANx->TID2 = TID;
	pCANx->TDA2 = DATA_A;
	pCANx->TDB2 = DATA_B;
}

void CAN_TBS3_WRITE(LPC_CAN_TypeDef *pCANx,CPU_INT32U DATA_A, CPU_INT32U DATA_B, CPU_INT32U TFI, CPU_INT32U TID)
{
	pCANx->TFI3 = TFI;
	pCANx->TID3 = TID;
	pCANx->TDA3 = DATA_A;
	pCANx->TDB3 = DATA_B;
}



/****************************** can TX process ***************************/
void CAN_tx_action(LPC_CAN_TypeDef *pCANx, CAN_RING_BUF_Type* buf,CPU_INT32U cnt,CPU_INT32U flag)
{
	uint32_t DATA[2] = { 0 }; // must init all data to 0
	uint32_t TFI = 0;
	uint32_t TID = 0;

    CPU_INT08U *pTxPrio = 0;

    if( pCANx == LPC_CAN1 ) {
        pTxPrio = &TxPrio1;
    } else if( pCANx == LPC_CAN2 ) {
        pTxPrio = &TxPrio2;
    }

    if(*pTxPrio == 255)return; /* 防止CAN发送优先级反转的情况发生 */

	cnt = (cnt >= 8)?8:cnt;
	if( CAN_RING_BUF_RD_BLOCK((CPU_INT08U*)&DATA[0],buf,cnt) == cnt)
	{
		//CAN_PRINT((0,"%d length:%d.\r\n",__LINE__,cnt));
		TFI = ( ((cnt<<16)&0x000F0000) | (*pTxPrio)++ );
		TID = buf->id;

	    if(flag == 1)
		{
		    CAN_TBS1_WRITE(pCANx,DATA[0],DATA[1],TFI,TID);
		}else if(flag == 2)
		{
			CAN_TBS2_WRITE(pCANx,DATA[0],DATA[1],TFI,TID);
		}else
		{
		    flag = 3; /* 确保flag为1,2,3中的数 */
			CAN_TBS3_WRITE(pCANx,DATA[0],DATA[1],TFI,TID);
		}

		pCANx->CMR = (0x10 << flag) | 0x01;
	}else
	{
		return;
	}
}

void CAN_tx_action_all(LPC_CAN_TypeDef *pCANx, CAN_RING_BUF_Type* buf,CPU_INT32U cnt)
{
	uint32_t DATA[6] = {0};// must init all data to 0
	uint32_t TFI     = 0;
	uint32_t TID     = 0;

    CPU_INT08U *pTxPrio = 0;

    if( pCANx == LPC_CAN1 ) {
        pTxPrio = &TxPrio1;
    } else if( pCANx == LPC_CAN2 ) {
        pTxPrio = &TxPrio2;
    }
  	cnt    = (cnt >= 24) ? 24:cnt;
	*pTxPrio = 0;
  	if( CAN_RING_BUF_RD_BLOCK((CPU_INT08U*)&DATA[0],buf,cnt) == cnt)
	{
		if( cnt<=8 )
		{
			TFI = ( ((cnt<<16) & 0x000F0000) | (*pTxPrio)++ );
			TID = buf->id;
			CAN_TBS1_WRITE(pCANx,DATA[0],DATA[1],TFI,TID);

			CAN_TBS1_EN(pCANx)

		}else if( cnt<=16 )
		{
			TFI = ( ((8<<16)&0x000F0000) | (*pTxPrio)++ );
            TID = buf->id;
			CAN_TBS1_WRITE(pCANx,DATA[0],DATA[1],TFI,TID);
			CAN_TBS1_EN(pCANx)

			TFI = ( (((cnt-8)<< 16)&0x000F0000) | (*pTxPrio)++);
            TID = buf->id;
			CAN_TBS2_WRITE(pCANx,DATA[2],DATA[3],TFI,TID);
			CAN_TBS2_EN(pCANx)
		}else  /* 到这里肯定是 16<cnt<=24 */
		{
			TFI = ( ((8 << 16) & 0x000F0000) | (*pTxPrio)++ );
            TID = buf->id;
			CAN_TBS1_WRITE(pCANx,DATA[0],DATA[1],TFI,TID);

			TFI = ( ((8<<16)&0x000F0000) | (*pTxPrio)++ );
            TID = buf->id;
			CAN_TBS2_WRITE(pCANx,DATA[2],DATA[3],TFI,TID);

			TFI = ( (((cnt-16)<<16) & 0x000F0000) | (*pTxPrio)++);
            TID = buf->id;
			CAN_TBS3_WRITE(pCANx,DATA[4],DATA[5],TFI,TID);

			CAN_TBS123_EN(pCANx)
		}
	}else
	{
		return;
	}
}

/****************************** can TX INT process ***************************/
void CAN_TBS_INTRoute(LPC_CAN_TypeDef *pCANx, CPU_INT32U idx_TBS)
{
    if( pCANx == LPC_CAN1 ) {
        if( !CAN_RING_BUF_IS_EMPTY( &can1TxRingBuf ) ) {
            CAN_tx_function( pCANx, &can1TxRingBuf, idx_TBS );
        }else if( !CAN_RING_BUF_IS_EMPTY( &can1Tx2RingBuf ) ){
						CAN_tx_function( pCANx, &can1Tx2RingBuf, idx_TBS );
				}else if( !CAN_RING_BUF_IS_EMPTY( &can1Tx3RingBuf ) ){
						CAN_tx_function( pCANx, &can1Tx3RingBuf, idx_TBS );
				}else if( !CAN_RING_BUF_IS_EMPTY( &can1UpgradeAckTxRingBuf ) ){
						CAN_tx_function( pCANx, &can1UpgradeAckTxRingBuf, idx_TBS );
				}else if( !CAN_RING_BUF_IS_EMPTY( &can1LEDTxRingBuf ) ){
						CAN_tx_function( pCANx, &can1LEDTxRingBuf, idx_TBS );
				}
    } else if( pCANx == LPC_CAN2 ) {
#if __CAN2_ENABLE__			
        if( !CAN_RING_BUF_IS_EMPTY( &can2TxRingBuf ) ) {
            CAN_tx_function( pCANx, &can2TxRingBuf, idx_TBS );
        }
#endif				
    }
}

void CAN_TxINTRoute(LPC_CAN_TypeDef *pCANx)
{
	switch(pCANx->SR & 0x040404)
	{
		case 0x04:
			CAN_TBS_INTRoute(pCANx,1);
		break;
		case 0x0400:
			CAN_TBS_INTRoute(pCANx,2);
		break;
		case 0x040000:
			CAN_TBS_INTRoute(pCANx,3);
		break;
		case 0x0404:
			CAN_TBS_INTRoute(pCANx,1);
			CAN_TBS_INTRoute(pCANx,2);
		break;
		case 0x040004:
			CAN_TBS_INTRoute(pCANx,1);
			CAN_TBS_INTRoute(pCANx,3);
		break;
		case 0x040400:
			CAN_TBS_INTRoute(pCANx,2);
			CAN_TBS_INTRoute(pCANx,3);
		break;
		case 0x040404:
			CAN_TBS_INTRoute(pCANx,4);
			break;
		default:
			return;
	}
}

void CAN_RxINTRoute(LPC_CAN_TypeDef *pCANx)
{
	msgRxBuf.cnt            = (pCANx->RFS>>16)&0x0000000F;  //将接收帧信息寄存器的值写入帧缓冲区结构体相应位
    msgRxBuf.id             = pCANx->RID&0x7ff;             //同上写入ID
	msgRxBuf.data.dwData[0] = pCANx->RDA;
	msgRxBuf.data.dwData[1] = pCANx->RDB;

	RELEASE_RECEIVE_BUFF_HW(pCANx);
	if( pCANx == LPC_CAN1 ) {
		if( msgRxBuf.id == can1RxRingBuf.id ) {
			CAN_RING_BUF_WR_BLOCK( &can1RxRingBuf, msgRxBuf.data.bData, msgRxBuf.cnt );
		}else if( msgRxBuf.id == can1Rx2RingBuf.id ) {
			CAN_RING_BUF_WR_BLOCK( &can1Rx2RingBuf, msgRxBuf.data.bData, msgRxBuf.cnt );
		}else if( msgRxBuf.id == can1RxUpgradeRingBuf.id ) {
			CAN_RING_BUF_WR_BLOCK( &can1RxUpgradeRingBuf, msgRxBuf.data.bData, msgRxBuf.cnt );
		}else if( msgRxBuf.id == can1RxLEDRingBuf.id ) {
			CAN_RING_BUF_WR_BLOCK( &can1RxLEDRingBuf, msgRxBuf.data.bData, msgRxBuf.cnt );
		}
	} else if( pCANx == LPC_CAN2 ) {
#if __CAN2_ENABLE__		
		if( msgRxBuf.id == can2RxRingBuf.id ) {
			CAN_RING_BUF_WR_BLOCK( &can2RxRingBuf, msgRxBuf.data.bData, msgRxBuf.cnt );
		}
#endif		
	} 
}



/*******************  (C) COPYRIGHT 2011 DJI ************END OF FILE***********/
