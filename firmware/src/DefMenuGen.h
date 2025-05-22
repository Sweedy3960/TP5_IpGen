// Ecole sup�rieure SL229_MINF TP
// Manipulation Tp3_MenuGen
// Cr�� le 9 Mai 2006 CHR
// Version 2016  du 03.02.2016
// Modif 2015 : utilisation de stdint.h
// Modif 2016 : ajout Duty pour PWM
// Modif 2018 SCA : suppression PWM et duty
// Definition pour le menuGen

#ifndef DEFMENUGEN_H
#define DEFMENUGEN_H

// Inclusion des types standards entiers (uint32_t, int16_t, ...)
#include <stdint.h>

// Constantes symboliques utilis�es pour le traitement du menu
#define MAGIC 0x123455AA   // Valeur constante pour v�rifier la validit� de la sauvegarde en m�moire
#define SELECT 0           // �tat "S�lection" dans le menu
#define EDIT 1             // �tat indiquant que l'utilisateur modifie une valeur

/**
 * @name E_FormesSignal
 * @brief �num�ration des types de formes de signal disponibles dans le g�n�rateur.
 */
typedef enum  { 
    SignalSinus,       // Signal sinusoidal
    SignalTriangle,    // Signal triangulaire
    SignalDentDeScie,  // Signal dent de scie
    SignalCarre        // Signal carr�
} E_FormesSignal;

/**
 * @name S_ParamGen
 * @brief Structure regroupant les param�tres du g�n�rateur de signal.
 */
typedef struct {
    E_FormesSignal Forme;  // Type de forme du signal
    int16_t Frequence;     // Fr�quence du signal en [Hz] (entre 20 et 2000)
    int16_t Amplitude;     // Amplitude cr�te du signal en [mV] (0 � +10000)
    int16_t Offset;        // D�calage en tension (offset) en [mV] (-5000 � +5000)
    uint32_t Magic;        // Valeur magique permettant de contr�ler la validit� des param�tres sauvegard�s
} S_ParamGen;

/**
 * @name MenuState_t
 * @brief �tats possibles de la machine � �tats pour la gestion du menu du g�n�rateur
 */
typedef enum {
    MENU_INIT = 0,          // �tat d'initialisation du menu
    MENU_FORME_SEL = 1,     // S�lection du type de forme du signal
    MENU_FREQ_SEL = 2,      // S�lection de la fr�quence
    MENU_AMPL_SEL = 3,      // S�lection de l'amplitude
    MENU_OFFSET_SEL = 4,    // S�lection de l'offset
    MENU_FORME_EDIT = 5,    // Modification du type de forme
    MENU_FREQ_EDIT = 6,     // Modification de la fr�quence
    MENU_AMPL_EDIT = 7,     // Modification de l'amplitude
    MENU_OFFSET_EDIT = 8,   // Modification de l'offset
    MENU_SAUVEGARDE = 9,    // Confirmation de sauvegarde en m�moire
    MENU_SAVEINFO = 10      // Affichage r�sultat de la sauvegarde
} MenuState_t;

#endif

