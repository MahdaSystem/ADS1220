/**
 **********************************************************************************
 * @file   ADS1220.c
 * @author Ali Moallem (https://github.com/AliMoal)
 * @brief  For working with ADS1220 IC
 **********************************************************************************
 *
 *! Copyright (c) 2022 Mahda Embedded System (MIT License)
 *!
 *! Permission is hereby granted, free of charge, to any person obtaining a copy
 *! of this software and associated documentation files (the "Software"), to deal
 *! in the Software without restriction, including without limitation the rights
 *! to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 *! copies of the Software, and to permit persons to whom the Software is
 *! furnished to do so, subject to the following conditions:
 *!
 *! The above copyright notice and this permission notice shall be included in all
 *! copies or substantial portions of the Software.
 *!
 *! THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 *! IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 *! FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 *! AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 *! LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 *! OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 *! SOFTWARE.
 *!
 **********************************************************************************
 **/
//* Private Includes -------------------------------------------------------------- //
#include "ADS1220.h"

//* Private Defines and Macros ---------------------------------------------------- //
#define ADS1220_RawToAdcValue(oneData) ((int32_t)(((oneData[0] << 16) | (oneData[1] << 8) | (oneData[2])) << 8) / 256)

//* Others ------------------------------------------------------------------------ //
#ifdef ADS1220_Debug_Enable
#include <stdio.h> // for debug
#define PROGRAMLOG(arg...) printf(arg)
#else
#define PROGRAMLOG(arg...)
#endif

/**
 ** ==================================================================================
 **                            ##### Private Enums #####                               
 ** ==================================================================================
 **/

typedef enum
ADS1220Commands_s {
	RESET_ADC			= 0x06,
	START_SYNC		= 0x08,
	POWERDOWN			= 0x02,
	RDATA					=	0x10,
	RREG					= 0x20,
	WREG					= 0x40
} ADS1220Commands_t;

typedef enum
ADS1220Register_s {
	REGISTER00h						=	0x00,
  REGISTER01h						=	0x01,
  REGISTER02h						=	0x02,
  REGISTER03h						=	0x03,
} ADS1220Register_t;

/**
 *! ==================================================================================
 *!                          ##### Private Functions #####                               
 *! ==================================================================================
 **/
static uint8_t
ADS1220_ReadReg (ADS1220_Handler_t *ADC_Handler, ADS1220Register_t ADS1220REG)
{
	uint8_t RecByte = 0;
  ADS1220_Delay_US(10);
	ADC_Handler->ADC_CS_LOW();
	ADS1220_Delay_US(10);
	ADC_Handler->ADC_Transmit(RREG | (ADS1220REG << 2));
	ADS1220_Delay_US(10);
	RecByte = ADC_Handler->ADC_Receive();
	ADS1220_Delay_US(10);
	ADC_Handler->ADC_CS_HIGH();
  ADS1220_Delay_US(10);
	return RecByte;
};

// This function Works but we do not need it
//static void ADS1220_ReadAllRegs (ADS1220_Handler *ADC_Handler, uint8_t *ReadValues /*It is = [0]: REG00h | Number of Elements: 4*/)
//{
//  ADS1220_Delay_US(10);
//	ADC_Handler->ADC_CS_LOW();
//	ADS1220_Delay_US(10);
//	ADC_Handler->ADC_Transmit(RREG | 3 /*(num reg to read: 4) - 1*/ /*starts from reg00h*/);
//	ADS1220_Delay_US(10);
//	ReadValues[0] = ADC_Handler->ADC_Receive();
//	ADS1220_Delay_US(10);
//  ReadValues[1] = ADC_Handler->ADC_Receive();
//	ADS1220_Delay_US(10);
//  ReadValues[2] = ADC_Handler->ADC_Receive();
//	ADS1220_Delay_US(10);
//  ReadValues[3] = ADC_Handler->ADC_Receive();
//	ADS1220_Delay_US(10);
//	ADC_Handler->ADC_CS_HIGH();
//  ADS1220_Delay_US(10);
//};

