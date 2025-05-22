// Ecole supérieure SL229_MINF TP
// Manipulation Tp3_MenuGen
// Créé le 9 Mai 2006 CHR
// Version 2016  du 03.02.2016
// Modif 2015 : utilisation de stdint.h
// Modif 2016 : ajout Duty pour PWM
// Modif 2018 SCA : suppression PWM et duty
// Definition pour le menuGen

#ifndef DEFMENUGEN_H
#define DEFMENUGEN_H

// Inclusion des types standards entiers (uint32_t, int16_t, ...)
#include <stdint.h>

// Constantes symboliques utilisées pour le traitement du menu
#define MAGIC 0x123455AA   // Valeur constante pour vérifier la validité de la sauvegarde en mémoire
#define SELECT 0           // État "Sélection" dans le menu
#define EDIT 1             // État indiquant que l'utilisateur modifie une valeur

/**
 * @name E_FormesSignal
 * @brief Énumération des types de formes de signal disponibles dans le générateur.
 */
typedef enum  { 
    SignalSinus,       // Signal sinusoidal
    SignalTriangle,    // Signal triangulaire
    SignalDentDeScie,  // Signal dent de scie
    SignalCarre        // Signal carré
} E_FormesSignal;

/**
 * @name S_ParamGen
 * @brief Structure regroupant les paramètres du générateur de signal.
 */
typedef struct {
    E_FormesSignal Forme;  // Type de forme du signal
    int16_t Frequence;     // Fréquence du signal en [Hz] (entre 20 et 2000)
    int16_t Amplitude;     // Amplitude crête du signal en [mV] (0 à +10000)
    int16_t Offset;        // Décalage en tension (offset) en [mV] (-5000 à +5000)
    uint32_t Magic;        // Valeur magique permettant de contrôler la validité des paramètres sauvegardés
} S_ParamGen;

/**
 * @name MenuState_t
 * @brief États possibles de la machine à états pour la gestion du menu du générateur
 */
typedef enum {
    MENU_INIT = 0,          // État d'initialisation du menu
    MENU_FORME_SEL = 1,     // Sélection du type de forme du signal
    MENU_FREQ_SEL = 2,      // Sélection de la fréquence
    MENU_AMPL_SEL = 3,      // Sélection de l'amplitude
    MENU_OFFSET_SEL = 4,    // Sélection de l'offset
    MENU_FORME_EDIT = 5,    // Modification du type de forme
    MENU_FREQ_EDIT = 6,     // Modification de la fréquence
    MENU_AMPL_EDIT = 7,     // Modification de l'amplitude
    MENU_OFFSET_EDIT = 8,   // Modification de l'offset
    MENU_SAUVEGARDE = 9,    // Confirmation de sauvegarde en mémoire
    MENU_SAVEINFO = 10      // Affichage résultat de la sauvegarde
} MenuState_t;

#endif

