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
void MENU_Initialize(S_ParamGen *pParam) {
    // (Ligne vide, aucune logique pour le moment)
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
void MENU_Execute(S_ParamGen *pParam) {
    static MenuState_t menu = MENU_INIT; // �tat courant du menu, initialis� � MENU_INIT
    static uint8_t saveOk = 0; // Flag indiquant si la sauvegarde est valid�e (1) ou annul�e (0)
    static uint8_t RefreshMenu = 0; // Flag pour redessiner le menu
    static uint8_t wait2s = 0; // Compteur pour g�rer l'affichage temporaire (ex: 2 secondes)

    // Machine � �tats du menu
    switch (menu) {
        case MENU_INIT: // �tat d'initialisation
            NVM_ReadBlock((uint32_t*) & pParamSave, sizeof (S_ParamGen)); // Lecture des param�tres en NVM
            // Test si la valeur Magic est correcte (v�rifie l'int�grit�)
            if (pParamSave.Magic == MAGIC) {
                // Si valide, on r�cup�re les valeurs sauvegard�es
                *pParam = pParamSave;
            } else {
                // Sinon, on initialise les param�tres par d�faut
                pParam->Amplitude = 0;
                pParam->Forme = SignalSinus;
                pParam->Frequence = 20;
                pParam->Magic = MAGIC;
                pParam->Offset = 0;
            }
            GENSIG_UpdatePeriode(pParam);
            GENSIG_UpdateSignal(pParam);
            MENU_Display(pParam, MENU_FORME_SEL); // Affiche le menu initial (forme s�lectionn�e)
            menu = MENU_FORME_SEL; // Passe � l'�tat MENU_FORME_SEL
            break;

            // -------------------------------------------------------------------
        case MENU_FORME_SEL: // �tat de s�lection de la forme d'onde
            if (RefreshMenu == 1) {
                RefreshMenu = 0; // R�initialise le flag
                MENU_Display(pParam, MENU_FORME_SEL); // Redessine l'�cran avec le nouvel �tat
            }
            if (Pec12IsPlus()) { // Si rotation du codeur (incr�ment)
                menu = MENU_FREQ_SEL; // Passe � la s�lection de la fr�quence
                RefreshMenu = 1; // Signale la n�cessit� de rafra�chir l'affichage
                Pec12ClearPlus(); // R�initialise l'�v�nement d'incr�ment
            }
            if (Pec12IsMinus()) { // Si rotation du codeur (d�cr�ment)
                menu = MENU_OFFSET_SEL; // Passe � la s�lection de l'offset
                RefreshMenu = 1; // Besoin de rafra�chir l'affichage
                Pec12ClearMinus(); // R�initialise l'�v�nement de d�cr�ment
            }
            if (Pec12IsOK()) { // Si appui court sur le codeur
                menu = MENU_FORME_EDIT; // Passe en mode �dition de la forme
                pParamSave = *pParam; // Sauvegarde temporaire du param�tre actuel
                RefreshMenu = 1; // Besoin de rafra�chir l'affichage
                Pec12ClearOK(); // R�initialise l'�v�nement OK
            }
            if (S9IsOK()) { // Si appui sur le bouton S9 (OK)
                menu = MENU_SAUVEGARDE; // Passe directement au menu de sauvegarde
                RefreshMenu = 1; // Besoin de rafra�chir l'affichage
                S9ClearOK(); // R�initialise l'�v�nement OK de S9
            }
            break;

            // -------------------------------------------------------------------
        case MENU_FORME_EDIT: // �tat d'�dition de la forme d'onde
            if (RefreshMenu == 1) {
                RefreshMenu = 0; // R�initialise le flag
                MENU_Display(pParam, MENU_FORME_EDIT); // Redessine l'�cran
            }
            if (Pec12IsPlus()) { // Si rotation incr�ment
                pParam->Forme = (pParam->Forme + 1) % 4; // Passe � la forme suivante (en mod 4)
                RefreshMenu = 1; // Besoin d'un rafra�chissement
                Pec12ClearPlus(); // R�initialise l'�v�nement
            }
            if (Pec12IsMinus()) { // Si rotation d�cr�ment
                pParam->Forme = (pParam->Forme - 1 + 4) % 4; // Passe � la forme pr�c�dente (en mod 4)
                RefreshMenu = 1; // Besoin de rafra�chir
                Pec12ClearMinus(); // R�initialise l'�v�nement
            }
            if (Pec12IsOK()) { // Si appui court (validation)
                menu = MENU_FORME_SEL; // Retourne � la s�lection
                GENSIG_UpdateSignal(pParam);
                RefreshMenu = 1; // Besoin de rafra�chir
                Pec12ClearOK(); // R�initialise l'�v�nement
            }
            if (Pec12IsESC()) { // Si appui long (annulation)
                menu = MENU_FORME_SEL; // Retourne � la s�lection
                pParam->Forme = pParamSave.Forme; // Restaure la forme pr�c�dente
                RefreshMenu = 1; // Besoin de rafra�chir
                Pec12ClearESC(); // R�initialise l'�v�nement
            }
            break;

            // -------------------------------------------------------------------
        case MENU_FREQ_SEL: // �tat de s�lection de la fr�quence
            if (RefreshMenu == 1) {
                RefreshMenu = 0; // R�initialise le flag
                MENU_Display(pParam, MENU_FREQ_SEL); // Redessine l'�cran
            }
            if (Pec12IsPlus()) { // Rotation incr�ment
                menu = MENU_AMPL_SEL; // Passe � la s�lection de l'amplitude
                RefreshMenu = 1; // Besoin de rafra�chir
                Pec12ClearPlus(); // R�initialise l'�v�nement
            }
            if (Pec12IsMinus()) { // Rotation d�cr�ment
                menu = MENU_FORME_SEL; // Retourne � la s�lection de la forme
                RefreshMenu = 1; // Besoin de rafra�chir
                Pec12ClearMinus(); // R�initialise l'�v�nement
            }
            if (Pec12IsOK()) { // Appui court
                menu = MENU_FREQ_EDIT; // Passe en �dition de la fr�quence
                pParamSave = *pParam; // Sauvegarde temporaire
                RefreshMenu = 1; // Besoin de rafra�chir
                Pec12ClearOK(); // R�initialise l'�v�nement
            }
            if (S9IsOK()) { // Appui sur le bouton S9
                menu = MENU_SAUVEGARDE; // Passe au menu de sauvegarde
                RefreshMenu = 1; // Besoin de rafra�chir
                S9ClearOK(); // R�initialise l'�v�nement
            }
            break;

            // -------------------------------------------------------------------
        case MENU_FREQ_EDIT: // �tat d'�dition de la fr�quence
            if (RefreshMenu == 1) {
                RefreshMenu = 0; // R�initialise le flag
                MENU_Display(pParam, MENU_FREQ_EDIT); // Redessine l'�cran
            }
            if (Pec12IsPlus()) { // Rotation incr�ment
                pParam->Frequence += 20; // Incr�mente la fr�quence de 20 Hz
                if (pParam->Frequence > 2000) { // Si d�passe 2000 Hz
                    pParam->Frequence = 20; // Reboucle � 20 Hz
                }
                RefreshMenu = 1; // Besoin de rafra�chir
                Pec12ClearPlus(); // R�initialise l'�v�nement
            }
            if (Pec12IsMinus()) { // Rotation d�cr�ment
                pParam->Frequence -= 20; // D�cr�mente la fr�quence de 20 Hz
                if (pParam->Frequence < 20) { // Si descend en dessous de 20 Hz
                    pParam->Frequence = 2000; // Reboucle � 2000 Hz
                }
                RefreshMenu = 1; // Besoin de rafra�chir
                Pec12ClearMinus(); // R�initialise l'�v�nement
            }
            if (Pec12IsOK()) { // Appui court (validation)
                menu = MENU_FREQ_SEL; // Retourne � la s�lection
                GENSIG_UpdatePeriode(pParam);
                GENSIG_UpdateSignal(pParam);
                RefreshMenu = 1; // Besoin de rafra�chir
                Pec12ClearOK(); // R�initialise
            }
            if (Pec12IsESC()) { // Appui long (annulation)
                menu = MENU_FREQ_SEL; // Retourne � la s�lection
                pParam->Frequence = pParamSave.Frequence; // Restaure la valeur pr�c�dente
                RefreshMenu = 1; // Besoin de rafra�chir
                Pec12ClearESC(); // R�initialise
            }
            break;

            // -------------------------------------------------------------------
        case MENU_AMPL_SEL: // �tat de s�lection de l'amplitude
            if (RefreshMenu == 1) {
                RefreshMenu = 0; // R�initialise le flag
                MENU_Display(pParam, MENU_AMPL_SEL); // Redessine l'�cran
            }
            if (Pec12IsPlus()) { // Rotation incr�ment
                menu = MENU_OFFSET_SEL; // Passe � la s�lection de l'offset
                RefreshMenu = 1; // Besoin de rafra�chir
                Pec12ClearPlus(); // R�initialise
            }
            if (Pec12IsMinus()) { // Rotation d�cr�ment
                menu = MENU_FREQ_SEL; // Retourne � la s�lection de la fr�quence
                RefreshMenu = 1; // Besoin de rafra�chir
                Pec12ClearMinus(); // R�initialise
            }
            if (Pec12IsOK()) { // Appui court
                menu = MENU_AMPL_EDIT; // Passe en �dition de l'amplitude
                pParamSave = *pParam; // Sauvegarde temporaire
                RefreshMenu = 1; // Besoin de rafra�chir
                Pec12ClearOK(); // R�initialise
            }
            if (S9IsOK()) { // Appui sur S9
                menu = MENU_SAUVEGARDE; // Passe au menu de sauvegarde
                RefreshMenu = 1; // Besoin de rafra�chir
                S9ClearOK(); // R�initialise
            }
            break;

            // -------------------------------------------------------------------
        case MENU_AMPL_EDIT: // �tat d'�dition de l'amplitude
            if (RefreshMenu == 1) {
                RefreshMenu = 0; // R�initialise le flag
                MENU_Display(pParam, MENU_AMPL_EDIT); // Redessine l'�cran
            }
            if (Pec12IsPlus()) { // Rotation incr�ment
                pParam->Amplitude += 100; // Incr�mente de 100 mV
                if (pParam->Amplitude > 10000) { // Si d�passe 10 000 mV
                    pParam->Amplitude = 0; // Reboucle � 0
                }
                RefreshMenu = 1; // Besoin de rafra�chir
                Pec12ClearPlus(); // R�initialise
            }
            if (Pec12IsMinus()) { // Rotation d�cr�ment
                pParam->Amplitude -= 100; // D�cr�mente de 100 mV
                if (pParam->Amplitude < 0) { // Si on passe en dessous de 0
                    pParam->Amplitude = 10000; // Reboucle � 10 000 mV
                }
                RefreshMenu = 1; // Besoin de rafra�chir
                Pec12ClearMinus(); // R�initialise
            }
            if (Pec12IsOK()) { // Appui court (validation)
                menu = MENU_AMPL_SEL; // Retour au mode s�lection
                GENSIG_UpdateSignal(pParam);
                RefreshMenu = 1; // Besoin de rafra�chir
                Pec12ClearOK(); // R�initialise
            }
            if (Pec12IsESC()) { // Appui long (annulation)
                menu = MENU_AMPL_SEL; // Retour � la s�lection
                pParam->Amplitude = pParamSave.Amplitude; // Restaure la valeur pr�c�dente
                RefreshMenu = 1; // Besoin de rafra�chir
                Pec12ClearESC(); // R�initialise
            }
            break;

            // -------------------------------------------------------------------
        case MENU_OFFSET_SEL: // �tat de s�lection de l'offset
            if (RefreshMenu == 1) {
                RefreshMenu = 0; // R�initialise le flag
                MENU_Display(pParam, MENU_OFFSET_SEL); // Redessine l'�cran
            }
            if (Pec12IsPlus()) { // Rotation incr�ment
                menu = MENU_FORME_SEL; // Passe � la s�lection de la forme
                RefreshMenu = 1; // Besoin de rafra�chir
                Pec12ClearPlus(); // R�initialise
            }
            if (Pec12IsMinus()) { // Rotation d�cr�ment
                menu = MENU_AMPL_SEL; // Retourne � la s�lection de l'amplitude
                RefreshMenu = 1; // Besoin de rafra�chir
                Pec12ClearMinus(); // R�initialise
            }
            if (Pec12IsOK()) { // Appui court
                menu = MENU_OFFSET_EDIT; // Passe en �dition de l'offset
                pParamSave = *pParam; // Sauvegarde temporaire
                RefreshMenu = 1; // Besoin de rafra�chir
                Pec12ClearOK(); // R�initialise
            }
            if (S9IsOK()) { // Appui sur S9
                menu = MENU_SAUVEGARDE; // Passe au menu de sauvegarde
                RefreshMenu = 1; // Besoin de rafra�chir
                S9ClearOK(); // R�initialise
            }
            break;

            // -------------------------------------------------------------------
        case MENU_OFFSET_EDIT: // �tat d'�dition de l'offset
            if (RefreshMenu == 1) {
                RefreshMenu = 0; // R�initialise le flag
                MENU_Display(pParam, MENU_OFFSET_EDIT); // Redessine l'�cran
            }
            if (Pec12IsPlus()) { // Rotation incr�ment
                pParam->Offset += 100; // Incr�mente de 100 mV
                if (pParam->Offset > 5000) {
                    pParam->Offset = 5000; // Bascule du max au min
                }
                RefreshMenu = 1; // Besoin de rafra�chir
                Pec12ClearPlus(); // R�initialise
            }
            if (Pec12IsMinus()) { // Rotation d�cr�ment
                pParam->Offset -= 100; // D�cr�mente de 100 mV
                if (pParam->Offset < -5000) {
                    pParam->Offset = -5000; // Bascule du min au max  
                }
                RefreshMenu = 1; // Besoin de rafra�chir
                Pec12ClearMinus(); // R�initialise
            }
            if (Pec12IsOK()) { // Appui court (validation)
                menu = MENU_OFFSET_SEL; // Retour � la s�lection
                GENSIG_UpdateSignal(pParam);
                RefreshMenu = 1; // Besoin de rafra�chir
                Pec12ClearOK(); // R�initialise
            }
            if (Pec12IsESC()) { // Appui long (annulation)
                menu = MENU_OFFSET_SEL; // Retour � la s�lection
                pParam->Offset = pParamSave.Offset; // Restaure la valeur pr�c�dente
                RefreshMenu = 1; // Besoin de rafra�chir
                Pec12ClearESC(); // R�initialise
            }
            break;

            // -------------------------------------------------------------------
        case MENU_SAUVEGARDE: // �tat proposant la sauvegarde
            if (RefreshMenu == 1) {
                RefreshMenu = 0; // R�initialise le flag
                MENU_Display(pParam, MENU_SAUVEGARDE); // Affiche l'�cran de sauvegarde
            }
            // Si appui long sur S9 (ESC), on valide la sauvegarde
            if (S9IsESC()) {
                menu = MENU_SAVEINFO; // Va afficher le r�sultat
                saveOk = 1; // Indique la sauvegarde valid�e
                RefreshMenu = 1; // Besoin de rafra�chir
                S9ClearESC(); // R�initialise l'�v�nement
                S9ClearOK(); // Au cas o� un OK reste actif
            }                // Sinon, toute autre action (Plus, Minus, OK) annule la sauvegarde
            else if ((Pec12IsPlus()) || (Pec12IsESC()) || (Pec12IsMinus()) || (Pec12IsOK())) {
                menu = MENU_SAVEINFO; // Va afficher le r�sultat
                saveOk = 0; // Indique la sauvegarde annul�e
                RefreshMenu = 1; // Besoin de rafra�chir
                Pec12ClearOK();
                Pec12ClearESC();
                Pec12ClearMinus();
                Pec12ClearPlus();
            }
            break;

            // -------------------------------------------------------------------
        case MENU_SAVEINFO: // �tat d'affichage du r�sultat de la sauvegarde
            if (RefreshMenu == 1) {
                RefreshMenu = 0; // R�initialise le flag
                ClearLcd(); // Efface l'�cran
                if (saveOk == 1) {
                    lcd_gotoxy(2, 3); // Positionne le curseur
                    NVM_WriteBlock((uint32_t*) pParam, sizeof (S_ParamGen)); // �criture en NVM
                    printf_lcd("Sauvegarde OK"); // Indique la r�ussite de la sauvegarde
                } else {
                    lcd_gotoxy(2, 3); // Positionne le curseur
                    printf_lcd("Sauvegarde ANNULEE!"); // Indique l'annulation
                }
            }
            // Incr�mente le compteur de temporisation
            wait2s++;
            // Apr�s 2 secondes (200 x 10 ms), on retourne au menu forme
            if (wait2s == 200) {
                menu = MENU_FORME_SEL; // Retourne � la s�lection de la forme
                RefreshMenu = 1; // Besoin de rafra�chir
            }
            break;
        default:
            // Formes non prise en compte
            break;

    }
}