static void
ADS1220_WriteReg (ADS1220_Handler_t *ADC_Handler, ADS1220Register_t ADS1220REG, uint8_t RegisterValue)
{
  ADS1220_Delay_US(10);
	ADC_Handler->ADC_CS_LOW();
	ADS1220_Delay_US(10);
	ADC_Handler->ADC_Transmit(WREG | (ADS1220REG << 2));
	ADS1220_Delay_US(10);
	ADC_Handler->ADC_Transmit(RegisterValue);
	ADS1220_Delay_US(10);
	ADC_Handler->ADC_CS_HIGH();
  ADS1220_Delay_US(10);
};

static void
ADS1220_WriteAllRegs (ADS1220_Handler_t *ADC_Handler, uint8_t *RegisterValue /*Must be = [0]: REG00h | Number of Elements: 4*/)
{
  ADS1220_Delay_US(10);
	ADC_Handler->ADC_CS_LOW();
	ADS1220_Delay_US(10);
	ADC_Handler->ADC_Transmit(WREG | 3 /*(num reg to write: 4) - 1*/ /*starts from reg00h*/);
	ADS1220_Delay_US(10);
	ADC_Handler->ADC_Transmit(RegisterValue[0]);
	ADS1220_Delay_US(10);
  ADC_Handler->ADC_Transmit(RegisterValue[1]);
	ADS1220_Delay_US(10);
  ADC_Handler->ADC_Transmit(RegisterValue[2]);
	ADS1220_Delay_US(10);
  ADC_Handler->ADC_Transmit(RegisterValue[3]);
	ADS1220_Delay_US(10);
	ADC_Handler->ADC_CS_HIGH();
  ADS1220_Delay_US(10);
};

static void
ADS1220_ReadDataWriteReg (ADS1220_Handler_t *ADC_Handler, ADS1220Register_t ADS1220REG, uint8_t RegisterValue, int32_t *ADCSample)
{
  ADS1220_Delay_US(10);
	ADC_Handler->ADC_CS_LOW();
	ADS1220_Delay_US(10);
  ADC_Handler->ADCDataValues.Part3 = ADC_Handler->ADC_TransmitReceive(0);
  ADS1220_Delay_US(10);
  ADC_Handler->ADCDataValues.Part2 = ADC_Handler->ADC_TransmitReceive(WREG | (ADS1220REG << 2));
  ADS1220_Delay_US(10);
  ADC_Handler->ADCDataValues.Part1 = ADC_Handler->ADC_TransmitReceive(RegisterValue);
	ADS1220_Delay_US(10);
	ADC_Handler->ADC_CS_HIGH();
  ADS1220_Delay_US(10);
  *ADCSample = ADC_Handler->ADCDataValues.INT32 / 256;
//  PROGRAMLOG("%x\r\n",(ADCDataValues.Part3<<16) | (ADCDataValues.Part2<<8) | (ADCDataValues.Part1));
};

/**
 ** ==================================================================================
 **                           ##### Public Functions #####                               
 ** ==================================================================================
 **/

/**
 * @brief  Initializes The ADC and Library
 * @note   If You pass Parameters as NULL, All Settings will set default. See ADS1220_Parameters struct to know what are default values
 * @param  ADC_Handler: Pointer Of Library Handler
 * @param  Parameters:  Pointer Of ADC Parameters
 * @retval None
 */
