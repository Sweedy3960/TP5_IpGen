/*******************************************************************************
  MPLAB Harmony Application Source File
  
  Company:
    Microchip Technology Inc.
  
  File Name:
    appgen.c

  Summary:
    This file contains the source code for the MPLAB Harmony application.

  Description:
    This file contains the source code for the MPLAB Harmony application.  It 
    implements the logic of the application's state machine and it may call 
    API routines of other MPLAB Harmony modules in the system, such as drivers,
    system services, and middleware.  However, it does not call any of the
    system interfaces (such as the "Initialize" and "Tasks" functions) of any of
    the modules in the system or make any assumptions about when those functions
    are called.  That is the responsibility of the configuration-specific system
    files.
 *******************************************************************************/

// DOM-IGNORE-BEGIN
/*******************************************************************************
Copyright (c) 2013-2014 released Microchip Technology Inc.  All rights reserved.

Microchip licenses to you the right to use, modify, copy and distribute
Software only when embedded on a Microchip microcontroller or digital signal
controller that is integrated into your product or third party product
(pursuant to the sublicense terms in the accompanying license agreement).

You should refer to the license agreement accompanying this Software for
additional information regarding your rights and obligations.

SOFTWARE AND DOCUMENTATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY KIND,
EITHER EXPRESS OR IMPLIED, INCLUDING WITHOUT LIMITATION, ANY WARRANTY OF
MERCHANTABILITY, TITLE, NON-INFRINGEMENT AND FITNESS FOR A PARTICULAR PURPOSE.
IN NO EVENT SHALL MICROCHIP OR ITS LICENSORS BE LIABLE OR OBLIGATED UNDER
CONTRACT, NEGLIGENCE, STRICT LIABILITY, CONTRIBUTION, BREACH OF WARRANTY, OR
OTHER LEGAL EQUITABLE THEORY ANY DIRECT OR INDIRECT DAMAGES OR EXPENSES
INCLUDING BUT NOT LIMITED TO ANY INCIDENTAL, SPECIAL, INDIRECT, PUNITIVE OR
CONSEQUENTIAL DAMAGES, LOST PROFITS OR LOST DATA, COST OF PROCUREMENT OF
SUBSTITUTE GOODS, TECHNOLOGY, SERVICES, OR ANY CLAIMS BY THIRD PARTIES
(INCLUDING BUT NOT LIMITED TO ANY DEFENSE THEREOF), OR OTHER SIMILAR COSTS.
 *******************************************************************************/
// DOM-IGNORE-END


// *****************************************************************************
// *****************************************************************************
// Section: Included Files 
// *****************************************************************************
// *****************************************************************************

#include "appgen.h"
#include "Mc32DriverLcd.h"
#include <stdbool.h>          // Permet l'utilisation du type bool
#include "app.h"             // Fichier principal de l'application (appData, etc.)
#include "Mc32DriverLcd.h"   // Gestion de l'affichage LCD
#include "Mc32gestSpiDac.h"  // Gestion SPI du DAC LTC2604
#include "MenuGen.h"         // Gestion du menu g?n?rique
#include "GesPec12.h"        // Gestion du codeur rotatif PEC12
#include "Generateur.h"      // Gestion du g?n?rateur de signal
#include "system_definitions.h"
#include "driver/tmr/drv_tmr_static.h"
// *****************************************************************************
// *****************************************************************************
// Section: Global Data Definitions
// *****************************************************************************
// *****************************************************************************

APPGEN_DATA appgenData; // Structure globale contenant l'?tat de l'application
S_ParamGen LocalParamGen; // Structure locale pour les param?tres du g?n?rateur
S_ParamGen RemoteParamGen;
// *****************************************************************************
// *****************************************************************************
// Section: Application Callback Functions
// *****************************************************************************
// *****************************************************************************
/**
 * @brief Callback Timer1 (1 ms). G?re des actions p?riodiques, notamment ScanBtn.
 * 
 * Appel? toutes les 1 ms, ce callback va inverser LED1_W, compter un d?lai
 * d'initialisation et g?rer la transition d'?tat de l'application.
 */
void App_Timer1Callback() {
    LED1_W = !LED1_R; // Inverse l'?tat de LED1_W en se basant sur LED1_R

    // Compteur pour les 3 premi?res secondes
    static uint16_t WaitIteration = 0; // Variable statique qui conserve sa valeur entre appels
    static uint8_t InitDone = 0; // Flag pour indiquer si l'init est termin?e

    // Lecture des signaux du codeur PEC12 et du bouton S9 (S_OK)
    ScanBtn(PEC12_A, PEC12_B, PEC12_PB, S_OK);

    // Pendant les 3 premi?res secondes, on incr?mente WaitIteration
    if ((WaitIteration <= WAIT_INIT) && (InitDone == 0)) {
        WaitIteration++; // Incr?mente le compteur d'attente
    } else {
        // Si on est toujours dans l'?tat d'attente d'init (APPGEN_INIT_WAIT)
        if (appgenData.state == APPGEN_STATE_INIT_WAIT) {
            APPGEN_UpdateState(APPGEN_STATE_INIT_CLEAR); // Change l'?tat de l'application
            WaitIteration = 0; // R?initialise le compteur
            InitDone = 1; // Note que l'init est termin?e
        } else {
            // Une fois l'init termin?e, on ex?cute p?riodiquement le SERVICE_TASKS
            if (WaitIteration >= 10) {
                WaitIteration = 0; // Reset du compteur
                APPGEN_UpdateState(APPGEN_STATE_SERVICE_TASKS); // Demande ex?cution des t?ches
            } else {
                WaitIteration++; // Incr?mente jusqu'? 10 pour la prochaine ex?cution
            }
        }
    }
}

