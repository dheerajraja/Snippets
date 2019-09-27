
/// @cond INCLUDE_FILE_DESCRIPTION
/**
 ** @file 
 ** @brief
 ** 	Implementation of the command handler routine
 **
 ** The command handler is called by the event manager every time a apdu is received by the protocol.
 ** The command handler evaluates the class and instruction byte of the first 5 bytes of the apdu. 
 ** Dependend on the type of command (case1, case2...) aditional data is received by the command handler. 
 ** Then the dedicated handling routine for this command is called.	Optional result data is send back to the 
 ** terminal. At least the command status (e.g. 0x90 0x00) is send back.
 **
 **/
/// @endcond 
/** @addtogroup IFX_COMMAND_GROUP
 *  @{
 */

/*  includes
 ****************************************************************/


/*  pragmas
 ****************************************************************/

/*  defines
 ****************************************************************/

/*  private prototypes
 ****************************************************************/

/*  constants
 ****************************************************************/

#define MAX_APDU_RES_LEN			256

/*  variables
 ****************************************************************/
 
/*  functions
 ****************************************************************/

static void add_CRC(uint8_t* Buf_Crc, uint16_t len)
{
	uint16_t i, p, q, r ,s;
	uint16_t  remainder = 0;
	uint8_t rgbCrcAddBuffer[255];

	memcpy(rgbCrcAddBuffer, Buf_Crc, len);

	// New CRC with 2-bytes first Seed = 0 and it repeats for number of bytes we have and returns 2-byte crc
		for (i=0; i < len - 2; i++)
		{
			p = (uint16_t)((remainder ^ rgbCrcAddBuffer[i]) & 0xff);
			q = (uint16_t)(p & 0x0f);
			r = (uint16_t)(p ^ (q << 4));
			s = (uint16_t)(r >> 4);
			
			remainder = (uint16_t)((((((r << 1) ^ s) << 4) ^ q) << 3) ^ s ^ (remainder >> 8));
		}

		remainder = (remainder << 8) | (remainder >> 8);

		memcpy(&Buf_Crc[len - 2], (void *)&remainder, 2);
}

static uint16_t send_previous_block_RX(uint8_t* TxBuf, uint16_t* Tx_Len)
{
	memcpy(TxBuf, send_previous_block, send_previous_length);

	*Tx_Len = send_previous_length;

	return I2CSW_SUCCESS;
}

static uint16_t send_RBlock_CRC(uint8_t* TxBuffer, uint16_t* TxLen)
{
	uint8_t Buffer[DEFAULT_TPDU_LENGTH];

	if (send_previous_block[TPDU_PCB] == LEN_REQ)
		return send_previous_block_RX(TxBuffer, TxLen);

	Buffer[0] = NAD_RES;
	Buffer[1] = CRC_ERROR_1;
	Buffer[2] = 0x00; // Indicating that I-Block is missing

//	Buffer[0] = NAD_RES;
//	Buffer[1] = CRC_ERROR_2;
//	Buffer[2] = 0x00; // Indicating that I-Block is missing

	add_CRC(Buffer, sizeof(Buffer));

	memcpy(TxBuffer, Buffer, sizeof(Buffer));

	*TxLen = sizeof(Buffer);

	// update previous block buffer and length
	memcpy(send_previous_block, Buffer, sizeof(Buffer));

	send_previous_length = sizeof(Buffer);

	return I2CSW_RBLOCK_SUCCESS;
}

static uint16_t check_CRC(uint8_t* ReceiveBuf, uint16_t ReceiveLen, uint8_t* TxBuffer, uint16_t* TxLen)
{
	uint16_t ret = I2CSW_EXEC_ERROR;
	uint16_t i, p, q, r ,s;
	uint16_t remainder = 0;
	uint8_t rgbCrcBuffer[MAX_APDU_BUFFER_LENGTH];

	memcpy (rgbCrcBuffer, ReceiveBuf, ReceiveLen);

		// Check CRC according to ISO/IEC 13239
		for (i=0; i < ReceiveLen - 2; i++)
		{
			p = (uint16_t)((remainder ^ rgbCrcBuffer[i]) & 0xff);
			q = (uint16_t)(p & 0x0f);
			r = (uint16_t)(p ^ (q << 4));
			s = (uint16_t)(r >> 4);
			
			remainder = (uint16_t)((((((r << 1) ^ s) << 4) ^ q) << 3) ^ s ^ (remainder >> 8));
		}

		remainder = (remainder << 8) | (remainder >> 8);

		if (memcmp(&ReceiveBuf[ReceiveLen - 2], (void *)&(remainder), 2) != 0)
		{
			ret = send_RBlock_CRC(TxBuffer, TxLen);
			return ret;
		}

		return I2CSW_SUCCESS;
}

static uint16_t send_RBlock(uint8_t* TxBuffer, uint16_t* TxLen)
{
	uint8_t Buffer[DEFAULT_TPDU_LENGTH];

	if (send_previous_block[TPDU_PCB] == LEN_REQ)
		return send_previous_block_RX(TxBuffer, TxLen);

	Buffer[0] = NAD_RES;
	Buffer[1] = OTHER_ERROR_1;
	Buffer[2] = 0x00; // Indicating that I-Block is missing

	add_CRC(Buffer, sizeof(Buffer));

	memcpy(TxBuffer, Buffer, sizeof(Buffer));

	*TxLen = sizeof(Buffer);

	// update previous block buffer and length
	memcpy(send_previous_block, Buffer, sizeof(Buffer));

	send_previous_length = sizeof(Buffer);

	return I2CSW_RBLOCK_SUCCESS;
}

static uint16_t check_NAD(uint8_t* RxBuffer, uint8_t* TxBuffer, uint16_t* TxLen)
{
	uint16_t ret = I2CSW_EXEC_ERROR;

	if (RxBuffer[0] != NAD_REQ)
	{
		ret = send_RBlock(TxBuffer, TxLen);
		return ret;
	}

	return I2CSW_SUCCESS;
}

static uint16_t check_LEN(uint8_t* ReceiveBuf, uint16_t ReceiveLen, uint8_t* TxBuffer, uint16_t* TxLen)
{
	uint16_t ret = I2CSW_EXEC_ERROR;

	if ((ReceiveLen - 5) != ReceiveBuf[TPDU_LEN])
	{
		ret = send_RBlock(TxBuffer, TxLen);
		return ret;
	}

	return I2CSW_SUCCESS;
}

static uint16_t wrap_I2C_SWR_Res(uint8_t* TxBuffer, uint16_t* TxLen)
{
	uint8_t Buffer[DEFAULT_TPDU_LENGTH];

	Buffer[0] = NAD_RES;
	Buffer[1] = SWR_RES;
	Buffer[2] = 0x00; // Indicating that I-Block is missing

	add_CRC(Buffer, sizeof(Buffer));

	memcpy(TxBuffer, Buffer, sizeof(Buffer));

	*TxLen = sizeof(Buffer);
	
	// update previous block buffer and length
	memcpy(send_previous_block, Buffer, sizeof(Buffer));

	send_previous_length = sizeof(Buffer);

	return I2CSW_SUCCESS;
}

static uint16_t wrap_I2C_RELEASE_Res(uint8_t* TxBuffer, uint16_t* TxLen)
{
	uint8_t Buffer[DEFAULT_TPDU_LENGTH];

	Buffer[0] = NAD_RES;
	Buffer[1] = RELEASE_RES;
	Buffer[2] = 0x00; // Indicating that I-Block is missing

	add_CRC(Buffer, sizeof(Buffer));

	memcpy(TxBuffer, Buffer, sizeof(Buffer));

	*TxLen = sizeof(Buffer);
	
	// update previous block buffer and length
	memcpy(send_previous_block, Buffer, sizeof(Buffer));

	send_previous_length = sizeof(Buffer);

	return I2CSW_SUCCESS;
}

static uint16_t wrap_I2C_WTX_Res(uint8_t* TxBuffer, uint16_t* TxLen, uint8_t* RxBuffer)
{
	uint8_t Buffer[WTX_RESPONSE_LENGTH];

	Buffer[0] = NAD_RES;
	Buffer[1] = WTX_RES;
	Buffer[2] = 0x02; // I-Block is same as the one which we received during IFS_REQ
	Buffer[3] = RxBuffer[3];

	add_CRC(Buffer, sizeof(Buffer));

	memcpy(TxBuffer, Buffer, sizeof(Buffer));

	*TxLen = sizeof(Buffer);
	
	// update previous block buffer and length
	memcpy(send_previous_block, Buffer, sizeof(Buffer));

	send_previous_length = sizeof(Buffer);

	return I2CSW_SUCCESS;
}

static uint16_t wrap_I2C_ABORT_Res(uint8_t* TxBuffer, uint16_t* TxLen)
{
	uint8_t Buffer[DEFAULT_TPDU_LENGTH];

	Buffer[0] = NAD_RES;
	Buffer[1] = ABORT_RES;
	Buffer[2] = 0x00; // Indicating that I-Block is missing

	add_CRC(Buffer, sizeof(Buffer));

	memcpy(TxBuffer, Buffer, sizeof(Buffer));

	*TxLen = sizeof(Buffer);
	
	// update previous block buffer and length
	memcpy(send_previous_block, Buffer, sizeof(Buffer));

	send_previous_length = sizeof(Buffer);

	return I2CSW_SUCCESS;
}

static uint16_t wrap_I2C_IFS_Res(uint8_t* TxBuffer, uint16_t* TxLen, uint8_t* RxBuffer)
{
	uint8_t Buffer[IFS_RESPONSE_LENGTH];

	Buffer[0] = NAD_RES;
	Buffer[1] = IFS_RES;
	Buffer[2] = 0x02; // I-Block is same as the one which we received during IFS_REQ
	Buffer[3] = RxBuffer[3];
	Buffer[4] = RxBuffer[4];

	add_CRC(Buffer, sizeof(Buffer));

	memcpy(TxBuffer, Buffer, sizeof(Buffer));

	*TxLen = sizeof(Buffer);
	
	// update previous block buffer and length
	memcpy(send_previous_block, Buffer, sizeof(Buffer));

	send_previous_length = sizeof(Buffer);

	return I2CSW_SUCCESS;
}

static uint16_t wrap_I2C_RESYNC_Res(uint8_t* TxBuffer, uint16_t* TxLen)
{
	uint8_t Buffer[DEFAULT_TPDU_LENGTH];

	Buffer[0] = NAD_RES;
	Buffer[1] = RESYNC_RES;
	Buffer[2] = 0x00; // Indicating that I-Block is missing

	add_CRC(Buffer, sizeof(Buffer));

	memcpy(TxBuffer, Buffer, sizeof(Buffer));

	*TxLen = sizeof(Buffer);
	
	// update previous block buffer and length
	memcpy(send_previous_block, Buffer, sizeof(Buffer));

	send_previous_length = sizeof(Buffer);

	return I2CSW_SUCCESS;
}

static uint16_t wrap_I2C_LEN_Req_Res(uint8_t* TxBuffer, uint16_t* TxLen, uint8_t* RxBuffer)
{
	uint8_t Buffer[RESPOND_MASTER_LENGTH_REQ];

	Buffer[0] = NAD_RES;
	Buffer[1] = LEN_RES;
	Buffer[2] = 0x01; // I-Block is same as the one which we received during LEN_REQ
	Buffer[3] = RxBuffer[3];

	add_CRC(Buffer, sizeof(Buffer));

	memcpy(TxBuffer, Buffer, sizeof(Buffer));

	*TxLen = sizeof(Buffer);
	
	// update previous block buffer and length
	memcpy(send_previous_block, Buffer, sizeof(Buffer));

	send_previous_length = sizeof(Buffer);

	return I2CSW_SUCCESS;
}

static uint16_t respond_LEN_RES(uint8_t* TxBuffer, uint16_t* TxLen, uint8_t* RxBuffer)
{
	memcpy(TxBuffer, rgbNextResponseApduBuf, next_tx_response_apdu_length);
	*TxLen = next_tx_response_apdu_length;
	
	// update previous block buffer and length
	memcpy(send_previous_block, rgbNextResponseApduBuf, next_tx_response_apdu_length);

	send_previous_length = next_tx_response_apdu_length;

	return I2CSW_SUCCESS;
}

static uint16_t wrap_I2C_CIP_Res(uint8_t* TxBuffer, uint16_t* TxLen)
{
	uint8_t Buffer[CIP_RESPONSE_LENGTH];

	Buffer[0] = NAD_RES;
	Buffer[1] = CIP_RES;
	Buffer[2] = CIP_LEN; // Indicating that I-Block is of 21-bytes
	Buffer[3] = PVER;
	Buffer[4] = 0x00;
	Buffer[5] = 0x00;
	Buffer[6] = 0x00;
	Buffer[7] = 0x00;
	Buffer[8] = RID;
	Buffer[9] = PLID;
	Buffer[10] = PLP_LEN;
	Buffer[11] = CLK_STR;
	Buffer[12] = PWT;
	Buffer[13] = MCF_1;
	Buffer[14] = MCF_2;
	Buffer[15] = MPOT;
	Buffer[16] = 0x00;
	Buffer[17] = RWGT;
	Buffer[18] = DLLP_LEN;
	Buffer[19] = 0x00;
	Buffer[20] = BWT;
	Buffer[21] = 0x00;
	Buffer[22] = IFSC;
	Buffer[23] = HB_LEN;

	add_CRC(Buffer, sizeof(Buffer));

	memcpy(TxBuffer, Buffer, sizeof(Buffer));

	*TxLen = sizeof(Buffer);
	
	// update previous block buffer and length
	memcpy(send_previous_block, Buffer, sizeof(Buffer));

	send_previous_length = sizeof(Buffer);

	return I2CSW_SUCCESS;
}

static uint16_t res_I2C_CIP(uint8_t* TxBuf, uint16_t* Tx_Len)
{
	uint16_t ret = I2CSW_EXEC_ERROR;

	PCB_IBLOCK_BLOCK_CHAINING = PCB_IBLOCK_N_BIT_INIT;
	PCB_IBLOCK = PCB_IBLOCK_N_BIT_INIT;
	blockChainLength = 0;
	current_Block_chaining_ack = 0;

	ret = wrap_I2C_CIP_Res(TxBuf, Tx_Len);

	return ret;
}