void
ADS1220_Init(ADS1220_Handler_t *ADC_Handler, ADS1220_Parameters_t * Parameters)
{
  PROGRAMLOG("------------------------------\r\nADS1220_Init...\r\n");
  if (!ADC_Handler) { PROGRAMLOG("ERROR Please Initialize ADC_Handler\r\n"); return; }
  PROGRAMLOG("%s",((ADC_Handler->ADC_TransmitReceive != NULL) & (ADC_Handler->ADC_DRDY_Read != NULL)) ? ("") : ("*** Warning! You Can NOT Use ReadAllContinuous Functions. ***\r\n*** Initialize both ADC_TransmitReceive and ADC_DRDY_Read in ADC_Handler struct ***\r\n\r\n"));
  
	ADS1220_Delay_US(100); // Wait after power up at least 50us + tclk
  ADC_Handler->ADC_CS_LOW();
	ADS1220_Delay_US(5);
	ADC_Handler->ADC_Transmit(RESET_ADC);
	ADS1220_Delay_US(5);
	ADC_Handler->ADC_CS_HIGH();
	ADS1220_Delay_US(100); // Wait after reset at least 50us + tclk
  
  PROGRAMLOG("Previous Regs Values: 0x%02X | 0x%02X | 0x%02X | 0x%02X\r\n",
    ADS1220_ReadReg(ADC_Handler,REGISTER00h),
    ADS1220_ReadReg(ADC_Handler,REGISTER01h),
    ADS1220_ReadReg(ADC_Handler,REGISTER02h),
    ADS1220_ReadReg(ADC_Handler,REGISTER03h)
  );
  
  uint8_t RegsValue[4] = {0};
  
  if (Parameters)
  {
    RegsValue[0] = (Parameters->InputMuxConfig << 4) | (Parameters->GainConfig << 1) | Parameters->PGAdisable;
    RegsValue[1] = (Parameters->DataRate << 5) | (Parameters->OperatingMode << 3) | (Parameters->ConversionMode << 2) | (Parameters->TempeSensorMode << 1) | Parameters->BurnOutCurrentSrc;
    RegsValue[2] = (Parameters->VoltageRef << 6) | (Parameters->FIRFilter << 4) | (Parameters->LowSidePwr << 3) | Parameters->IDACcurrent;
    RegsValue[3] = (Parameters->IDAC1routing << 5) | (Parameters->IDAC2routing << 2) | (Parameters->DRDYMode << 1);
    ADS1220_WriteAllRegs(ADC_Handler,RegsValue);
    
    PROGRAMLOG("New      Regs Values: 0x%02X | 0x%02X | 0x%02X | 0x%02X\r\n"
               "Correct  Regs Values: 0x%02X | 0x%02X | 0x%02X | 0x%02X\r\n",
    ADS1220_ReadReg(ADC_Handler,REGISTER00h),
    ADS1220_ReadReg(ADC_Handler,REGISTER01h),
    ADS1220_ReadReg(ADC_Handler,REGISTER02h),
    ADS1220_ReadReg(ADC_Handler,REGISTER03h),
    RegsValue[0],RegsValue[1],RegsValue[2],RegsValue[3]
    );
  }
  else
  {
    ADS1220_WriteAllRegs(ADC_Handler,RegsValue);
    
    PROGRAMLOG("Defualt  Regs Mode:   0x%02X | 0x%02X | 0x%02X | 0x%02X\r\n"
               "Must     Regs be:     0x00 | 0x00 | 0x00 | 0x00\r\n",
    ADS1220_ReadReg(ADC_Handler,REGISTER00h),
    ADS1220_ReadReg(ADC_Handler,REGISTER01h),
    ADS1220_ReadReg(ADC_Handler,REGISTER02h),
    ADS1220_ReadReg(ADC_Handler,REGISTER03h)
    );
  }
};

/**
 * @brief  Starts (in Single-shot mode) or Syncs (in Continuous conversion mode) Conversion
 * @param  ADC_Handler: Pointer Of Library Handler
 * @retval None
 */
void
ADS1220_StartSync(ADS1220_Handler_t *ADC_Handler)
{
  ADS1220_Delay_US(10);
  ADC_Handler->ADC_CS_LOW();
  ADS1220_Delay_US(10);
  ADC_Handler->ADC_Transmit(START_SYNC);
  ADS1220_Delay_US(10);
	ADC_Handler->ADC_CS_HIGH();
  ADS1220_Delay_US(10);
}

/**
 * @brief  Resets ADS1220
 * @param  ADC_Handler: Pointer Of Library Handler
 * @retval None
 */
void
ADS1220_Reset(ADS1220_Handler_t *ADC_Handler)
{
  ADS1220_Delay_US(10);
  ADC_Handler->ADC_CS_LOW();
  ADS1220_Delay_US(10);
  ADC_Handler->ADC_Transmit(RESET_ADC);
  ADS1220_Delay_US(10);
	ADC_Handler->ADC_CS_HIGH();
  ADS1220_Delay_US(100);
}

/**
 * @brief  Enables Power down
 * @param  ADC_Handler: Pointer Of Library Handler
 * @retval None
 */
