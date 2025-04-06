#include "ADS1299_Library.h"
#include "spi.h"
#include "lwrb.h"
//#include "usart.h"
//#include "Debug.h"



#define Dummy_Byte                      0xFF
#define csLow()                      HAL_GPIO_WritePin(SPI1_CS_GPIO_Port, SPI1_CS_Pin, GPIO_PIN_RESET)
#define csHigh()                     HAL_GPIO_WritePin(SPI1_CS_GPIO_Port, SPI1_CS_Pin, GPIO_PIN_SET)

/**
  * @brief  SPI传输读取一字节
  * @param  byte 要传输的字节
  * @retval 读取的字节
  */

uint8_t    ADS_xfer(uint8_t byte)
{
    uint8_t d_read,d_send=byte;

    if(HAL_SPI_TransmitReceive(&hspi1,&d_send,&d_read,1,0xFFFFFF)!=HAL_OK)
        d_read=Dummy_Byte;

    return d_read;
}

uint8_t    regData[24]; // array is used to mirror register data
/**
  * @brief  写寄存器
  * @param  _address  寄存器地址
	* @param  _value    要写入的值
	* @param  target_SS 选择对应的ADS1299
  * @retval None
  */
void ADS_WREG(uint8_t _address, uint8_t _value)
{                                 //  Write ONE register at _address
    uint8_t opcode1 = _address + 0x40; //  WREG expects 010rrrrr where rrrrr = _address
    csLow();               //  open SPI
    ADS_xfer(opcode1);                  //  Send WREG command & address
    ADS_xfer(0x00);                     //  Send number of registers to read -1
    ADS_xfer(_value);                   //  Write the value to the register
    csHigh();              //  close SPI
    regData[_address] = _value;     //  update the mirror array

}
/**
  * @brief  读寄存器
  * @param  _address  寄存器地址
	* @param  target_SS 选择对应的ADS1299
  * @retval 读取的值
  */
uint8_t ADS_RREG(uint8_t _address)
{                                 //  reads ONE register at _address
    uint8_t opcode1 = _address + 0x20; //  RREG expects 001rrrrr where rrrrr = _address
    csLow();                //  open SPI
    ADS_xfer(opcode1);                  //  opcode1
    ADS_xfer(0x00);                     //  opcode2
    regData[_address] = ADS_xfer(0x00); //  update mirror location with returned byte
    csHigh();               //  close SPI

    return regData[_address]; // return requested register value
}
/**
  * @brief  低功耗模式
	* @param  target_SS 选择对应的ADS1299
  * @retval None
  */
void ADS_STANDBY()
{
    csLow();
    ADS_xfer(_STANDBY);
    HAL_Delay(1); //must wait 18 tCLK cycles to execute this command (Datasheet, pg. 35)
    csHigh();
}

/**
  * @brief  从Standby模式唤醒
	* @param  target_SS 选择对应的ADS1299
  * @retval None
  */
void ADS_WAKEUP()
{ // reset all the registers to default settings
    csLow();
    ADS_xfer(_WAKEUP);
    csHigh();
    HAL_Delay(1); //must wait at least 4 tCLK cycles after executing this command (Datasheet, pg. 37)
}
/**
  * @brief  复位
	* @param  target_SS 选择对应的ADS1299
  * @retval None
  */
void ADS_RESET()
{ // reset all the registers to default settings
    csLow();
    ADS_xfer(_RESET);
    HAL_Delay(1); //must wait 18 tCLK cycles to execute this command (Datasheet, pg. 35)
    csHigh();
}
/**
  * @brief  停止数据连续读取模式
	* @param  target_SS 选择对应的ADS1299
  * @retval None
  */
void ADS_SDATAC()
{
    csLow();
    ADS_xfer(_SDATAC);
    csHigh();
    HAL_Delay(1); //must wait at least 4 tCLK cycles after executing this command (Datasheet, pg. 37)
}
/**
  * @brief  读取ADS1299的ID
	* @param  target_SS 选择对应的ADS1299
  * @retval ID值
  */