static uint16_t res_I2C_LEN_REQ(uint8_t* TxBuf, uint16_t* Tx_Len, uint8_t* RxBuf)
{
	uint16_t ret = I2CSW_EXEC_ERROR;

	ret = wrap_I2C_LEN_Req_Res(TxBuf, Tx_Len, RxBuf);

	return ret;
}

static uint16_t res_I2C_LEN_RES(uint8_t* TxBuf, uint16_t* Tx_Len, uint8_t* RxBuf)
{
	uint16_t ret = I2CSW_EXEC_ERROR;

	ret = respond_LEN_RES(TxBuf, Tx_Len, RxBuf);

	return ret;
}

static uint16_t res_I2C_RESYNC(uint8_t* TxBuf, uint16_t* Tx_Len)
{
	uint16_t ret  = I2CSW_EXEC_ERROR;

	ret = wrap_I2C_RESYNC_Res(TxBuf, Tx_Len);

	return  ret;
}

static uint16_t res_I2C_IFS(uint8_t* TxBuf, uint16_t* Tx_Len, uint8_t* RxBuf)
{
	uint16_t ret  = I2CSW_EXEC_ERROR;

	ret = wrap_I2C_IFS_Res(TxBuf, Tx_Len, RxBuf);

	return  ret;
}

static uint16_t res_I2C_ABORT(uint8_t* TxBuf, uint16_t* Tx_Len)
{
	uint16_t ret  = I2CSW_EXEC_ERROR;

	ret = wrap_I2C_ABORT_Res(TxBuf, Tx_Len);

	return  ret;
}

static uint16_t res_I2C_WTX(uint8_t* TxBuf, uint16_t* Tx_Len, uint8_t* RxBuf)
{
	uint16_t ret  = I2CSW_EXEC_ERROR;

	ret = wrap_I2C_WTX_Res(TxBuf, Tx_Len, RxBuf);

	return  ret;
}

static uint16_t res_I2C_RELEASE(uint8_t* TxBuf, uint16_t* Tx_Len)
{
	uint16_t ret  = I2CSW_EXEC_ERROR;

	ret = wrap_I2C_RELEASE_Res(TxBuf, Tx_Len);

	return ret;
}

static uint16_t res_I2C_SWR(uint8_t* TxBuf, uint16_t* Tx_Len)
{
	uint16_t ret = I2CSW_EXEC_ERROR;

	ret = wrap_I2C_SWR_Res(TxBuf, Tx_Len);

	return ret;
}

static uint16_t prepare_Tpdu_Response_Block_Chaining(uint16_t ApduResLength, uint8_t* TxBuffer, uint16_t* TxLen)
{
	uint8_t rgbResponseApduBuffer[255];

	rgbResponseApduBuffer[0] = NAD_RES;
	if ((current_Block_chaining_ack == 0x00) || (current_Block_chaining_ack == ACK_BLOCK_CHAINING_2))
	{
		rgbResponseApduBuffer[1] = ACK_BLOCK_CHAINING_1;
		current_Block_chaining_ack = ACK_BLOCK_CHAINING_1;
	}

	else if (current_Block_chaining_ack == ACK_BLOCK_CHAINING_1)
	{
		rgbResponseApduBuffer[1] = ACK_BLOCK_CHAINING_2;
		current_Block_chaining_ack = ACK_BLOCK_CHAINING_2;
	}

	rgbResponseApduBuffer[2] = 0x00; // Indicating that INF is absent

	add_CRC(rgbResponseApduBuffer, ApduResLength);

	memcpy(TxBuffer, rgbResponseApduBuffer, ApduResLength);

	*TxLen = ApduResLength;
	
	// update previous block buffer and length
	memcpy(send_previous_block, rgbResponseApduBuffer, ApduResLength);

	send_previous_length = ApduResLength;

	return I2CSW_SUCCESS;
}

static uint16_t prepare_Tpdu_Response(const uint8_t* ApduResBuffer, uint16_t ApduResLength, uint8_t* TxBuffer, uint16_t* TxLen)
{
	uint8_t Buffer[255];

	// For next transmission
	Buffer[0] = NAD_RES;
	Buffer[1] = PCB_IBLOCK;
	Buffer[2] = ApduResLength; // Indicating that I-Block is of n-bytes
	memcpy(&Buffer[3], ApduResBuffer, ApduResLength);

	add_CRC(Buffer, ApduResLength + DEFAULT_TPDU_LENGTH);

	memcpy(TxBuffer, Buffer, ApduResLength + DEFAULT_TPDU_LENGTH);
	
	*TxLen = ApduResLength + DEFAULT_TPDU_LENGTH;

	// update previous block buffer and length
	memcpy(send_previous_block, Buffer, ApduResLength + DEFAULT_TPDU_LENGTH);

	send_previous_length = ApduResLength + DEFAULT_TPDU_LENGTH;

	return I2CSW_SUCCESS;
}

// Resynchronization - rules 6.4, 7.1, 7.4.2 and 7.4.3 - During the transmission protocol
UINT16 processScenario_35(uint8_t *rgbRxBuffer, uint16_t length_Rx, uint8_t *rgbTxBuffer, uint16_t *length_Tx)
{
	NORETURN = 0xFF;

	return I2CSW_SUCCESS;
}

// Resynchronization - rules 7.1 and rules 7.4.1
UINT16 processScenario_34(uint8_t *rgbRxBuffer, uint16_t length_Rx, uint8_t *rgbTxBuffer, uint16_t *length_Tx)
{
	uint16_t ret = I2CSW_DUPLICATE_I_BLOCK;
	uint16_t apdu_response_len = 0;
	uint8_t rgbResponseApduBuffer[MAX_APDU_RES_LEN];

	resync_error++;

	if (rgbRxBuffer[TPDU_PCB] == RESYNC_REQ)
	{
		rgbResponseApduBuffer[0] = NAD_RES;
		rgbResponseApduBuffer[1] = RESYNC_RES;
		rgbResponseApduBuffer[2] = INF_NILL; // Indicating that INF is absent

		add_CRC(rgbResponseApduBuffer, DEFAULT_TPDU_LENGTH);

		memcpy(rgbTxBuffer, rgbResponseApduBuffer, DEFAULT_TPDU_LENGTH);

		*length_Tx = DEFAULT_TPDU_LENGTH;

		return ret = I2CSW_SUCCESS;
	}
	else if ((rgbRxBuffer[TPDU_PCB] == PCB_IBLOCK_1) && (resync_error > 1))
	{
		// start transmitting a chain
		PCB_IBLOCK = PCB_IBLOCK_1;

		memcpy(rgbResponseApduBuffer, &rgbRxBuffer[TPDU_LEN], length_Rx - DEFAULT_TPDU_LENGTH);

		apdu_response_len = length_Rx - DEFAULT_TPDU_LENGTH;	

		resync_error = 0x00;

		// Prepare TPDU response
		return ret = prepare_Tpdu_Response(rgbResponseApduBuffer, apdu_response_len, rgbTxBuffer, length_Tx);
	}
	else
	{
		NORETURN = 0xFF;

		return I2CSW_SUCCESS;
	}

	return ret;
}

// Resynchronization - rules 7.1 and rules 7.4.1
UINT16 processScenario_33(uint8_t *rgbRxBuffer, uint16_t length_Rx, uint8_t *rgbTxBuffer, uint16_t *length_Tx)
{
	NORETURN = 0xFF;

	return I2CSW_SUCCESS;
}

// Resynchronization
UINT16 processScenario_32(uint8_t *rgbRxBuffer, uint16_t length_Rx, uint8_t *rgbTxBuffer, uint16_t *length_Tx)
{
	uint16_t ret = I2CSW_DUPLICATE_I_BLOCK;
	uint16_t apdu_response_len = 0;
	uint8_t rgbResponseApduBuffer[MAX_APDU_RES_LEN];

	// Prepare response
	if ((rgbRxBuffer[TPDU_PCB] != RESYNC_REQ) && (rgbRxBuffer[TPDU_PCB] != PCB_IBLOCK_1))
	{
		rgbResponseApduBuffer[0] = NAD_RES;
		rgbResponseApduBuffer[1] = INTRO_ERROR;
		rgbResponseApduBuffer[2] = INF_NILL; // Indicating that INF is absent

		add_CRC(rgbResponseApduBuffer, DEFAULT_TPDU_LENGTH);

		memcpy(rgbTxBuffer, rgbResponseApduBuffer, DEFAULT_TPDU_LENGTH);

		*length_Tx = DEFAULT_TPDU_LENGTH;

		return ret = I2CSW_SUCCESS;
	}
	if (rgbRxBuffer[TPDU_PCB] == RESYNC_REQ)
	{
		rgbResponseApduBuffer[0] = NAD_RES;
		rgbResponseApduBuffer[1] = RESYNC_RES;
		rgbResponseApduBuffer[2] = INF_NILL; // Indicating that INF is absent

		add_CRC(rgbResponseApduBuffer, DEFAULT_TPDU_LENGTH);

		memcpy(rgbTxBuffer, rgbResponseApduBuffer, DEFAULT_TPDU_LENGTH);

		*length_Tx = DEFAULT_TPDU_LENGTH;

		return ret = I2CSW_SUCCESS;
	}
	if (rgbRxBuffer[TPDU_PCB] == PCB_IBLOCK_1)
	{
		// start transmitting a chain
		PCB_IBLOCK = PCB_IBLOCK_1;

		memcpy(rgbResponseApduBuffer, &rgbRxBuffer[TPDU_LEN], length_Rx - DEFAULT_TPDU_LENGTH);

		apdu_response_len = length_Rx - DEFAULT_TPDU_LENGTH;	

		// Prepare TPDU response
		return ret = prepare_Tpdu_Response(rgbResponseApduBuffer, apdu_response_len, rgbTxBuffer, length_Tx);
	}

	return ret;
}

// Resynchronization - rule 6.2 and rule 7.3
UINT16 processScenario_31(uint8_t *rgbRxBuffer, uint16_t length_Rx, uint8_t *rgbTxBuffer, uint16_t *length_Tx)
{
	uint16_t ret = I2CSW_DUPLICATE_I_BLOCK;
	uint16_t apdu_response_len = 0;
	uint8_t rgbResponseApduBuffer[MAX_APDU_RES_LEN];

	// Prepare response
	if ((rgbRxBuffer[TPDU_PCB] != RESYNC_REQ) && (rgbRxBuffer[TPDU_PCB] != PCB_IBLOCK_1))
	{
		rgbResponseApduBuffer[0] = NAD_RES;
		rgbResponseApduBuffer[1] = OTHER_ERROR_1;
		rgbResponseApduBuffer[2] = INF_NILL; // Indicating that INF is absent

		add_CRC(rgbResponseApduBuffer, DEFAULT_TPDU_LENGTH);

		memcpy(rgbTxBuffer, rgbResponseApduBuffer, DEFAULT_TPDU_LENGTH);

		*length_Tx = DEFAULT_TPDU_LENGTH;

		return ret = I2CSW_SUCCESS;
	}
	if (rgbRxBuffer[TPDU_PCB] == RESYNC_REQ)
	{
		rgbResponseApduBuffer[0] = NAD_RES;
		rgbResponseApduBuffer[1] = RESYNC_RES;
		rgbResponseApduBuffer[2] = INF_NILL; // Indicating that INF is absent

		add_CRC(rgbResponseApduBuffer, DEFAULT_TPDU_LENGTH);

		memcpy(rgbTxBuffer, rgbResponseApduBuffer, DEFAULT_TPDU_LENGTH);

		*length_Tx = DEFAULT_TPDU_LENGTH;

		return ret = I2CSW_SUCCESS;
	}
	if (rgbRxBuffer[TPDU_PCB] == PCB_IBLOCK_1)
	{
		// start transmitting a chain
		PCB_IBLOCK = PCB_IBLOCK_1;

		memcpy(rgbResponseApduBuffer, &rgbRxBuffer[TPDU_LEN], length_Rx - DEFAULT_TPDU_LENGTH);

		apdu_response_len = length_Rx - DEFAULT_TPDU_LENGTH;	

		// Prepare TPDU response
		return ret = prepare_Tpdu_Response(rgbResponseApduBuffer, apdu_response_len, rgbTxBuffer, length_Tx);
	}

	return ret;
}

// Resynchronization - rule 6.2 and rule 7.3
UINT16 processScenario_30(uint8_t *rgbRxBuffer, uint16_t length_Rx, uint8_t *rgbTxBuffer, uint16_t *length_Tx)
{
	uint16_t ret = I2CSW_DUPLICATE_I_BLOCK;
	uint16_t apdu_response_len = 0;
	uint8_t rgbResponseApduBuffer[MAX_APDU_RES_LEN];
	
	resync_error++;

	// Prepare response
	if ((rgbRxBuffer[TPDU_PCB] == RESYNC_REQ) && (resync_error == 0x01))
	{
		rgbResponseApduBuffer[0] = NAD_RES;
		rgbResponseApduBuffer[1] = INTRO_ERROR;
		rgbResponseApduBuffer[2] = INF_NILL; // Indicating that INF is absent

		add_CRC(rgbResponseApduBuffer, DEFAULT_TPDU_LENGTH);

		memcpy(rgbTxBuffer, rgbResponseApduBuffer, DEFAULT_TPDU_LENGTH);

		*length_Tx = DEFAULT_TPDU_LENGTH;

		return ret = I2CSW_SUCCESS;
	}
	if ((rgbRxBuffer[TPDU_PCB] == RESYNC_REQ) && (resync_error == 0x02))
	{
		rgbResponseApduBuffer[0] = NAD_RES;
		rgbResponseApduBuffer[1] = RESYNC_RES;
		rgbResponseApduBuffer[2] = INF_NILL; // Indicating that INF is absent

		add_CRC(rgbResponseApduBuffer, DEFAULT_TPDU_LENGTH);

		memcpy(rgbTxBuffer, rgbResponseApduBuffer, DEFAULT_TPDU_LENGTH);

		*length_Tx = DEFAULT_TPDU_LENGTH;

		resync_error = 0x00;

		return ret = I2CSW_SUCCESS;
	}
	if (rgbRxBuffer[TPDU_PCB] == PCB_IBLOCK_1)
	{
		// start transmitting a chain
		PCB_IBLOCK = PCB_IBLOCK_1;

		memcpy(rgbResponseApduBuffer, &rgbRxBuffer[TPDU_LEN], length_Rx - DEFAULT_TPDU_LENGTH);

		apdu_response_len = length_Rx - DEFAULT_TPDU_LENGTH;	

		// Prepare TPDU response
		return ret = prepare_Tpdu_Response(rgbResponseApduBuffer, apdu_response_len, rgbTxBuffer, length_Tx);
	}

	return ret;
}

