////////////////////////////////////////////////////////////////////////////////////////////////////
// fileName: can.c
//
// Description: Low level CAN functions
//
// Author: Enovation controls
//
//         http://www.enovationcontrols.com
//
// copyright Copyright 2024 by Enovation Controls.
//            This file and/or parts of it are subject to
//            software license terms.
//
//            Without written approval of Enovation Controls this
//            software may not be copied, redistributed or used
//            in any other way.
//
////////////////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////////////////////
// Includes
////////////////////////////////////////////////////////////////////////////////////////////////////
#include <stdbool.h>
#include <stdint.h>
#include "CAN.h"
#include "TickTimers.h"
#include "MiscFunctions.h"



////////////////////////////////////////////////////////////////////////////////////////////////////
//Defines
////////////////////////////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////////////////////////////
// Typedefs
////////////////////////////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////////////////////////////
// Private variables
////////////////////////////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////////////////////////////
// Private interface
////////////////////////////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////////////////////////////
// Public interface
////////////////////////////////////////////////////////////////////////////////////////////////////
//CAN RX FIFO
uint8_t can_fifo_in_pointer = 0;
uint8_t can_fifo_out_pointer = 0;
uint32_t can_fifo_arbid[CAN_FIFO_DEPTH];
uint8_t can_fifo_dlen[CAN_FIFO_DEPTH];
uint8_t can_fifo_data[CAN_FIFO_DEPTH][8];

CAN_TxHeaderTypeDef CAN_tx_header; 		//header for message transmissions
uint32_t CAN_tx_mailbox;

uint8_t can_tx_data[8];
uint8_t can_rx_data[8];

uint32_t can_rx_count = 0;
uint32_t can_tx_count = 0;

//J1939 variables
uint16_t j1939EngineSpeed;
uint16_t j1939EngineOilPressure;
int16_t j1939EngineCoolantTemp;
uint16_t j1939EngineLoad;


////////////////////////////////////////////////////////////////////////
//
// clear_CAN_tx_data()
//
// Sets all CAN TX data to 0xFF
//
//    Parameters:
//       none
//
////////////////////////////////////////////////////////////////////////
void clear_CAN_tx_data()
{
	uint8_t i;

	for(i=0;i<8;++i)
		can_tx_data[i] = 0xFF;
}


////////////////////////////////////////////////////////////////////////
//
// CAN_transmit_coms()
//
// Main function for all CAN transmits
//
//    Parameters:
//       none
//
////////////////////////////////////////////////////////////////////////
void CAN_transmit_coms()
{
	 //Transmit 29 bit message at 50mS rate
     if(can_tx_timer == 0 && HAL_CAN_GetTxMailboxesFreeLevel(&hcan1) != 0)
	 {
		can_tx_timer = 50;

		CAN_tx_header.IDE=CAN_ID_EXT; 							//set identifier to extended
		CAN_tx_header.DLC=8; 									//set message data length (0-8)
		CAN_tx_header.RTR=CAN_RTR_DATA; 						//set data type to DATA
		CAN_tx_header.ExtId=0x18FEF100 + CAN_SOURCE_ADDRESS;	//set message identifier (29 bits)

		clear_CAN_tx_data();

		can_tx_data[0] = 0x00;
		can_tx_data[1] = 0x11;
		can_tx_data[2] = 0x22;

		if (HAL_CAN_AddTxMessage(&hcan1, &CAN_tx_header, can_tx_data, &CAN_tx_mailbox) != HAL_OK)
		{
			//Error_Handler ();
		}
		else
		{
			//Successful TX
			++can_tx_count;
		}
	 }
}


