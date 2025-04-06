# stm32H7_memorymap
For peripheral DMA RAM etc. practicing

- Scatter file: Load multiple pieces of memory separately
- Attention:  
    * DTCM and ITCM **don't support** DMA1,DMA2 and BDMA, only support MDMA  

    * AXI SRAM, SRAM1, SRAM2, SRAM3 **don't support** BDMA, support MDMA, DMA1 and DMA2 

    * SRAM4 support all DMA ( MDMA, DMA1, DMA2 and BDMA )  


### 06.04.2025
- (Demo) ADS1299 /SPI1 /DMA1 /AXI_RAM
- Sct file archived
- Base on hardware Dev1.3