// Resynchronization - rule 6.2
UINT16 processScenario_29(uint8_t *rgbRxBuffer, uint16_t length_Rx, uint8_t *rgbTxBuffer, uint16_t *length_Tx)
{
	uint16_t ret = I2CSW_DUPLICATE_I_BLOCK;
	uint16_t apdu_response_len = 0;
	uint8_t rgbResponseApduBuffer[MAX_APDU_RES_LEN];

	// Prepare response
	if (rgbRxBuffer[TPDU_PCB] == RESYNC_REQ)
	{
		rgbResponseApduBuffer[0] = NAD_RES;
		rgbResponseApduBuffer[1] = RESYNC_RES;
		rgbResponseApduBuffer[2] = INF_NILL; // Indicating that INF is absent

		add_CRC(rgbResponseApduBuffer, DEFAULT_TPDU_LENGTH);

		memcpy(rgbTxBuffer, rgbResponseApduBuffer, DEFAULT_TPDU_LENGTH);

		*length_Tx = DEFAULT_TPDU_LENGTH;

		return ret = I2CSW_SUCCESS;
	}
	if (rgbRxBuffer[TPDU_PCB] == PCB_IBLOCK_1)
	{
		// start transmitting a chain
		PCB_IBLOCK = PCB_IBLOCK_1;

		memcpy(rgbResponseApduBuffer, &rgbRxBuffer[TPDU_LEN], length_Rx - DEFAULT_TPDU_LENGTH);

		apdu_response_len = length_Rx - DEFAULT_TPDU_LENGTH;	

		// Prepare TPDU response
		return ret = prepare_Tpdu_Response(rgbResponseApduBuffer, apdu_response_len, rgbTxBuffer, length_Tx);
	}

	return ret;
}

// Chain receiver initiates chain abortion - The interface device initiates a chain abortion
UINT16 processScenario_28(uint8_t *rgbRxBuffer, uint16_t length_Rx, uint8_t *rgbTxBuffer, uint16_t *length_Tx)
{
	uint16_t ret = I2CSW_DUPLICATE_I_BLOCK;
	uint8_t rgbResponseApduBuffer[MAX_APDU_RES_LEN];

	// Prepare response
	if (rgbRxBuffer[TPDU_PCB] == PCB_IBLOCK_1)
	{
		// start transmitting a chain
		PCB_IBLOCK = PCB_BLOCK_CHAINING_1;

		rgbResponseApduBuffer[0] = NAD_RES;
		rgbResponseApduBuffer[1] = PCB_IBLOCK;
		rgbResponseApduBuffer[2] = INF_NILL; // Indicating that INF is absent

		add_CRC(rgbResponseApduBuffer, DEFAULT_TPDU_LENGTH);

		memcpy(rgbTxBuffer, rgbResponseApduBuffer, DEFAULT_TPDU_LENGTH);

		*length_Tx = DEFAULT_TPDU_LENGTH;

		return ret = I2CSW_SUCCESS;
	}
	if (rgbRxBuffer[TPDU_PCB] == ACK_BLOCK_CHAINING_1)
	{
		uint8_t Buffer[DEFAULT_TPDU_LENGTH];

		Buffer[0] = NAD_RES;
		Buffer[1] = PCB_BLOCK_CHAINING_2;
		Buffer[2] = INF_NILL;

		add_CRC(Buffer, sizeof(Buffer));

		memcpy(rgbTxBuffer, Buffer, sizeof(Buffer));

		*length_Tx = sizeof(Buffer);
		
		return I2CSW_SUCCESS;
	}

	if (rgbRxBuffer[TPDU_PCB] == ABORT_REQ)
	{
		uint8_t Buffer[DEFAULT_TPDU_LENGTH];

		Buffer[0] = NAD_RES;
		Buffer[1] = ABORT_RES;
		Buffer[2] = INF_NILL;

		add_CRC(Buffer, sizeof(Buffer));

		memcpy(rgbTxBuffer, Buffer, sizeof(Buffer));

		*length_Tx = sizeof(Buffer);
		
		return I2CSW_SUCCESS;
	}

	if (rgbRxBuffer[TPDU_PCB] == PCB_IBLOCK_2)
	{
		NORETURN = 0xFF;

		return I2CSW_SUCCESS;
	}

	return ret;
}

// Chain receiver initiates chain abortion - The card initiates a chain abortion
UINT16 processScenario_27(uint8_t *rgbRxBuffer, uint16_t length_Rx, uint8_t *rgbTxBuffer, uint16_t *length_Tx)
{
	uint16_t ret = I2CSW_DUPLICATE_I_BLOCK;
	uint8_t rgbResponseApduBuffer[MAX_APDU_RES_LEN];

	// Prepare response
	if (rgbRxBuffer[TPDU_PCB] == PCB_BLOCK_CHAINING_1)
	{
		// start transmitting a chain
		PCB_IBLOCK = ACK_BLOCK_CHAINING_1;

		rgbResponseApduBuffer[0] = NAD_RES;
		rgbResponseApduBuffer[1] = PCB_IBLOCK;
		rgbResponseApduBuffer[2] = INF_NILL; // Indicating that INF is absent

		add_CRC(rgbResponseApduBuffer, DEFAULT_TPDU_LENGTH);

		memcpy(rgbTxBuffer, rgbResponseApduBuffer, DEFAULT_TPDU_LENGTH);

		*length_Tx = DEFAULT_TPDU_LENGTH;

		return ret = I2CSW_SUCCESS;
	}
	if (rgbRxBuffer[TPDU_PCB] == PCB_BLOCK_CHAINING_2)
	{
		uint8_t Buffer[DEFAULT_TPDU_LENGTH];

		Buffer[0] = NAD_RES;
		Buffer[1] = ABORT_REQ;
		Buffer[2] = INF_NILL;

		add_CRC(Buffer, sizeof(Buffer));

		memcpy(rgbTxBuffer, Buffer, sizeof(Buffer));

		*length_Tx = sizeof(Buffer);
		
		return I2CSW_SUCCESS;
	}

	if (rgbRxBuffer[TPDU_PCB] == ABORT_RES)
	{
		uint8_t Buffer[DEFAULT_TPDU_LENGTH];

		Buffer[0] = NAD_RES;
		Buffer[1] = ACK_BLOCK_CHAINING_2;
		Buffer[2] = INF_NILL;

		add_CRC(Buffer, sizeof(Buffer));

		memcpy(rgbTxBuffer, Buffer, sizeof(Buffer));

		*length_Tx = sizeof(Buffer);
		
		return I2CSW_SUCCESS;
	}

	if (rgbRxBuffer[TPDU_PCB] == PCB_IBLOCK_1)
	{
		NORETURN = 0xFF;

		return I2CSW_SUCCESS;
	}

	return ret;
}

// Card initiates a chain Abortion - A3.4.3- rule 9
UINT16 processScenario_26(uint8_t *rgbRxBuffer, uint16_t length_Rx, uint8_t *rgbTxBuffer, uint16_t *length_Tx)
{
	uint16_t ret = I2CSW_DUPLICATE_I_BLOCK;
	uint16_t apdu_response_len = 0;
	uint8_t rgbResponseApduBuffer[MAX_APDU_RES_LEN];

	// Prepare response
	if (rgbRxBuffer[TPDU_PCB] == PCB_IBLOCK_1)
	{
		// start transmitting a chain
		PCB_IBLOCK = PCB_BLOCK_CHAINING_1;

		rgbResponseApduBuffer[0] = NAD_RES;
		rgbResponseApduBuffer[1] = PCB_IBLOCK;
		rgbResponseApduBuffer[2] = INF_NILL; // Indicating that INF is absent

		add_CRC(rgbResponseApduBuffer, DEFAULT_TPDU_LENGTH);

		memcpy(rgbTxBuffer, rgbResponseApduBuffer, DEFAULT_TPDU_LENGTH);

		*length_Tx = DEFAULT_TPDU_LENGTH;

		return ret = I2CSW_SUCCESS;
	}
	if (rgbRxBuffer[TPDU_PCB] == ACK_BLOCK_CHAINING_1)
	{
		uint8_t Buffer[DEFAULT_TPDU_LENGTH];

		Buffer[0] = NAD_RES;
		Buffer[1] = ABORT_REQ;
		Buffer[2] = INF_NILL;

		add_CRC(Buffer, sizeof(Buffer));

		memcpy(rgbTxBuffer, Buffer, sizeof(Buffer));

		*length_Tx = sizeof(Buffer);
		
		return I2CSW_SUCCESS;
	}

	if (rgbRxBuffer[TPDU_PCB] == ABORT_RES)
	{
		apdu_response_len = 5;
		rgbResponseApduBuffer[0] = NAD_RES;
		rgbResponseApduBuffer[1] = PCB_IBLOCK_2;
		rgbResponseApduBuffer[2] = 0x05; // Indicating that INF is absent
		rgbResponseApduBuffer[3] = 0x01;
		rgbResponseApduBuffer[4] = 0x01;
		rgbResponseApduBuffer[5] = 0x01;
		rgbResponseApduBuffer[6] = 0x01;
		rgbResponseApduBuffer[7] = 0x01;

		add_CRC(rgbResponseApduBuffer, apdu_response_len);

		memcpy(rgbTxBuffer, rgbResponseApduBuffer, apdu_response_len);

		*length_Tx = apdu_response_len;

		return ret = I2CSW_SUCCESS;
	}

	if (rgbRxBuffer[TPDU_PCB] == PCB_IBLOCK_2)
	{
		PCB_IBLOCK = PCB_IBLOCK_1;
		memcpy(rgbResponseApduBuffer, &rgbRxBuffer[TPDU_LEN], length_Rx - DEFAULT_TPDU_LENGTH);

		apdu_response_len = length_Rx - DEFAULT_TPDU_LENGTH;		

		// Prepare TPDU response
		return ret = prepare_Tpdu_Response(rgbResponseApduBuffer, apdu_response_len, rgbTxBuffer, length_Tx);
	}

	return ret;
}

// Chain transmitter initiates chain abortion - A3.4.3- rule 9
UINT16 processScenario_25(uint8_t *rgbRxBuffer, uint16_t length_Rx, uint8_t *rgbTxBuffer, uint16_t *length_Tx)
{
	uint16_t ret = I2CSW_DUPLICATE_I_BLOCK;
	uint16_t apdu_response_len = 0;
	uint8_t rgbResponseApduBuffer[MAX_APDU_RES_LEN];

	// Prepare response
	if (rgbRxBuffer[TPDU_PCB] == PCB_BLOCK_CHAINING_1)
	{
		// start transmitting a chain
		PCB_IBLOCK = ACK_BLOCK_CHAINING_1;

		rgbResponseApduBuffer[0] = NAD_RES;
		rgbResponseApduBuffer[1] = PCB_IBLOCK;
		rgbResponseApduBuffer[2] = INF_NILL; // Indicating that INF is absent

		add_CRC(rgbResponseApduBuffer, DEFAULT_TPDU_LENGTH);

		memcpy(rgbTxBuffer, rgbResponseApduBuffer, DEFAULT_TPDU_LENGTH);

		*length_Tx = DEFAULT_TPDU_LENGTH;

		return ret = I2CSW_SUCCESS;
	}
	if (rgbRxBuffer[TPDU_PCB] == ABORT_REQ)
	{
		uint8_t Buffer[DEFAULT_TPDU_LENGTH];

		Buffer[0] = NAD_RES;
		Buffer[1] = ABORT_RES;
		Buffer[2] = INF_NILL;

		add_CRC(Buffer, sizeof(Buffer));

		memcpy(rgbTxBuffer, Buffer, sizeof(Buffer));

		*length_Tx = sizeof(Buffer);
		
		return I2CSW_SUCCESS;
	}

	if (rgbRxBuffer[TPDU_PCB] == PCB_IBLOCK_2)
	{
		apdu_response_len = 5;
		rgbResponseApduBuffer[0] = NAD_RES;
		rgbResponseApduBuffer[1] = PCB_IBLOCK_1;
		rgbResponseApduBuffer[2] = 0x05; // Indicating that INF is absent
		rgbResponseApduBuffer[3] = 0x01;
		rgbResponseApduBuffer[4] = 0x01;
		rgbResponseApduBuffer[5] = 0x01;
		rgbResponseApduBuffer[6] = 0x01;
		rgbResponseApduBuffer[7] = 0x01;

		add_CRC(rgbResponseApduBuffer, apdu_response_len);

		memcpy(rgbTxBuffer, rgbResponseApduBuffer, apdu_response_len);

		*length_Tx = apdu_response_len;

		return ret = I2CSW_SUCCESS;
	}

	if (rgbRxBuffer[TPDU_PCB] == PCB_IBLOCK_1)
	{
		NORETURN = 0xFF;

		return I2CSW_SUCCESS;
	}

	return ret;
}

