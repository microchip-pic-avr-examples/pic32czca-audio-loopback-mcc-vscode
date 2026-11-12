/*******************************************************************************
  Audio Application Source File

  Company:
    Microchip Technology Inc.

  File Name:
    app_audio.c

  Summary:
    I2S Audio implementation for 16-bit stereo audio using SPI_IXS peripheral

  Description:
    Implements I2S audio streaming with WM8904 codec using SPI_IXS0 configured
    for 16-bit stereo audio at 48 kHz. Bidirectional DMA transfers provide
    microphone input (RX) and audio output (TX) simultaneously.

    Audio Configuration:
    - Sample Rate: 48 kHz
    - Bit Depth: 16-bit
    - Channels: 2 (Stereo)
    - Frame Size: 4 bytes (2 channels × 2 bytes)
    - Buffer Size: 512 samples per channel (1024 bytes)
    - BCLK: 1.536 MHz (48 kHz × 2 channels × 16 bits)
    - GCLK: 12.288 MHz (BCLK × 8, per the SPI_IXS BRG setting)

    DMA Configuration:
    - Channel 0: RX (SPI_IXS0 → Buffer, Trigger ID 79)
    - Channel 1: TX (Buffer → SPI_IXS0, Trigger ID 80)
    - Double buffering on both channels

 *******************************************************************************/

 //DOM-IGNORE-BEGIN
/*******************************************************************************
* Copyright (C) 2026 Microchip Technology Inc. and its subsidiaries.
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
//DOM-IGNORE-END

#include "app_audio.h"
#include <string.h>


/* Audio DMA Buffers (32-byte aligned)
 * Double-buffering: Buffer0 and Buffer1 alternate
 * Size: 512 samples × 2 bytes = 1024 bytes per buffer
 * Used by both RX (mic input) and TX (audio output) DMA channels */
static __attribute__((aligned(32))) uint16_t Buffer0[APP_AUDIO_BUF_SIZE] = {0};
static __attribute__((aligned(32))) uint16_t Buffer1[APP_AUDIO_BUF_SIZE] = {0};

/* Application state data */
static APP_AUDIO_DATA app_audioData;

// *****************************************************************************
// *****************************************************************************
// Section: Application Callback Routines
// *****************************************************************************
// *****************************************************************************


// *****************************************************************************
// *****************************************************************************
// Section: Application Local Functions
// *****************************************************************************
// *****************************************************************************

/**
 * @brief Blocking delay using SYS_TIME service
 * @param ms Delay duration in milliseconds
 * @note Used for I2C timing delays during codec initialization
 */
static void APP_AUDIO_DelayMS(int ms)
{
    SYS_TIME_HANDLE timer = SYS_TIME_HANDLE_INVALID;

    if (SYS_TIME_DelayMS(ms, &timer) != SYS_TIME_SUCCESS)
        return;

    while (SYS_TIME_DelayIsComplete(timer) == false);
}


/**
 * @brief Write to WM8904 codec register via I2C
 * @param addr  WM8904 register address (8-bit)
 * @param val   16-bit register value to write
 * @param delay Delay in ms after write (for codec settling time)
 * @return true on success
 * @note Uses polling mode - waits for specified delay after each write
 */
static bool APP_AUDIO_CODEC_I2C_Write (uint8_t addr, uint16_t val, uint8_t delay)
{
    app_audioData.txBuffer[0] = addr;
    app_audioData.txBuffer[1] = (val & 0xFF00) >> 8;  /* MSB first */
    app_audioData.txBuffer[2] = (val & 0x00FF);       /* LSB second */

    DRV_I2C_WriteTransfer(app_audioData.i2cHandle,
                                APP_AUDIO_CODEC_ADDR,
                                (void *)app_audioData.txBuffer,
                                3);

    APP_AUDIO_DelayMS(delay);

    return true;
}

/**
 * @brief Verify WM8904 codec presence by reading chip ID
 * @return true if correct chip ID (0x8904) is read from register 0x00
 * @note This confirms I2C communication and correct codec hardware
 */
