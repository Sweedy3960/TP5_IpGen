// Tp3  manipulation MenuGen avec PEC12
// C. HUBER  10/02/2015 pour SLO2 2014-2015
// Fichier MenuGen.c
// Gestion du menu  du générateur
// Traitement cyclique à 10 ms

// ================================
// Include standard libraries
// ================================

#include <stdint.h>   // Types entiers standard (uint8_t, etc.)
#include <stdbool.h>  // Définition du type booléen

// ================================
// Include project-specific headers
// ================================

#include "appgen.h"           // Gestion de l'état de l'application
#include "MenuGen.h"       // Déclarations des fonctions du menu
#include "Mc32DriverLcd.h" // Gestion de l'affichage LCD
#include "GesPec12.h"      // Gestion du codeur rotatif PEC12
#include "Mc32NVMUtil.h"   // Gestion de la mémoire non volatile (NVM)
#include "Generateur.h"      // Gestion du générateur de signal

// ================================
// Variables globales
// ================================

S_ParamGen pParamSave; // Stores saved parameters (paramètres sauvegardés)

// ================================
// Fonctions
// ================================

/**
 * @brief Initialise le menu et les paramètres du générateur.
 * @param pParam Pointeur vers la structure contenant les paramètres du générateur.
 * @return Cette fonction ne renvoie rien.
 * 
 * Remarque : Cette fonction est prévue pour initialiser les paramètres du menu.
 *            Actuellement, elle est vide et peut être complétée selon les besoins.
 */
void MENU_Initialize(S_ParamGen *pParam) {
    // (Ligne vide, aucune logique pour le moment)
}

// -------------------------------------------------------------------

/**
 * @brief Affiche le menu et les paramètres courants sur l'écran LCD.
 * 
 * @param pParam Pointeur vers la structure de paramètres du générateur.
 * @param menu Indice ou état courant du menu (pour déterminer l'affichage).
 * @return Cette fonction ne renvoie rien.
 */
void MENU_Display(S_ParamGen *pParam, uint8_t menu) {
    // Tableau de chaînes décrivant les formes d'onde possibles
    const char MenuFormes [4] [21] = {"Sinus", "Triangle", "DentDeScie", "Carre"};
    ClearLcd(); // Efface l'affichage LCD

    // Vérifie que l'indice du menu est inférieur à 9 (sinon, autre affichage)
    if (menu < 9) {
        lcd_gotoxy(2, 1); // Positionne le curseur LCD à la colonne 2, ligne 1
        printf_lcd("Forme ="); // Affiche le libellé "Forme ="
        lcd_gotoxy(11, 1); // Positionne le curseur LCD à la colonne 11, ligne 1
        printf_lcd("%s", MenuFormes[pParam->Forme]); // Affiche la forme actuelle depuis le tableau

        lcd_gotoxy(2, 2); // Positionne le curseur en colonne 2, ligne 2
        printf_lcd("Freq [Hz]"); // Affiche le libellé de la fréquence
        lcd_gotoxy(13, 2); // Se positionne en colonne 13, ligne 2
        printf_lcd("%d", pParam->Frequence); // Affiche la fréquence actuelle

        lcd_gotoxy(2, 3); // Positionne le curseur en colonne 2, ligne 3
        printf_lcd("Ampl [mV]"); // Affiche le libellé de l'amplitude
        lcd_gotoxy(13, 3); // Se positionne en colonne 13, ligne 3
        printf_lcd("%d", pParam->Amplitude); // Affiche l'amplitude actuelle

        lcd_gotoxy(2, 4); // Positionne le curseur en colonne 2, ligne 4
        printf_lcd("Offest [mV]"); // Affiche le libellé de l'offset (note : coquille "Offest")
        lcd_gotoxy(13, 4); // Se positionne en colonne 13, ligne 4
        printf_lcd("%d", (int) pParam->Offset); // Affiche la valeur d'offset

        // Affiche un caractère spécial (* ou ?) selon l'élément de menu sélectionné
        if (menu <= 4) {
            lcd_gotoxy(1, menu); // Place le curseur sur la ligne du menu sélectionné
            printf_lcd("*"); // Affiche un astérisque pour indiquer la sélection
        } else {
            menu = menu - 4; // Ajuste l'indice de menu (pour un second niveau, par ex.)
            lcd_gotoxy(1, menu); // Place le curseur
            printf_lcd("?"); // Affiche un point d'interrogation pour marquer la sélection
        }
    } else {
        // Si menu >= 9, on propose la sauvegarde
        lcd_gotoxy(2, 2); // Positionne le curseur en colonne 2, ligne 2
        printf_lcd("Sauvegarde ?"); // Affiche le message de sauvegarde
        lcd_gotoxy(2, 3); // Positionne le curseur en colonne 2, ligne 3
        printf_lcd("(appui long)"); // Indique la méthode d'activation
    }
}

