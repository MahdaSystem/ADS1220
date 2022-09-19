/**
 **********************************************************************************
 * @file   ADS1220.h
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
//* Define to prevent recursive inclusion ---------------------------------------- //
#ifndef ADS1220_H
#define ADS1220_H

#ifdef __cplusplus
extern "C" {
#endif

//* Includes ---------------------------------------------------------------------- //
#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

//? User Configurations and Notes ------------------------------------------------- //
// SPI Configuration : 8Bits, CPOL=LOW(0), CPHA=2EDGE(1), Max speed (period): 150ns (6.66MHz)
#define ADS1220_USE_MACRO_DELAY         0    // 0: Use handler delay ,So you have to set ADC_Delay_US in Handler | 1: use Macro delay, So you have to set ADS1220_MACRO_DELAY_US Macro
//#define ADS1220_MACRO_DELAY_US(x)            // If you want to use Macro delay, place your delay function in microseconds here
// #define ADS1220_Debug_Enable                 // Uncomment if you want to use (depends on printf in stdio.h)
//#pragma anon_unions                         // Uncomment this line if yu are using Keil software
//? ------------------------------------------------------------------------------- //

//* Defines and Macros ------------------------------------------------------------ //
#define ADCValueToVoltage(x/*ADCvalue*/, v/*VREFF*/, g/*gain*/) (x * (float)v / 0x7FFFFF / g) // Use this to convert ADC value to Voltage - It Works
#define ADCValueToWeight(x/*ADCvalue*/, mw/*Max Weight*/, s/*Sensitivity*/) (x * (float)mw / 0x7FFFFF /s )
#define ADCValueToDisplacement(x/*ADCvalue*/, md /*Max Displacement*/) (x * (float)md / 0x7FFFFF)
#define ADCValueToTemperature(x/*ADCvalue*/, t/*Temperature*/) (x * (float)t / (float)0x7FFFFF)
#define ADCValueToTemperatureRTD(x/*ADCvalue*/, r/*RREF*/, r0/*R0-NOMINAL*/, a/*Alpha-TCR*/, g/*gain*/) ((x / (float)0x7FFFFF * 2.0f * r / g / r0 - 1) / a) // TCR(a) = (R100 - R0) / (R0 * 100)
#define ADCValueToCurrent(x/*ADCvalue*/, r/*Resistance*/, v/*VREFF*/, g/*gain*/) (x / (float)r * v / (float)0x7FFFFF / g)

//! DO NOT USE OR EDIT THIS BLOCK ------------------------------------------------- //
#if ADS1220_USE_MACRO_DELAY == 0
#define ADS1220_Delay_US(x)   ADC_Handler->ADC_Delay_US(x)
#else
#define ADS1220_Delay_US(x)   ADS1220_MACRO_DELAY_US(x)
#ifndef ADS1220_MACRO_DELAY_US
#error "ADS1220_MACRO_DELAY_US is not defined. Please Use handler delay or config ADS1220_MACRO_DELAY_US macro, You can choose it on ADS1220_USE_MACRO_DELAY define"
#endif
#endif

typedef union
ADS1220_OneSample_u {
  struct {
    uint32_t Zero :8; // Always Zero
    uint32_t Part1:8;
    uint32_t Part2:8;
    uint32_t Part3:8;
  };
  int32_t INT32;
} ADS1220_OneSample_t;
//! ------------------------------------------------------------------------------- //

/**
 ** ==================================================================================
 **                                ##### Enums #####                               
 ** ==================================================================================
 **/

/**
 * @brief  These bits configure the input multiplexer.
 * @note   For settings where AINN = AVSS, the PGA must be disabled (PGA_BYPASS = 1) and only gains 1, 2, and 4 can be used.
 */