// Chaining Function - A3.4.2 - The card transmits a chain - rule 7.1
UINT16 processScenario_24(uint8_t *rgbRxBuffer, uint16_t length_Rx, uint8_t *rgbTxBuffer, uint16_t *length_Tx)
{
	uint16_t ret = I2CSW_DUPLICATE_I_BLOCK;
	uint16_t apdu_response_len = 0;
	uint8_t rgbResponseApduBuffer[MAX_APDU_RES_LEN];

	// Prepare response
	if (rgbRxBuffer[TPDU_PCB] == PCB_IBLOCK_1)
	{
		// start transmitting a chain
		PCB_IBLOCK = PCB_BLOCK_CHAINING_1;

		rgbResponseApduBuffer[0] = NAD_RES;
		rgbResponseApduBuffer[1] = PCB_IBLOCK;
		rgbResponseApduBuffer[2] = INF_NILL; // Indicating that INF is absent

		add_CRC(rgbResponseApduBuffer, DEFAULT_TPDU_LENGTH);

		memcpy(rgbTxBuffer, rgbResponseApduBuffer, DEFAULT_TPDU_LENGTH);

		*length_Tx = DEFAULT_TPDU_LENGTH;

		return ret = I2CSW_SUCCESS;
	}
	if ((rgbRxBuffer[TPDU_PCB] != ACK_BLOCK_CHAINING_1) && (rgbRxBuffer[TPDU_PCB] != PCB_IBLOCK_1) && (rgbRxBuffer[TPDU_PCB] != PCB_IBLOCK_2))
	{
		apdu_response_len = DEFAULT_TPDU_LENGTH;
		rgbResponseApduBuffer[0] = NAD_RES;
		rgbResponseApduBuffer[1] = INTRO_ERROR;
		rgbResponseApduBuffer[2] = 0x00; // Indicating that INF is absent

		add_CRC(rgbResponseApduBuffer, apdu_response_len);

		memcpy(rgbTxBuffer, rgbResponseApduBuffer, apdu_response_len);

		*length_Tx = apdu_response_len;

		return ret = I2CSW_SUCCESS;
	}

	if (rgbRxBuffer[TPDU_PCB] == ACK_BLOCK_CHAINING_1)
	{
		apdu_response_len = 5;
		rgbResponseApduBuffer[0] = NAD_RES;
		rgbResponseApduBuffer[1] = PCB_IBLOCK_2;
		rgbResponseApduBuffer[2] = 0x05; // Indicating that INF is absent
		rgbResponseApduBuffer[3] = 0x01;
		rgbResponseApduBuffer[4] = 0x01;
		rgbResponseApduBuffer[5] = 0x01;
		rgbResponseApduBuffer[6] = 0x01;
		rgbResponseApduBuffer[7] = 0x01;

		add_CRC(rgbResponseApduBuffer, apdu_response_len);

		memcpy(rgbTxBuffer, rgbResponseApduBuffer, apdu_response_len);

		*length_Tx = apdu_response_len;

		return ret = I2CSW_SUCCESS;
	}

	if (rgbRxBuffer[TPDU_PCB] == PCB_IBLOCK_2)
	{
		PCB_IBLOCK = PCB_IBLOCK_1;
		memcpy(rgbResponseApduBuffer, &rgbRxBuffer[TPDU_LEN], length_Rx - DEFAULT_TPDU_LENGTH);

		apdu_response_len = length_Rx - DEFAULT_TPDU_LENGTH;		

		// Prepare TPDU response
		return ret = prepare_Tpdu_Response(rgbResponseApduBuffer, apdu_response_len, rgbTxBuffer, length_Tx);
	}

	return ret;
}

// Chaining Function - A3.4.2 - The card transmits a chain - rule 7.1
UINT16 processScenario_23(uint8_t *rgbRxBuffer, uint16_t length_Rx, uint8_t *rgbTxBuffer, uint16_t *length_Tx)
{
	uint16_t ret = I2CSW_DUPLICATE_I_BLOCK;
	uint16_t apdu_response_len = 0;
	uint8_t rgbResponseApduBuffer[MAX_APDU_RES_LEN];

	// Prepare response
	if (rgbRxBuffer[TPDU_PCB] == PCB_IBLOCK_1)
	{
		// start transmitting a chain
		PCB_IBLOCK = PCB_BLOCK_CHAINING_1;

		rgbResponseApduBuffer[0] = NAD_RES;
		rgbResponseApduBuffer[1] = PCB_IBLOCK;
		rgbResponseApduBuffer[2] = INF_NILL; // Indicating that INF is absent

		add_CRC(rgbResponseApduBuffer, DEFAULT_TPDU_LENGTH);

		memcpy(rgbTxBuffer, rgbResponseApduBuffer, DEFAULT_TPDU_LENGTH);

		*length_Tx = DEFAULT_TPDU_LENGTH;

		return ret = I2CSW_SUCCESS;
	}
	if ((rgbRxBuffer[TPDU_PCB] != ACK_BLOCK_CHAINING_1) && (rgbRxBuffer[TPDU_PCB] != PCB_IBLOCK_1) && (rgbRxBuffer[TPDU_PCB] != PCB_IBLOCK_2))
	{
		apdu_response_len = DEFAULT_TPDU_LENGTH;
		rgbResponseApduBuffer[0] = NAD_RES;
		rgbResponseApduBuffer[1] = OTHER_ERROR_2;
		rgbResponseApduBuffer[2] = 0x00; // Indicating that INF is absent

		add_CRC(rgbResponseApduBuffer, apdu_response_len);

		memcpy(rgbTxBuffer, rgbResponseApduBuffer, apdu_response_len);

		*length_Tx = apdu_response_len;

		return ret = I2CSW_SUCCESS;
	}

	if (rgbRxBuffer[TPDU_PCB] == ACK_BLOCK_CHAINING_1)
	{
		apdu_response_len = 5;
		rgbResponseApduBuffer[0] = NAD_RES;
		rgbResponseApduBuffer[1] = PCB_IBLOCK_2;
		rgbResponseApduBuffer[2] = 0x05; // Indicating that INF is absent
		rgbResponseApduBuffer[3] = 0x01;
		rgbResponseApduBuffer[4] = 0x01;
		rgbResponseApduBuffer[5] = 0x01;
		rgbResponseApduBuffer[6] = 0x01;
		rgbResponseApduBuffer[7] = 0x01;

		add_CRC(rgbResponseApduBuffer, apdu_response_len);

		memcpy(rgbTxBuffer, rgbResponseApduBuffer, apdu_response_len);

		*length_Tx = apdu_response_len;

		return ret = I2CSW_SUCCESS;
	}

	if (rgbRxBuffer[TPDU_PCB] == PCB_IBLOCK_2)
	{
		PCB_IBLOCK = PCB_IBLOCK_1;
		memcpy(rgbResponseApduBuffer, &rgbRxBuffer[TPDU_LEN], length_Rx - DEFAULT_TPDU_LENGTH);

		apdu_response_len = length_Rx - DEFAULT_TPDU_LENGTH;		

		// Prepare TPDU response
		return ret = prepare_Tpdu_Response(rgbResponseApduBuffer, apdu_response_len, rgbTxBuffer, length_Tx);
	}

	return ret;
}

// Chaining Function - A3.4.1 - The interface device transmits a chain - rule 7.1
UINT16 processScenario_22(uint8_t *rgbRxBuffer, uint16_t length_Rx, uint8_t *rgbTxBuffer, uint16_t *length_Tx)
{
	uint16_t ret = I2CSW_DUPLICATE_I_BLOCK;
	uint16_t apdu_response_len = 0;
	uint8_t rgbResponseApduBuffer[MAX_APDU_RES_LEN];

	// Prepare response
	if (rgbRxBuffer[TPDU_PCB] == PCB_BLOCK_CHAINING_1)
	{
		apdu_response_len = DEFAULT_TPDU_LENGTH;
		rgbResponseApduBuffer[0] = NAD_RES;
		rgbResponseApduBuffer[1] = INTRO_ERROR;
		rgbResponseApduBuffer[2] = 0x00; // Indicating that INF is absent

		add_CRC(rgbResponseApduBuffer, apdu_response_len);

		memcpy(rgbTxBuffer, rgbResponseApduBuffer, apdu_response_len);

		*length_Tx = apdu_response_len;

		return ret = I2CSW_SUCCESS;
	}
	if ((rgbRxBuffer[TPDU_PCB] != OTHER_ERROR_1) && (rgbRxBuffer[TPDU_PCB] != PCB_BLOCK_CHAINING_1) && (rgbRxBuffer[TPDU_PCB] != PCB_BLOCK_CHAINING_2) && (rgbRxBuffer[TPDU_PCB] != PCB_IBLOCK_1) && (rgbRxBuffer[TPDU_PCB] != PCB_IBLOCK_2))
	{
		apdu_response_len = DEFAULT_TPDU_LENGTH;
		rgbResponseApduBuffer[0] = NAD_RES;
		rgbResponseApduBuffer[1] = ACK_BLOCK_CHAINING_1;
		rgbResponseApduBuffer[2] = 0x00; // Indicating that INF is absent

		add_CRC(rgbResponseApduBuffer, apdu_response_len);

		memcpy(rgbTxBuffer, rgbResponseApduBuffer, apdu_response_len);

		*length_Tx = apdu_response_len;

		return ret = I2CSW_SUCCESS;
	}

	if (rgbRxBuffer[TPDU_PCB] == PCB_BLOCK_CHAINING_2)
	{
		apdu_response_len = DEFAULT_TPDU_LENGTH;
		rgbResponseApduBuffer[0] = NAD_RES;
		rgbResponseApduBuffer[1] = ACK_BLOCK_CHAINING_2;
		rgbResponseApduBuffer[2] = 0x00; // Indicating that INF is absent

		add_CRC(rgbResponseApduBuffer, apdu_response_len);

		memcpy(rgbTxBuffer, rgbResponseApduBuffer, apdu_response_len);

		*length_Tx = apdu_response_len;

		return ret = I2CSW_SUCCESS;
	}

	if (rgbRxBuffer[TPDU_PCB] == PCB_IBLOCK_1)
	{
		PCB_IBLOCK = PCB_IBLOCK_1;
		memcpy(rgbResponseApduBuffer, &rgbRxBuffer[TPDU_LEN], length_Rx - DEFAULT_TPDU_LENGTH);

		apdu_response_len = length_Rx - DEFAULT_TPDU_LENGTH;		

		// Prepare TPDU response
		return ret = prepare_Tpdu_Response(rgbResponseApduBuffer, apdu_response_len, rgbTxBuffer, length_Tx);
	}

	if (rgbRxBuffer[TPDU_PCB] == PCB_IBLOCK_2)
	{
		NORETURN = 0xFF;

		return I2CSW_SUCCESS;
	}

	return ret;
}

// Chaining Function - A3.4.1 - The interface device transmits a chain
UINT16 processScenario_21(uint8_t *rgbRxBuffer, uint16_t length_Rx, uint8_t *rgbTxBuffer, uint16_t *length_Tx)
{
	uint16_t ret = I2CSW_DUPLICATE_I_BLOCK;
	uint16_t apdu_response_len = 0;
	uint8_t rgbResponseApduBuffer[MAX_APDU_RES_LEN];

	// Prepare response
	if (rgbRxBuffer[TPDU_PCB] == PCB_BLOCK_CHAINING_1)
	{
		apdu_response_len = DEFAULT_TPDU_LENGTH;
		rgbResponseApduBuffer[0] = NAD_RES;
		rgbResponseApduBuffer[1] = INTRO_ERROR;
		rgbResponseApduBuffer[2] = 0x00; // Indicating that INF is absent

		add_CRC(rgbResponseApduBuffer, apdu_response_len);

		memcpy(rgbTxBuffer, rgbResponseApduBuffer, apdu_response_len);

		*length_Tx = apdu_response_len;

		return ret = I2CSW_SUCCESS;
	}
	if (rgbRxBuffer[TPDU_PCB] == OTHER_ERROR_1)
	{
		apdu_response_len = DEFAULT_TPDU_LENGTH;
		rgbResponseApduBuffer[0] = NAD_RES;
		rgbResponseApduBuffer[1] = ACK_BLOCK_CHAINING_1;
		rgbResponseApduBuffer[2] = 0x00; // Indicating that INF is absent

		add_CRC(rgbResponseApduBuffer, apdu_response_len);

		memcpy(rgbTxBuffer, rgbResponseApduBuffer, apdu_response_len);

		*length_Tx = apdu_response_len;

		return ret = I2CSW_SUCCESS;
	}

	if (rgbRxBuffer[TPDU_PCB] == PCB_BLOCK_CHAINING_2)
	{
		apdu_response_len = DEFAULT_TPDU_LENGTH;
		rgbResponseApduBuffer[0] = NAD_RES;
		rgbResponseApduBuffer[1] = ACK_BLOCK_CHAINING_2;
		rgbResponseApduBuffer[2] = 0x00; // Indicating that INF is absent

		add_CRC(rgbResponseApduBuffer, apdu_response_len);

		memcpy(rgbTxBuffer, rgbResponseApduBuffer, apdu_response_len);

		*length_Tx = apdu_response_len;

		return ret = I2CSW_SUCCESS;
	}

	if (rgbRxBuffer[TPDU_PCB] == PCB_IBLOCK_1)
	{
		memcpy(rgbResponseApduBuffer, &rgbRxBuffer[TPDU_LEN], length_Rx - DEFAULT_TPDU_LENGTH);

		apdu_response_len = length_Rx - DEFAULT_TPDU_LENGTH;		

		// Prepare TPDU response
		return ret = prepare_Tpdu_Response(rgbResponseApduBuffer, apdu_response_len, rgbTxBuffer, length_Tx);
	}

	if (rgbRxBuffer[TPDU_PCB] == PCB_IBLOCK_2)
	{
		NORETURN = 0xFF;

		return I2CSW_SUCCESS;
	}

	return ret;
}

