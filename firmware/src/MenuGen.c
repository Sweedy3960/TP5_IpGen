// Tp3  manipulation MenuGen avec PEC12
// C. HUBER  10/02/2015 pour SLO2 2014-2015
// Fichier MenuGen.c
// Gestion du menu  du g�n�rateur
// Traitement cyclique � 10 ms

// ================================
// Include standard libraries
// ================================

#include <stdint.h>   // Types entiers standard (uint8_t, etc.)
#include <stdbool.h>  // D�finition du type bool�en

// ================================
// Include project-specific headers
// ================================

#include "appgen.h"           // Gestion de l'�tat de l'application
#include "MenuGen.h"       // D�clarations des fonctions du menu
#include "Mc32DriverLcd.h" // Gestion de l'affichage LCD
#include "GesPec12.h"      // Gestion du codeur rotatif PEC12
#include "Mc32NVMUtil.h"   // Gestion de la m�moire non volatile (NVM)
#include "Generateur.h"      // Gestion du g�n�rateur de signal

// ================================
// Variables globales
// ================================

S_ParamGen pParamSave; // Stores saved parameters (param�tres sauvegard�s)
uint8_t saveUSBRequest;
const char forme[4][21] = {"Sinus", "Triangle", "DentDeScie", "Carre"};
// ================================
// Fonctions
// ================================

/**
 * @brief Initialise le menu et les param�tres du g�n�rateur.
 * @param pParam Pointeur vers la structure contenant les param�tres du g�n�rateur.
 * @return Cette fonction ne renvoie rien.
 * 
 * Remarque : Cette fonction est pr�vue pour initialiser les param�tres du menu.
 *            Actuellement, elle est vide et peut �tre compl�t�e selon les besoins.
 */
void MENU_Initialize(S_ParamGen *pParam, bool local)
{
    char car = (local) ? '*' : '#';
  
    lcd_gotoxy(1, 1);
    printf_lcd("%cForme = %10s", car, forme[pParam->Forme]);

    lcd_gotoxy(1, 2);
    printf_lcd("%cFreq [Hz] = %4d", car, pParam->Frequence);

    lcd_gotoxy(1, 3);
    printf_lcd("%cAmpl [mV] = %5d", car, pParam->Amplitude);

    lcd_gotoxy(1, 4);
    printf_lcd("%cOffset[mV]= %5d", car, pParam->Offset);
}

// -------------------------------------------------------------------

/**
 * @brief Affiche le menu et les param�tres courants sur l'�cran LCD.
 * 
 * @param pParam Pointeur vers la structure de param�tres du g�n�rateur.
 * @param menu Indice ou �tat courant du menu (pour d�terminer l'affichage).
 * @return Cette fonction ne renvoie rien.
 */
void MENU_Display(S_ParamGen *pParam, uint8_t menu) {
    // Tableau de cha�nes d�crivant les formes d'onde possibles
    const char MenuFormes [4] [21] = {"Sinus", "Triangle", "DentDeScie", "Carre"};
    ClearLcd(); // Efface l'affichage LCD

    // V�rifie que l'indice du menu est inf�rieur � 9 (sinon, autre affichage)
    if (menu < 9) {
        lcd_gotoxy(2, 1); // Positionne le curseur LCD � la colonne 2, ligne 1
        printf_lcd("Forme ="); // Affiche le libell� "Forme ="
        lcd_gotoxy(11, 1); // Positionne le curseur LCD � la colonne 11, ligne 1
        printf_lcd("%s", MenuFormes[pParam->Forme]); // Affiche la forme actuelle depuis le tableau

        lcd_gotoxy(2, 2); // Positionne le curseur en colonne 2, ligne 2
        printf_lcd("Freq [Hz]"); // Affiche le libell� de la fr�quence
        lcd_gotoxy(13, 2); // Se positionne en colonne 13, ligne 2
        printf_lcd("%d", pParam->Frequence); // Affiche la fr�quence actuelle

        lcd_gotoxy(2, 3); // Positionne le curseur en colonne 2, ligne 3
        printf_lcd("Ampl [mV]"); // Affiche le libell� de l'amplitude
        lcd_gotoxy(13, 3); // Se positionne en colonne 13, ligne 3
        printf_lcd("%d", pParam->Amplitude); // Affiche l'amplitude actuelle

        lcd_gotoxy(2, 4); // Positionne le curseur en colonne 2, ligne 4
        printf_lcd("Offest [mV]"); // Affiche le libell� de l'offset (note : coquille "Offest")
        lcd_gotoxy(13, 4); // Se positionne en colonne 13, ligne 4
        printf_lcd("%d", (int) pParam->Offset); // Affiche la valeur d'offset

        // Affiche un caract�re sp�cial (* ou ?) selon l'�l�ment de menu s�lectionn�
        if (menu <= 4) {
            lcd_gotoxy(1, menu); // Place le curseur sur la ligne du menu s�lectionn�
            printf_lcd("*"); // Affiche un ast�risque pour indiquer la s�lection
        } else {
            menu = menu - 4; // Ajuste l'indice de menu (pour un second niveau, par ex.)
            lcd_gotoxy(1, menu); // Place le curseur
            printf_lcd("?"); // Affiche un point d'interrogation pour marquer la s�lection
        }
    } else {
        // Si menu >= 9, on propose la sauvegarde
        lcd_gotoxy(2, 2); // Positionne le curseur en colonne 2, ligne 2
        printf_lcd("Sauvegarde ?"); // Affiche le message de sauvegarde
        lcd_gotoxy(2, 3); // Positionne le curseur en colonne 2, ligne 3
        printf_lcd("(appui long)"); // Indique la m�thode d'activation
    }
}

