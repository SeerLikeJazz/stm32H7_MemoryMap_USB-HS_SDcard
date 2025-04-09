# stm32H7_MemoryMap_USB-HS_SDcard
For peripheral DMA RAM etc. practicing

- Scatter file: Load multiple pieces of memory separately
- Attention:  
    * DTCM and ITCM **don't support** DMA1,DMA2 and BDMA, only support MDMA  

    * AXI SRAM, SRAM1, SRAM2, SRAM3 **don't support** BDMA, support MDMA, DMA1 and DMA2 

    * SRAM4 support all DMA ( MDMA, DMA1, DMA2 and BDMA )  

### 09.04.2025
- (Demo) USB-HS /DMA
- fix:Problem with STM32H747 HS USB DMA.  
  In fact, you should not allocate more than (4096 – 72 (for DMA descs)) bytes in total -> 0x3EE words  
/* USER CODE BEGIN TxRx_Configuration */  
HAL_PCDEx_SetRxFiFo(&hpcd_USB_OTG_HS, 0x200);  
HAL_PCDEx_SetTxFiFo(&hpcd_USB_OTG_HS, 0, 0x80);  
HAL_PCDEx_SetTxFiFo(&hpcd_USB_OTG_HS, 1, 0x100);  
/* USER CODE END TxRx_Configuration */  
- **DCache** Enable, would crash ！！！


### 06.04.2025
- (Demo) ADS1299 /SPI1 /DMA1 /AXI_RAM
- Sct file archived
- Base on hardware Dev1.3