typedef enum
ADS1220_InputMuxConfig_e {
  P0N1      = 0,  // AINP = AIN0, AINN = AIN1 (default)
  P0N2      = 1,  // AINP = AIN0, AINN = AIN2
  P0N3      = 2,  // AINP = AIN0, AINN = AIN3
  P1N2      = 3,  // AINP = AIN1, AINN = AIN2
  P1N3      = 4,  // AINP = AIN1, AINN = AIN3
  P2N3      = 5,  // AINP = AIN2, AINN = AIN3
  P1N0      = 6,  // AINP = AIN1, AINN = AIN0
  P3N2      = 7,  // AINP = AIN3, AINN = AIN2
  P0NAVSS   = 8,  // AINP = AIN0, AINN = AVSS
  P1NAVSS   = 9,  // AINP = AIN1, AINN = AVSS
  P2NAVSS   = 10, // AINP = AIN2, AINN = AVSS
  P3NAVSS   = 11, // AINP = AIN3, AINN = AVSS
  Mode1     = 12, // (V(REFPx) - V(REFNx)) / 4 monitor (PGA bypassed)
  Mode2     = 13, // (AVDD - AVSS) / 4 monitor (PGA bypassed)
  Mode3     = 14  // AINP and AINN shorted to (AVDD + AVSS) / 2
} ADS1220_InputMuxConfig_t;

/**
 * @brief  These bits configure the device gain. Gains 1, 2, and 4 can be used without the PGA.
 * @note   In this case, gain is obtained by a switched-capacitor structure.
 */
typedef enum
ADS1220_GainConfig_e {
  _1_       = 0, // (default)
  _2_       = 1,
  _4_       = 2,
  _8_       = 3,
  _16_      = 4,
  _32_      = 5,
  _64_      = 6,
  _128_     = 7
} ADS1220_GainConfig_t;

/**
 * @brief  These bits control the data rate setting depending on the selected operating mode.
 */
typedef enum
ADS1220_DataRate_e {
  // For Normal Mode
  _20_SPS_      = 0, // (defualt)
  _45_SPS_      = 1,
  _30_SPS_      = 2,
  _175_SPS_     = 3,
  _330_SPS_     = 4,
  _600_SPS_     = 5,
  _1000_SPS_    = 6,
  // For Duty Cycle Mode
  _5_SPS_       = 0, // (defualt)
  _11_25_SPS_   = 1, // 11.25
  _22_5_SPS_    = 2,
  _44_SPS_      = 3,
  _82_5_SPS_    = 4,
  _150_SPS_     = 5,
  _250_SPS_     = 6,
  // For Turbo Mode
  _40_SPS_      = 0, // (defualt)
  _90_SPS_      = 1,
  _180_SPS_     = 2,
  _350_SPS_     = 3,
  _660_SPS_     = 4,
  _1200_SPS_    = 5,
  _2000_SPS_    = 6
} ADS1220_DataRate_t;

/**
 * @brief  These bits control the operating mode the device operates in.
 */
typedef enum
ADS1220_OperatingMode_e {
  NormalMode    = 0, // (256-kHz modulator clock, default)
  DutyCycleMode = 1, // (internal duty cycle of 1:4)
  TurboMode     = 2  // 512-kHz modulator clock
} ADS1220_OperatingMode_t;

/**
 * @brief  These bits select the voltage reference source that is used for the conversion.
 */
typedef enum
ADS1220_VoltageRef_e { 
  Internal      = 0, // Internal 2.048-V reference selected (default)
  ExternalREF0  = 1, // External reference selected using dedicated REFP0 and REFN0 inputs
  ExternalREF1  = 2, // External reference selected using AIN0/REFP1 and AIN3/REFN1 inputs
  AnalogSupply  = 3  // Analog supply (AVDD - AVSS) used as reference
} ADS1220_VoltageRef_t;

/**
 * @brief  These bits configure the filter coefficients for the internal FIR filter.
 * @note   Only use these bits together with the 20-SPS setting in normal mode and the 5-SPS 
 *         setting in duty-cycle mode. Set to 00 for all other data rates.
 */
typedef enum
ADS1220_FIRFilter_e {
  No50or60Hz    = 0, // No 50-Hz or 60-Hz rejection (default)
  S50or60Hz     = 1, // Simultaneous 50-Hz and 60-Hz rejection
  Rej50Hz       = 2, // 50-Hz rejection only
  Rej60Hz       = 3  // 60-Hz rejection only
} ADS1220_FIRFilter_t;

/**
 * @brief  These bits set the current for both IDAC1 and IDAC2 excitation current sources.
 */
typedef enum
ADS1220_IDACcurrent_e {
  Off           = 0, // (default)
  _10uA_        = 1,
  _50uA_        = 2,
  _100uA_       = 3,
  _250uA_       = 4,
  _500uA_       = 5,
  _1000uA_      = 6,
  _1500uA_      = 7
} ADS1220_IDACcurrent_t;

/**
 * @brief  These bits select the channel where IDAC1 or IDAC2 is routed to. 
 */