// -------------------------------------------------------------------

/**
 * @brief Gère la logique du menu en fonction de l'état actuel et des actions utilisateur.
 * 
 * @param pParam Pointeur vers la structure de paramètres du générateur.
 * @return Cette fonction ne renvoie rien.
 * 
 * Le menu évolue selon les actions sur le codeur PEC12 et le bouton S9 :
 * - Navigation entre différentes sections (forme, fréquence, amplitude, offset)
 * - Édition des valeurs (via rotation du codeur)
 * - Sauvegarde ou annulation
 */
void MENU_Execute(S_ParamGen *pParam) {
    static MenuState_t menu = MENU_INIT; // État courant du menu, initialisé à MENU_INIT
    static uint8_t saveOk = 0; // Flag indiquant si la sauvegarde est validée (1) ou annulée (0)
    static uint8_t RefreshMenu = 0; // Flag pour redessiner le menu
    static uint8_t wait2s = 0; // Compteur pour gérer l'affichage temporaire (ex: 2 secondes)

    // Machine à états du menu
    switch (menu) {
        case MENU_INIT: // État d'initialisation
            NVM_ReadBlock((uint32_t*) & pParamSave, sizeof (S_ParamGen)); // Lecture des paramètres en NVM
            // Test si la valeur Magic est correcte (vérifie l'intégrité)
            if (pParamSave.Magic == MAGIC) {
                // Si valide, on récupère les valeurs sauvegardées
                *pParam = pParamSave;
            } else {
                // Sinon, on initialise les paramètres par défaut
                pParam->Amplitude = 0;
                pParam->Forme = SignalSinus;
                pParam->Frequence = 20;
                pParam->Magic = MAGIC;
                pParam->Offset = 0;
            }
            GENSIG_UpdatePeriode(pParam);
            GENSIG_UpdateSignal(pParam);
            MENU_Display(pParam, MENU_FORME_SEL); // Affiche le menu initial (forme sélectionnée)
            menu = MENU_FORME_SEL; // Passe à l'état MENU_FORME_SEL
            break;

            // -------------------------------------------------------------------
        case MENU_FORME_SEL: // État de sélection de la forme d'onde
            if (RefreshMenu == 1) {
                RefreshMenu = 0; // Réinitialise le flag
                MENU_Display(pParam, MENU_FORME_SEL); // Redessine l'écran avec le nouvel état
            }
            if (Pec12IsPlus()) { // Si rotation du codeur (incrément)
                menu = MENU_FREQ_SEL; // Passe à la sélection de la fréquence
                RefreshMenu = 1; // Signale la nécessité de rafraîchir l'affichage
                Pec12ClearPlus(); // Réinitialise l'événement d'incrément
            }
            if (Pec12IsMinus()) { // Si rotation du codeur (décrément)
                menu = MENU_OFFSET_SEL; // Passe à la sélection de l'offset
                RefreshMenu = 1; // Besoin de rafraîchir l'affichage
                Pec12ClearMinus(); // Réinitialise l'événement de décrément
            }
            if (Pec12IsOK()) { // Si appui court sur le codeur
                menu = MENU_FORME_EDIT; // Passe en mode édition de la forme
                pParamSave = *pParam; // Sauvegarde temporaire du paramètre actuel
                RefreshMenu = 1; // Besoin de rafraîchir l'affichage
                Pec12ClearOK(); // Réinitialise l'événement OK
            }
            if (S9IsOK()) { // Si appui sur le bouton S9 (OK)
                menu = MENU_SAUVEGARDE; // Passe directement au menu de sauvegarde
                RefreshMenu = 1; // Besoin de rafraîchir l'affichage
                S9ClearOK(); // Réinitialise l'événement OK de S9
            }
            break;

            // -------------------------------------------------------------------
        case MENU_FORME_EDIT: // État d'édition de la forme d'onde
            if (RefreshMenu == 1) {
                RefreshMenu = 0; // Réinitialise le flag
                MENU_Display(pParam, MENU_FORME_EDIT); // Redessine l'écran
            }
            if (Pec12IsPlus()) { // Si rotation incrément
                pParam->Forme = (pParam->Forme + 1) % 4; // Passe à la forme suivante (en mod 4)
                RefreshMenu = 1; // Besoin d'un rafraîchissement
                Pec12ClearPlus(); // Réinitialise l'événement
            }
            if (Pec12IsMinus()) { // Si rotation décrément
                pParam->Forme = (pParam->Forme - 1 + 4) % 4; // Passe à la forme précédente (en mod 4)
                RefreshMenu = 1; // Besoin de rafraîchir
                Pec12ClearMinus(); // Réinitialise l'événement
            }
            if (Pec12IsOK()) { // Si appui court (validation)
                menu = MENU_FORME_SEL; // Retourne à la sélection
                GENSIG_UpdateSignal(pParam);
                RefreshMenu = 1; // Besoin de rafraîchir
                Pec12ClearOK(); // Réinitialise l'événement
            }
            if (Pec12IsESC()) { // Si appui long (annulation)
                menu = MENU_FORME_SEL; // Retourne à la sélection
                pParam->Forme = pParamSave.Forme; // Restaure la forme précédente
                RefreshMenu = 1; // Besoin de rafraîchir
                Pec12ClearESC(); // Réinitialise l'événement
            }
            break;

            // -------------------------------------------------------------------
        case MENU_FREQ_SEL: // État de sélection de la fréquence
            if (RefreshMenu == 1) {
                RefreshMenu = 0; // Réinitialise le flag
                MENU_Display(pParam, MENU_FREQ_SEL); // Redessine l'écran
            }
            if (Pec12IsPlus()) { // Rotation incrément
                menu = MENU_AMPL_SEL; // Passe à la sélection de l'amplitude
                RefreshMenu = 1; // Besoin de rafraîchir
                Pec12ClearPlus(); // Réinitialise l'événement
            }
            if (Pec12IsMinus()) { // Rotation décrément
                menu = MENU_FORME_SEL; // Retourne à la sélection de la forme
                RefreshMenu = 1; // Besoin de rafraîchir
                Pec12ClearMinus(); // Réinitialise l'événement
            }
            if (Pec12IsOK()) { // Appui court
                menu = MENU_FREQ_EDIT; // Passe en édition de la fréquence
                pParamSave = *pParam; // Sauvegarde temporaire
                RefreshMenu = 1; // Besoin de rafraîchir
                Pec12ClearOK(); // Réinitialise l'événement
            }
            if (S9IsOK()) { // Appui sur le bouton S9
                menu = MENU_SAUVEGARDE; // Passe au menu de sauvegarde
                RefreshMenu = 1; // Besoin de rafraîchir
                S9ClearOK(); // Réinitialise l'événement
            }
            break;

            // -------------------------------------------------------------------
        case MENU_FREQ_EDIT: // État d'édition de la fréquence
            if (RefreshMenu == 1) {
                RefreshMenu = 0; // Réinitialise le flag
                MENU_Display(pParam, MENU_FREQ_EDIT); // Redessine l'écran
            }
            if (Pec12IsPlus()) { // Rotation incrément
                pParam->Frequence += 20; // Incrémente la fréquence de 20 Hz
                if (pParam->Frequence > 2000) { // Si dépasse 2000 Hz
                    pParam->Frequence = 20; // Reboucle à 20 Hz
                }
                RefreshMenu = 1; // Besoin de rafraîchir
                Pec12ClearPlus(); // Réinitialise l'événement
            }
            if (Pec12IsMinus()) { // Rotation décrément
                pParam->Frequence -= 20; // Décrémente la fréquence de 20 Hz
                if (pParam->Frequence < 20) { // Si descend en dessous de 20 Hz
                    pParam->Frequence = 2000; // Reboucle à 2000 Hz
                }
                RefreshMenu = 1; // Besoin de rafraîchir
                Pec12ClearMinus(); // Réinitialise l'événement
            }
            if (Pec12IsOK()) { // Appui court (validation)
                menu = MENU_FREQ_SEL; // Retourne à la sélection
                GENSIG_UpdatePeriode(pParam);
                GENSIG_UpdateSignal(pParam);
                RefreshMenu = 1; // Besoin de rafraîchir
                Pec12ClearOK(); // Réinitialise
            }
            if (Pec12IsESC()) { // Appui long (annulation)
                menu = MENU_FREQ_SEL; // Retourne à la sélection
                pParam->Frequence = pParamSave.Frequence; // Restaure la valeur précédente
                RefreshMenu = 1; // Besoin de rafraîchir
                Pec12ClearESC(); // Réinitialise
            }
            break;

            // -------------------------------------------------------------------
        case MENU_AMPL_SEL: // État de sélection de l'amplitude
            if (RefreshMenu == 1) {
                RefreshMenu = 0; // Réinitialise le flag
                MENU_Display(pParam, MENU_AMPL_SEL); // Redessine l'écran
            }
            if (Pec12IsPlus()) { // Rotation incrément
                menu = MENU_OFFSET_SEL; // Passe à la sélection de l'offset
                RefreshMenu = 1; // Besoin de rafraîchir
                Pec12ClearPlus(); // Réinitialise
            }
            if (Pec12IsMinus()) { // Rotation décrément
                menu = MENU_FREQ_SEL; // Retourne à la sélection de la fréquence
                RefreshMenu = 1; // Besoin de rafraîchir
                Pec12ClearMinus(); // Réinitialise
            }
            if (Pec12IsOK()) { // Appui court
                menu = MENU_AMPL_EDIT; // Passe en édition de l'amplitude
                pParamSave = *pParam; // Sauvegarde temporaire
                RefreshMenu = 1; // Besoin de rafraîchir
                Pec12ClearOK(); // Réinitialise
            }
            if (S9IsOK()) { // Appui sur S9
                menu = MENU_SAUVEGARDE; // Passe au menu de sauvegarde
                RefreshMenu = 1; // Besoin de rafraîchir
                S9ClearOK(); // Réinitialise
            }
            break;

            // -------------------------------------------------------------------
        case MENU_AMPL_EDIT: // État d'édition de l'amplitude
            if (RefreshMenu == 1) {
                RefreshMenu = 0; // Réinitialise le flag
                MENU_Display(pParam, MENU_AMPL_EDIT); // Redessine l'écran
            }
            if (Pec12IsPlus()) { // Rotation incrément
                pParam->Amplitude += 100; // Incrémente de 100 mV
                if (pParam->Amplitude > 10000) { // Si dépasse 10 000 mV
                    pParam->Amplitude = 0; // Reboucle à 0
                }
                RefreshMenu = 1; // Besoin de rafraîchir
                Pec12ClearPlus(); // Réinitialise
            }
            if (Pec12IsMinus()) { // Rotation décrément
                pParam->Amplitude -= 100; // Décrémente de 100 mV
                if (pParam->Amplitude < 0) { // Si on passe en dessous de 0
                    pParam->Amplitude = 10000; // Reboucle à 10 000 mV
                }
                RefreshMenu = 1; // Besoin de rafraîchir
                Pec12ClearMinus(); // Réinitialise
            }
            if (Pec12IsOK()) { // Appui court (validation)
                menu = MENU_AMPL_SEL; // Retour au mode sélection
                GENSIG_UpdateSignal(pParam);
                RefreshMenu = 1; // Besoin de rafraîchir
                Pec12ClearOK(); // Réinitialise
            }
            if (Pec12IsESC()) { // Appui long (annulation)
                menu = MENU_AMPL_SEL; // Retour à la sélection
                pParam->Amplitude = pParamSave.Amplitude; // Restaure la valeur précédente
                RefreshMenu = 1; // Besoin de rafraîchir
                Pec12ClearESC(); // Réinitialise
            }
            break;

            // -------------------------------------------------------------------
        case MENU_OFFSET_SEL: // État de sélection de l'offset
            if (RefreshMenu == 1) {
                RefreshMenu = 0; // Réinitialise le flag
                MENU_Display(pParam, MENU_OFFSET_SEL); // Redessine l'écran
            }
            if (Pec12IsPlus()) { // Rotation incrément
                menu = MENU_FORME_SEL; // Passe à la sélection de la forme
                RefreshMenu = 1; // Besoin de rafraîchir
                Pec12ClearPlus(); // Réinitialise
            }
            if (Pec12IsMinus()) { // Rotation décrément
                menu = MENU_AMPL_SEL; // Retourne à la sélection de l'amplitude
                RefreshMenu = 1; // Besoin de rafraîchir
                Pec12ClearMinus(); // Réinitialise
            }
            if (Pec12IsOK()) { // Appui court
                menu = MENU_OFFSET_EDIT; // Passe en édition de l'offset
                pParamSave = *pParam; // Sauvegarde temporaire
                RefreshMenu = 1; // Besoin de rafraîchir
                Pec12ClearOK(); // Réinitialise
            }
            if (S9IsOK()) { // Appui sur S9
                menu = MENU_SAUVEGARDE; // Passe au menu de sauvegarde
                RefreshMenu = 1; // Besoin de rafraîchir
                S9ClearOK(); // Réinitialise
            }
            break;

            // -------------------------------------------------------------------
        case MENU_OFFSET_EDIT: // État d'édition de l'offset
            if (RefreshMenu == 1) {
                RefreshMenu = 0; // Réinitialise le flag
                MENU_Display(pParam, MENU_OFFSET_EDIT); // Redessine l'écran
            }
            if (Pec12IsPlus()) { // Rotation incrément
                pParam->Offset += 100; // Incrémente de 100 mV
                if (pParam->Offset > 5000) {
                    pParam->Offset = 5000; // Bascule du max au min
                }
                RefreshMenu = 1; // Besoin de rafraîchir
                Pec12ClearPlus(); // Réinitialise
            }
            if (Pec12IsMinus()) { // Rotation décrément
                pParam->Offset -= 100; // Décrémente de 100 mV
                if (pParam->Offset < -5000) {
                    pParam->Offset = -5000; // Bascule du min au max  
                }
                RefreshMenu = 1; // Besoin de rafraîchir
                Pec12ClearMinus(); // Réinitialise
            }
            if (Pec12IsOK()) { // Appui court (validation)
                menu = MENU_OFFSET_SEL; // Retour à la sélection
                GENSIG_UpdateSignal(pParam);
                RefreshMenu = 1; // Besoin de rafraîchir
                Pec12ClearOK(); // Réinitialise
            }
            if (Pec12IsESC()) { // Appui long (annulation)
                menu = MENU_OFFSET_SEL; // Retour à la sélection
                pParam->Offset = pParamSave.Offset; // Restaure la valeur précédente
                RefreshMenu = 1; // Besoin de rafraîchir
                Pec12ClearESC(); // Réinitialise
            }
            break;

            // -------------------------------------------------------------------
        case MENU_SAUVEGARDE: // État proposant la sauvegarde
            if (RefreshMenu == 1) {
                RefreshMenu = 0; // Réinitialise le flag
                MENU_Display(pParam, MENU_SAUVEGARDE); // Affiche l'écran de sauvegarde
            }
            // Si appui long sur S9 (ESC), on valide la sauvegarde
            if (S9IsESC()) {
                menu = MENU_SAVEINFO; // Va afficher le résultat
                saveOk = 1; // Indique la sauvegarde validée
                RefreshMenu = 1; // Besoin de rafraîchir
                S9ClearESC(); // Réinitialise l'événement
                S9ClearOK(); // Au cas où un OK reste actif
            }                // Sinon, toute autre action (Plus, Minus, OK) annule la sauvegarde
            else if ((Pec12IsPlus()) || (Pec12IsESC()) || (Pec12IsMinus()) || (Pec12IsOK())) {
                menu = MENU_SAVEINFO; // Va afficher le résultat
                saveOk = 0; // Indique la sauvegarde annulée
                RefreshMenu = 1; // Besoin de rafraîchir
                Pec12ClearOK();
                Pec12ClearESC();
                Pec12ClearMinus();
                Pec12ClearPlus();
            }
            break;

            // -------------------------------------------------------------------
        case MENU_SAVEINFO: // État d'affichage du résultat de la sauvegarde
            if (RefreshMenu == 1) {
                RefreshMenu = 0; // Réinitialise le flag
                ClearLcd(); // Efface l'écran
                if (saveOk == 1) {
                    lcd_gotoxy(2, 3); // Positionne le curseur
                    NVM_WriteBlock((uint32_t*) pParam, sizeof (S_ParamGen)); // Écriture en NVM
                    printf_lcd("Sauvegarde OK"); // Indique la réussite de la sauvegarde
                } else {
                    lcd_gotoxy(2, 3); // Positionne le curseur
                    printf_lcd("Sauvegarde ANNULEE!"); // Indique l'annulation
                }
            }
            // Incrémente le compteur de temporisation
            wait2s++;
            // Après 2 secondes (200 x 10 ms), on retourne au menu forme
            if (wait2s == 200) {
                menu = MENU_FORME_SEL; // Retourne à la sélection de la forme
                RefreshMenu = 1; // Besoin de rafraîchir
            }
            break;
        default:
            // Formes non prise en compte
            break;

    }
}