void
ADS1220_PowerDown(ADS1220_Handler_t *ADC_Handler)
{
  ADS1220_Delay_US(10);
  ADC_Handler->ADC_CS_LOW();
  ADS1220_Delay_US(10);
  ADC_Handler->ADC_Transmit(POWERDOWN);
  ADS1220_Delay_US(10);
	ADC_Handler->ADC_CS_HIGH();
  ADS1220_Delay_US(10);
}

/**
 * @brief  Reads Data for both Single-shot and Continuous conversion Modes
 * @note   Call This function when DRDY pin got LOW
 * @param  ADC_Handler: Pointer Of Library Handler
 * @param  ADCSample:   Pointer Of Samples Array | Number of Elements: 1 (To read other channels user should config InputMuxConfig in ADS1220_Parameters or USE ReadAll functions)
 * @retval None
 */
void
ADS1220_ReadData(ADS1220_Handler_t *ADC_Handler, int32_t *ADCSample)
{
  ADS1220_Delay_US(10);
  ADC_Handler->ADC_CS_LOW();
  ADS1220_Delay_US(10);
  ADC_Handler->ADCDataValues.Part3 = ADC_Handler->ADC_Receive();
  ADS1220_Delay_US(10);
  ADC_Handler->ADCDataValues.Part2 = ADC_Handler->ADC_Receive();
  ADS1220_Delay_US(10);
  ADC_Handler->ADCDataValues.Part1 = ADC_Handler->ADC_Receive();
  ADS1220_Delay_US(10);
	ADC_Handler->ADC_CS_HIGH();
  ADS1220_Delay_US(10);
  *ADCSample = ADC_Handler->ADCDataValues.INT32 / 256;
//  PROGRAMLOG("Data read: 0x%02X\r\n",*ADCSample);
}

/**
 * @brief  Changes the Configurations
 * @note   Pass Parameters as NULL to change settings to default Values. See ADS1220_Parameters struct to know what are default values
 * @param  ADC_Handler: Pointer Of Library Handler
 * @param  Parameters:  Pointer Of ADC Parameters
 * @retval None
 */
void
ADS1220_ChangeConfig(ADS1220_Handler_t *ADC_Handler, ADS1220_Parameters_t * Parameters)
{
  PROGRAMLOG("Previous Regs Values: 0x%02X | 0x%02X | 0x%02X | 0x%02X\r\n",
    ADS1220_ReadReg(ADC_Handler,REGISTER00h),
    ADS1220_ReadReg(ADC_Handler,REGISTER01h),
    ADS1220_ReadReg(ADC_Handler,REGISTER02h),
    ADS1220_ReadReg(ADC_Handler,REGISTER03h)
  );
  
  uint8_t RegsValue[4] = {0};
  if (Parameters)
  {
    RegsValue[0] = (Parameters->InputMuxConfig << 4) | (Parameters->GainConfig << 1) | Parameters->PGAdisable;
    RegsValue[1] = (Parameters->DataRate << 5) | (Parameters->OperatingMode << 3) | (Parameters->ConversionMode << 2) | (Parameters->TempeSensorMode << 1) | Parameters->BurnOutCurrentSrc;
    RegsValue[2] = (Parameters->VoltageRef << 6) | (Parameters->FIRFilter << 4) | (Parameters->LowSidePwr << 3) | Parameters->IDACcurrent;
    RegsValue[3] = (Parameters->IDAC1routing << 5) | (Parameters->IDAC2routing << 2) | (Parameters->DRDYMode << 1);
    ADS1220_WriteAllRegs(ADC_Handler,RegsValue);
    
    PROGRAMLOG("New      Regs Values: 0x%02X | 0x%02X | 0x%02X | 0x%02X\r\n"
               "Correct  Regs Values: 0x%02X | 0x%02X | 0x%02X | 0x%02X\r\n",
    ADS1220_ReadReg(ADC_Handler,REGISTER00h),
    ADS1220_ReadReg(ADC_Handler,REGISTER01h),
    ADS1220_ReadReg(ADC_Handler,REGISTER02h),
    ADS1220_ReadReg(ADC_Handler,REGISTER03h),
    RegsValue[0],RegsValue[1],RegsValue[2],RegsValue[3]
    );
  }
  else 
  {
    ADS1220_WriteAllRegs(ADC_Handler,RegsValue);
    
    PROGRAMLOG("Defualt  Regs Mode:   0x%02X | 0x%02X | 0x%02X | 0x%02X\r\n"
               "Must     Regs be:     0x00 | 0x00 | 0x00 | 0x00\r\n",
    ADS1220_ReadReg(ADC_Handler,REGISTER00h),
    ADS1220_ReadReg(ADC_Handler,REGISTER01h),
    ADS1220_ReadReg(ADC_Handler,REGISTER02h),
    ADS1220_ReadReg(ADC_Handler,REGISTER03h)
    );
  }
}