typedef enum
ADS1220_IDACrouting_e {
  Disabled      = 0, // IDAC disabled (default)
  AIN0REFP1     = 1, // IDAC connected to AIN0/REFP1
  AIN1          = 2, // IDAC connected to AIN1
  AIN2          = 3, // IDAC connected to AIN2
  AIN3REFN1     = 4, // IDAC connected to AIN3/REFN1
  REFP0         = 5, // IDAC connected to REFP0
  REFN0         = 6  // IDAC connected to REFN0
} ADS1220_IDACrouting_t;

/**
 ** ==================================================================================
 **                               ##### Structs #####                               
 ** ==================================================================================
 **/

/**
 * @brief  Handling Library
 * @note   User MUST configure This at the begining of the program before ADS1230_Init
 */
typedef struct
ADS1220_Handler_s {
  void (*ADC_CS_HIGH)(void);                     // Must be initialized
  void (*ADC_CS_LOW)(void);                      // Must be initialized
  void (*ADC_Transmit)(uint8_t Data);            // Must be initialized
  uint8_t (*ADC_Receive)(void);                  // Must be initialized
  uint8_t (*ADC_TransmitReceive)(uint8_t Data);  // Can be initialized - Initialize this when you want to use ReadAllContinuous functions
  uint8_t (*ADC_DRDY_Read)(void);                // Can be initialized - Initialize this when you want to use ReadAllContinuous functions
  void (*ADC_Delay_US)(uint32_t);                // Must be initialized (Place here your delay in MicroSecond)
  ADS1220_OneSample_t ADCDataValues;              //!!! DO NOT USE OR EDIT THIS !!!
} ADS1220_Handler_t;

/**
 * @brief  ADC Parameters
 * @note   User Can configure This at the begining of the program before ADS1230_Init
 */
typedef struct
ADS1220_Parameters_s {
  // REG 00h:
  // See ADS1220_InputMuxConfig enum
  // IMPORTANT NOTE : For settings where AINN = AVSS, the PGA must be disabled (PGA_BYPASS = 1) and only gains 1, 2, and 4 can be used.
  ADS1220_InputMuxConfig_t InputMuxConfig; // default: AINP = AIN0, AINN = AIN1
  // See ADS1220_GainConfig enum
  ADS1220_GainConfig_t     GainConfig; // default: 1
  // Disables and bypasses the internal low-noise PGA
  // Disabling the PGA reduces overall power consumption and allows the commonmode
  // voltage range (VCM) to span from AVSS � 0.1 V to AVDD + 0.1 V.
  // The PGA can only be disabled for gains 1, 2, and 4.
  // The PGA is always enabled for gain settings 8 to 128, regardless of the
  // PGA_BYPASS setting.
  bool                      PGAdisable; // 0: PGA enabled (default) | 1: PGA disabled and bypassed
  
  // REG 01h:
  // See ADS1220_DataRate enum
  ADS1220_DataRate_t       DataRate; // default: NormalMode: 20SPS | DutyCycleMode: 5SPS | TurboMode: 40SPS
  // See ADS1220_OperatingMode enum
  ADS1220_OperatingMode_t  OperatingMode; // default: NormalMode
  // This bit sets the conversion mode for the device.
  bool ConversionMode; // 0: Single-shot mode (default) | 1: Continuous conversion mode
  // This bit enables the internal temperature sensor and puts the device in temperature sensor mode.
  // The settings of configuration register 0 have no effect and the device uses the
  // internal reference for measurement when temperature sensor mode is enabled.
  bool                     TempeSensorMode; // 0: Disables temperature sensor (default) | 1: Enables temperature sensor
  // This bit controls the 10-�A, burn-out current sources.
  // The burn-out current sources can be used to detect sensor faults such as wire
  // breaks and shorted sensors.
  bool                     BurnOutCurrentSrc; // 0: Current sources off (default) | 1: Current sources on
  
  // REG 02h:
  // See ADS1220_VoltageRef enum
  ADS1220_VoltageRef_t     VoltageRef; // default: Internal 2.048V
  // See ADS1220_FIRFilter enum
  ADS1220_FIRFilter_t      FIRFilter; // default: No rejection
  // This bit configures the behavior of the low-side switch connected between AIN3/REFN1 and AVSS.
  bool                     LowSodePwr; // 0: Switch is always open (default) | 1: Switch automatically closes when the START/SYNC command is sent and opens when the POWERDOWN command is issued
  // See ADS1220_IDACcurrent enum
  ADS1220_IDACcurrent_t    IDACcurrent;
  
  // REG 03h:
  // See ADS1220_IDACrouting enum , This is for IDAC1
  ADS1220_IDACrouting_t    IDAC1routing; // default: Off
  // See ADS1220_IDACrouting enum , This is for IDAC2
  ADS1220_IDACrouting_t    IDAC2routing; // default: Off
  // This bit controls the behavior of the DOUT/DRDY pin when new data are ready.
  bool                     DRDYMode; // 0: Only the dedicated DRDY pin is used to indicate when data are ready (default) | 1: Data ready is indicated simultaneously on DOUT/DRDY and DRDY
} ADS1220_Parameters_t;