// -------------------------------------------------------------------

/**
 * @brief G�re la logique du menu en fonction de l'�tat actuel et des actions utilisateur.
 * 
 * @param pParam Pointeur vers la structure de param�tres du g�n�rateur.
 * @return Cette fonction ne renvoie rien.
 * 
 * Le menu �volue selon les actions sur le codeur PEC12 et le bouton S9 :
 * - Navigation entre diff�rentes sections (forme, fr�quence, amplitude, offset)
 * - �dition des valeurs (via rotation du codeur)
 * - Sauvegarde ou annulation
 */
void MENU_Execute(S_ParamGen *pParam, bool local)
{
    static uint8_t etat = SELECTFORME;
    static uint8_t ancienEtat = SELECTFORME;
    static uint8_t attenteSave = 0;
    static uint8_t editionMode = 0;
    static char carEdit = '?';
    static char car = '*';
    static S_ParamGen valTmp;
    static uint8_t premiere = 1;
    static uint16_t compteurUSB;
    
    // Si une requ�te USB est en attente, on force l'�tat SAVEUSB
    if (saveUSBRequest == 1)
    {
        saveUSBRequest = 0;
        etat = SAVEUSB;
    }
    
    // Si USB est actif (local == false), on bloque toute interaction SAUF si on est en �tat SAVEUSB
    if (!local && etat != SAVEUSB)
    {
        lcd_gotoxy(1, 1);
        printf_lcd("#Forme = %10s", forme[pParam->Forme]);

        lcd_gotoxy(1, 2);
        printf_lcd("#Freq [Hz] = %4d", pParam->Frequence);

        lcd_gotoxy(1, 3);
        printf_lcd("#Ampl [mV] = %5d", pParam->Amplitude);

        lcd_gotoxy(1, 4);
        printf_lcd("#Offset[mV]= %5d", pParam->Offset);

        // verrouille toute interaction
        Pec12ClearOK();
        Pec12ClearESC();
        Pec12ClearMinus();
        Pec12ClearPlus();
        S9ClearOK();
        S9ClearESC();
        editionMode = 0;
        return;
    }

    // Init : �viter d'effacer l'�cran pendant le SAVEUSB
    if (premiere && etat != SAVEUSB)
    {
        MENU_Initialize(pParam,false);
        valTmp = *pParam;
        premiere = 0;
    }

    // Si on change de ligne s�lectionn�e : efface les anciens symboles
    if ((ancienEtat != etat) && ((ancienEtat % 2 == 1) || (etat % 2 == 1)))
    {
        lcd_gotoxy(1, 1); printf_lcd(" ");
        lcd_gotoxy(1, 2); printf_lcd(" ");
        lcd_gotoxy(1, 3); printf_lcd(" ");
        lcd_gotoxy(1, 4); printf_lcd(" ");
    }
    ancienEtat = etat;

    car = (editionMode) ? carEdit : '*';

    if (!editionMode)
    {
        if (etat != SAVEMODE)
        {
            if (etat != SAVEUSB)
            {
                lcd_gotoxy(2, 1);
                printf_lcd("Forme = %10s", forme[pParam->Forme]);

                lcd_gotoxy(2, 2);
                printf_lcd("Freq [Hz] = %4d", pParam->Frequence);

                lcd_gotoxy(2, 3);
                printf_lcd("Ampl [mV] = %5d", pParam->Amplitude);

                lcd_gotoxy(2, 4);
                printf_lcd("Offset[mV]= %5d", pParam->Offset);
            }

            if (S9IsOK())
            {
                S9ClearOK(); S9ClearESC();
                etat = SAVEMODE;
            }

            if (Pec12IsOK())
            {
                Pec12ClearOK();
                editionMode = 1;
                etat += 1;
                car = carEdit;
            }

            if (Pec12IsPlus())
            {
                Pec12ClearPlus();
                etat = (etat >= SELECTOFFSET) ? SELECTFORME : etat + 2;
            }

            if (Pec12IsMinus())
            {
                Pec12ClearMinus();
                etat = (etat <= SELECTFORME) ? SELECTOFFSET : etat - 2;
            }
        }
    }
    else
    {
        if (etat != SAVEUSB)
        {
            lcd_gotoxy(2, 1);
            printf_lcd("Forme = %10s", forme[valTmp.Forme]);

            lcd_gotoxy(2, 2);
            printf_lcd("Freq [Hz] = %4d", valTmp.Frequence);

            lcd_gotoxy(2, 3);
            printf_lcd("Ampl [mV] = %5d", valTmp.Amplitude);

            lcd_gotoxy(2, 4);
            printf_lcd("Offset[mV]= %5d", valTmp.Offset);
        }

        if (Pec12IsOK())
        {
            Pec12ClearOK();
            *pParam = valTmp;
            etat -= 1;
            editionMode = 0;
            car = '*';
        }

        if (Pec12IsESC())
        {
            Pec12ClearESC();
            valTmp = *pParam;
            etat -= 1;
            editionMode = 0;
            car = '*';
        }

        if (Pec12IsPlus())
        {
            Pec12ClearPlus();
            switch (etat)
            {
                case REGLAGEFORME:
                    valTmp.Forme = (valTmp.Forme + 1) % 4;
                    break;
                case REGLAGEFREQUENCE:
                    valTmp.Frequence = (valTmp.Frequence < FREQUENCE_MAX) ?
                                       valTmp.Frequence + PAS_FREQUENCE : FREQUENCE_MIN;
                    break;
                case REGLAGEAMPLITUDE:
                    valTmp.Amplitude = (valTmp.Amplitude < AMPLITUDE_MAX) ?
                                        valTmp.Amplitude + PAS_AMPLITUDE : AMPLITUDE_MIN;
                    break;
                case REGLAGEOFFSET:
                    if (valTmp.Offset < OFFSET_MAX) valTmp.Offset += PAS_OFFSET;
                    break;
            }
        }

        if (Pec12IsMinus())
        {
            Pec12ClearMinus();
            switch (etat)
            {
                case REGLAGEFORME:
                    valTmp.Forme = (valTmp.Forme > 0) ? valTmp.Forme - 1 : 3;
                    break;
                case REGLAGEFREQUENCE:
                    valTmp.Frequence = (valTmp.Frequence > FREQUENCE_MIN) ?
                                       valTmp.Frequence - PAS_FREQUENCE : FREQUENCE_MAX;
                    break;
                case REGLAGEAMPLITUDE:
                    valTmp.Amplitude = (valTmp.Amplitude > AMPLITUDE_MIN) ?
                                       valTmp.Amplitude - PAS_AMPLITUDE : AMPLITUDE_MAX;
                    break;
                case REGLAGEOFFSET:
                    if (valTmp.Offset > OFFSET_MIN) valTmp.Offset -= PAS_OFFSET;
                    break;
            }
        }
    }

    switch(etat)
    {
        /////       FORME       /////
        case SELECTFORME :
            lcd_gotoxy(1,1);
            printf_lcd("%c", car);
            break;
        case REGLAGEFORME :
            lcd_gotoxy(1,1);
            printf_lcd("%c", car);            
            break;
        /////       FREQUENCE       /////   
        case SELECTFREQUENCE:
            lcd_gotoxy(1,2);
            printf_lcd("%c", car);
            break;
        case REGLAGEFREQUENCE :
            lcd_gotoxy(1,2);
            printf_lcd("%c", car);

            break;
        /////       AMPLITUDE       /////     
        case SELECTAMPLITUDE :
            lcd_gotoxy(1,3);
            printf_lcd("%c", car);
            break;
        case REGLAGEAMPLITUDE :
            lcd_gotoxy(1,3);
            printf_lcd("%c", car);

            break;
        /////       OFFSET       /////     
        case SELECTOFFSET :
            lcd_gotoxy(1,4);
            printf_lcd("%c", car);
            break;    
        case REGLAGEOFFSET :
            lcd_gotoxy(1,4);
            printf_lcd("%c", car);  
            break;

        case SAVEMODE:
            if(S9IsESC())
            {
                attenteSave++;
                pParam->Magic = MAGIC;  
                //I2C_WriteSEEPROM((uint32_t*)pParam, 0x00, sizeof(S_ParamGen));
                //delay_msCt(5);
                lcd_ClearLine(2);
                lcd_ClearLine(3);
                lcd_gotoxy(1,2);
                printf_lcd("    Sauvegarde OK");


            }

            else if(Pec12IsESC() || Pec12IsPlus() || Pec12IsMinus() || S9IsOK())
            {
                attenteSave++;
                lcd_gotoxy(1,2);
                printf_lcd(" Sauvegarde annul�e ");
            }
            if(attenteSave > 0)
            {
                attenteSave++;
                if(attenteSave > 200)
                {
                    MENU_Initialize(pParam,false);
                    attenteSave = 0;
                    etat = SELECTFORME;
                }
            }
            else
            {
                lcd_ClearLine(1);
                lcd_ClearLine(4);
                lcd_gotoxy(1,2);
                printf_lcd("    Sauvegarde ?    ");
                lcd_gotoxy(1,3);
                printf_lcd("    (Appui long)    "); 
            }
            break;
        case SAVEUSB :
            if (compteurUSB == 0)
            {
                MENU_DemandeSave();        // Affiche "Sauvegarde OK"
            }

            compteurUSB++;

            if (compteurUSB >= 1000)       // 1000 � 2 ms = 2 secondes (si appel toutes les 2 ms)
            {
                compteurUSB = 0;
                etat = SELECTFORME;
                MENU_Initialize(pParam,false);
            }
            break;
                

   
        }
}
