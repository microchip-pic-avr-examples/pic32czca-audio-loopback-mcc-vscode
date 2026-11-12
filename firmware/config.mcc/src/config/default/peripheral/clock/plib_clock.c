/*******************************************************************************
 CLOCK PLIB

  Company:
    Microchip Technology Inc.

  File Name:
    plib_clock.c

  Summary:
    CLOCK PLIB Implementation File.

  Description:
    None

*******************************************************************************/

/*******************************************************************************
* Copyright (C) 2021 Microchip Technology Inc. and its subsidiaries.
*
* Subject to your compliance with these terms, you may use Microchip software
* and any derivatives exclusively with Microchip products. It is your
* responsibility to comply with third party license terms applicable to your
* use of third party software (including open source software) that may
* accompany Microchip software.
*
* THIS SOFTWARE IS SUPPLIED BY MICROCHIP "AS IS". NO WARRANTIES, WHETHER
* EXPRESS, IMPLIED OR STATUTORY, APPLY TO THIS SOFTWARE, INCLUDING ANY IMPLIED
* WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY, AND FITNESS FOR A
* PARTICULAR PURPOSE.
*
* IN NO EVENT WILL MICROCHIP BE LIABLE FOR ANY INDIRECT, SPECIAL, PUNITIVE,
* INCIDENTAL OR CONSEQUENTIAL LOSS, DAMAGE, COST OR EXPENSE OF ANY KIND
* WHATSOEVER RELATED TO THE SOFTWARE, HOWEVER CAUSED, EVEN IF MICROCHIP HAS
* BEEN ADVISED OF THE POSSIBILITY OR THE DAMAGES ARE FORESEEABLE. TO THE
* FULLEST EXTENT ALLOWED BY LAW, MICROCHIP'S TOTAL LIABILITY ON ALL CLAIMS IN
* ANY WAY RELATED TO THIS SOFTWARE WILL NOT EXCEED THE AMOUNT OF FEES, IF ANY,
* THAT YOU HAVE PAID DIRECTLY TO MICROCHIP FOR THIS SOFTWARE.
*******************************************************************************/

#include "plib_clock.h"
#include "device.h"

static void OSCCTRL_Initialize(void)
{
    /****************** XOSC Initialization ********************************/

    /* Configure External Oscillator */
    OSCCTRL_REGS->OSCCTRL_XOSCCTRLA = OSCCTRL_XOSCCTRLA_STARTUP(14U) | OSCCTRL_XOSCCTRLA_SWBEN_Msk | OSCCTRL_XOSCCTRLA_ENABLE_Msk;
    while((OSCCTRL_REGS->OSCCTRL_STATUS & OSCCTRL_STATUS_XOSCRDY_Msk) != OSCCTRL_STATUS_XOSCRDY_Msk)
    {
        /* Waiting for the XOSC Ready state */
    }

}

static void OSC32KCTRL_Initialize(void)
{

    OSC32KCTRL_REGS->OSC32KCTRL_CLKSELCTRL = OSC32KCTRL_CLKSELCTRL_RTCSEL(1U);
}

static void PLL0_Initialize(void)
{
    /* Enable Additional Voltage Regulator */
    SUPC_REGS->SUPC_VREGCTRL |= SUPC_VREGCTRL_AVREGEN(4U);
    while ((SUPC_REGS->SUPC_STATUS & ((uint32_t)4U << SUPC_STATUS_ADDVREGRDY_Pos)) != ((uint32_t)4U << SUPC_STATUS_ADDVREGRDY_Pos))
    {
        /* Do Nothing */
    }


    /****************** PLL0 Initialization  *********************************/

    /* Configure PLL0 */
    /* Disable PLL0 and clear PLL0 control register */
    OSCCTRL_REGS->OSCCTRL_PLL0CTRL = 0U;
    OSCCTRL_REGS->OSCCTRL_PLL0REFDIV = OSCCTRL_PLL0REFDIV_REFDIV(1U);
    OSCCTRL_REGS->OSCCTRL_PLL0FBDIV = OSCCTRL_PLL0FBDIV_FBDIV(125U);

    OSCCTRL_REGS->OSCCTRL_FRACDIV0 = OSCCTRL_FRACDIV0_REMDIV(0U) | OSCCTRL_FRACDIV0_INTDIV(0U);
    while((OSCCTRL_REGS->OSCCTRL_SYNCBUSY & OSCCTRL_SYNCBUSY_FRACDIV0_Msk) == OSCCTRL_SYNCBUSY_FRACDIV0_Msk)
    {
        /* Waiting for the synchronization */
    }

    OSCCTRL_REGS->OSCCTRL_PLL0POSTDIVA = OSCCTRL_PLL0POSTDIVA_OUTEN0_Msk | OSCCTRL_PLL0POSTDIVA_POSTDIV0(5U) | OSCCTRL_PLL0POSTDIVA_OUTEN1_Msk | OSCCTRL_PLL0POSTDIVA_POSTDIV1(12U) | OSCCTRL_PLL0POSTDIVA_OUTEN2_Msk | OSCCTRL_PLL0POSTDIVA_POSTDIV2(10U) | OSCCTRL_PLL0POSTDIVA_OUTEN3_Msk | OSCCTRL_PLL0POSTDIVA_POSTDIV3(15U);

    OSCCTRL_REGS->OSCCTRL_PLL0CTRL |= OSCCTRL_PLL0CTRL_BWSEL(0x1U) | OSCCTRL_PLL0CTRL_REFSEL(1U) | OSCCTRL_PLL0CTRL_ENABLE_Msk;

    while((OSCCTRL_REGS->OSCCTRL_STATUS & OSCCTRL_STATUS_PLL0LOCK_Msk) != OSCCTRL_STATUS_PLL0LOCK_Msk)
    {
        /* Waiting for the Ready state */
    }
}

