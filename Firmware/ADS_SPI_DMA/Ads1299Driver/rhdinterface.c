#include "spi.h"
#include "userconfig.h"
#include "lwrb.h"

extern 		lwrb_t lwrb_buff;

volatile uint32_t command_transfer_state = TRANSFER_COMPLETE;

uint8_t eCon_Message[150] = {0xBB,0xAA};

/**
  * @brief  更新两片ADS1299的数据
  * @retval None
  */
__attribute__((section (".RAM_D1"))) uint8_t command_sequence_MISO[BYTES_PER_SEQUENCE + 2] = {0xBB, 0xAA};
void updateBoardData(void)
{
	// If previous DMA transfer has not completed, SPI communication from previous sample has not finished.
	// This is a critical error that will halt execution. To avoid this, all processing from previous interrupt
	// must conclude sooner (most likely, this would be waiting on SPI transfer completion, in which case
	// fewer channels can be included in the command sequence, or the SPI communication itself must be sped up).
	if (command_transfer_state == TRANSFER_WAIT) {
		while(1);
	}
	// Update variable indicate to wait until SPI DMA transfer completes.
	command_transfer_state = TRANSFER_WAIT;
	if (HAL_SPI_Receive_DMA(&hspi1, (&command_sequence_MISO[2]), BYTES_PER_SEQUENCE) != HAL_OK)
	{
		Error_Handler();
	}	
}
/*数据包格式：BBAA + 8ch*3bytes*4chips + 校验位 + 标签位 + 电池电量 + 包序号 = 102*/
uint8_t Last4Bytes[4] = {0};

void write_data_to_memory()
{
	lwrb_write(&lwrb_buff, command_sequence_MISO, 2);
	
	for(uint8_t i=5;i<108;i=i+27){
		lwrb_write(&lwrb_buff, &command_sequence_MISO[i], 24);
	}
	//加校验测试
		Last4Bytes[0] = 0;
    for(int i=2;i<110;i++) {
        Last4Bytes[0] += command_sequence_MISO[i];
    }
		Last4Bytes[0] = ~(Last4Bytes[0] - 0xC0 - 0xC0 - 0xC0 - 0xC0);
	//加校验测试	
	Last4Bytes[3]++;
	lwrb_write(&lwrb_buff, Last4Bytes, 4);
}

void HAL_SPI_RxCpltCallback(SPI_HandleTypeDef *hspi)
{
	write_data_to_memory();
	// Update state variable to show that transfer has completed.
	command_transfer_state = TRANSFER_COMPLETE;
}

/*校验位函数*/
uint8_t eCon_Checksum(uint8_t *content,uint8_t len)
{
    uint8_t result = 0;
    for(int i=0;i<len;i++) {
        result += content[i];
    }
    return ~result;
}