/**
 * @brief  Changes Gains
 * @param  ADC_Handler: Pointer Of Library Handler
 * @param  GainConfig:  Gain Configuration
 *                      - See ADS1220_GainConfig enum
 * @retval None
 */
void
ADS1220_ChangeGain(ADS1220_Handler_t *ADC_Handler, ADS1220_GainConfig_t GainConfig)
{
  PROGRAMLOG("Prevoius Gain: 2^%d\r\n",(ADS1220_ReadReg(ADC_Handler,REGISTER00h) >> 1) & 7);
  ADS1220_WriteReg(ADC_Handler,REGISTER00h,ADS1220_ReadReg(ADC_Handler,REGISTER00h) & (0xF1 | (GainConfig << 1)));
  PROGRAMLOG("New      Gain: 2^%d\r\n",(ADS1220_ReadReg(ADC_Handler,REGISTER00h) >> 1) & 7);
}

/**
 * @brief  Activates Single-Shot Mode (Deactivates Continuous Mode)
 * @param  ADC_Handler: Pointer Of Library Handler
 * @retval None
 */
void
ADS1220_ActivateSingleShotMode(ADS1220_Handler_t *ADC_Handler)
{
  ADS1220_WriteReg(ADC_Handler,REGISTER01h,ADS1220_ReadReg(ADC_Handler,REGISTER01h) & 0xFB);
  PROGRAMLOG("Single-Shot Mode is %s\r\n",((ADS1220_ReadReg(ADC_Handler,REGISTER01h) >> 2) & 1) ? ("Deactive") : ("Active"));
}

/**
 * @brief  Activates Continuous Mode (Deactivates Single-Shot Mode)
 * @param  ADC_Handler: Pointer Of Library Handler
 * @retval None
 */
void
ADS1220_ActivateContinuousMode(ADS1220_Handler_t *ADC_Handler)
{
  ADS1220_WriteReg(ADC_Handler,REGISTER01h,ADS1220_ReadReg(ADC_Handler,REGISTER01h) | 0x04);
  PROGRAMLOG("Continuous Mode is %s\r\n",((ADS1220_ReadReg(ADC_Handler,REGISTER01h) >> 2) & 1) ? ("Active") : ("Deactive"));
}

/**
 * @brief  Reads All channels data for Single-shot Mode.
 *         Channel1: AINP = AIN0, AINN = AIN1
 *         Channel2: AINP = AIN2, AINN = AIN3
 * @note   ADS1220 Must be in Single-shot Mode.
 *         Pass GainConfig as NULL to use current values for gain configurations.
 *         At the end, configurations will be changed to previous values.
 * @param  ADC_Handler: Pointer Of Library Handler
 * @param  ADCSample:   Pointer Of Samples Array | Number of Element: 2 | [0]: Channel1
 * @param  GainConfig:  Gain Configuration | Number of Element: 2 | [0]: Channel1
 *                      - See ADS1220_GainConfig enum
 * @retval None
 */