static void PLL1_Initialize(void)
{
    /* Enable Additional Voltage Regulator */
    SUPC_REGS->SUPC_VREGCTRL |= SUPC_VREGCTRL_AVREGEN(4U);
    while ((SUPC_REGS->SUPC_STATUS & ((uint32_t)4U << SUPC_STATUS_ADDVREGRDY_Pos)) != ((uint32_t)4U << SUPC_STATUS_ADDVREGRDY_Pos))
    {
        /* Do Nothing */
    }


    /****************** PLL1 Initialization  *********************************/

    /* Configure PLL1 */
    /* Disable PLL1 and clear PLL1 control register */
    OSCCTRL_REGS->OSCCTRL_PLL1CTRL = 0U;
    OSCCTRL_REGS->OSCCTRL_PLL1REFDIV = OSCCTRL_PLL1REFDIV_REFDIV(1U);
    OSCCTRL_REGS->OSCCTRL_PLL1FBDIV = OSCCTRL_PLL1FBDIV_FBDIV(128U);

    OSCCTRL_REGS->OSCCTRL_FRACDIV1 = OSCCTRL_FRACDIV1_REMDIV(0U) | OSCCTRL_FRACDIV1_INTDIV(0U);
    while((OSCCTRL_REGS->OSCCTRL_SYNCBUSY & OSCCTRL_SYNCBUSY_FRACDIV1_Msk) == OSCCTRL_SYNCBUSY_FRACDIV1_Msk)
    {
        /* Waiting for the synchronization */
    }

    OSCCTRL_REGS->OSCCTRL_PLL1POSTDIVA = OSCCTRL_PLL1POSTDIVA_OUTEN0_Msk | OSCCTRL_PLL1POSTDIVA_POSTDIV0(25U);

    OSCCTRL_REGS->OSCCTRL_PLL1CTRL |= OSCCTRL_PLL1CTRL_BWSEL(0x1U) | OSCCTRL_PLL1CTRL_REFSEL(1U) | OSCCTRL_PLL1CTRL_ENABLE_Msk;

    while((OSCCTRL_REGS->OSCCTRL_STATUS & OSCCTRL_STATUS_PLL1LOCK_Msk) != OSCCTRL_STATUS_PLL1LOCK_Msk)
    {
        /* Waiting for the Ready state */
    }
}

static void swDelayUs(uint32_t cpu_clock, uint32_t delay_us)
{
    uint32_t startCount, endCount;

    /* Calculate the end count for the given delay */
    endCount = (cpu_clock / 1000000U) * delay_us;

    startCount = DWT->CYCCNT;
    while((DWT->CYCCNT - startCount) < endCount)
	{
		/* Do Nothing */
	}
}