/**
 ** ==================================================================================
 **                          ##### Public Functions #####                               
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
ADS1220_Init(ADS1220_Handler_t *ADC_Handler, ADS1220_Parameters_t * Parameters);

/**
 * @brief  Starts (in Single-shot mode) or Syncs (in Continuous conversion mode) Conversion
 * @param  ADC_Handler: Pointer Of Library Handler
 * @retval None
 */
void
ADS1220_StartSync(ADS1220_Handler_t *ADC_Handler);

/**
 * @brief  Resets ADS1220
 * @param  ADC_Handler: Pointer Of Library Handler
 * @retval None
 */
void
ADS1220_Reset(ADS1220_Handler_t *ADC_Handler);

/**
 * @brief  Enables Power down
 * @param  ADC_Handler: Pointer Of Library Handler
 * @retval None
 */
void
ADS1220_PowerDown(ADS1220_Handler_t *ADC_Handler);

/**
 * @brief  Reads Data for both Single-shot and Continuous conversion Modes
 * @note   Call This function when DRDY pin got LOW
 * @param  ADC_Handler: Pointer Of Library Handler
 * @param  ADCSample:   Pointer Of Samples Array | Number of Elements: 1 (To read other channels user should config InputMuxConfig in ADS1220_Parameters or USE ReadAll functions)
 * @retval None
 */
void
ADS1220_ReadData(ADS1220_Handler_t *ADC_Handler, int32_t *ADCSample);

/**
 * @brief  Changes the Configurations
 * @note   Pass Parameters as NULL to change settings to default Values. See ADS1220_Parameters struct to know what are default values
 * @param  ADC_Handler: Pointer Of Library Handler
 * @param  Parameters:  Pointer Of ADC Parameters
 * @retval None
 */
void
ADS1220_ChangeConfig(ADS1220_Handler_t *ADC_Handler, ADS1220_Parameters_t *Parameters);

/**
 * @brief  Changes Gains
 * @param  ADC_Handler: Pointer Of Library Handler
 * @param  GainConfig:  Gain Configuration
 *                      - See ADS1220_GainConfig enum
 * @retval None
 */
void
ADS1220_ChangeGain(ADS1220_Handler_t *ADC_Handler, ADS1220_GainConfig_t GainConfig);

/**
 * @brief  Activates Single-Shot Mode (Deactivates Continuous Mode)
 * @param  ADC_Handler: Pointer Of Library Handler
 * @retval None
 */
void
ADS1220_ActivateSingleShotMode(ADS1220_Handler_t *ADC_Handler);

/**
 * @brief  Activates Continuous Mode (Deactivates Single-Shot Mode)
 * @param  ADC_Handler: Pointer Of Library Handler
 * @retval None
 */
void
ADS1220_ActivateContinuousMode(ADS1220_Handler_t *ADC_Handler);

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
ADS1220_ReadAllSingleShotDiff(ADS1220_Handler_t *ADC_Handler, int32_t *ADCSample, ADS1220_GainConfig_t *GainConfig);

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
ADS1220_ReadAllContinuousDiff(ADS1220_Handler_t *ADC_Handler, int32_t *ADCSample, ADS1220_GainConfig_t *GainConfig);

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
ADS1220_ReadAllSingleShotAVSS(ADS1220_Handler_t *ADC_Handler, int32_t *ADCSample, ADS1220_GainConfig_t *GainConfig);

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
ADS1220_ReadAllContinuousAVSS(ADS1220_Handler_t *ADC_Handler, int32_t *ADCSample, ADS1220_GainConfig_t *GainConfig);


#endif