void
ADS1220_ReadAllSingleShotDiff(ADS1220_Handler_t *ADC_Handler, int32_t *ADCSample, ADS1220_GainConfig_t *GainConfig)
{
  uint8_t Reg00hValue = ADS1220_ReadReg(ADC_Handler,REGISTER00h);
  
  ADS1220_StartSync(ADC_Handler);          // FOR WAKING UP. if you are using this function consecutively and quickly (less than ~54511.71875 us), you can comment this line
  while(ADC_Handler->ADC_DRDY_Read());     // FOR WAKING UP. if you are using this function consecutively and quickly (less than ~54511.71875 us), you can comment this line

  if (GainConfig) {  
    if (((Reg00hValue & 1) | (GainConfig[0] << 1)) != Reg00hValue)  
      ADS1220_WriteReg(ADC_Handler,REGISTER00h,(Reg00hValue & 1) | (GainConfig[0] << 1));
    else ADS1220_StartSync(ADC_Handler);
    while(ADC_Handler->ADC_DRDY_Read());
    ADS1220_ReadData(ADC_Handler,&ADCSample[0]);
    
    ADS1220_WriteReg(ADC_Handler,REGISTER00h, 0x50 | (Reg00hValue & 1) | (GainConfig[1] << 1));
    while(ADC_Handler->ADC_DRDY_Read());
    ADS1220_ReadData(ADC_Handler,&ADCSample[1]);
  }
  else
  { 
    if ((Reg00hValue & 0x0F) != Reg00hValue)  
      ADS1220_WriteReg(ADC_Handler,REGISTER00h, (Reg00hValue & 0x0F));
    else ADS1220_StartSync(ADC_Handler);
    while(ADC_Handler->ADC_DRDY_Read());
    ADS1220_ReadData(ADC_Handler,&ADCSample[0]);
    
    ADS1220_WriteReg(ADC_Handler,REGISTER00h, 0x50 | (Reg00hValue & 0x0F));
    while(ADC_Handler->ADC_DRDY_Read());
    ADS1220_ReadData(ADC_Handler,&ADCSample[1]);
  }
  ADS1220_WriteReg(ADC_Handler,REGISTER00h,Reg00hValue);
}

/**
 * @brief  Reads All channels data for Continuous conversion Mode.
 *         Channel1: AINP = AIN0, AINN = AIN1
 *         Channel2: AINP = AIN2, AINN = AIN3
 * @note   ADS1220 Must be in Continuous conversion Mode.
 *         Pass GainConfig as NULL to use current values for gain configurations.
 *         At the end, configurations will be changed to previous values.
 * @param  ADC_Handler: Pointer Of Library Handler
 * @param  ADCSample:   Pointer Of Samples Array | Number of Element: 2 | [0]: Channel1
 * @param  GainConfig:  Gain Configuration | Number of Element: 2 | [0]: Channel1
 *                      - See ADS1220_GainConfig enum
 * @retval None
 */
void
ADS1220_ReadAllContinuousDiff(ADS1220_Handler_t *ADC_Handler, int32_t *ADCSample, ADS1220_GainConfig_t *GainConfig)
{
  ADS1220_StartSync(ADC_Handler);          // FOR WAKING UP. if you are using this function consecutively and quickly (less than ~54511.71875 us), you can comment this line
  while(ADC_Handler->ADC_DRDY_Read());     // FOR WAKING UP. if you are using this function consecutively and quickly (less than ~54511.71875 us), you can comment this line
  
  if (GainConfig) {
    uint8_t Reg00hValue = ADS1220_ReadReg(ADC_Handler,REGISTER00h);
    ADS1220_WriteReg(ADC_Handler,REGISTER00h,(Reg00hValue & 1) | (GainConfig[0] << 1));
    while(ADC_Handler->ADC_DRDY_Read());
    ADS1220_ReadDataWriteReg(ADC_Handler, REGISTER00h, 0x50 | (Reg00hValue & 1) | (GainConfig[1] << 1), &ADCSample[0]);
    while(ADC_Handler->ADC_DRDY_Read());
    ADS1220_ReadDataWriteReg(ADC_Handler, REGISTER00h, Reg00hValue, &ADCSample[1]);
  }
  else
  {
    uint8_t Reg00hValue = ADS1220_ReadReg(ADC_Handler,REGISTER00h);
    ADS1220_WriteReg(ADC_Handler,REGISTER00h,Reg00hValue & 0x0F);
    while(ADC_Handler->ADC_DRDY_Read());
    ADS1220_ReadDataWriteReg(ADC_Handler, REGISTER00h, 0x50 | (Reg00hValue & 0x0F), &ADCSample[0]);
    while(ADC_Handler->ADC_DRDY_Read());
    ADS1220_ReadDataWriteReg(ADC_Handler, REGISTER00h, Reg00hValue, &ADCSample[1]);
  }
}