static void GCLK0_Initialize(void)
{

    /* Selection of the Clock Domain Division Factor */
    MCLK_REGS->MCLK_CLKDIV[1] = MCLK_CLKDIV_DIV(0x02U);

    while((MCLK_REGS->MCLK_INTFLAG & MCLK_INTFLAG_CKRDY_Msk) != MCLK_INTFLAG_CKRDY_Msk)
    {
        /* Wait for the Main Clock to be Ready */
    }
    // 100000000 Hz
    GCLK_REGS->GCLK_GENCTRL[0] = GCLK_GENCTRL_DIV(1U) | GCLK_GENCTRL_SRC(9U) | GCLK_GENCTRL_GENEN_Msk;

    while((GCLK_REGS->GCLK_SYNCBUSY & GCLK_SYNCBUSY_GENCTRL0_Msk) == GCLK_SYNCBUSY_GENCTRL0_Msk)
    {
        /* Wait for the Generator 0 synchronization */
    }

    CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
    DWT->LAR = 0xC5ACCE55U;
    DWT->CYCCNT = 0U;
    DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;
    swDelayUs(100000000U, 1U);

    // 150000000 Hz
    GCLK_REGS->GCLK_GENCTRL[0] = GCLK_GENCTRL_DIV(1U) | GCLK_GENCTRL_SRC(8U) | GCLK_GENCTRL_GENEN_Msk;

    while((GCLK_REGS->GCLK_SYNCBUSY & GCLK_SYNCBUSY_GENCTRL0_Msk) == GCLK_SYNCBUSY_GENCTRL0_Msk)
    {
        /* Wait for the Generator 0 synchronization */
    }

    swDelayUs(150000000U, 1U);

    // 125000000 Hz
    GCLK_REGS->GCLK_GENCTRL[0] = GCLK_GENCTRL_DIV(1U) | GCLK_GENCTRL_SRC(7U) | GCLK_GENCTRL_GENEN_Msk;

    while((GCLK_REGS->GCLK_SYNCBUSY & GCLK_SYNCBUSY_GENCTRL0_Msk) == GCLK_SYNCBUSY_GENCTRL0_Msk)
    {
        /* Wait for the Generator 0 synchronization */
    }

    swDelayUs(125000000U, 1U);

    // 300000000 Hz
    GCLK_REGS->GCLK_GENCTRL[0] = GCLK_GENCTRL_DIV(1U) | GCLK_GENCTRL_SRC(6U) | GCLK_GENCTRL_GENEN_Msk;

    while((GCLK_REGS->GCLK_SYNCBUSY & GCLK_SYNCBUSY_GENCTRL0_Msk) == GCLK_SYNCBUSY_GENCTRL0_Msk)
    {
        /* Wait for the Generator 0 synchronization */
    }
}

void CLOCK_CpuStepUp(void)
{
    // 150000000 Hz
    GCLK_REGS->GCLK_GENCTRL[0] = GCLK_GENCTRL_DIV(1U) | GCLK_GENCTRL_SRC(8U) | GCLK_GENCTRL_GENEN_Msk;

    while((GCLK_REGS->GCLK_SYNCBUSY & GCLK_SYNCBUSY_GENCTRL0_Msk) == GCLK_SYNCBUSY_GENCTRL0_Msk)
    {
        /* Wait for the Generator 0 synchronization */
    }

    swDelayUs(150000000U, 1U);

    // 125000000 Hz
    GCLK_REGS->GCLK_GENCTRL[0] = GCLK_GENCTRL_DIV(1U) | GCLK_GENCTRL_SRC(7U) | GCLK_GENCTRL_GENEN_Msk;

    while((GCLK_REGS->GCLK_SYNCBUSY & GCLK_SYNCBUSY_GENCTRL0_Msk) == GCLK_SYNCBUSY_GENCTRL0_Msk)
    {
        /* Wait for the Generator 0 synchronization */
    }

    swDelayUs(125000000U, 1U);

    // 300000000 Hz
    GCLK_REGS->GCLK_GENCTRL[0] = GCLK_GENCTRL_DIV(1U) | GCLK_GENCTRL_SRC(6U) | GCLK_GENCTRL_GENEN_Msk;

    while((GCLK_REGS->GCLK_SYNCBUSY & GCLK_SYNCBUSY_GENCTRL0_Msk) == GCLK_SYNCBUSY_GENCTRL0_Msk)
    {
        /* Wait for the Generator 0 synchronization */
    }


}