static bool APP_AUDIO_CODEC_ID_Check (void)
{
    app_audioData.txBuffer[0] = 0x00;  /* Register 0: Chip ID */

    DRV_I2C_WriteReadTransfer(app_audioData.i2cHandle,
                                APP_AUDIO_CODEC_ADDR,
                                (void *)app_audioData.txBuffer,
                                1,
                                (void *)app_audioData.rxBuffer,
                                2);

    APP_AUDIO_DelayMS(10);

    /* Reconstruct 16-bit ID (MSB first) */
    uint16_t id = app_audioData.rxBuffer[1] | (app_audioData.rxBuffer[0] << 8);

    if(id != APP_AUDIO_CODEC_ID)
    {
        return false;
    }

    return true;
}

/**
 * @brief Configure WM8904 codec with initialization sequence
 * @return true on success
 * @note Writes complete register sequence from wm8904_48k_16bit[] table
 *       Total time: ~471ms (sum of per-register delays for power-up, FLL lock, etc.)
 */
static bool APP_AUDIO_CODEC_Config (void)
{
    for (uint32_t i = 0; i < WM8904_48K_16BIT_COUNT; i++)
    {
        APP_AUDIO_CODEC_I2C_Write(wm8904_48k_16bit[i].reg,
            wm8904_48k_16bit[i].val,
            wm8904_48k_16bit[i].delay);
    }

    return true;
}

/**
 * @brief Configure SPI_IXS0 peripheral for I2S master mode
 * @return true on success
 * @note Configures: baud rate, I2S mode, master clocking, stereo, 16-bit word size
 *       Must be called before DMA setup
 */
static bool AUDIO_ConfigureI2S(void)
{
    /*
     * For 48 kHz stereo with 16-bit samples per channel:
     * BCLK = GCLK / (2 * (BRG + 1))
     * BCLK = 48000 * 2 channels * 16 bit = 1.536 MHz
     * With GCLK = 12.288 MHz:
     * BRG = (12.288 MHz / (2 * 1.536 MHz)) - 1 = 3
     */
    SPI_IXS0_REGS->SPI_IXS_BRG = SPI_IXS_BRG_BRG(3);

    /* SELCTRL: Select audio mode with bidirectional transfer */
    SPI_IXS0_REGS->SPI_IXS_SELCTRL = SPI_IXS_SELCTRL_IGNROV_Msk |                                       /* Ignore RX overflow errors */
                                    SPI_IXS_SELCTRL_IGNTUR_Msk |                                        /* Ignore TX underrun errors */
                                    SPI_IXS_SELCTRL_DATFMTLR(0) |                                       /* Left-justified (MSB first, I2S standard) */
                                    SPI_IXS_SELCTRL_CPHA(1) |                                           /* Clock phase: data valid on falling edge */
                                    SPI_IXS_SELCTRL_CPOL(1) |                                           /* Clock polarity: idle high */
                                    SPI_IXS_SELCTRL_STXISEL(SPI_IXS_SELCTRL_STXISEL_TXBNFULL_Val) |     /* TX interrupt: buffer not full */
                                    SPI_IXS_SELCTRL_SRXISEL(SPI_IXS_SELCTRL_SRXISEL_RXBNEMPTY_Val) |    /* RX interrupt: buffer not empty */
                                    SPI_IXS_SELCTRL_MODEEN(SPI_IXS_SELCTRL_MODEEN_AUDEN_Val);           /* Enable audio mode */

    /* SPICTRL: Enable master mode (MCU generates BCLK and LRCLK) */
    SPI_IXS0_REGS->SPI_IXS_SPICTRL = SPI_IXS_SPICTRL_MSTEN(1);

    /* FRAMECTRL: Configure for stereo (2 channels) */
    SPI_IXS0_REGS->SPI_IXS_FRAMECTRL = SPI_IXS_FRAMECTRL_FRMPOL(0) |                                /* Frame polarity: LRCLK low = left */
                                    SPI_IXS_FRAMECTRL_FRMCNT(1) |                                   /* Frame count: 2 channels (CNT2) */
                                    SPI_IXS_FRAMECTRL_FRMCOINC(0) |                                 /* Frame coincidence disabled */
                                    SPI_IXS_FRAMECTRL_FRMSLV(0) |                                   /* Frame master (not slave) */
                                    SPI_IXS_FRAMECTRL_FRMSYPW(SPI_IXS_FRAMECTRL_FRMSYPW_SYPW1_Val); /* LRCLK pulse width = 1 slot */

    /* AUDCTRL: Audio control for 16-bit I2S standard mode */
    SPI_IXS0_REGS->SPI_IXS_AUDCTRL = SPI_IXS_AUDCTRL_AUDMOD(SPI_IXS_AUDCTRL_AUDMOD_STDMODE_Val) |   /* Standard I2S mode */
                                    SPI_IXS_AUDCTRL_AUDFMT(SPI_IXS_AUDCTRL_AUDFMT_I2SLEGACY_Val) |  /* I2S Philips format */
                                    SPI_IXS_AUDCTRL_AUDWDMODE(0) |                                  /* Word size: 16-bit (0=16-bit) */
                                    SPI_IXS_AUDCTRL_AUDMONO(0);                                     /* Stereo mode (not mono) */

    return true;
}

