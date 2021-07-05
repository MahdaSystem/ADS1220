#include "ADS1220.h"

#define ADS1220_RawToAdcValue(oneData) ((int32_t)(((oneData[0] << 16) | (oneData[1] << 8) | (oneData[2])) << 8) / 256)
#ifdef Debug_Enable
#include <stdio.h> // for debug
#define PROGRAMLOG(arg...) printf(arg)
#else
#define PROGRAMLOG(arg...)
#endif

typedef enum ADS1220Commands_s {
	RESET_ADC			= 0x06,
	START_SYNC		= 0x08,
	POWERDOWN			= 0x02,
	RDATA					=	0x10,
	RREG					= 0x20,
	WREG					= 0x40
} ADS1220Commands;

typedef enum ADS1220Register_s {
	REGISTER00h						=	0x00,
  REGISTER01h						=	0x01,
  REGISTER02h						=	0x02,
  REGISTER03h						=	0x03,
} ADS1220Register;

#pragma anon_unions
typedef union ADS131_OneSample_u {
  struct {
    uint32_t Zero :8; // Always Zero
    uint32_t Part1:8;
    uint32_t Part2:8;
    uint32_t Part3:8;
  };
  int32_t INT32;
} ADS131_OneSample;

static ADS131_OneSample ADCDataValues = {0};


static uint8_t ADS1220_ReadReg (ADS1220_Handler *ADC_Handler,ADS1220Register ADS1220REG)
{
	uint8_t RecByte = 0;
	ADC_Handler->ADC_CS_LOW();
	Delay_US(5);
	ADC_Handler->ADC_Transmit(RREG | (ADS1220REG << 2));
	Delay_US(5);
	RecByte = ADC_Handler->ADC_Receive();
	Delay_US(5);
	ADC_Handler->ADC_CS_HIGH();
	return RecByte;
};

// This function Works but we do not need it
//static void ADS1220_ReadAllRegs (ADS1220_Handler *ADC_Handler, uint8_t *ReadValues /*It is = [0]: REG00h | Number of Elements: 4*/)
//{
//	ADC_Handler->ADC_CS_LOW();
//	Delay_US(5);
//	ADC_Handler->ADC_Transmit(RREG | 3 /*(num reg to read: 4) - 1*//*starts from reg00h*/);
//	Delay_US(5);
//	ReadValues[0] = ADC_Handler->ADC_Receive();
//	Delay_US(5);
//  ReadValues[1] = ADC_Handler->ADC_Receive();
//	Delay_US(5);
//  ReadValues[2] = ADC_Handler->ADC_Receive();
//	Delay_US(5);
//  ReadValues[3] = ADC_Handler->ADC_Receive();
//	Delay_US(5);
//	ADC_Handler->ADC_CS_HIGH();
//};

// This function Works but we do not need it
//static void ADS1220_WriteReg (ADS1220_Handler *ADC_Handler,ADS1220Register ADS1220REG, uint8_t RegisterValue)
//{
//	ADC_Handler->ADC_CS_LOW();
//	Delay_US(5);
//	ADC_Handler->ADC_Transmit(WREG | (ADS1220REG << 2));
//	Delay_US(5);
//	ADC_Handler->ADC_Transmit(RegisterValue);
//	Delay_US(5);
//	ADC_Handler->ADC_CS_HIGH();
//};

static void ADS1220_WriteAllRegs (ADS1220_Handler *ADC_Handler, uint8_t *RegisterValue /*Must be = [0]: REG00h | Number of Elements: 4*/)
{
	ADC_Handler->ADC_CS_LOW();
	Delay_US(5);
	ADC_Handler->ADC_Transmit(WREG | 3 /*(num reg to read: 4) - 1*//*starts from reg00h*/);
	Delay_US(5);
	ADC_Handler->ADC_Transmit(RegisterValue[0]);
	Delay_US(5);
  ADC_Handler->ADC_Transmit(RegisterValue[1]);
	Delay_US(5);
  ADC_Handler->ADC_Transmit(RegisterValue[2]);
	Delay_US(5);
  ADC_Handler->ADC_Transmit(RegisterValue[3]);
	Delay_US(5);
	ADC_Handler->ADC_CS_HIGH();
};

void ADS1220_Init(ADS1220_Handler *ADC_Handler, ADS1220_Parameters * Parameters)
{
  if (!ADC_Handler)
     return;

	Delay_US(100); // Wait after reset at least 50us + tclk
  ADC_Handler->ADC_CS_LOW();
	Delay_US(5);
	ADC_Handler->ADC_Transmit(RESET_ADC);
	Delay_US(5);
	ADC_Handler->ADC_CS_HIGH();
	Delay_US(100); // Wait after reset at least 50us + tclk
  
  uint8_t RegsValue[4] = {0};
  
  if (Parameters)
  {
    RegsValue[0] = (Parameters->InputMuxConfig << 4) | (Parameters->PGAdisable << 1) | Parameters->PGAdisable;
    RegsValue[1] = (Parameters->DataRate << 5) | (Parameters->OperatingMode << 3) | (Parameters->ConversionMode << 2) | (Parameters->TempeSensorMode << 1) | Parameters->BurnOutCurrentSrc;
    RegsValue[2] = (Parameters->VoltageRef << 6) | (Parameters->FIRFilter << 4) | (Parameters->LowSodePwr << 3) | Parameters->IDACcurrent;
    RegsValue[3] = (Parameters->IDAC1routing << 5) | (Parameters->IDAC2routing << 2) | (Parameters->DRDYMode << 1);
    ADS1220_WriteAllRegs(ADC_Handler,RegsValue);
    
    PROGRAMLOG("Read    Values: 0x%02X | 0x%02X | 0x%02X | 0x%02X\r\n"
               "Correct Values: 0x%02X | 0x%02X | 0x%02X | 0x%02X\r\n",
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
    
    PROGRAMLOG("Defualt Mode: 0x%02X | 0x%02X | 0x%02X | 0x%02X\r\n"
               "Must    be:   0x00 | 0x00 | 0x00 | 0x00\r\n",
    ADS1220_ReadReg(ADC_Handler,REGISTER00h),
    ADS1220_ReadReg(ADC_Handler,REGISTER01h),
    ADS1220_ReadReg(ADC_Handler,REGISTER02h),
    ADS1220_ReadReg(ADC_Handler,REGISTER03h)
    );
  }
  
};

void ADS1220_StartSync(ADS1220_Handler *ADC_Handler)
{
  ADC_Handler->ADC_CS_LOW();
  Delay_US(5);
  ADC_Handler->ADC_Transmit(START_SYNC);
  Delay_US(5);
	ADC_Handler->ADC_CS_HIGH();
}

void ADS1220_ReadData(ADS1220_Handler *ADC_Handler, int32_t *ADCSample)
{
  ADC_Handler->ADC_CS_LOW();
  Delay_US(5);
  ADCDataValues.Part3 = ADC_Handler->ADC_Receive();
  Delay_US(1);
  ADCDataValues.Part2 = ADC_Handler->ADC_Receive();
  Delay_US(1);
  ADCDataValues.Part1 = ADC_Handler->ADC_Receive();
  Delay_US(1);
  Delay_US(5);
	ADC_Handler->ADC_CS_HIGH();
  *ADCSample = ADCDataValues.INT32 / 255;
}
