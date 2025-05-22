#ifndef GesPEc12_h
#define GesPEc12_h

// GesPec12.h
// C. HUBER    14/05/2014
// Création pour migration sur PIC32MX

/**
 * @file GesPec12.h
 * @brief Interface de gestion du codeur incrémental PEC12 et bouton associé (gestion anti-rebond incluse).
 *
 * Ces fonctions permettent la détection cyclique (toutes les 1 ms) :
 * - des rotations dans les deux sens (incrément/décrément),
 * - des pressions courtes (action OK) et longues (action ESC),
 * - ainsi que de l'absence prolongée d'activité.
 *
 * Chaque fonction utilise un descripteur global mis à jour automatiquement.
 */

#include <stdbool.h>
#include <stdint.h>

// Structure du descripteur d'état du PEC12
// (Migration : utilisation de champs de 1 bit pour optimisation mémoire)
typedef struct {
    uint8_t Inc : 1;               // Indique un événement d'incrémentation
    uint8_t Dec : 1;               // Indique un événement de décrémentation
    uint8_t OK  : 1;               // Indique une action OK (appui court du bouton)
    uint8_t ESC : 1;               // Indique une action ESC (appui long du bouton)
    uint8_t NoActivity : 1;        // Indique une absence d'activité prolongée
    uint16_t PressDuration;        // Durée de la pression du bouton poussoir en ms
    uint16_t InactivityDuration;   // Durée de l'inactivité en ms
} S_Pec12_Descriptor;

// Structure du descripteur d'état pour le bouton S9 seul
typedef struct {
    uint8_t OK : 1;                // Indique un événement bouton OK (pression courte)
    uint8_t ESC : 1;               // Indique un événement action ESC (pression longue)
    uint8_t NoActivity : 1;        // Indique une absence d'activité prolongée
    uint16_t PressDuration;        // Durée de la pression sur le bouton S9 en ms
    uint16_t InactivityDuration;   // Durée d'inactivité en ms (non utilisée ici, optionnelle)
} S_PB_Descriptor;

//============================================================================
// @name ScanBtn
// @brief Scanne et gère l'état du codeur PEC12 ainsi que les boutons associés (PEC12 + S9).
//
// Cette fonction doit être appelée cycliquement toutes les 1 ms pour assurer
// la détection correcte des événements (rotation incrémentale, décrémentale,
// pression bouton poussoir PEC12, et bouton S9) avec traitement anti-rebond.
//
// Détection rotation :
// - Sens horaire (CW) si front descendant sur B avec A à l'état haut
// - Sens anti-horaire (CCW) si front descendant sur B avec A à l'état bas
/**
 * @attention Un cran génère une impulsion complète (les 4 combinaisons A/B)
 * Le traitement doit donc s'effectuer uniquement sur les fronts descendants du signal B.
 *
 * Séquence rotation horaire (CW) typique :
 *     __________                      ________________
 * B:            |____________________|
 *     ___________________                       _________
 * A:                     |_____________________|
 *
 * Sens anti-horaire (CCW) :
 *     ____________________                      _________
 * B:                      |____________________|
 *     __________                       __________________
 * A:            |_____________________|
 */
// @param ValA État logique du signal A du PEC12
// @param ValB État logique du signal B du PEC12
// @param ValPB Valeur logique du bouton poussoir PEC12 (0 actif)
// @param ValS9 État logique du bouton S9
void ScanBtn(bool ValA, bool ValB, bool ValPB, bool ValS9);


/**
 * @name Pec12Init
 * @brief Initialise les structures et variables nécessaires à la gestion du PEC12.
 *
 * À appeler au démarrage du programme pour réinitialiser l'état de gestion du PEC12.
 */
void Pec12Init(void);

/**
 * @name Pec12IsPlus
 * @brief Vérifie si une rotation dans le sens horaire (incrémentation) a été détectée.
 *
 * @return true si une rotation incrémentale a été détectée depuis la dernière remise à zéro
 */
bool Pec12IsPlus(void);

/**
 * @name Pec12IsMinus
 * @brief Vérifie si une rotation anti-horaire (décrément) a été détectée.
 *
 * @return true si une rotation décrémentale est détectée depuis la dernière remise à zéro
 */
bool Pec12IsMinus(void);

/**
 * @name Pec12IsOK
 * @brief Vérifie si un appui court (moins de 500 ms) sur le bouton PEC12 est détecté.
 *
 * @return true si un appui court a eu lieu depuis la dernière remise à zéro
 */
bool Pec12IsOK(void);

/**
 * @name Pec12IsESC
 * @brief Vérifie si un appui long (?500 ms) sur le bouton PEC12 est détecté.
 *
 * @return true si un appui long est détecté depuis la dernière remise à zéro
 */
bool Pec12IsESC(void);

/**
 * @name Pec12NoActivity
 * @brief Vérifie l'absence prolongée d'activité (5 secondes).
 *
 * @return true si aucune activité n'a été détectée pendant la durée définie
 */
bool Pec12NoActivity(void);

/**
 * @name S9IsOK
 * @brief Détecte un appui court sur le bouton S9.
 *
 * @return true si un appui court (<500 ms) sur S9 a eu lieu depuis la dernière remise à zéro
 */
bool S9IsOK(void);

/**
 * @name S9IsESC
 * @brief Détecte si un appui long (?500 ms) sur le bouton S9 a eu lieu.
 *
 * @return true si l'appui long est détecté depuis la dernière remise à zéro
 */
bool S9IsESC(void);



// Fonctions pour remettre à zéro les événements détectés :
void Pec12ClearPlus(void);        // Annule indication incrément
void Pec12ClearMinus(void);       // Annule indication décrément
void Pec12ClearOK(void);          // Annule indication action OK
void Pec12ClearESC(void);         // Annule indication action ESC
void Pec12ClearInactivity(void);  // Annule indication d'inactivité
// Remise à zéro des indicateurs pour le bouton S9 :
void S9ClearOK(void);             // Annule indication appui court sur S9
void S9ClearESC(void);            // Annule indication appui long (ESC) sur S9


/**
 * @name Pec12NoActivity
 * @brief Indique si le PEC12 n'a détecté aucune activité prolongée.
 *
 * @return true si aucune activité n'est détectée depuis la durée définie (généralement 5s)
 */
bool Pec12NoActivity(void);


#endif