/**
 * @brief Callback Timer3. G?re l'ex?cution du g?n?rateur de signal.
 * 
 * Appel? p?riodiquement, ce callback allume LED0, ex?cute la g?n?ration de signal,
 * puis ?teint LED0.
 */
void App_Timer3Callback() { // Force LED0 ? l'?tat bas (active)
    BSP_LEDOn(BSP_LED_0);
    GENSIG_Execute(); // G?n?re le signal selon les param?tres en cours
    BSP_LEDOff(BSP_LED_0);
}


// *****************************************************************************
// *****************************************************************************
// Section: Application Local Functions
// *****************************************************************************
// *****************************************************************************
/**
 * @brief Nettoie l'?cran LCD en effa?ant chacune des lignes 1 ? 4.
 * 
 * @details Cette fonction appelle `lcd_ClearLine()` pour les 4 lignes du LCD,
 *          afin de supprimer tout affichage courant avant de r??crire des
 *          informations.
 */
void ClearLcd() {
    lcd_ClearLine(1); // Efface la ligne 1
    lcd_ClearLine(2); // Efface la ligne 2
    lcd_ClearLine(3); // Efface la ligne 3
    lcd_ClearLine(4); // Efface la ligne 4
}
/**
 * @brief Met ? jour l'?tat de l'application.
 * 
 * @param NewState Le nouvel ?tat ? appliquer ? l'application.
 * 
 * @details Cette fonction permet de forcer le passage d'un ?tat ? un autre
 *          depuis d'autres parties du code (ex: Timer1Callback).
 */
void APPGEN_UpdateState(APPGEN_STATES NewState) {
    appgenData.state = NewState; // Affecte le nouvel ?tat
}
// *****************************************************************************
// *****************************************************************************
// Section: Application Initialization and State Machine Functions
// *****************************************************************************
// *****************************************************************************

/*******************************************************************************
  Function:
    void APPGEN_Initialize ( void )

  Remarks:
    See prototype in appgen.h.
 */

void APPGEN_Initialize ( void )
{
    /* Place the App state machine in its initial state. */
    appgenData.state = APPGEN_STATE_INIT;

    
    /* TODO: Initialize your application's state machine and other
     * parameters.
     */
}


/******************************************************************************
  Function:
    void APPGEN_Tasks ( void )

  Remarks:
    See prototype in appgen.h.
 */

void APPGEN_Tasks(void) {
    // V?rifie l'?tat courant de l'application
    switch (appgenData.state) {
        case APPGEN_STATE_INIT:
        {
            // Initialisation du LCD
            lcd_init();
            lcd_bl_on();

            // Initialisation du SPI pour le DAC
            SPI_InitLTC2604();

            // Initialisation du codeur PEC12
            Pec12Init();

            // Initialisation du menu
            MENU_Initialize(&LocalParamGen);

            // Initialisation du g?n?rateur
            GENSIG_Initialize(&LocalParamGen);

            // Affichage initial sur l'?cran LCD
            lcd_gotoxy(1, 1);
            printf_lcd("TP2 USART 2024-25");

            lcd_gotoxy(1, 2);
            printf_lcd("acl");

            lcd_gotoxy(1, 3);
            printf_lcd("tasiiiiiiilllooooo");

            // D?marre les timers TMR0 et TMR1
            DRV_TMR0_Initialize();
            DRV_TMR1_Initialize();
            DRV_TMR0_Start();
            DRV_TMR1_Start();

            // Passe ? l'?tat d'attente init
            appgenData.state = APPGEN_STATE_INIT_WAIT;
            break;
        }

        case APPGEN_STATE_INIT_WAIT:
            // Rien ? faire de particulier ici, tout est g?r? par le callback Timer1
            break;

        case APPGEN_STATE_INIT_CLEAR:
            // Efface l'?cran LCD une fois l'init termin?e (apr?s 3s)
            ClearLcd();
            // Puis passe ? l'?tat d'attente
            appgenData.state = APPGEN_STATE_WAIT;
            break;

        case APPGEN_STATE_WAIT:
            // Etat d'attente : on ne fait rien tant qu'on n'a pas ?t? relanc? par Timer1
            break;

        case APPGEN_STATE_SERVICE_TASKS:
            // Bascule une LED (LED_2) pour indiquer un cycle de service
            BSP_LEDToggle(BSP_LED_2);

            // Ex?cute le menu
            MENU_Execute(&LocalParamGen);

            // Une fois fait, repasse en mode attente
            appgenData.state = APPGEN_STATE_WAIT;
            break;

        default:
        {
            // Etat par d?faut (devrait ne jamais arriver).
            // On peut ?ventuellement y g?rer une erreur syst?me.
            break;
        }
    }
}

 

/*******************************************************************************
 End of File
 */