/**
 * @brief Configure DMA channels for I2S audio streaming
 * @return true on success
 * @note Sets up circular DMA with double buffering:
 *       - Channel 0 (RX): SPI_IXS0_BUF → Buffer0/Buffer1 (microphone input)
 *       - Channel 1 (TX): Buffer0/Buffer1 → SPI_IXS0_BUF (audio output)
 *       Enables SPI_IXS0 peripheral after DMA setup
 */
static bool AUDIO_ConfigureDMA(void)
{
    static DMA_DESCRIPTOR_REGS txdmaDescRegs[2] = {0};
    static DMA_DESCRIPTOR_REGS rxdmaDescRegs[2] = {0};

    /* ===== RX DMA Channel 0: Microphone Input (SPI_IXS0 → Memory) ===== */

    /* RX Descriptor 0: SPI_IXS0_BUF → Buffer0 */
    rxdmaDescRegs[0].DMA_BDNXT = (uint32_t)&rxdmaDescRegs[1];            /* Link to descriptor 1 */
    rxdmaDescRegs[0].DMA_BDCFG.SSA = 1;                                  /* Source address fixed */
    rxdmaDescRegs[0].DMA_BDCFG.DSA = 1;                                  /* Destination address increments */
    rxdmaDescRegs[0].DMA_BDCFG.XSIZ = 1;                                 /* Transfer size control enabled */
    rxdmaDescRegs[0].DMA_BDCFG.ENABLE = 1;                               /* Descriptor enabled */
    rxdmaDescRegs[0].DMA_BDCFG.LLEN = 1;                                 /* Linked list enabled */
    rxdmaDescRegs[0].DMA_BDDSA = (uint32_t)Buffer0;                      /* Destination: Buffer0 */
    rxdmaDescRegs[0].DMA_BDSSA = (uint32_t)&SPI_IXS0_REGS->SPI_IXS_BUF;  /* Source: SPI_IXS0 RX buffer */
    rxdmaDescRegs[0].DMA_BDXSIZ.CSZ = 2;                                 /* Cell size: 2 bytes (16-bit samples) */
    rxdmaDescRegs[0].DMA_BDXSIZ.BLKSZ = APP_AUDIO_BUF_SIZE;              /* Block size: 512 samples */

    /* RX Descriptor 1: SPI_IXS0_BUF → Buffer1 */

    rxdmaDescRegs[1].DMA_BDNXT = (uint32_t)&rxdmaDescRegs[0];            /* Link back to descriptor 0 (circular) */
    rxdmaDescRegs[1].DMA_BDCFG.SSA = 1;                                  /* Source address fixed */
    rxdmaDescRegs[1].DMA_BDCFG.DSA = 1;                                  /* Destination address increments */
    rxdmaDescRegs[1].DMA_BDCFG.XSIZ = 1;                                 /* Transfer size control enabled */
    rxdmaDescRegs[1].DMA_BDCFG.ENABLE = 1;                               /* Descriptor enabled */
    rxdmaDescRegs[1].DMA_BDCFG.LLEN = 1;                                 /* Linked list enabled */
    rxdmaDescRegs[1].DMA_BDDSA = (uint32_t)Buffer1;                      /* Destination: Buffer1 */
    rxdmaDescRegs[1].DMA_BDSSA = (uint32_t)&SPI_IXS0_REGS->SPI_IXS_BUF;  /* Source: SPI_IXS0 RX buffer */
    rxdmaDescRegs[1].DMA_BDXSIZ.CSZ = 2;                                 /* Cell size: 2 bytes */
    rxdmaDescRegs[1].DMA_BDXSIZ.BLKSZ = APP_AUDIO_BUF_SIZE;              /* Block size: 512 samples */

    /* Start RX DMA channel with linked list */
    DMA_ChannelLinkedListTransfer(DMA_CHANNEL_0, rxdmaDescRegs);

    /* ===== TX DMA Channel 1: Audio Output (Memory → SPI_IXS0) ===== */

    /* TX Descriptor 0: Buffer0 → SPI_IXS0_BUF */
    txdmaDescRegs[0].DMA_BDNXT = (uint32_t)&txdmaDescRegs[1];            /* Link to descriptor 1 */
    txdmaDescRegs[0].DMA_BDCFG.SSA = 1;                                  /* Source address increments */
    txdmaDescRegs[0].DMA_BDCFG.DSA = 1;                                  /* Destination address fixed */
    txdmaDescRegs[0].DMA_BDCFG.XSIZ = 1;                                 /* Transfer size control enabled */
    txdmaDescRegs[0].DMA_BDCFG.ENABLE = 1;                               /* Descriptor enabled */
    txdmaDescRegs[0].DMA_BDCFG.LLEN = 1;                                 /* Linked list enabled */
    txdmaDescRegs[0].DMA_BDSSA = (uint32_t)Buffer0;                      /* Source: Buffer0 */
    txdmaDescRegs[0].DMA_BDDSA = (uint32_t)&SPI_IXS0_REGS->SPI_IXS_BUF;  /* Destination: SPI_IXS0 TX buffer */
    txdmaDescRegs[0].DMA_BDXSIZ.CSZ = 2;                                 /* Cell size: 2 bytes (16-bit samples) */
    txdmaDescRegs[0].DMA_BDXSIZ.BLKSZ = APP_AUDIO_BUF_SIZE;              /* Block size: 512 samples */

    /* TX Descriptor 1: Buffer1 → SPI_IXS0_BUF */

    txdmaDescRegs[1].DMA_BDNXT = (uint32_t)&txdmaDescRegs[0];            /* Link back to descriptor 0 (circular) */
    txdmaDescRegs[1].DMA_BDCFG.SSA = 1;                                  /* Source address increments */
    txdmaDescRegs[1].DMA_BDCFG.DSA = 1;                                  /* Destination address fixed */
    txdmaDescRegs[1].DMA_BDCFG.XSIZ = 1;                                 /* Transfer size control enabled */
    txdmaDescRegs[1].DMA_BDCFG.ENABLE = 1;                               /* Descriptor enabled */
    txdmaDescRegs[1].DMA_BDCFG.LLEN = 1;                                 /* Linked list enabled */
    txdmaDescRegs[1].DMA_BDSSA = (uint32_t)Buffer1;                      /* Source: Buffer1 */
    txdmaDescRegs[1].DMA_BDDSA = (uint32_t)&SPI_IXS0_REGS->SPI_IXS_BUF;  /* Destination: SPI_IXS0 TX buffer */
    txdmaDescRegs[1].DMA_BDXSIZ.CSZ = 2;                                 /* Cell size: 2 bytes */
    txdmaDescRegs[1].DMA_BDXSIZ.BLKSZ = APP_AUDIO_BUF_SIZE;              /* Block size: 512 samples */

    /* Start TX DMA channel with linked list */
    DMA_ChannelLinkedListTransfer(DMA_CHANNEL_1, txdmaDescRegs);

    /* Enable SPI_IXS0 peripheral to begin audio streaming */
    SPI_IXS0_REGS->SPI_IXS_CTRLA |= SPI_IXS_CTRLA_ENABLE_Msk;

    return true;
}