/**
 * @brief  Reads All channels data for Single-shot Mode.
 *         Channel1: AINP = AIN0, AINN = AVSS
 *         Channel2: AINP = AIN1, AINN = AVSS
 *         Channel3: AINP = AIN2, AINN = AVSS
 *         Channel4: AINP = AIN3, AINN = AVSS
 * @note   ADS1220 Must be in Single-shot Mode.
 *         Pass GainConfig as NULL to use current values for gain configurations.
 *         The PGA will be disabled (PGA_BYPASS = 1)
 *         At the end, configurations will be changed to previous values.
 * @param  ADC_Handler: Pointer Of Library Handler
 * @param  ADCSample:   Pointer Of Samples Array | Number of Element: 4 | [0]: Channel1
 * @param  GainConfig:  Gain Configuration | Number of Element: 4 | [0]: Channel1
 *         @note        Gains must be only 1, 2, and 4. See ADS1220_InputMuxConfig struct for more details.
 *                      - See ADS1220_GainConfig enum
 * @retval None
 */
void
ADS1220_ReadAllSingleShotAVSS(ADS1220_Handler_t *ADC_Handler, int32_t *ADCSample, ADS1220_GainConfig_t *GainConfig)
{
  uint8_t Reg00hValue = ADS1220_ReadReg(ADC_Handler,REGISTER00h);
  
  ADS1220_StartSync(ADC_Handler);          // FOR WAKING UP. if you are using this function consecutively and quickly (less than ~54511.71875 us), you can comment this line
  while(ADC_Handler->ADC_DRDY_Read());     // FOR WAKING UP. if you are using this function consecutively and quickly (less than ~54511.71875 us), you can comment this line
  
  if (GainConfig)
  { 
    if ((0x81 | (GainConfig[0] << 1)) != Reg00hValue)
      ADS1220_WriteReg(ADC_Handler,REGISTER00h, 0x81 | (GainConfig[0] << 1));
    else ADS1220_StartSync(ADC_Handler);
    while(ADC_Handler->ADC_DRDY_Read());
    ADS1220_ReadData(ADC_Handler,&ADCSample[0]);
    
    ADS1220_WriteReg(ADC_Handler,REGISTER00h, 0x91 | (GainConfig[1] << 1));
    while(ADC_Handler->ADC_DRDY_Read());
    ADS1220_ReadData(ADC_Handler,&ADCSample[1]);
    
    ADS1220_WriteReg(ADC_Handler,REGISTER00h, 0xA1 | (GainConfig[2] << 1));
    while(ADC_Handler->ADC_DRDY_Read());
    ADS1220_ReadData(ADC_Handler,&ADCSample[2]);
    
    ADS1220_WriteReg(ADC_Handler,REGISTER00h, 0xB1 | (GainConfig[3] << 1));
    while(ADC_Handler->ADC_DRDY_Read());
    ADS1220_ReadData(ADC_Handler,&ADCSample[3]);
  }
  else
  {    
    if ((0x81 | (Reg00hValue & 0x06)) != Reg00hValue)
      ADS1220_WriteReg(ADC_Handler,REGISTER00h, 0x81 | (Reg00hValue & 0x06));
    else ADS1220_StartSync(ADC_Handler);
    while(ADC_Handler->ADC_DRDY_Read());
    ADS1220_ReadData(ADC_Handler,&ADCSample[0]);
    
    ADS1220_WriteReg(ADC_Handler,REGISTER00h, 0x91 | (Reg00hValue & 0x06));
    while(ADC_Handler->ADC_DRDY_Read());
    ADS1220_ReadData(ADC_Handler,&ADCSample[1]);
    
    ADS1220_WriteReg(ADC_Handler,REGISTER00h, 0xA1 | (Reg00hValue & 0x06));
    while(ADC_Handler->ADC_DRDY_Read());
    ADS1220_ReadData(ADC_Handler,&ADCSample[2]);
    
    ADS1220_WriteReg(ADC_Handler,REGISTER00h, 0xB1 | (Reg00hValue & 0x06));
    while(ADC_Handler->ADC_DRDY_Read());
    ADS1220_ReadData(ADC_Handler,&ADCSample[3]);
  }
  ADS1220_WriteReg(ADC_Handler,REGISTER00h,Reg00hValue);
}