void CLOCK_CpuStepDown(void)
{
    // 125000000 Hz
    GCLK_REGS->GCLK_GENCTRL[0] = GCLK_GENCTRL_DIV(1U) | GCLK_GENCTRL_SRC(7U) | GCLK_GENCTRL_GENEN_Msk;

    while((GCLK_REGS->GCLK_SYNCBUSY & GCLK_SYNCBUSY_GENCTRL0_Msk) == GCLK_SYNCBUSY_GENCTRL0_Msk)
    {
        /* Wait for the Generator 0 synchronization */
    }

    swDelayUs(125000000U, 1U);

    // 150000000 Hz
    GCLK_REGS->GCLK_GENCTRL[0] = GCLK_GENCTRL_DIV(1U) | GCLK_GENCTRL_SRC(8U) | GCLK_GENCTRL_GENEN_Msk;

    while((GCLK_REGS->GCLK_SYNCBUSY & GCLK_SYNCBUSY_GENCTRL0_Msk) == GCLK_SYNCBUSY_GENCTRL0_Msk)
    {
        /* Wait for the Generator 0 synchronization */
    }

    swDelayUs(150000000U, 1U);

    // 100000000 Hz
    GCLK_REGS->GCLK_GENCTRL[0] = GCLK_GENCTRL_DIV(1U) | GCLK_GENCTRL_SRC(9U) | GCLK_GENCTRL_GENEN_Msk;

    while((GCLK_REGS->GCLK_SYNCBUSY & GCLK_SYNCBUSY_GENCTRL0_Msk) == GCLK_SYNCBUSY_GENCTRL0_Msk)
    {
        /* Wait for the Generator 0 synchronization */
    }


}

static void GCLK1_Initialize(void)
{
    // 300000000 Hz
    GCLK_REGS->GCLK_GENCTRL[1] = GCLK_GENCTRL_DIV(2U) | GCLK_GENCTRL_SRC(6U) | GCLK_GENCTRL_GENEN_Msk;

    while((GCLK_REGS->GCLK_SYNCBUSY & GCLK_SYNCBUSY_GENCTRL1_Msk) == GCLK_SYNCBUSY_GENCTRL1_Msk)
    {
        /* Wait for the Generator 1 synchronization */
    }
}


static void GCLK2_Initialize(void)
{
    // 300000000 Hz
    GCLK_REGS->GCLK_GENCTRL[2] = GCLK_GENCTRL_DIV(5U) | GCLK_GENCTRL_SRC(10U) | GCLK_GENCTRL_GENEN_Msk;

    while((GCLK_REGS->GCLK_SYNCBUSY & GCLK_SYNCBUSY_GENCTRL2_Msk) == GCLK_SYNCBUSY_GENCTRL2_Msk)
    {
        /* Wait for the Generator 2 synchronization */
    }
}



void CLOCK_Initialize (void)
{
    /* Function to Initialize the Oscillators */
    OSCCTRL_Initialize();

    /* Function to Initialize the 32KHz Oscillators */
    OSC32KCTRL_Initialize();

    PLL1_Initialize();
    PLL0_Initialize();
    GCLK2_Initialize();
    GCLK0_Initialize();
    GCLK1_Initialize();



    /* Selection of the Generator and write Lock for SERCOM1_CORE */
    GCLK_REGS->GCLK_PCHCTRL[22] = GCLK_PCHCTRL_GEN(0x1)  | GCLK_PCHCTRL_CHEN_Msk;

    while ((GCLK_REGS->GCLK_PCHCTRL[22] & GCLK_PCHCTRL_CHEN_Msk) != GCLK_PCHCTRL_CHEN_Msk)
    {
        /* Wait for synchronization */
    }
    /* Selection of the Generator and write Lock for SERCOM3_CORE */
    GCLK_REGS->GCLK_PCHCTRL[24] = GCLK_PCHCTRL_GEN(0x1)  | GCLK_PCHCTRL_CHEN_Msk;

    while ((GCLK_REGS->GCLK_PCHCTRL[24] & GCLK_PCHCTRL_CHEN_Msk) != GCLK_PCHCTRL_CHEN_Msk)
    {
        /* Wait for synchronization */
    }
    /* Selection of the Generator and write Lock for TCC0 */
    GCLK_REGS->GCLK_PCHCTRL[31] = GCLK_PCHCTRL_GEN(0x1)  | GCLK_PCHCTRL_CHEN_Msk;

    while ((GCLK_REGS->GCLK_PCHCTRL[31] & GCLK_PCHCTRL_CHEN_Msk) != GCLK_PCHCTRL_CHEN_Msk)
    {
        /* Wait for synchronization */
    }
    /* Selection of the Generator and write Lock for SPI_IXS0 */
    GCLK_REGS->GCLK_PCHCTRL[44] = GCLK_PCHCTRL_GEN(0x2)  | GCLK_PCHCTRL_CHEN_Msk;

    while ((GCLK_REGS->GCLK_PCHCTRL[44] & GCLK_PCHCTRL_CHEN_Msk) != GCLK_PCHCTRL_CHEN_Msk)
    {
        /* Wait for synchronization */
    }
}
