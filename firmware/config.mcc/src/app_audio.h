/*******************************************************************************
  MPLAB Harmony Application Header File

  Company:
    Microchip Technology Inc.

  File Name:
    app_audio.h

  Summary:
    I2S audio interface header for WM8904 codec with 16-bit stereo @ 48 kHz

  Description:
    Provides prototypes and configuration data for I2S audio streaming using
    SPI_IXS0 peripheral. Includes WM8904 codec initialization sequence and
    audio buffer configuration.

    Hardware:
    - WM8904 codec @ I2C address 0x1A
    - SPI_IXS0 in I2S master mode
    - DMA Channel 0: RX (microphone input)
    - DMA Channel 1: TX (audio output)
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

#ifndef _APP_AUDIO_H
#define _APP_AUDIO_H

// *****************************************************************************
// *****************************************************************************
// Section: Included Files
// *****************************************************************************
// *****************************************************************************

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>
#include "configuration.h"
#include "definitions.h"

// DOM-IGNORE-BEGIN
#ifdef __cplusplus  // Provide C++ Compatibility

extern "C" {

#endif
// DOM-IGNORE-END

// *****************************************************************************
// *****************************************************************************
// Section: I2S Audio Configuration (16-bit Stereo @ 48 kHz)
// *****************************************************************************
// *****************************************************************************

#define APP_AUDIO_CODEC_ADDR              0x1A      /* WM8904 I2C address */
#define APP_AUDIO_CODEC_ID                0x8904    /* Expected chip ID */

#define APP_AUDIO_BUF_SIZE                512       /* DMA buffer size (samples per channel) */

/* WM8904 register configuration structure */
typedef struct
{
    uint8_t  reg;      /* WM8904 register address */
    uint16_t val;      /* 16-bit register value */
    uint8_t delay;     /* Delay in ms after write */
} WM8904_RegWrite_t;

/* WM8904 initialization sequence for 48 kHz, 16-bit I2S mode
 * Configures: power, clocks (FLL), DACs/ADCs, volumes, I2S interface
 */
static const WM8904_RegWrite_t wm8904_48k_16bit[] =
{
    {0x00, 0x0000, 100},  /* R0: Software reset */
    {0x04, 0x0008, 1},    /* R4: Bias control */
    {0x05, 0x0047, 5},    /* R5: VMID fast start */
    {0x05, 0x0043, 1},    /* R5: VMID normal operation */
    {0x04, 0x0009, 1},    /* R4: BIAS enable */
    {0x0E, 0x0003, 1},    /* R14: Headphone PGAs */
    {0x21, 0x0000, 1},    /* R33: DAC unmute */
    {0x3D, 0x0000, 1},    /* R61: Analog output configuration */
    {0x62, 0x0001, 1},    /* R98: Charge pump enable */
    {0x68, 0x0001, 1},    /* R104: Class-W dynamic output control */
    {0x7E, 0x000A, 1},    /* R126: Digital I/O pull configuration */
    {0x06, 0x0001, 1},    /* R6: Microphone bias enable */
    {0x19, 0x0002, 1},    /* R25: Audio interface (I2S, 16-bit) */
    {0x12, 0x000C, 5},    /* R18: DACs enable */
    {0x5A, 0x0011, 1},    /* R90: Headphone output enable (stage 1) */
    {0x5A, 0x0033, 1},    /* R90: Headphone output enable (stage 2) */
    {0x43, 0x000F, 1},    /* R67: DC servo enable */
    {0x44, 0x00F0, 100},  /* R68: DC servo startup (wait 100ms) */
    {0x5A, 0x0077, 1},    /* R90: Headphone output enable (stage 3) */
    {0x5A, 0x00FF, 1},    /* R90: Headphone output fully enabled */
    {0x1E, 0x00B9, 1},    /* R30: DAC digital volume L */
    {0x1F, 0x00B9, 1},    /* R31: DAC digital volume R */
    {0x1E, 0x01B9, 1},    /* R30: Volume update bit set */
    {0x1F, 0x01B9, 5},    /* R31: Volume update bit set */
    {0x39, 0x0039, 1},    /* R57: Headphone analog volume L */
    {0x3A, 0x0039, 1},    /* R58: Headphone analog volume R */
    {0x39, 0x00B9, 1},    /* R57: Volume update bit set */
    {0x3A, 0x00B9, 100},  /* R58: Volume update bit set (wait 100ms) */
    {0x12, 0x000F, 1},    /* R18: DACs + ADCs enable */
    {0x2C, 0x0080, 1},    /* R44: ADC input mute L */
    {0x2D, 0x0080, 1},    /* R45: ADC input mute R */
    {0x0C, 0x0003, 1},    /* R12: Input PGAs enable */
    {0x18, 0x0050, 100},  /* R24: Clock configuration (wait 100ms) */
    {0x75, 0x0700, 1},    /* R117: FLL Control 2 (OUTDIV=8, FRATIO=1) */
    {0x76, 0x0000, 1},    /* R118: FLL Control 3 (K=0, integer mode) */
    {0x77, 0x0800, 1},    /* R119: FLL Control 4 (N=64, GAIN=0) */
    {0x78, 0x0001, 1},    /* R120: FLL Control 5 (ref=BCLK) */
    {0x74, 0x0002, 1},    /* R116: FLL oscillator enable */
    {0x74, 0x0003, 5},    /* R116: FLL enable, wait for lock */
    {0x16, 0x4006, 5},    /* R22: SYSCLK from FLL, enable system/DSP clocks */
    {0x1E, 0x009E, 1},    /* R30: DAC digital volume L (adjusted) */
    {0x1F, 0x009E, 1},    /* R31: DAC digital volume R (adjusted) */
    {0x1E, 0x019E, 1},    /* R30: Volume update */
    {0x1F, 0x019E, 5},    /* R31: Volume update */
    {0x21, 0x0600, 5},    /* R33: DAC soft mute/unmute ramp */
    {0x2C, 0x001C, 1},    /* R44: ADC input unmute L, enable input */
    {0x2D, 0x001C, 1},    /* R45: ADC input unmute R, enable input */
};