uint8_t ADS_getDeviceID()
{ // simple hello world com check
    uint8_t data = ADS_RREG(ID_REG);
    return data;
}
/**
  * @brief  ADS1299开始转换
	* @param  target_SS 选择对应的ADS1299
  * @retval None
  */
void  ADS_START()
{ //start data conversion
    csLow();
    ADS_xfer(_START); // KEEP ON-BOARD AND ON-DAISY IN SYNC
    csHigh();
}


void  ADS_STOP()
{
    csLow();
    ADS_xfer(_STOP); 
    csHigh();
}
/**
  * @brief  数据连续读取模式
	* @param  target_SS 选择对应的ADS1299
  * @retval None
  */
void  ADS_RDATAC()
{
    csLow();
    ADS_xfer(_RDATAC); // read data continuous
    csHigh();
//    HAL_Delay(1);
}
/**
  * @brief  初始化Ads1299
	* @param  None
  * @retval None
  */

void initialize_ads(SAMPLE_RATE sr)
{
    HAL_Delay(200);// recommended power up sequence requiers >Tpor (2的18次Tclk) pdf.70

    ADS_RESET();
    HAL_Delay(10);// recommended to wait 18 Tclk before using device

    ADS_SDATAC();
    HAL_Delay(10);

    uint8_t output_data_rate;
    switch(sr) {
        case SAMPLE_RATE_250:
            output_data_rate= 0x06;
            break;
        case SAMPLE_RATE_500:
            output_data_rate= 0x05;
            break;
        case SAMPLE_RATE_1000:
            output_data_rate= 0x04;
            break;
        case SAMPLE_RATE_2000:
						output_data_rate= 0x03;
            break;
        case SAMPLE_RATE_4000:
						output_data_rate= 0x02;
            break;
        case SAMPLE_RATE_8000:
            break;
        case SAMPLE_RATE_16000:
            break;
        default:;
    }
    ADS_WREG(CONFIG1,0x90|output_data_rate);//tell on-board ADS to output its clk, set the data rate to xxxSPS
    HAL_Delay(200);
    ADS_WREG(CONFIG3,0xEC);//BIASREF signal \ BIAS buffer power
    HAL_Delay(10);

    if(ADS_getDeviceID() != ADS_ID) {
        Error_Handler();
    };
    HAL_Delay(10);
}


/**
  * @brief  设置Ads1299模式（1-Impedance、2-Normal、3-Short noise、4-Test wave）
	* @param  None
  * @retval None
  */