// IFS Adjustment - rule 7.3 - The card requests for an IFS adjustment
UINT16 processScenario_20(uint8_t *rgbRxBuffer, uint16_t length_Rx, uint8_t *rgbTxBuffer, uint16_t *length_Tx)
{
	uint16_t ret = I2CSW_DUPLICATE_I_BLOCK;
	uint16_t apdu_response_len = 0;
	uint8_t rgbResponseApduBuffer[MAX_APDU_RES_LEN];

	// Prepare response
	if (rgbRxBuffer[TPDU_PCB] == PCB_IBLOCK_1)
	{
		uint8_t Buffer[IFS_RESPONSE_LENGTH];

		Buffer[0] = NAD_RES;
		Buffer[1] = IFS_REQ;
		Buffer[2] = 0x02; // I-Block is same as the one which we received during IFS_REQ
		Buffer[3] = rgbRxBuffer[3];

		add_CRC(Buffer, sizeof(Buffer));

		memcpy(rgbTxBuffer, Buffer, sizeof(Buffer));

		*length_Tx = sizeof(Buffer);
		
		return I2CSW_SUCCESS;
	}

	if (rgbRxBuffer[TPDU_PCB] == IFS_RES)
	{
		PCB_IBLOCK = INTRO_ERROR;

		rgbResponseApduBuffer[0] = 0x00;
		rgbResponseApduBuffer[1] = 0x00;
		rgbResponseApduBuffer[2] = 0x00;
		rgbResponseApduBuffer[3] = 0x00;
		rgbResponseApduBuffer[4] = 0x00;

		apdu_response_len = 5;	

		// Prepare TPDU response
		return ret = prepare_Tpdu_Response(rgbResponseApduBuffer, apdu_response_len, rgbTxBuffer, length_Tx);
	}

	if ((rgbRxBuffer[TPDU_PCB] != OTHER_ERROR_1) && (rgbRxBuffer[TPDU_PCB] != PCB_IBLOCK_1) && (rgbRxBuffer[TPDU_PCB] != IFS_RES) && (rgbRxBuffer[TPDU_PCB] != PCB_IBLOCK_2))
	{
		PCB_IBLOCK = OTHER_ERROR_2;

		rgbResponseApduBuffer[0] = PCB_IBLOCK;
		rgbResponseApduBuffer[1] = NAD_RES;
		rgbResponseApduBuffer[2] = INF_NILL; // Indicating that INF is absent

		add_CRC(rgbResponseApduBuffer, DEFAULT_TPDU_LENGTH);

		memcpy(rgbTxBuffer, rgbResponseApduBuffer, DEFAULT_TPDU_LENGTH);

		*length_Tx = DEFAULT_TPDU_LENGTH;

		return ret = I2CSW_SUCCESS;
	}
	
	if (rgbRxBuffer[TPDU_PCB] == OTHER_ERROR_1)
	{
		PCB_IBLOCK = PCB_IBLOCK_1;

		rgbResponseApduBuffer[0] = 0x00;
		rgbResponseApduBuffer[1] = 0x00;
		rgbResponseApduBuffer[2] = 0x00;
		rgbResponseApduBuffer[3] = 0x00;
		rgbResponseApduBuffer[4] = 0x00;

		apdu_response_len = 5;	

		// Prepare TPDU response
		return ret = prepare_Tpdu_Response(rgbResponseApduBuffer, apdu_response_len, rgbTxBuffer, length_Tx);
	}

	if (rgbRxBuffer[TPDU_PCB] == PCB_IBLOCK_2)
	{
		NORETURN = 0xFF;

		return I2CSW_SUCCESS;
	}

	return ret;
}

// IFS Adjustment - rule 7.3 - The card requests for an IFS adjustment
UINT16 processScenario_19(uint8_t *rgbRxBuffer, uint16_t length_Rx, uint8_t *rgbTxBuffer, uint16_t *length_Tx)
{
	uint16_t ret = I2CSW_DUPLICATE_I_BLOCK;
	uint16_t apdu_response_len = 0;
	uint8_t rgbResponseApduBuffer[MAX_APDU_RES_LEN];

	// Prepare response
	if (rgbRxBuffer[TPDU_PCB] == PCB_IBLOCK_1)
	{
		uint8_t Buffer[IFS_RESPONSE_LENGTH];

		Buffer[0] = NAD_RES;
		Buffer[1] = IFS_REQ;
		Buffer[2] = 0x02; // I-Block is same as the one which we received during IFS_REQ
		Buffer[3] = rgbRxBuffer[3];

		add_CRC(Buffer, sizeof(Buffer));

		memcpy(rgbTxBuffer, Buffer, sizeof(Buffer));

		*length_Tx = sizeof(Buffer);
		
		return I2CSW_SUCCESS;
	}

	if (rgbRxBuffer[TPDU_PCB] == IFS_RES)
	{
		PCB_IBLOCK = INTRO_ERROR;

		rgbResponseApduBuffer[0] = 0x00;
		rgbResponseApduBuffer[1] = 0x00;
		rgbResponseApduBuffer[2] = 0x00;
		rgbResponseApduBuffer[3] = 0x00;
		rgbResponseApduBuffer[4] = 0x00;

		apdu_response_len = 5;	

		// Prepare TPDU response
		return ret = prepare_Tpdu_Response(rgbResponseApduBuffer, apdu_response_len, rgbTxBuffer, length_Tx);
	}

	if (rgbRxBuffer[TPDU_PCB] == OTHER_ERROR_1)
	{
		PCB_IBLOCK = PCB_IBLOCK_1;

		rgbResponseApduBuffer[0] = 0x00;
		rgbResponseApduBuffer[1] = 0x00;
		rgbResponseApduBuffer[2] = 0x00;
		rgbResponseApduBuffer[3] = 0x00;
		rgbResponseApduBuffer[4] = 0x00;

		apdu_response_len = 5;	

		// Prepare TPDU response
		return ret = prepare_Tpdu_Response(rgbResponseApduBuffer, apdu_response_len, rgbTxBuffer, length_Tx);
	}

	if (rgbRxBuffer[TPDU_PCB] == PCB_IBLOCK_2)
	{
		NORETURN = 0xFF;

		return I2CSW_SUCCESS;
	}

	return ret;
}

// IFS Adjustment - rule 7.3 - The card requests for an IFS adjustment
UINT16 processScenario_18(uint8_t *rgbRxBuffer, uint16_t length_Rx, uint8_t *rgbTxBuffer, uint16_t *length_Tx)
{
	uint16_t ret = I2CSW_DUPLICATE_I_BLOCK;
	uint16_t apdu_response_len = 0;
	uint8_t rgbResponseApduBuffer[MAX_APDU_RES_LEN];

	// Prepare response
	if (rgbRxBuffer[TPDU_PCB] == PCB_IBLOCK_1)
	{
		uint8_t Buffer[IFS_RESPONSE_LENGTH];

		Buffer[0] = NAD_RES;
		Buffer[1] = IFS_REQ;
		Buffer[2] = 0x02; // I-Block is same as the one which we received during IFS_REQ
		Buffer[3] = rgbRxBuffer[3];

		add_CRC(Buffer, sizeof(Buffer));

		memcpy(rgbTxBuffer, Buffer, sizeof(Buffer));

		*length_Tx = sizeof(Buffer);
		
		return I2CSW_SUCCESS;
	}

	if ((rgbRxBuffer[TPDU_PCB] != IFS_RES) && (rgbRxBuffer[TPDU_PCB] != PCB_IBLOCK_1) && (rgbRxBuffer[TPDU_PCB] != PCB_IBLOCK_2))
	{
		uint8_t Buffer[IFS_RESPONSE_LENGTH];

		Buffer[0] = NAD_RES;
		Buffer[1] = IFS_REQ;
		Buffer[2] = 0x02; // I-Block is same as the one which we received during IFS_REQ
		Buffer[3] = rgbRxBuffer[3];

		add_CRC(Buffer, sizeof(Buffer));

		memcpy(rgbTxBuffer, Buffer, sizeof(Buffer));

		*length_Tx = sizeof(Buffer);
		
		return I2CSW_SUCCESS;
	}

	if (rgbRxBuffer[TPDU_PCB] == IFS_RES)
	{
		PCB_IBLOCK = PCB_IBLOCK_1;

		rgbResponseApduBuffer[0] = 0x00;
		rgbResponseApduBuffer[1] = 0x00;
		rgbResponseApduBuffer[2] = 0x00;
		rgbResponseApduBuffer[3] = 0x00;
		rgbResponseApduBuffer[4] = 0x00;

		apdu_response_len = 5;	

		// Prepare TPDU response
		return ret = prepare_Tpdu_Response(rgbResponseApduBuffer, apdu_response_len, rgbTxBuffer, length_Tx);
	}

	if (rgbRxBuffer[TPDU_PCB] == PCB_IBLOCK_2)
	{
		NORETURN = 0xFF;

		return I2CSW_SUCCESS;
	}

	return ret;
}

// IFS Adjustment - rule 7.3 - The card requests for an IFS adjustment
UINT16 processScenario_17(uint8_t *rgbRxBuffer, uint16_t length_Rx, uint8_t *rgbTxBuffer, uint16_t *length_Tx)
{
	uint16_t ret = I2CSW_DUPLICATE_I_BLOCK;
	uint16_t apdu_response_len = 0;
	uint8_t rgbResponseApduBuffer[MAX_APDU_RES_LEN];

	// Prepare response
	if (rgbRxBuffer[TPDU_PCB] == PCB_IBLOCK_1)
	{
		uint8_t Buffer[IFS_RESPONSE_LENGTH];

		Buffer[0] = NAD_RES;
		Buffer[1] = INTRO_ERROR;
		Buffer[2] = 0x02; // I-Block is same as the one which we received during IFS_REQ
		Buffer[3] = rgbRxBuffer[3];

		add_CRC(Buffer, sizeof(Buffer));

		memcpy(rgbTxBuffer, Buffer, sizeof(Buffer));

		*length_Tx = sizeof(Buffer);
		
		return I2CSW_SUCCESS;
	}

	if ((rgbRxBuffer[TPDU_PCB] != OTHER_ERROR_1) && (rgbRxBuffer[TPDU_PCB] != PCB_IBLOCK_1) && (rgbRxBuffer[TPDU_PCB] != PCB_IBLOCK_2) && (rgbRxBuffer[TPDU_PCB] != IFS_RES))
	{
		uint8_t Buffer[IFS_RESPONSE_LENGTH];

		Buffer[0] = NAD_RES;
		Buffer[1] = IFS_REQ;
		Buffer[2] = 0x02; // I-Block is same as the one which we received during IFS_REQ
		Buffer[3] = rgbRxBuffer[3];

		add_CRC(Buffer, sizeof(Buffer));

		memcpy(rgbTxBuffer, Buffer, sizeof(Buffer));

		*length_Tx = sizeof(Buffer);
		
		return I2CSW_SUCCESS;
	}

	if (rgbRxBuffer[TPDU_PCB] == IFS_RES)
	{
		PCB_IBLOCK = PCB_IBLOCK_1;

		rgbResponseApduBuffer[0] = 0x00;
		rgbResponseApduBuffer[1] = 0x00;
		rgbResponseApduBuffer[2] = 0x00;
		rgbResponseApduBuffer[3] = 0x00;
		rgbResponseApduBuffer[4] = 0x00;

		apdu_response_len = 5;	

		// Prepare TPDU response
		return ret = prepare_Tpdu_Response(rgbResponseApduBuffer, apdu_response_len, rgbTxBuffer, length_Tx);
	}

	if (rgbRxBuffer[TPDU_PCB] == PCB_IBLOCK_2)
	{
		NORETURN = 0xFF;

		return I2CSW_SUCCESS;
	}

	return ret;
}

// IFS Adjustment - rule 7.3 - The card requests for an IFS adjustment
UINT16 processScenario_16(uint8_t *rgbRxBuffer, uint16_t length_Rx, uint8_t *rgbTxBuffer, uint16_t *length_Tx)
{
	uint16_t ret = I2CSW_DUPLICATE_I_BLOCK;
	uint16_t apdu_response_len = 0;
	uint8_t rgbResponseApduBuffer[MAX_APDU_RES_LEN];

	// Prepare response
	if (rgbRxBuffer[TPDU_PCB] == PCB_IBLOCK_1)
	{
		uint8_t Buffer[IFS_RESPONSE_LENGTH];

		Buffer[0] = NAD_RES;
		Buffer[1] = INTRO_ERROR;
		Buffer[2] = 0x02; // I-Block is same as the one which we received during IFS_REQ
		Buffer[3] = rgbRxBuffer[3];

		add_CRC(Buffer, sizeof(Buffer));

		memcpy(rgbTxBuffer, Buffer, sizeof(Buffer));

		*length_Tx = sizeof(Buffer);
		
		return I2CSW_SUCCESS;
	}

	if (rgbRxBuffer[TPDU_PCB] == OTHER_ERROR_1)
	{
		uint8_t Buffer[IFS_RESPONSE_LENGTH];

		Buffer[0] = NAD_RES;
		Buffer[1] = IFS_REQ;
		Buffer[2] = 0x02; // I-Block is same as the one which we received during IFS_REQ
		Buffer[3] = rgbRxBuffer[3];

		add_CRC(Buffer, sizeof(Buffer));

		memcpy(rgbTxBuffer, Buffer, sizeof(Buffer));

		*length_Tx = sizeof(Buffer);
		
		return I2CSW_SUCCESS;
	}

	if (rgbRxBuffer[TPDU_PCB] == IFS_RES)
	{
		PCB_IBLOCK = PCB_IBLOCK_1;

		rgbResponseApduBuffer[0] = 0x00;
		rgbResponseApduBuffer[1] = 0x00;
		rgbResponseApduBuffer[2] = 0x00;
		rgbResponseApduBuffer[3] = 0x00;
		rgbResponseApduBuffer[4] = 0x00;

		apdu_response_len = 5;	

		// Prepare TPDU response
		return ret = prepare_Tpdu_Response(rgbResponseApduBuffer, apdu_response_len, rgbTxBuffer, length_Tx);
	}

	return ret;
}