// *****************************************************************************
// *****************************************************************************
// Section: Application Initialization and State Machine Functions
// *****************************************************************************
// *****************************************************************************

/*******************************************************************************
  Function:
    void APP_AUDIO_Initialize ( void )

  Remarks:
    See prototype in app_audio.h.
 */

/**
 * @brief Initialize audio application state
 * @note Called once from SYS_Initialize before entering main task loop
 */
void APP_AUDIO_Initialize(void)
{
    /* Initialize state machine to INIT state */
    app_audioData.state = APP_AUDIO_STATE_INIT;

    /* Clear DMA buffers to silence */
    memset(Buffer0, 0, sizeof(Buffer0));
    memset(Buffer1, 0, sizeof(Buffer1));

    SYS_CONSOLE_PRINT("AUDIO: Initialized (16-bit stereo @ 48 kHz)\r\n");
}

/******************************************************************************
  Function:
    void APP_AUDIO_Tasks ( void )

  Remarks:
    See prototype in app_audio.h.
 */

/**
 * @brief Audio application state machine
 * @note Called from SYS_Tasks on every pass of the main loop (no fixed period)
 *       States: INIT → CONFIGURE_CODEC → CONFIGURE_I2S → STREAMING
 */
void APP_AUDIO_Tasks(void)
{
    switch (app_audioData.state)
    {
        case APP_AUDIO_STATE_INIT:
        {
            SYS_CONSOLE_PRINT("AUDIO: Starting initialization\r\n");

            /* Open I2C driver for codec communication (SERCOM3) */
            app_audioData.i2cHandle = DRV_I2C_Open(DRV_I2C_INDEX_0, DRV_IO_INTENT_READWRITE);

            if(app_audioData.i2cHandle != DRV_HANDLE_INVALID)
            {
                SYS_CONSOLE_PRINT("AUDIO: I2C driver opened successfully\r\n");
                app_audioData.state = APP_AUDIO_STATE_CONFIGURE_CODEC;
            }
            else
            {
                SYS_CONSOLE_PRINT("AUDIO: ERROR - Failed to open I2C driver\r\n");
                app_audioData.state = APP_AUDIO_STATE_ERROR;
            }

            break;
        }

        case APP_AUDIO_STATE_CONFIGURE_CODEC:
        {
            /* Verify codec is present and responding */
            if (APP_AUDIO_CODEC_ID_Check())
            {
                SYS_CONSOLE_PRINT("AUDIO: WM8904 codec detected \r\n");

                /* Configure codec registers */
                if (APP_AUDIO_CODEC_Config())
                {
                    SYS_CONSOLE_PRINT("AUDIO: Codec configured successfully\r\n");
                    app_audioData.state = APP_AUDIO_STATE_CONFIGURE_I2S;
                }
                else
                {
                    SYS_CONSOLE_PRINT("AUDIO: ERROR - Codec configuration failed\r\n");
                    app_audioData.state = APP_AUDIO_STATE_ERROR;
                }
            }
            else
            {
                SYS_CONSOLE_PRINT("AUDIO: ERROR - WM8904 codec not detected\r\n");
                app_audioData.state = APP_AUDIO_STATE_ERROR;
            }

            break;
        }

        case APP_AUDIO_STATE_CONFIGURE_I2S:
        {
            /* Configure SPI_IXS0 peripheral for I2S master mode */
            if (AUDIO_ConfigureI2S())
            {
                SYS_CONSOLE_PRINT("AUDIO: I2S configured (master mode, 2 channels, 16-bit)\r\n");

                /* Setup bidirectional DMA for audio streaming */
                if (AUDIO_ConfigureDMA())
                {
                    SYS_CONSOLE_PRINT("AUDIO: DMA configured - streaming started\r\n");
                    app_audioData.state = APP_AUDIO_STATE_STREAMING;
                }
                else
                {
                    SYS_CONSOLE_PRINT("AUDIO: ERROR - DMA configuration failed\r\n");
                    app_audioData.state = APP_AUDIO_STATE_ERROR;
                }
            }
            else
            {
                SYS_CONSOLE_PRINT("AUDIO: ERROR - I2S configuration failed\r\n");
                app_audioData.state = APP_AUDIO_STATE_ERROR;
            }
            break;
        }

        case APP_AUDIO_STATE_STREAMING:
        {
            /* Audio runs independently under DMA; this state only toggles the
             * LED as a heartbeat (blocking 250 ms delay per pass) */

            LED_Toggle();
            APP_AUDIO_DelayMS(250);

            break;
        }

        case APP_AUDIO_STATE_ERROR:
        {
            /* Error state - stays here until reset
             * Could implement recovery: retry I2C, reset codec, etc. */
            break;
        }

        case APP_AUDIO_STATE_IDLE:
        default:
        {
            /* Idle/undefined state */
            break;
        }
    }
}

/*******************************************************************************
 End of File
*/