void ADS_ModeSelect(uint8_t mode)
{
    switch(mode) {
        case Impedance:
        {
            ADS_WREG(LOFF,0x02);
            HAL_Delay(10);

            ADS_WREG(BIAS_SENSP,0xFF);
            HAL_Delay(10);
            ADS_WREG(BIAS_SENSN,0xFF);
            HAL_Delay(10);
            ADS_WREG(LOFF_SENSP,0xFF);
            HAL_Delay(10);
            ADS_WREG(LOFF_SENSN,0x00);
            HAL_Delay(10);
            ADS_WREG(LOFF_FLIP,0x00);
            HAL_Delay(10);
            ADS_WREG(MISC1,0x20);//设置SRB1
            HAL_Delay(10);

            ADS_WREG(CH1SET,0x60);
            HAL_Delay(10);
            ADS_WREG(CH2SET,0x60);
            HAL_Delay(10);
            ADS_WREG(CH3SET,0x60);
            HAL_Delay(10);
            ADS_WREG(CH4SET,0x60);
            HAL_Delay(10);
            ADS_WREG(CH5SET,0x60);
            HAL_Delay(10);
            ADS_WREG(CH6SET,0x60);
            HAL_Delay(10);
            ADS_WREG(CH7SET,0x60);
            HAL_Delay(10);
            ADS_WREG(CH8SET,0x60);
            HAL_Delay(10);

            ADS_START();
//            HAL_Delay(40);
            ADS_RDATAC();
            csLow();//DMA开启前拉低片选
        }
            break;
        case Normal:
        {
            ADS_WREG(BIAS_SENSP,0xFF);
            HAL_Delay(10);
            ADS_WREG(BIAS_SENSN,0xFF);
            HAL_Delay(10);
            ADS_WREG(MISC1,0x20);//设置SRB1
            HAL_Delay(10);

            ADS_WREG(CH1SET,0x60);
            HAL_Delay(10);
            ADS_WREG(CH2SET,0x60);
            HAL_Delay(10);
            ADS_WREG(CH3SET,0x60);
            HAL_Delay(10);
            ADS_WREG(CH4SET,0x60);
            HAL_Delay(10);
            ADS_WREG(CH5SET,0x60);
            HAL_Delay(10);
            ADS_WREG(CH6SET,0x60);
            HAL_Delay(10);
            ADS_WREG(CH7SET,0x60);
            HAL_Delay(10);
            ADS_WREG(CH8SET,0x60);
            HAL_Delay(10);

            ADS_START();
//            HAL_Delay(40);
            ADS_RDATAC();
            csLow();//DMA开启前拉低片选
        }
            break;
        case InternalShort:
        {
            ADS_WREG(CONFIG2,0xC0);
            HAL_Delay(10);
            ADS_WREG(CH1SET,0x01);
            HAL_Delay(10);
            ADS_WREG(CH2SET,0x01);
            HAL_Delay(10);
            ADS_WREG(CH3SET,0x01);
            HAL_Delay(10);
            ADS_WREG(CH4SET,0x01);
            HAL_Delay(10);
            ADS_WREG(CH5SET,0x01);
            HAL_Delay(10);
            ADS_WREG(CH6SET,0x01);
            HAL_Delay(10);
            ADS_WREG(CH7SET,0x01);
            HAL_Delay(10);
            ADS_WREG(CH8SET,0x01);
            HAL_Delay(10);

            ADS_START();
//            HAL_Delay(40);
            ADS_RDATAC();
            csLow();//DMA开启前拉低片选
        }
            break;
        case TestSignal:
        {
            ADS_WREG(CONFIG2,0xD0);
            HAL_Delay(10);
            ADS_WREG(CH1SET,0x05);
            HAL_Delay(10);
            ADS_WREG(CH2SET,0x05);
            HAL_Delay(10);
            ADS_WREG(CH3SET,0x05);
            HAL_Delay(10);
            ADS_WREG(CH4SET,0x05);
            HAL_Delay(10);
            ADS_WREG(CH5SET,0x05);
            HAL_Delay(10);
            ADS_WREG(CH6SET,0x05);
            HAL_Delay(10);
            ADS_WREG(CH7SET,0x05);
            HAL_Delay(10);
            ADS_WREG(CH8SET,0x05);
            HAL_Delay(10);

            ADS_START();
//            HAL_Delay(40);
            ADS_RDATAC();						
						csLow();//DMA开启前拉低片选
        }
            break;
        default:;
    }
}



extern lwrb_t lwrb_buff;
/**
 * ADS运行模式选择
 * */
uint8_t Is_standby = 1;

void ADS_state_choose(uint8_t EEG_State,SAMPLE_RATE sr)
{
    if(Is_standby){
        Is_standby = 0;
        ADS_WAKEUP();
    }

    switch(EEG_State) {
        case IMPEDANCING:
            initialize_ads(sr);
            ADS_ModeSelect(Impedance);
						HAL_NVIC_EnableIRQ(EXTI4_IRQn);
            break;

        case WAVE:
            initialize_ads(sr);
            ADS_ModeSelect(Normal);
						HAL_NVIC_EnableIRQ(EXTI4_IRQn);
            break;

        case INTERNALSHORT:
            initialize_ads(sr);
            ADS_ModeSelect(InternalShort);
						HAL_NVIC_EnableIRQ(EXTI4_IRQn);
            break;

        case TESTSIGAL:
            initialize_ads(sr);
            ADS_ModeSelect(TestSignal);
						HAL_NVIC_EnableIRQ(EXTI4_IRQn);
            break;

        case STOP:
						HAL_NVIC_DisableIRQ(EXTI4_IRQn);
						ADS_STOP();
				ADS_STOP();
						ADS_SDATAC();
            ADS_STANDBY();//进入低功耗模式            
            lwrb_reset(&lwrb_buff);//清空缓存数据
            Is_standby = 1;
            break;

        default:;
    }
}















