# ADS1220
This is a C library for using ADS1220 \
Please read the .h file documentation.
## How to use
You should initialize handler structure first. (`ADS1220_Handler_t`) \
Then you should call initialize function. (`ADS1220_Init`) \
Then you can use other functions. \
**NOTE:** Information about handler structure, init function and others are in .h file. 

## Example
<details>
<summary>
STM32 HAL
</summary>

```c
#include "ADS1220.h"
void CS_UP(void)
{
    // GPIO_CS_GROUP and GPIO_CS_PIN depend on your schematic.
    HAL_GPIO_WritePin(GPIO_CS_GROUP, GPIO_CS_PIN, GPIO_PIN_SET);
}
void CS_DOWN(void)
{
    // GPIO_CS_GROUP and GPIO_CS_PIN depend on your schematic.
    HAL_GPIO_WritePin(GPIO_CS_GROUP, GPIO_CS_PIN, GPIO_PIN_RESET);
}
void TRANSMIT(uint8_t data)
{
    // SPI_DRIVER depends on your configuration.
    HAL_SPI_Transmit (SPI_DRIVER, &data, sizeof(uint8_t), HAL_MAX_DELAY);
}
uint8_t RECEIVE(void)
{
    uint8_t dataR = 0;
    // SPI_DRIVER depends on your configuration.
    HAL_SPI_Receive (SPI_DRIVER, &dataR, sizeof(uint8_t), HAL_MAX_DELAY);
    return dataR;
}
void DELAY(uint32_t us)
{
    // DELAY_US depends on your code.
    DELAY_US(us);
}
ADS1220_Handler_t Handler = {0};

void main (void)
{
    // Initialize all STM32 peripherals including SPI needed for ADS1220 with desire configuration before call ADS1220 init function.

    Handler.ADC_CS_HIGH = CS_UP;
    Handler.ADC_CS_LOW = CS_DOWN;
    Handler.ADC_Transmit = TRANSMIT;
    Handler.ADC_Receive = RECEIVE;
    Handler.ADC_Delay_US = DELAY;
    
    // Passing Parameters as NULL to use default configurations.
    ADS1220_Init(&Handler, NULL);

    // Default conversion mode is Single-shot
    ADS1220_StartSync(&Handler);

    // GPIO_DRDY_GROUP and GPIO_DRDY_PIN depend on your schematic.
    while(HAL_GPIO_ReadPin(GPIO_DRDY_GROUP, GPIO_DRDY_PIN) == GPIO_PIN_SET){}

    int32_t ADC_Data;
    ADS1220_ReadData(&Handler, &ADC_Data);

    // 2.048 is internal voltage reference and is used as default config.
    // 1 is default adc gain value and it must be equivalent to the gain config in ADS1220_Parameters_t.
    printf("ADC Raw: 0x%X | ADC Voltage : %f\r\n", ADC_Data, ADCValueToVoltage(ADC_Data, 2.048, 1));

    while(1)
    {
    }
}
```
</details>