// Waiting Time Extension - rule 7.3 - The card requests for a waiting time extension
UINT16 processScenario_15(uint8_t *rgbRxBuffer, uint16_t length_Rx, uint8_t *rgbTxBuffer, uint16_t *length_Tx)
{
	uint16_t ret = I2CSW_DUPLICATE_I_BLOCK;
	uint16_t apdu_response_len = 0;
	uint8_t rgbResponseApduBuffer[MAX_APDU_RES_LEN];

	// Prepare response
	if (rgbRxBuffer[TPDU_PCB] == PCB_IBLOCK_1)
	{
		uint8_t Buffer[WTX_RESPONSE_LENGTH];

		Buffer[0] = NAD_RES;
		Buffer[1] = INTRO_ERROR;
		Buffer[2] = 0x02; // I-Block is same as the one which we received during IFS_REQ
		Buffer[3] = rgbRxBuffer[3];

		add_CRC(Buffer, sizeof(Buffer));

		memcpy(rgbTxBuffer, Buffer, sizeof(Buffer));

		*length_Tx = sizeof(Buffer);
		
		return I2CSW_SUCCESS;
	}

	if ((rgbRxBuffer[TPDU_PCB] != OTHER_ERROR_1) && (rgbRxBuffer[TPDU_PCB] != PCB_IBLOCK_1) && (rgbRxBuffer[TPDU_PCB] != PCB_IBLOCK_2) && (rgbRxBuffer[TPDU_PCB] != WTX_RES))
	{
		uint8_t Buffer[WTX_RESPONSE_LENGTH];

		Buffer[0] = NAD_RES;
		Buffer[1] = WTX_REQ;
		Buffer[2] = 0x02; // I-Block is same as the one which we received during IFS_REQ
		Buffer[3] = rgbRxBuffer[3];

		add_CRC(Buffer, sizeof(Buffer));

		memcpy(rgbTxBuffer, Buffer, sizeof(Buffer));

		*length_Tx = sizeof(Buffer);
		
		return I2CSW_SUCCESS;
	}

	if (rgbRxBuffer[TPDU_PCB] == WTX_RES)
	{
		PCB_IBLOCK = PCB_IBLOCK_1;

		rgbResponseApduBuffer[0] = 0x00;
		rgbResponseApduBuffer[1] = 0x00;
		rgbResponseApduBuffer[2] = 0x00;
		rgbResponseApduBuffer[3] = 0x00;
		rgbResponseApduBuffer[4] = 0x00;

		apdu_response_len = 5;	

		// Prepare TPDU response
		return ret = prepare_Tpdu_Response(rgbResponseApduBuffer, apdu_response_len, rgbTxBuffer, length_Tx);
	}

	if (rgbRxBuffer[TPDU_PCB] == PCB_IBLOCK_2)
	{
		NORETURN = 0xFF;

		return I2CSW_SUCCESS;
	}

	return ret;
}

// Waiting Time Extension - rule 7.3 - The card requests for a waiting time extension
UINT16 processScenario_14(uint8_t *rgbRxBuffer, uint16_t length_Rx, uint8_t *rgbTxBuffer, uint16_t *length_Tx)
{
	uint16_t ret = I2CSW_DUPLICATE_I_BLOCK;
	uint16_t apdu_response_len = 0;
	uint8_t rgbResponseApduBuffer[MAX_APDU_RES_LEN];

	// Prepare response
	if (rgbRxBuffer[TPDU_PCB] == PCB_IBLOCK_1)
	{
		uint8_t Buffer[WTX_RESPONSE_LENGTH];

		Buffer[0] = NAD_RES;
		Buffer[1] = INTRO_ERROR;
		Buffer[2] = 0x02; // I-Block is same as the one which we received during IFS_REQ
		Buffer[3] = rgbRxBuffer[3];

		add_CRC(Buffer, sizeof(Buffer));

		memcpy(rgbTxBuffer, Buffer, sizeof(Buffer));

		*length_Tx = sizeof(Buffer);
		
		return I2CSW_SUCCESS;
	}

	if (rgbRxBuffer[TPDU_PCB] == OTHER_ERROR_1)
	{
		uint8_t Buffer[WTX_RESPONSE_LENGTH];

		Buffer[0] = NAD_RES;
		Buffer[1] = WTX_REQ;
		Buffer[2] = 0x02; // I-Block is same as the one which we received during IFS_REQ
		Buffer[3] = rgbRxBuffer[3];

		add_CRC(Buffer, sizeof(Buffer));

		memcpy(rgbTxBuffer, Buffer, sizeof(Buffer));

		*length_Tx = sizeof(Buffer);
		
		return I2CSW_SUCCESS;
	}

	if (rgbRxBuffer[TPDU_PCB] == WTX_RES)
	{
		PCB_IBLOCK = PCB_IBLOCK_1;

		rgbResponseApduBuffer[0] = 0x00;
		rgbResponseApduBuffer[1] = 0x00;
		rgbResponseApduBuffer[2] = 0x00;
		rgbResponseApduBuffer[3] = 0x00;
		rgbResponseApduBuffer[4] = 0x00;

		apdu_response_len = 5;	

		// Prepare TPDU response
		return ret = prepare_Tpdu_Response(rgbResponseApduBuffer, apdu_response_len, rgbTxBuffer, length_Tx);
	}

	if (rgbRxBuffer[TPDU_PCB] == PCB_IBLOCK_2)
	{
		NORETURN = 0xFF;

		return I2CSW_SUCCESS;
	}

	return ret;
}

// Error Handling - rules 7.1, 7.2 and 7.6
UINT16 processScenario_13(uint8_t *rgbRxBuffer, uint16_t length_Rx, uint8_t *rgbTxBuffer, uint16_t *length_Tx)
{
	uint16_t ret = I2CSW_DUPLICATE_I_BLOCK;
	uint16_t apdu_response_len = 0;
	uint8_t rgbResponseApduBuffer[MAX_APDU_RES_LEN];

	// Prepare response
	if (rgbRxBuffer[TPDU_PCB] == PCB_IBLOCK_1)
	{
		// start transmitting a chain
		PCB_IBLOCK = INTRO_ERROR;

		memcpy(rgbResponseApduBuffer, &rgbRxBuffer[TPDU_LEN], length_Rx - DEFAULT_TPDU_LENGTH);

		apdu_response_len = length_Rx - DEFAULT_TPDU_LENGTH;	

		// Prepare TPDU response
		return ret = prepare_Tpdu_Response(rgbResponseApduBuffer, apdu_response_len, rgbTxBuffer, length_Tx);
	}

	if ((rgbRxBuffer[TPDU_PCB] != OTHER_ERROR_1) && (rgbRxBuffer[TPDU_PCB] != PCB_IBLOCK_1) && (rgbRxBuffer[TPDU_PCB] != PCB_IBLOCK_2))
	{
		// start transmitting a chain
		PCB_IBLOCK = INTRO_ERROR;

		rgbResponseApduBuffer[0] = PCB_IBLOCK;
		rgbResponseApduBuffer[1] = NAD_RES;
		rgbResponseApduBuffer[2] = INF_NILL; // Indicating that INF is absent

		add_CRC(rgbResponseApduBuffer, DEFAULT_TPDU_LENGTH);

		memcpy(rgbTxBuffer, rgbResponseApduBuffer, DEFAULT_TPDU_LENGTH);

		*length_Tx = DEFAULT_TPDU_LENGTH;

		return ret = I2CSW_SUCCESS;
	}

	if (rgbRxBuffer[TPDU_PCB] == OTHER_ERROR_1)
	{
		PCB_IBLOCK = PCB_IBLOCK_1;

		rgbResponseApduBuffer[0] = 0x00;
		rgbResponseApduBuffer[1] = 0x00;
		rgbResponseApduBuffer[2] = 0x00;
		rgbResponseApduBuffer[3] = 0x00;
		rgbResponseApduBuffer[4] = 0x00;

		apdu_response_len = 5;	

		// Prepare TPDU response
		return ret = prepare_Tpdu_Response(rgbResponseApduBuffer, apdu_response_len, rgbTxBuffer, length_Tx);
	}

	if (rgbRxBuffer[TPDU_PCB] == PCB_IBLOCK_2)
	{
		NORETURN = 0xFF;

		return I2CSW_SUCCESS;
	}

	return ret;
}

// Error Handling - rules 7.1, 7.2 and 7.6
UINT16 processScenario_12(uint8_t *rgbRxBuffer, uint16_t length_Rx, uint8_t *rgbTxBuffer, uint16_t *length_Tx)
{
	uint16_t ret = I2CSW_DUPLICATE_I_BLOCK;
	uint16_t apdu_response_len = 0;
	uint8_t rgbResponseApduBuffer[MAX_APDU_RES_LEN];

	// Prepare response
	if (rgbRxBuffer[TPDU_PCB] == PCB_IBLOCK_1)
	{
		// start transmitting a chain
		PCB_IBLOCK = INTRO_ERROR;

		memcpy(rgbResponseApduBuffer, &rgbRxBuffer[TPDU_LEN], length_Rx - DEFAULT_TPDU_LENGTH);

		apdu_response_len = length_Rx - DEFAULT_TPDU_LENGTH;	

		// Prepare TPDU response
		return ret = prepare_Tpdu_Response(rgbResponseApduBuffer, apdu_response_len, rgbTxBuffer, length_Tx);
	}

	if ((rgbRxBuffer[TPDU_PCB] != OTHER_ERROR_1) && (rgbRxBuffer[TPDU_PCB] != PCB_IBLOCK_1) && (rgbRxBuffer[TPDU_PCB] != PCB_IBLOCK_2))
	{
		PCB_IBLOCK = OTHER_ERROR_2;

		rgbResponseApduBuffer[0] = PCB_IBLOCK;
		rgbResponseApduBuffer[1] = NAD_RES;
		rgbResponseApduBuffer[2] = INF_NILL; // Indicating that INF is absent

		add_CRC(rgbResponseApduBuffer, DEFAULT_TPDU_LENGTH);

		memcpy(rgbTxBuffer, rgbResponseApduBuffer, DEFAULT_TPDU_LENGTH);

		*length_Tx = DEFAULT_TPDU_LENGTH;

		return ret = I2CSW_SUCCESS;
	}

	if (rgbRxBuffer[TPDU_PCB] == OTHER_ERROR_1)
	{
		PCB_IBLOCK = PCB_IBLOCK_1;

		rgbResponseApduBuffer[0] = 0x00;
		rgbResponseApduBuffer[1] = 0x00;
		rgbResponseApduBuffer[2] = 0x00;
		rgbResponseApduBuffer[3] = 0x00;
		rgbResponseApduBuffer[4] = 0x00;

		apdu_response_len = 5;	

		// Prepare TPDU response
		return ret = prepare_Tpdu_Response(rgbResponseApduBuffer, apdu_response_len, rgbTxBuffer, length_Tx);
	}

	if (rgbRxBuffer[TPDU_PCB] == PCB_IBLOCK_2)
	{
		NORETURN = 0xFF;

		return I2CSW_SUCCESS;
	}

	return ret;
}

// Error Handling - rules 7.1 and 7.6
UINT16 processScenario_11(uint8_t *rgbRxBuffer, uint16_t length_Rx, uint8_t *rgbTxBuffer, uint16_t *length_Tx)
{
	uint16_t ret = I2CSW_DUPLICATE_I_BLOCK;
	uint16_t apdu_response_len = 0;
	uint8_t rgbResponseApduBuffer[MAX_APDU_RES_LEN];

	// Prepare response
	if (rgbRxBuffer[TPDU_PCB] == PCB_IBLOCK_1)
	{
		// start transmitting a chain
		PCB_IBLOCK = INTRO_ERROR;

		memcpy(rgbResponseApduBuffer, &rgbRxBuffer[TPDU_LEN], length_Rx - DEFAULT_TPDU_LENGTH);

		apdu_response_len = length_Rx - DEFAULT_TPDU_LENGTH;	

		// Prepare TPDU response
		return ret = prepare_Tpdu_Response(rgbResponseApduBuffer, apdu_response_len, rgbTxBuffer, length_Tx);
	}

	if ((rgbRxBuffer[TPDU_PCB] != OTHER_ERROR_1)&& (rgbRxBuffer[TPDU_PCB] != PCB_IBLOCK_1) && (rgbRxBuffer[TPDU_PCB] != PCB_IBLOCK_2))
	{
		// start transmitting a chain
		PCB_IBLOCK = OTHER_ERROR_2;

		rgbResponseApduBuffer[0] = PCB_IBLOCK;
		rgbResponseApduBuffer[1] = NAD_RES;
		rgbResponseApduBuffer[2] = INF_NILL; // Indicating that INF is absent

		add_CRC(rgbResponseApduBuffer, DEFAULT_TPDU_LENGTH);

		memcpy(rgbTxBuffer, rgbResponseApduBuffer, DEFAULT_TPDU_LENGTH);

		*length_Tx = DEFAULT_TPDU_LENGTH;

		return ret = I2CSW_SUCCESS;
	}

	if (rgbRxBuffer[TPDU_PCB] == OTHER_ERROR_1)
	{
		PCB_IBLOCK = PCB_IBLOCK_1;

		rgbResponseApduBuffer[0] = 0x00;
		rgbResponseApduBuffer[1] = 0x00;
		rgbResponseApduBuffer[2] = 0x00;
		rgbResponseApduBuffer[3] = 0x00;
		rgbResponseApduBuffer[4] = 0x00;

		apdu_response_len = 5;	

		// Prepare TPDU response
		return ret = prepare_Tpdu_Response(rgbResponseApduBuffer, apdu_response_len, rgbTxBuffer, length_Tx);
	}

	if (rgbRxBuffer[TPDU_PCB] == PCB_IBLOCK_2)
	{
		NORETURN = 0xFF;

		return I2CSW_SUCCESS;
	}

	return ret;
}

// Error Handling - rules 7.1, 7.5 and 7.6
UINT16 processScenario_10(uint8_t *rgbRxBuffer, uint16_t length_Rx, uint8_t *rgbTxBuffer, uint16_t *length_Tx)
{
	uint16_t ret = I2CSW_DUPLICATE_I_BLOCK;
	uint16_t apdu_response_len = 0;
	uint8_t rgbResponseApduBuffer[MAX_APDU_RES_LEN];

	// Prepare response
	if ((rgbRxBuffer[TPDU_PCB] != PCB_IBLOCK_1) && (rgbRxBuffer[TPDU_PCB] != OTHER_ERROR_1) && (rgbRxBuffer[TPDU_PCB] != PCB_IBLOCK_2))
	{
		// start transmitting a chain
		PCB_IBLOCK = INTRO_ERROR;

		rgbResponseApduBuffer[0] = PCB_IBLOCK;
		rgbResponseApduBuffer[1] = NAD_RES;
		rgbResponseApduBuffer[2] = INF_NILL; // Indicating that INF is absent

		add_CRC(rgbResponseApduBuffer, DEFAULT_TPDU_LENGTH);

		memcpy(rgbTxBuffer, rgbResponseApduBuffer, DEFAULT_TPDU_LENGTH);

		*length_Tx = DEFAULT_TPDU_LENGTH;

		return ret = I2CSW_SUCCESS;
	}

	if (rgbRxBuffer[TPDU_PCB] == OTHER_ERROR_1)
	{
		// start transmitting a chain
		PCB_IBLOCK = OTHER_ERROR_1;

		rgbResponseApduBuffer[0] = PCB_IBLOCK;
		rgbResponseApduBuffer[1] = NAD_RES;
		rgbResponseApduBuffer[2] = INF_NILL; // Indicating that INF is absent

		add_CRC(rgbResponseApduBuffer, DEFAULT_TPDU_LENGTH);

		memcpy(rgbTxBuffer, rgbResponseApduBuffer, DEFAULT_TPDU_LENGTH);

		*length_Tx = DEFAULT_TPDU_LENGTH;

		return ret = I2CSW_SUCCESS;
	}

	if (rgbRxBuffer[TPDU_PCB] == PCB_IBLOCK_1)
	{
		// start transmitting a chain
		PCB_IBLOCK = PCB_IBLOCK_1;

		memcpy(rgbResponseApduBuffer, &rgbRxBuffer[TPDU_LEN], length_Rx - DEFAULT_TPDU_LENGTH);

		apdu_response_len = length_Rx - DEFAULT_TPDU_LENGTH;	

		// Prepare TPDU response
		return ret = prepare_Tpdu_Response(rgbResponseApduBuffer, apdu_response_len, rgbTxBuffer, length_Tx);
	}

	if (rgbRxBuffer[TPDU_PCB] == PCB_IBLOCK_2)
	{
		NORETURN = 0xFF;

		return I2CSW_SUCCESS;
	}

	return ret;
}