////////////////////////////////////////////////////////////////////////
//
// CAN_receive_coms()
//
// Main function for all CAN reception
//
//    Parameters:
//       none
//
////////////////////////////////////////////////////////////////////////
void CAN_receive_coms()
{
	uint8_t i;
	uint8_t can_rx_Dlen;
	uint16_t j1939PGN;


	while(can_fifo_in_pointer != can_fifo_out_pointer)
	{
		//CAN RX data available from RX FIFO: reset receive timeout
		can_rx_timer = CAN_RX_TIMEOUT;

		++can_rx_count;

		//Extract all data for next received message
		can_rx_Dlen = can_fifo_dlen[can_fifo_out_pointer];			//Message data length
		j1939PGN = ((can_fifo_arbid[can_fifo_out_pointer] & 0x00FFFF00) / 0x100);

		for(i=0;i<can_rx_Dlen;++i)
			can_rx_data[i] = can_fifo_data[can_fifo_out_pointer][i];	//Message data

		//Process received data here
		//Extract SPN data from PGN
		switch(j1939PGN)
		{
			case 0xF004:
				//SPN190: Engine speed
				j1939EngineSpeed = make16(can_rx_data[4], can_rx_data[3]);
				j1939EngineSpeed /= 8;				//J1939 scaling
				break;
			case 0xFEEF:
				//SPN 100: Engine pressures
				j1939EngineOilPressure = can_rx_data[3];
				j1939EngineOilPressure *= 4;		//j1939 scaling
				break;
			case 0xFEEE:
				//SPN 110: Engine temperatures
				j1939EngineCoolantTemp = can_rx_data[0];
				j1939EngineCoolantTemp -= 40;		//j1939 offset
				break;
			case 0xF003:
				//SPN 92: Engine load
				j1939EngineLoad = can_rx_data[2];
				break;
		}

		//Update FIFO out pointer
		if(++can_fifo_out_pointer >= CAN_FIFO_DEPTH)
			can_fifo_out_pointer = 0;
	}

	//////////////////////////////
	// Check for CAN RX timeout //
	//////////////////////////////
	if(can_rx_timer == 0)
	{
		//RX timeout:
		j1939EngineSpeed = 0xFFFF;
		j1939EngineOilPressure = 0xFFFF;
		j1939EngineCoolantTemp = 0xFFFF;
		j1939EngineLoad = 0xFFFF;
	}
}



void HAL_CAN_RxFifo0MsgPendingCallback(CAN_HandleTypeDef *hcan)
{
	CAN_RxHeaderTypeDef CAN_rx_header;
	uint8_t CAN_rx_data[8];
	uint8_t i;

	do
	{
		HAL_CAN_GetRxMessage(&hcan1, CAN_RX_FIFO0, &CAN_rx_header, CAN_rx_data);

		//Store latest 29 bit data in software FIFO
		if(CAN_rx_header.IDE == CAN_ID_EXT)
		{

			can_fifo_arbid[can_fifo_in_pointer] = CAN_rx_header.ExtId;
			can_fifo_dlen[can_fifo_in_pointer] = CAN_rx_header.DLC;
			for(i=0;i<8;++i)
				can_fifo_data[can_fifo_in_pointer][i] = CAN_rx_data[i];

			if(++can_fifo_in_pointer >= CAN_FIFO_DEPTH)
				can_fifo_in_pointer = 0;

		}
	}while(HAL_CAN_GetRxFifoFillLevel(&hcan1, CAN_RX_FIFO0) != 0);
}

void CAN_InitTask(void)
{
    CAN_FilterTypeDef canfilterconfig;

    canfilterconfig.FilterActivation = CAN_FILTER_ENABLE;
    canfilterconfig.FilterBank = 0;  // single can interfaces only have 14, 0-13
    canfilterconfig.FilterFIFOAssignment = CAN_FILTER_FIFO0;
    canfilterconfig.FilterIdHigh = 0;
    canfilterconfig.FilterIdLow = 0;
    canfilterconfig.FilterMaskIdHigh = 0x0;
    canfilterconfig.FilterMaskIdLow = 0x0000;
    canfilterconfig.FilterMode = CAN_FILTERMODE_IDMASK;
    canfilterconfig.FilterScale = CAN_FILTERSCALE_32BIT;
    canfilterconfig.SlaveStartFilterBank = 20;  // how many filters to assign to the CAN1 (master can)

    HAL_CAN_ConfigFilter(&hcan1, &canfilterconfig);

    //
    HAL_CAN_Start(&hcan1);

    // activate rx notifications
    HAL_CAN_ActivateNotification(&hcan1, CAN_IT_RX_FIFO0_MSG_PENDING);
}