#define WM8904_48K_16BIT_COUNT  (sizeof(wm8904_48k_16bit) / sizeof(wm8904_48k_16bit[0]))

// *****************************************************************************
// *****************************************************************************
// Section: Type Definitions
// *****************************************************************************
// *****************************************************************************

// *****************************************************************************
/* Application states

  Summary:
    Application states enumeration

  Description:
    This enumeration defines the valid application states.  These states
    determine the behavior of the application at various times.
*/

typedef enum
{
    APP_AUDIO_STATE_INIT=0,            /* Initialize I2C driver for codec access */
    APP_AUDIO_STATE_CONFIGURE_CODEC,   /* Verify WM8904 ID and configure registers */
    APP_AUDIO_STATE_CONFIGURE_I2S,     /* Configure SPI_IXS0 for I2S master mode */
    APP_AUDIO_STATE_STREAMING,         /* Active audio streaming via DMA */
    APP_AUDIO_STATE_ERROR,             /* Error state (codec or I2S failure) */
    APP_AUDIO_STATE_IDLE               /* Idle state */

} APP_AUDIO_STATES;


// *****************************************************************************
/* Application Data

  Summary:
    Holds application data

  Description:
    This structure holds the application's data.

  Remarks:
    Application strings and buffers are be defined outside this structure.
 */

typedef struct
{
    APP_AUDIO_STATES state;                        /* Current state machine state */

    DRV_HANDLE i2cHandle;                          /* I2C driver handle for codec communication */

    volatile DRV_I2C_TRANSFER_EVENT transferStatus; /* Transfer status (unused in polling mode) */

    uint8_t txBuffer[4];                           /* I2C transmit buffer (reg addr + 16-bit data) */
    uint8_t rxBuffer[4];                           /* I2C receive buffer (for ID verification) */

} APP_AUDIO_DATA;

// *****************************************************************************
// *****************************************************************************
// Section: Application Callback Routines
// *****************************************************************************
// *****************************************************************************
/* These routines are called by drivers when certain events occur.
*/

// *****************************************************************************
// *****************************************************************************
// Section: Application Initialization and State Machine Functions
// *****************************************************************************
// *****************************************************************************

/*******************************************************************************
  Function:
    void APP_AUDIO_Initialize ( void )

  Summary:
     MPLAB Harmony application initialization routine.

  Description:
    This function initializes the Harmony application.  It places the
    application in its initial state and prepares it to run so that its
    APP_AUDIO_Tasks function can be called.

  Precondition:
    All other system initialization routines should be called before calling
    this routine (in "SYS_Initialize").

  Parameters:
    None.

  Returns:
    None.

  Example:
    <code>
    APP_AUDIO_Initialize();
    </code>

  Remarks:
    This routine must be called from the SYS_Initialize function.
*/

void APP_AUDIO_Initialize ( void );


/*******************************************************************************
  Function:
    void APP_AUDIO_Tasks ( void )

  Summary:
    MPLAB Harmony Demo application tasks function

  Description:
    This routine is the Harmony Demo application's tasks function.  It
    defines the application's state machine and core logic.

  Precondition:
    The system and application initialization ("SYS_Initialize") should be
    called before calling this.

  Parameters:
    None.

  Returns:
    None.

  Example:
    <code>
    APP_AUDIO_Tasks();
    </code>

  Remarks:
    This routine must be called from SYS_Tasks() routine.
 */

void APP_AUDIO_Tasks( void );

//DOM-IGNORE-BEGIN
#ifdef __cplusplus
}
#endif
//DOM-IGNORE-END

#endif /* _APP_AUDIO_H */

/*******************************************************************************
 End of File
 */