// Error Handling - rules 7.1 and 7.6
UINT16 processScenario_9(uint8_t *rgbRxBuffer, uint16_t length_Rx, uint8_t *rgbTxBuffer, uint16_t *length_Tx)
{
	uint16_t ret = I2CSW_DUPLICATE_I_BLOCK;
	uint16_t apdu_response_len = 0;
	uint8_t rgbResponseApduBuffer[MAX_APDU_RES_LEN];
	
	// Prepare response
	if (rgbRxBuffer[TPDU_PCB] == PCB_IBLOCK_1)
	{
		// start transmitting a chain
		PCB_IBLOCK = INTRO_ERROR;

		memcpy(rgbResponseApduBuffer, &rgbRxBuffer[TPDU_LEN], length_Rx - DEFAULT_TPDU_LENGTH);

		apdu_response_len = length_Rx - DEFAULT_TPDU_LENGTH;	

		// Prepare TPDU response
		return ret = prepare_Tpdu_Response(rgbResponseApduBuffer, apdu_response_len, rgbTxBuffer, length_Tx);
	}
	
	if (rgbRxBuffer[TPDU_PCB] == OTHER_ERROR_1)
	{
		return ret = send_previous_block_RX(rgbTxBuffer, length_Tx);
	}

	if (rgbRxBuffer[TPDU_PCB] == PCB_IBLOCK_2)
	{
		NORETURN = 0xFF;

		return I2CSW_SUCCESS;
	}

	return ret;
}

// Error Handling - Exchange of I-Blocks - At the start of transmission protocol
UINT16 processScenario_8(uint8_t *rgbRxBuffer, uint16_t length_Rx, uint8_t *rgbTxBuffer, uint16_t *length_Tx)
{
	uint16_t ret = I2CSW_DUPLICATE_I_BLOCK;
	uint16_t apdu_response_len = 0;
	uint8_t rgbResponseApduBuffer[MAX_APDU_RES_LEN];

	// Prepare response
	if (rgbRxBuffer[TPDU_PCB] != PCB_IBLOCK_1)
	{
		// start transmitting a chain
		PCB_IBLOCK = OTHER_ERROR_1;

		rgbResponseApduBuffer[0] = PCB_IBLOCK;
		rgbResponseApduBuffer[1] = NAD_RES;
		rgbResponseApduBuffer[2] = INF_NILL; // Indicating that INF is absent

		add_CRC(rgbResponseApduBuffer, DEFAULT_TPDU_LENGTH);

		memcpy(rgbTxBuffer, rgbResponseApduBuffer, DEFAULT_TPDU_LENGTH);

		*length_Tx = DEFAULT_TPDU_LENGTH;

		return ret = I2CSW_SUCCESS;
	}

	if (rgbRxBuffer[TPDU_PCB] == PCB_IBLOCK_1)
	{
		// start transmitting a chain
		PCB_IBLOCK = PCB_IBLOCK_1;

		memcpy(rgbResponseApduBuffer, &rgbRxBuffer[TPDU_LEN], length_Rx - DEFAULT_TPDU_LENGTH);

		apdu_response_len = length_Rx - DEFAULT_TPDU_LENGTH;	

		// Prepare TPDU response
		return ret = prepare_Tpdu_Response(rgbResponseApduBuffer, apdu_response_len, rgbTxBuffer, length_Tx);
	}

	return ret;
}

// Chaining function - The Card uses M-Bit to fore an Acknowledgement for a transmitted i-block
UINT16 processScenario_7(uint8_t *rgbRxBuffer, uint16_t length_Rx, uint8_t *rgbTxBuffer, uint16_t *length_Tx)
{
	uint16_t ret = I2CSW_DUPLICATE_I_BLOCK;
	uint8_t rgbResponseApduBuffer[MAX_APDU_RES_LEN];
	
	// Prepare response
	if (rgbRxBuffer[TPDU_PCB] == PCB_IBLOCK_1)
	{
		// start transmitting a chain
		PCB_IBLOCK = PCB_BLOCK_CHAINING_1;

		rgbResponseApduBuffer[0] = PCB_IBLOCK;
		rgbResponseApduBuffer[1] = NAD_RES;
		rgbResponseApduBuffer[2] = INF_NILL; // Indicating that INF is absent

		add_CRC(rgbResponseApduBuffer, DEFAULT_TPDU_LENGTH);

		memcpy(rgbTxBuffer, rgbResponseApduBuffer, DEFAULT_TPDU_LENGTH);

		*length_Tx = DEFAULT_TPDU_LENGTH;

		return ret = I2CSW_SUCCESS;
	}

	if (rgbRxBuffer[TPDU_PCB] == ACK_BLOCK_CHAINING_1)
	{
		// start transmitting a chain
		PCB_IBLOCK = PCB_IBLOCK_2;

		rgbResponseApduBuffer[0] = PCB_IBLOCK;
		rgbResponseApduBuffer[1] = NAD_RES;
		rgbResponseApduBuffer[2] = INF_NILL; // Indicating that INF is absent

		add_CRC(rgbResponseApduBuffer, DEFAULT_TPDU_LENGTH);

		memcpy(rgbTxBuffer, rgbResponseApduBuffer, DEFAULT_TPDU_LENGTH);

		*length_Tx = DEFAULT_TPDU_LENGTH;

		return ret = I2CSW_SUCCESS;
	}

	if (rgbRxBuffer[TPDU_PCB] == PCB_IBLOCK_2)
	{
		PCB_IBLOCK = rgbRxBuffer[TPDU_PCB];

		// start transmitting a chain
		PCB_IBLOCK = PCB_IBLOCK_1;

		rgbResponseApduBuffer[0] = PCB_IBLOCK;
		rgbResponseApduBuffer[1] = NAD_RES;
		rgbResponseApduBuffer[2] = INF_NILL; // Indicating that INF is absent

		add_CRC(rgbResponseApduBuffer, DEFAULT_TPDU_LENGTH);

		memcpy(rgbTxBuffer, rgbResponseApduBuffer, DEFAULT_TPDU_LENGTH);

		*length_Tx = DEFAULT_TPDU_LENGTH;

		return ret = I2CSW_SUCCESS;
	}

	return ret;
}

// Chaining function - The Card transmits a chain
UINT16 processScenario_6(uint8_t *rgbRxBuffer, uint16_t length_Rx, uint8_t *rgbTxBuffer, uint16_t *length_Tx)
{
	uint16_t ret = I2CSW_DUPLICATE_I_BLOCK;
	uint8_t rgbResponseApduBuffer[MAX_APDU_RES_LEN];
	
	// Prepare response
	if (rgbRxBuffer[TPDU_PCB] == PCB_IBLOCK_1)
	{
		// start transmitting a chain
		PCB_IBLOCK = PCB_BLOCK_CHAINING_1;

		rgbResponseApduBuffer[0] = PCB_IBLOCK;
		rgbResponseApduBuffer[1] = NAD_RES;
		rgbResponseApduBuffer[2] = INF_NILL; // Indicating that INF is absent

		add_CRC(rgbResponseApduBuffer, DEFAULT_TPDU_LENGTH);

		memcpy(rgbTxBuffer, rgbResponseApduBuffer, DEFAULT_TPDU_LENGTH);

		*length_Tx = DEFAULT_TPDU_LENGTH;

		return ret = I2CSW_SUCCESS;
	}

	if (rgbRxBuffer[TPDU_PCB] == ACK_BLOCK_CHAINING_1)
	{
		// start transmitting a chain
		PCB_IBLOCK = PCB_IBLOCK_2;

		rgbResponseApduBuffer[0] = PCB_IBLOCK;
		rgbResponseApduBuffer[1] = NAD_RES;
		rgbResponseApduBuffer[2] = INF_NILL; // Indicating that INF is absent

		add_CRC(rgbResponseApduBuffer, DEFAULT_TPDU_LENGTH);

		memcpy(rgbTxBuffer, rgbResponseApduBuffer, DEFAULT_TPDU_LENGTH);

		*length_Tx = DEFAULT_TPDU_LENGTH;

		return ret = I2CSW_SUCCESS;
	}

	if (rgbRxBuffer[TPDU_PCB] == PCB_IBLOCK_2)
	{
		PCB_IBLOCK = rgbRxBuffer[TPDU_PCB];

		// start transmitting a chain
		PCB_IBLOCK = PCB_IBLOCK_1;

		rgbResponseApduBuffer[0] = PCB_IBLOCK;
		rgbResponseApduBuffer[1] = NAD_RES;
		rgbResponseApduBuffer[2] = INF_NILL; // Indicating that INF is absent

		add_CRC(rgbResponseApduBuffer, DEFAULT_TPDU_LENGTH);

		memcpy(rgbTxBuffer, rgbResponseApduBuffer, DEFAULT_TPDU_LENGTH);

		*length_Tx = DEFAULT_TPDU_LENGTH;

		return ret = I2CSW_SUCCESS;
	}

	return ret;
}

// Chaining function - The Interface device transmits a chain
UINT16 processScenario_5(uint8_t *rgbRxBuffer, uint16_t length_Rx, uint8_t *rgbTxBuffer, uint16_t *length_Tx)
{
	uint16_t ret = I2CSW_DUPLICATE_I_BLOCK;
	uint16_t apdu_response_len = 0;
	uint8_t rgbResponseApduBuffer[MAX_APDU_RES_LEN];

	// Prepare response
	if ((rgbRxBuffer[TPDU_PCB] == PCB_BLOCK_CHAINING_1) || ((rgbRxBuffer[TPDU_PCB] == PCB_BLOCK_CHAINING_2)))
	{
		apdu_response_len = DEFAULT_TPDU_LENGTH;
		return ret = prepare_Tpdu_Response_Block_Chaining(apdu_response_len, rgbTxBuffer, length_Tx);
	}

	if (rgbRxBuffer[TPDU_PCB] == PCB_IBLOCK_1)
	{
		PCB_IBLOCK = rgbRxBuffer[TPDU_PCB];

		memcpy(rgbResponseApduBuffer, &rgbRxBuffer[TPDU_LEN], length_Rx - DEFAULT_TPDU_LENGTH);

		apdu_response_len = length_Rx - DEFAULT_TPDU_LENGTH;	

		// Prepare TPDU response
		return ret = prepare_Tpdu_Response(rgbResponseApduBuffer, apdu_response_len, rgbTxBuffer, length_Tx);
	}

	if (rgbRxBuffer[TPDU_PCB] == PCB_IBLOCK_2)
	{
		NORETURN = 0xFF;

		return I2CSW_SUCCESS;
	}

	return ret;
}

// Interface Device initiates an IFS adjustment
UINT16 processScenario_4(uint8_t *rgbRxBuffer, uint16_t length_Rx, uint8_t *rgbTxBuffer, uint16_t *length_Tx)
{
	uint16_t ret = I2CSW_DUPLICATE_I_BLOCK;
	uint16_t apdu_response_len = 0;
	uint8_t rgbResponseApduBuffer[MAX_APDU_RES_LEN];

	PCB_IBLOCK = rgbRxBuffer[TPDU_PCB];

	if (PCB_IBLOCK == PCB_IBLOCK_1)
	{
		memcpy(rgbResponseApduBuffer, &rgbRxBuffer[TPDU_LEN], length_Rx - DEFAULT_TPDU_LENGTH);

		apdu_response_len = length_Rx - DEFAULT_TPDU_LENGTH;	

		// Prepare TPDU response
		return ret = prepare_Tpdu_Response(rgbResponseApduBuffer, apdu_response_len, rgbTxBuffer, length_Tx);
	}
	
	if (PCB_IBLOCK == IFS_REQ)
	{
		uint8_t Buffer[IFS_RESPONSE_LENGTH];

		Buffer[0] = NAD_RES;
		Buffer[1] = IFS_RES;
		Buffer[2] = 0x02; // I-Block is same as the one which we received during IFS_REQ
		Buffer[3] = rgbRxBuffer[3];

		add_CRC(Buffer, sizeof(Buffer));

		memcpy(rgbTxBuffer, Buffer, sizeof(Buffer));

		*length_Tx = sizeof(Buffer);
		
		return I2CSW_SUCCESS;
	}
	else if (PCB_IBLOCK == PCB_IBLOCK_2)
	{
		memcpy(rgbResponseApduBuffer, &rgbRxBuffer[TPDU_LEN], length_Rx - DEFAULT_TPDU_LENGTH);

		apdu_response_len = length_Rx - DEFAULT_TPDU_LENGTH;	

		// Prepare TPDU response
		return ret = prepare_Tpdu_Response(rgbResponseApduBuffer, apdu_response_len, rgbTxBuffer, length_Tx);
	}

	return ret;
}