/**
 * @brief  Reads All channels data for Continuous conversion Mode.
 *         Channel1: AINP = AIN0, AINN = AVSS
 *         Channel2: AINP = AIN1, AINN = AVSS
 *         Channel3: AINP = AIN2, AINN = AVSS
 *         Channel4: AINP = AIN3, AINN = AVSS
 * @note   ADS1220 Must be in Continuous conversion Mode.
 *         Pass GainConfig as NULL to use current values for gain configurations.
 *         The PGA will be disabled (PGA_BYPASS = 1)
 *         At the end, configurations will be changed to previous values.
 * @param  ADC_Handler: Pointer Of Library Handler
 * @param  ADCSample:   Pointer Of Samples Array | Number of Element: 4 | [0]: Channel1
 * @param  GainConfig:  Gain Configuration | Number of Element: 4 | [0]: Channel1
 *         @note        Gains must be only 1, 2, and 4. See ADS1220_InputMuxConfig struct for more details.
 *                      - See ADS1220_GainConfig enum
 * @retval None
 */
void
ADS1220_ReadAllContinuousAVSS(ADS1220_Handler_t *ADC_Handler, int32_t *ADCSample, ADS1220_GainConfig_t *GainConfig)
{
  uint8_t Reg00hValue = ADS1220_ReadReg(ADC_Handler,REGISTER00h);
  
  ADS1220_StartSync(ADC_Handler);          // FOR WAKING UP. if you are using this function consecutively and quickly (less than ~54511.71875 us), you can comment this line
  while(ADC_Handler->ADC_DRDY_Read());     // FOR WAKING UP. if you are using this function consecutively and quickly (less than ~54511.71875 us), you can comment this line
  
  if (GainConfig)
  {
    ADS1220_WriteReg(ADC_Handler,REGISTER00h, 0x81 | (GainConfig[0] << 1));
    while(ADC_Handler->ADC_DRDY_Read());
    ADS1220_ReadDataWriteReg(ADC_Handler, REGISTER00h, 0x91 | (GainConfig[1] << 1), &ADCSample[0]);
    while(ADC_Handler->ADC_DRDY_Read());
    ADS1220_ReadDataWriteReg(ADC_Handler, REGISTER00h, 0xA1 | (GainConfig[2] << 1), &ADCSample[1]);
    while(ADC_Handler->ADC_DRDY_Read());
    ADS1220_ReadDataWriteReg(ADC_Handler, REGISTER00h, 0xB1 | (GainConfig[3] << 1), &ADCSample[2]);
    while(ADC_Handler->ADC_DRDY_Read());
    ADS1220_ReadDataWriteReg(ADC_Handler, REGISTER00h, Reg00hValue, &ADCSample[3]);
  }
  else
  {
    ADS1220_WriteReg(ADC_Handler,REGISTER00h, 0x81 | (Reg00hValue & 0x06));
    while(ADC_Handler->ADC_DRDY_Read());
    ADS1220_ReadDataWriteReg(ADC_Handler, REGISTER00h, 0x91 | (Reg00hValue & 0x06), &ADCSample[0]);
    while(ADC_Handler->ADC_DRDY_Read());
    ADS1220_ReadDataWriteReg(ADC_Handler, REGISTER00h, 0xA1 | (Reg00hValue & 0x06), &ADCSample[1]);
    while(ADC_Handler->ADC_DRDY_Read());
    ADS1220_ReadDataWriteReg(ADC_Handler, REGISTER00h, 0xB1 | (Reg00hValue & 0x06), &ADCSample[2]);
    while(ADC_Handler->ADC_DRDY_Read());
    ADS1220_ReadDataWriteReg(ADC_Handler, REGISTER00h, Reg00hValue, &ADCSample[3]);
  }
}

