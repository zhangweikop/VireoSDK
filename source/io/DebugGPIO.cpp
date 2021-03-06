#include "TypeDefiner.h"
#include "Instruction.h"

#if kVireoOS_ZynqARM
#include "xgpiops.h"
#define kuZED_LED_PIN 47
#define kuZED_Button_PIN 51

static XGpioPs _gGPIO;
static XGpioPs_Config *_gpGPIOPsConfig = null;

void InitGPIO()
{
	int status;
	_gpGPIOPsConfig = XGpioPs_LookupConfig(XPAR_PS7_GPIO_0_DEVICE_ID);
	status = XGpioPs_CfgInitialize(&_gGPIO, _gpGPIOPsConfig, _gpGPIOPsConfig->BaseAddr);
	if (status != XST_SUCCESS) {
		printf(" Zynq GPIO init failure\n");
	}

	 XGpioPs_SetDirectionPin(&_gGPIO, kuZED_LED_PIN, 1);
	 XGpioPs_SetOutputEnablePin(&_gGPIO, kuZED_LED_PIN, 1);
	 XGpioPs_SetDirectionPin(&_gGPIO, kuZED_Button_PIN, 0x0);
}
#endif

using namespace Vireo;
//------------------------------------------------------------
VIREO_FUNCTION_SIGNATURE1(DebugLED, Boolean)
{
#if kVireoOS_ZynqARM
	if (!_gpGPIOPsConfig)
		InitGPIO();

	int value =  _Param(0);
	XGpioPs_WritePin(&_gGPIO, kuZED_LED_PIN, value);
#endif
    return _NextInstruction();
}
//------------------------------------------------------------
VIREO_FUNCTION_SIGNATURE1(DebugButton, Boolean)
{
#if kVireoOS_ZynqARM
	if (!_gpGPIOPsConfig)
		InitGPIO();

	_Param(0) = XGpioPs_ReadPin(&_gGPIO, kuZED_Button_PIN);
#endif
    return _NextInstruction();
}

DEFINE_VIREO_BEGIN(DebugGPIO)
    DEFINE_VIREO_FUNCTION(DebugLED, "p(i(.Boolean))");
    DEFINE_VIREO_FUNCTION(DebugButton, "p(o(.Boolean))");
DEFINE_VIREO_END()