// Card initiates an IFS adjustment
UINT16 processScenario_3(uint8_t *rgbRxBuffer, uint16_t length_Rx, uint8_t *rgbTxBuffer, uint16_t *length_Tx)
{
	uint16_t ret = I2CSW_DUPLICATE_I_BLOCK;
	uint16_t apdu_response_len = 0;
	uint8_t rgbResponseApduBuffer[MAX_APDU_RES_LEN];

	PCB_IBLOCK = rgbRxBuffer[TPDU_PCB];

	if (PCB_IBLOCK == PCB_IBLOCK_1)
	{
		uint8_t Buffer[IFS_RESPONSE_LENGTH];

		Buffer[0] = NAD_RES;
		Buffer[1] = IFS_REQ;
		Buffer[2] = 0x02; // I-Block is same as the one which we received during IFS_REQ
		Buffer[3] = rgbRxBuffer[3];

		add_CRC(Buffer, sizeof(Buffer));

		memcpy(rgbTxBuffer, Buffer, sizeof(Buffer));

		*length_Tx = sizeof(Buffer);
		
		return I2CSW_SUCCESS;
	}
	else if (PCB_IBLOCK == IFS_RES)
	{
		rgbResponseApduBuffer[0] = 0x01;
		rgbResponseApduBuffer[1] = 0x02;
		rgbResponseApduBuffer[2] = 0x03;
		rgbResponseApduBuffer[3] = 0x04;
		rgbResponseApduBuffer[4] = 0x05;

		apdu_response_len = 5;		

		// Prepare TPDU response
		return ret = prepare_Tpdu_Response(rgbResponseApduBuffer, apdu_response_len, rgbTxBuffer, length_Tx);
	}

	return ret;
}

// Card Requests for waiting time extensions
UINT16 processScenario_2(uint8_t *rgbRxBuffer, uint16_t length_Rx, uint8_t *rgbTxBuffer, uint16_t *length_Tx)
{
	uint16_t ret = I2CSW_DUPLICATE_I_BLOCK;
	uint16_t apdu_response_len = 0;
	uint8_t rgbResponseApduBuffer[MAX_APDU_RES_LEN];

	PCB_IBLOCK = rgbRxBuffer[TPDU_PCB];

	if (PCB_IBLOCK == PCB_IBLOCK_1)
	{
		uint8_t Buffer[WTX_RESPONSE_LENGTH];

		Buffer[0] = NAD_RES;
		Buffer[1] = WTX_REQ;
		Buffer[2] = 0x02; // I-Block is same as the one which we received during IFS_REQ
		Buffer[3] = rgbRxBuffer[3];

		add_CRC(Buffer, sizeof(Buffer));

		memcpy(rgbTxBuffer, Buffer, sizeof(Buffer));

		*length_Tx = sizeof(Buffer);
		
		return I2CSW_SUCCESS;
	}
	else if (PCB_IBLOCK == WTX_RES)
	{
		rgbResponseApduBuffer[0] = 0x01;
		rgbResponseApduBuffer[1] = 0x02;
		rgbResponseApduBuffer[2] = 0x03;
		rgbResponseApduBuffer[3] = 0x04;
		rgbResponseApduBuffer[4] = 0x05;

		apdu_response_len = 5;		

		// Prepare TPDU response
		return ret = prepare_Tpdu_Response(rgbResponseApduBuffer, apdu_response_len, rgbTxBuffer, length_Tx);
	}

	return ret;
}

// Simple Exchange of I-Blocks
UINT16 processScenario_1(uint8_t *rgbRxBuffer, uint16_t length_Rx, uint8_t *rgbTxBuffer, uint16_t *length_Tx)
{
	uint16_t ret = I2CSW_DUPLICATE_I_BLOCK;
	uint16_t apdu_response_len = 0;
	uint8_t rgbResponseApduBuffer[MAX_APDU_RES_LEN];

	PCB_IBLOCK = rgbRxBuffer[TPDU_PCB];

	if (PCB_IBLOCK == PCB_IBLOCK_1)
	{
		memcpy(rgbResponseApduBuffer, &rgbRxBuffer[TPDU_LEN], length_Rx - DEFAULT_TPDU_LENGTH);

		apdu_response_len = length_Rx - DEFAULT_TPDU_LENGTH;		

		// Prepare TPDU response
		return ret = prepare_Tpdu_Response(rgbResponseApduBuffer, apdu_response_len, rgbTxBuffer, length_Tx);
	}
	else if (PCB_IBLOCK == PCB_IBLOCK_2)
	{
		memcpy(rgbResponseApduBuffer, &rgbRxBuffer[TPDU_LEN], length_Rx - DEFAULT_TPDU_LENGTH);

		apdu_response_len = length_Rx - DEFAULT_TPDU_LENGTH;

		// Prepare TPDU response
		return ret = prepare_Tpdu_Response(rgbResponseApduBuffer, apdu_response_len, rgbTxBuffer, length_Tx);
	}

	return ret;
}

static void init_params()
{
	if (initialised == 0xABAB)
	{
		PCB_IBLOCK_BLOCK_CHAINING = PCB_IBLOCK_N_BIT_INIT;
		PCB_IBLOCK = PCB_IBLOCK_N_BIT_INIT;
		blockChainLength = 0;
		current_Block_chaining_ack = 0;
		resync_error = 0x00;
		initialised = 0xFF;
	}
}

UINT16 process_Scenario(uint8_t *rgbTxBuffer, uint16_t* length_Tx, uint8_t *RxBuffer, uint16_t length_Rx)
{
	uint16_t ret = I2CSW_UNKNOWN_SCENARIO;

	// Scenario Selection
		switch(ScenarioState)
		{
			case Scenario_1:
				ret = processScenario_1(RxBuffer, length_Rx, rgbTxBuffer, length_Tx);
				break;
			case Scenario_2:
				ret = processScenario_2(RxBuffer, length_Rx, rgbTxBuffer, length_Tx);
				break;
			case Scenario_3:
				ret = processScenario_3(RxBuffer, length_Rx, rgbTxBuffer, length_Tx);
				break;
			case Scenario_4:
				ret = processScenario_4(RxBuffer, length_Rx, rgbTxBuffer, length_Tx);
				break;
			case Scenario_5:
				ret = processScenario_5(RxBuffer, length_Rx, rgbTxBuffer, length_Tx);
				break;
			case Scenario_6:
				ret = processScenario_6(RxBuffer, length_Rx, rgbTxBuffer, length_Tx);
				break;
			case Scenario_7:
				ret = processScenario_7(RxBuffer, length_Rx, rgbTxBuffer, length_Tx);
				break;
			case Scenario_8:
				ret = processScenario_8(RxBuffer, length_Rx, rgbTxBuffer, length_Tx);
				break;
			case Scenario_9:
				ret = processScenario_9(RxBuffer, length_Rx, rgbTxBuffer, length_Tx);
				break;
			case Scenario_10:
				ret = processScenario_10(RxBuffer, length_Rx, rgbTxBuffer, length_Tx);
				break;
			case Scenario_11:
				ret = processScenario_11(RxBuffer, length_Rx, rgbTxBuffer, length_Tx);
				break;
			case Scenario_12:
				ret = processScenario_12(RxBuffer, length_Rx, rgbTxBuffer, length_Tx);
				break;
			case Scenario_13:
				ret = processScenario_13(RxBuffer, length_Rx, rgbTxBuffer, length_Tx);
				break;
			case Scenario_14:
				ret = processScenario_14(RxBuffer, length_Rx, rgbTxBuffer, length_Tx);
				break;
			case Scenario_15:
				ret = processScenario_15(RxBuffer, length_Rx, rgbTxBuffer, length_Tx);
				break;
			case Scenario_16:
				ret = processScenario_16(RxBuffer, length_Rx, rgbTxBuffer, length_Tx);
				break;
			case Scenario_17:
				ret = processScenario_17(RxBuffer, length_Rx, rgbTxBuffer, length_Tx);
				break;
			case Scenario_18:
				ret = processScenario_18(RxBuffer, length_Rx, rgbTxBuffer, length_Tx);
				break;
			case Scenario_19:
				ret = processScenario_19(RxBuffer, length_Rx, rgbTxBuffer, length_Tx);
				break;
			case Scenario_20:
				ret = processScenario_20(RxBuffer, length_Rx, rgbTxBuffer, length_Tx);
				break;
			case Scenario_21:
				ret = processScenario_21(RxBuffer, length_Rx, rgbTxBuffer, length_Tx);
				break;
			case Scenario_22:
				ret = processScenario_22(RxBuffer, length_Rx, rgbTxBuffer, length_Tx);
				break;
			case Scenario_23:
				ret = processScenario_23(RxBuffer, length_Rx, rgbTxBuffer, length_Tx);
				break;
			case Scenario_24:
				ret = processScenario_24(RxBuffer, length_Rx, rgbTxBuffer, length_Tx);
				break;
			case Scenario_25:
				ret = processScenario_25(RxBuffer, length_Rx, rgbTxBuffer, length_Tx);
				break;
			case Scenario_26:
				ret = processScenario_26(RxBuffer, length_Rx, rgbTxBuffer, length_Tx);
				break;
			case Scenario_27:
				ret = processScenario_27(RxBuffer, length_Rx, rgbTxBuffer, length_Tx);
				break;
			case Scenario_28:
				ret = processScenario_28(RxBuffer, length_Rx, rgbTxBuffer, length_Tx);
				break;
			case Scenario_29:
				ret = processScenario_29(RxBuffer, length_Rx, rgbTxBuffer, length_Tx);
				break;
			case Scenario_30:
				ret = processScenario_30(RxBuffer, length_Rx, rgbTxBuffer, length_Tx);
				break;
			case Scenario_31:
				ret = processScenario_31(RxBuffer, length_Rx, rgbTxBuffer, length_Tx);
				break;
			case Scenario_32:
				ret = processScenario_32(RxBuffer, length_Rx, rgbTxBuffer, length_Tx);
				break;
			case Scenario_33:
				ret = processScenario_33(RxBuffer, length_Rx, rgbTxBuffer, length_Tx);
				break;
			case Scenario_34:
				ret = processScenario_34(RxBuffer, length_Rx, rgbTxBuffer, length_Tx);
				break;
			case Scenario_35:
				ret = processScenario_35(RxBuffer, length_Rx, rgbTxBuffer, length_Tx);
				break;
			default:
				break;
		}
		return ret;
}

static UINT16 prepare_forScenario(uint8_t *rgbScenarioBuffer, uint16_t scenarioLength, uint8_t *txBuffer, uint16_t *txLength)
{
	uint16_t ret = I2CSW_DUPLICATE_I_BLOCK;
	uint16_t apdu_response_len = 0;
	uint8_t rgbResponseApduBuffer[MAX_APDU_RES_LEN];

	memcpy((void *)ScenarioState, &rgbScenarioBuffer[1], scenarioLength - 1);

	if ((ScenarioState == Scenario_29) || (ScenarioState == Scenario_30) || (ScenarioState == Scenario_31) || (ScenarioState == Scenario_32))
	{
		PCB_IBLOCK = PCB_IBLOCK_1;

		rgbResponseApduBuffer[0] = 0x01;
		rgbResponseApduBuffer[1] = 0x02;
		rgbResponseApduBuffer[2] = 0x03;
		rgbResponseApduBuffer[3] = 0x04;
		rgbResponseApduBuffer[4] = 0x05;

		apdu_response_len = 5;		

		// Prepare TPDU response
		return ret = prepare_Tpdu_Response(rgbResponseApduBuffer, apdu_response_len, txBuffer, txLength);
	}
	else
	{
		// return the same bytes you received from master during scenario preparation
		memcpy(txBuffer, rgbScenarioBuffer, scenarioLength);

		*txLength = scenarioLength;

		return ret = I2CSW_SUCCESS;
	}

	return ret;
}

UINT16 unWrapReq_I2C(uint8_t *rgbRxBuffer, uint16_t length_Rx, uint8_t *rgbTxBuffer, uint16_t* length_Tx)
{
	uint8_t RxBuffer[254];
	uint16_t ret = I2CSW_EXEC_ERROR;

	memcpy(RxBuffer, rgbRxBuffer, length_Rx);
	
	init_params();

	NORETURN = 0xAA;

	if (length_Rx == ScenarioSelectionCommand)
	{
		return ret = prepare_forScenario(rgbRxBuffer, length_Rx, rgbTxBuffer, length_Tx);
	}
	else
	{
			// Check if the NAD is proper HD to SE
		if ((ret = check_NAD(rgbRxBuffer, rgbTxBuffer, length_Tx)) != I2CSW_SUCCESS)
			return ret;

		// Check for CRC
		if ((ret = check_CRC(rgbRxBuffer, length_Rx, rgbTxBuffer, length_Tx)) != I2CSW_SUCCESS)
			return ret;

		// Check for proper length LEN and Block size
		if ((ret = check_LEN(rgbRxBuffer, length_Rx, rgbTxBuffer, length_Tx)) != I2CSW_SUCCESS)
			return ret;

		if (ScenarioState != 0xFF)
		{
			return ret = process_Scenario(rgbTxBuffer, length_Tx, RxBuffer, length_Rx);
		}
		else
		{
			switch (RxBuffer[TPDU_PCB])
			{
				case CIP_REQ:
					ret = res_I2C_CIP(rgbTxBuffer, length_Tx);
					break;
				case LEN_REQ: // Method-1
					ret = res_I2C_LEN_REQ(rgbTxBuffer, length_Tx, RxBuffer);
					break;
				case LEN_RES:
					ret = res_I2C_LEN_RES(rgbTxBuffer, length_Tx, RxBuffer);
					break;
				case RESYNC_REQ:
					ret = res_I2C_RESYNC(rgbTxBuffer, length_Tx);
					break;
				case IFS_REQ:
					ret = res_I2C_IFS(rgbTxBuffer, length_Tx, RxBuffer);
					break;
				case ABORT_REQ:
					ret = res_I2C_ABORT(rgbTxBuffer, length_Tx);
					break;
				case WTX_REQ:
					ret = res_I2C_WTX(rgbTxBuffer, length_Tx, RxBuffer);
					break;
				case RELEASE_REQ:
					ret = res_I2C_RELEASE(rgbTxBuffer, length_Tx);
					break;
				case SWR_REQ:
					ret = res_I2C_SWR(rgbTxBuffer, length_Tx);
					break;
				default:
					break;
			}
		}
	}

	return ret;
}

/** @} */// end of IFX_COMMAND_GROUP
