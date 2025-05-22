#ifndef GesPEc12_h
#define GesPEc12_h

// GesPec12.h
// C. HUBER    14/05/2014
// Cr�ation pour migration sur PIC32MX

/**
 * @file GesPec12.h
 * @brief Interface de gestion du codeur incr�mental PEC12 et bouton associ� (gestion anti-rebond incluse).
 *
 * Ces fonctions permettent la d�tection cyclique (toutes les 1 ms) :
 * - des rotations dans les deux sens (incr�ment/d�cr�ment),
 * - des pressions courtes (action OK) et longues (action ESC),
 * - ainsi que de l'absence prolong�e d'activit�.
 *
 * Chaque fonction utilise un descripteur global mis � jour automatiquement.
 */

#include <stdbool.h>
#include <stdint.h>

// Structure du descripteur d'�tat du PEC12
// (Migration : utilisation de champs de 1 bit pour optimisation m�moire)
typedef struct {
    uint8_t Inc : 1;               // Indique un �v�nement d'incr�mentation
    uint8_t Dec : 1;               // Indique un �v�nement de d�cr�mentation
    uint8_t OK  : 1;               // Indique une action OK (appui court du bouton)
    uint8_t ESC : 1;               // Indique une action ESC (appui long du bouton)
    uint8_t NoActivity : 1;        // Indique une absence d'activit� prolong�e
    uint16_t PressDuration;        // Dur�e de la pression du bouton poussoir en ms
    uint16_t InactivityDuration;   // Dur�e de l'inactivit� en ms
} S_Pec12_Descriptor;

// Structure du descripteur d'�tat pour le bouton S9 seul
typedef struct {
    uint8_t OK : 1;                // Indique un �v�nement bouton OK (pression courte)
    uint8_t ESC : 1;               // Indique un �v�nement action ESC (pression longue)
    uint8_t NoActivity : 1;        // Indique une absence d'activit� prolong�e
    uint16_t PressDuration;        // Dur�e de la pression sur le bouton S9 en ms
    uint16_t InactivityDuration;   // Dur�e d'inactivit� en ms (non utilis�e ici, optionnelle)
} S_PB_Descriptor;

//============================================================================
// @name ScanBtn
// @brief Scanne et g�re l'�tat du codeur PEC12 ainsi que les boutons associ�s (PEC12 + S9).
//
// Cette fonction doit �tre appel�e cycliquement toutes les 1 ms pour assurer
// la d�tection correcte des �v�nements (rotation incr�mentale, d�cr�mentale,
// pression bouton poussoir PEC12, et bouton S9) avec traitement anti-rebond.
//
// D�tection rotation :
// - Sens horaire (CW) si front descendant sur B avec A � l'�tat haut
// - Sens anti-horaire (CCW) si front descendant sur B avec A � l'�tat bas
/**
 * @attention Un cran g�n�re une impulsion compl�te (les 4 combinaisons A/B)
 * Le traitement doit donc s'effectuer uniquement sur les fronts descendants du signal B.
 *
 * S�quence rotation horaire (CW) typique :
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
// @param ValA �tat logique du signal A du PEC12
// @param ValB �tat logique du signal B du PEC12
// @param ValPB Valeur logique du bouton poussoir PEC12 (0 actif)
// @param ValS9 �tat logique du bouton S9
void ScanBtn(bool ValA, bool ValB, bool ValPB, bool ValS9);


/**
 * @name Pec12Init
 * @brief Initialise les structures et variables n�cessaires � la gestion du PEC12.
 *
 * � appeler au d�marrage du programme pour r�initialiser l'�tat de gestion du PEC12.
 */
void Pec12Init(void);

/**
 * @name Pec12IsPlus
 * @brief V�rifie si une rotation dans le sens horaire (incr�mentation) a �t� d�tect�e.
 *
 * @return true si une rotation incr�mentale a �t� d�tect�e depuis la derni�re remise � z�ro
 */
bool Pec12IsPlus(void);

/**
 * @name Pec12IsMinus
 * @brief V�rifie si une rotation anti-horaire (d�cr�ment) a �t� d�tect�e.
 *
 * @return true si une rotation d�cr�mentale est d�tect�e depuis la derni�re remise � z�ro
 */
bool Pec12IsMinus(void);

/**
 * @name Pec12IsOK
 * @brief V�rifie si un appui court (moins de 500 ms) sur le bouton PEC12 est d�tect�.
 *
 * @return true si un appui court a eu lieu depuis la derni�re remise � z�ro
 */
bool Pec12IsOK(void);

/**
 * @name Pec12IsESC
 * @brief V�rifie si un appui long (?500 ms) sur le bouton PEC12 est d�tect�.
 *
 * @return true si un appui long est d�tect� depuis la derni�re remise � z�ro
 */
bool Pec12IsESC(void);

/**
 * @name Pec12NoActivity
 * @brief V�rifie l'absence prolong�e d'activit� (5 secondes).
 *
 * @return true si aucune activit� n'a �t� d�tect�e pendant la dur�e d�finie
 */
bool Pec12NoActivity(void);

/**
 * @name S9IsOK
 * @brief D�tecte un appui court sur le bouton S9.
 *
 * @return true si un appui court (<500 ms) sur S9 a eu lieu depuis la derni�re remise � z�ro
 */
bool S9IsOK(void);

/**
 * @name S9IsESC
 * @brief D�tecte si un appui long (?500 ms) sur le bouton S9 a eu lieu.
 *
 * @return true si l'appui long est d�tect� depuis la derni�re remise � z�ro
 */
bool S9IsESC(void);



// Fonctions pour remettre � z�ro les �v�nements d�tect�s :
void Pec12ClearPlus(void);        // Annule indication incr�ment
void Pec12ClearMinus(void);       // Annule indication d�cr�ment
void Pec12ClearOK(void);          // Annule indication action OK
void Pec12ClearESC(void);         // Annule indication action ESC
void Pec12ClearInactivity(void);  // Annule indication d'inactivit�
// Remise � z�ro des indicateurs pour le bouton S9 :
void S9ClearOK(void);             // Annule indication appui court sur S9
void S9ClearESC(void);            // Annule indication appui long (ESC) sur S9


/**
 * @name Pec12NoActivity
 * @brief Indique si le PEC12 n'a d�tect� aucune activit� prolong�e.
 *
 * @return true si aucune activit� n'est d�tect�e depuis la dur�e d�finie (g�n�ralement 5s)
 */
bool Pec12NoActivity(void);


#endif
