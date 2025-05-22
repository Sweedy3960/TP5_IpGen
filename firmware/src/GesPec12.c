// GesPec12.c - Gestion du codeur PEC12 et des boutons associ�s
// C. HUBER    09/02/2015
//
// Fonctionnalit�s :
// - D�tection des rotations du codeur PEC12 (incr�ment, d�cr�ment).
// - Gestion des boutons poussoirs du PEC12 et du bouton S9.
// - Extinction automatique du r�tro�clairage apr�s inactivit�.
//
// La fonction ScanBtn() doit �tre appel�e cycliquement toutes les 1 ms.


// ================================
// Inclusion des biblioth�ques n�cessaires
// ================================
#include "GesPec12.h"        // D�finitions des structures et prototypes li�s au PEC12
#include "Mc32Debounce.h"    // Gestion de l'anti-rebond des boutons et codeurs
#include "Mc32DriverLcd.h"   // Fonctions pour l'affichage sur �cran LCD

#define AFK_TIME 5000 // Temps d'inactivit� avant extinction du r�tro�clairage (5s)


// ================================
// D�claration des structures et variables globales
// ================================

S_SwitchDescriptor DescrA; // Signal A du codeur PEC12 (pour l'anti-rebond)
S_SwitchDescriptor DescrB; // Signal B du codeur PEC12 (pour l'anti-rebond)
S_SwitchDescriptor DescrPB; // Bouton poussoir int�gr� au PEC12 (pour l'anti-rebond)
S_SwitchDescriptor DescrS9; // Bouton poussoir externe S9 (pour l'anti-rebond)

// Descripteur principal du PEC12 (incr�ment, d�cr�ment, OK, ESC, etc.)
S_Pec12_Descriptor Pec12;
// Descripteur pour le bouton S9 (OK, ESC, etc.)
S_PB_Descriptor S9;


// ================================
// Fonctions d'initialisation
// ================================

/**
 * @brief Initialise les descripteurs et r�initialise les variables d'�tat du PEC12 et du bouton S9.
 * @note Cette fonction doit �tre appel�e au d�marrage pour pr�parer l'anti-rebond et les compteurs d'�v�nements.
 */
void Pec12Init(void) {
    // Initialisation des structures anti-rebond pour chaque signal
    DebounceInit(&DescrA); // Pr�pare le traitement anti-rebond pour le signal A
    DebounceInit(&DescrB); // Pr�pare le traitement anti-rebond pour le signal B
    DebounceInit(&DescrPB); // Pr�pare le traitement anti-rebond pour le bouton poussoir du PEC12
    DebounceInit(&DescrS9); // Pr�pare le traitement anti-rebond pour le bouton poussoir S9

    // R�initialisation des variables du PEC12
    Pec12.Inc = 0; // Remet � z�ro le flag d'incr�ment
    Pec12.Dec = 0; // Remet � z�ro le flag de d�cr�ment
    Pec12.OK = 0; // Remet � z�ro l'�v�nement OK
    Pec12.ESC = 0; // Remet � z�ro l'�v�nement ESC
    Pec12.NoActivity = 0; // Remet � z�ro le flag d'activit� (0 = pas d'activit�)
    Pec12.PressDuration = 0; // Remet � z�ro la dur�e de pression
    Pec12.InactivityDuration = 0; // Remet � z�ro la dur�e d'inactivit�

    // R�initialisation des variables du bouton S9
    S9.OK = 0; // Remet � z�ro l'�v�nement OK pour S9
    S9.ESC = 0; // Remet � z�ro l'�v�nement ESC pour S9
    S9.NoActivity = 0; // Remet � z�ro le flag d'activit� pour S9
    S9.PressDuration = 0; // Remet � z�ro la dur�e de pression pour S9
    S9.InactivityDuration = 0; // Remet � z�ro la dur�e d'inactivit� pour S9
}


// ================================
// Gestion des �v�nements du PEC12 et des boutons
// ================================

/**
 * @brief Scanne l'�tat du codeur PEC12 et des boutons associ�s, et met � jour les �v�nements.
 * @param ValA  �tat logique du signal A du codeur
 * @param ValB  �tat logique du signal B du codeur
 * @param ValPB �tat du bouton poussoir du codeur (PEC12)
 * @param ValS9 �tat du bouton poussoir S9
 * @note Cette fonction doit �tre appel�e toutes les 1 ms pour un traitement fiable de l'anti-rebond et de la d�tection d'�v�nements.
 */
void ScanBtn(bool ValA, bool ValB, bool ValPB, bool ValS9) {
    // Applique l'anti-rebond sur toutes les entr�es
    DoDebounce(&DescrA, ValA); // Met � jour l'�tat d�bounc� du signal A
    DoDebounce(&DescrB, ValB); // Met � jour l'�tat d�bounc� du signal B
    DoDebounce(&DescrPB, ValPB); // Met � jour l'�tat d�bounc� du bouton PEC12
    DoDebounce(&DescrS9, ValS9); // Met � jour l'�tat d�bounc� du bouton S9

    // ================================
    // D�tection de la rotation du codeur PEC12
    // ================================
    if (DebounceIsPressed(&DescrB)) { // V�rifie si le signal B vient d'�tre press� (transition d�tect�e)
        DebounceClearPressed(&DescrB); // Efface l'�v�nement press� pour �viter toute redondance
        Pec12.NoActivity = 1; // Indique qu'une activit� a eu lieu (pas d'inactivit�)

        if (DebounceGetInput(&DescrA) == 0) {
            Pec12.Inc = 1; // Si A est � 0, on consid�re que le mouvement est un incr�ment
            Pec12.Dec = 0; // Annule tout d�cr�ment �ventuel
        } else {
            Pec12.Dec = 1; // Sinon, c'est un d�cr�ment
            Pec12.Inc = 0; // Annule tout incr�ment �ventuel
        }
    }

    // ================================
    // Gestion du bouton poussoir du PEC12
    // ================================
    if (DebounceIsPressed(&DescrPB)) { // V�rifie si le bouton PEC12 vient d'�tre press�
        Pec12.NoActivity = 1; // Signale une activit� pour r�initialiser l'inactivit�
        DebounceClearPressed(&DescrPB); // Efface l'�v�nement press�
        Pec12.PressDuration = 0; // R�initialise la dur�e de pression � chaque nouvel appui
    }
    else if (DebounceGetInput(&DescrPB) == 0) { // V�rifie si le bouton PEC12 est maintenu enfonc�
        Pec12.PressDuration++; // Incr�mente la dur�e de pression tant que c'est maintenu
    }
    else if (DebounceIsReleased(&DescrPB)) { // V�rifie si le bouton PEC12 vient d'�tre rel�ch�
        DebounceClearReleased(&DescrPB); // Efface l'�v�nement rel�ch�

        if (Pec12.PressDuration < 500) {
            Pec12.OK = 1; // Pression courte (moins de 500 ms) = validation OK
        } else {
            Pec12.ESC = 1; // Pression longue (500 ms ou plus) = ESC
        }

        Pec12.PressDuration = 0; // R�initialise la dur�e de pression apr�s rel�chement
    }

    // ================================
    // Gestion du bouton S9
    // ================================
    if (DebounceIsPressed(&DescrS9)) { // V�rifie si le bouton S9 vient d'�tre press�
        Pec12.NoActivity = 1; // Signale une activit� (utilisateur)
        DebounceClearPressed(&DescrS9); // Efface l'�v�nement press�
        S9.PressDuration = 0; // R�initialise la dur�e de pression pour S9
    } else if (DebounceGetInput(&DescrS9) == 0) { // V�rifie si le bouton S9 est maintenu enfonc�
        S9.PressDuration++; // Incr�mente la dur�e de pression pour S9
    } else if (DebounceIsReleased(&DescrS9)) { // V�rifie si le bouton S9 vient d'�tre rel�ch�
        DebounceClearReleased(&DescrS9); // Efface l'�v�nement rel�ch�

        if (S9.PressDuration < 500) {
            S9.OK = 1; // Pression courte = OK
        } else {
            S9.ESC = 1; // Pression longue = ESC
        }

        S9.PressDuration = 0; // R�initialise la dur�e de pression apr�s rel�chement
    }

    // ================================
    // Gestion de l'inactivit� et extinction du r�tro�clairage
    // ================================
    if (Pec12.NoActivity == 0) { // V�rifie s'il n'y a eu aucune activit� lors de ce cycle
        if (Pec12.InactivityDuration >= AFK_TIME) {
            lcd_bl_off(); // �teint le r�tro�clairage si l'inactivit� d�passe AFK_TIME
        } else {
            Pec12.InactivityDuration++; // Incr�mente le compteur d'inactivit�
        }
    } else {
        Pec12ClearInactivity(); // Remet � z�ro l'inactivit� si une activit� est d�tect�e
        lcd_bl_on(); // Rallume le r�tro�clairage en cas d'activit�
    }
}


// ================================
// Fonctions de test et annulation des �v�nements
// ================================

/**
 * @brief V�rifie si l'�v�nement d'incr�ment est actif sur le codeur PEC12.
 * @return true si l'�v�nement d'incr�ment (Inc) est actif, false sinon.
 */
bool Pec12IsPlus(void) {
    return (Pec12.Inc);
} // Retourne l'�tat du flag Inc

/**
 * @brief V�rifie si l'�v�nement de d�cr�ment est actif sur le codeur PEC12.
 * @return true si l'�v�nement de d�cr�ment (Dec) est actif, false sinon.
 */
bool Pec12IsMinus(void) {
    return (Pec12.Dec);
} // Retourne l'�tat du flag Dec

/**
 * @brief V�rifie si l'�v�nement OK (pression courte) est actif sur le codeur PEC12.
 * @return true si l'�v�nement OK est actif, false sinon.
 */
bool Pec12IsOK(void) {
    return (Pec12.OK);
} // Retourne l'�tat du flag OK

/**
 * @brief V�rifie si l'�v�nement ESC (pression longue) est actif sur le codeur PEC12.
 * @return true si l'�v�nement ESC est actif, false sinon.
 */
bool Pec12IsESC(void) {
    return (Pec12.ESC);
} // Retourne l'�tat du flag ESC

/**
 * @brief V�rifie s'il y a eu de l'activit� sur le codeur PEC12 durant le cycle actuel.
 * @return true si une activit� a �t� d�tect�e, false sinon.
 */
bool Pec12NoActivity(void) {
    return (Pec12.NoActivity);
} // Retourne l'�tat du flag NoActivity

/**
 * @brief V�rifie si l'�v�nement OK est actif sur le bouton S9.
 * @return true si l'�v�nement OK est actif, false sinon.
 */
bool S9IsOK(void) {
    return (S9.OK);
} // Retourne l'�tat du flag OK pour S9

/**
 * @brief V�rifie si l'�v�nement ESC est actif sur le bouton S9.
 * @return true si l'�v�nement ESC est actif, false sinon.
 */
bool S9IsESC(void) {
    return (S9.ESC);
} // Retourne l'�tat du flag ESC pour S9

/**
 * @brief Annule les flags d'incr�ment et de d�cr�ment du codeur PEC12 (les remet � z�ro).
 */
void Pec12ClearPlus(void) {
    Pec12.Inc = 0;
    Pec12.Dec = 0;
} // R�initialise Inc et Dec

/**
 * @brief Annule les flags de d�cr�ment et d'incr�ment du codeur PEC12 (les remet � z�ro).
 */
void Pec12ClearMinus(void) {
    Pec12.Dec = 0;
    Pec12.Inc = 0;
} // R�initialise Dec et Inc

/**
 * @brief Annule le flag OK du codeur PEC12.
 */
void Pec12ClearOK(void) {
    Pec12.OK = 0;
} // R�initialise OK

/**
 * @brief Annule le flag ESC du codeur PEC12.
 */
void Pec12ClearESC(void) {
    Pec12.ESC = 0;
} // R�initialise ESC

/**
 * @brief R�initialise l'�tat d'activit� et la dur�e d'inactivit� du codeur PEC12.
 */
void Pec12ClearInactivity(void) {
    Pec12.NoActivity = 0;
    Pec12.InactivityDuration = 0;
} // R�initialise NoActivity et InactivityDuration

/**
 * @brief Annule le flag OK du bouton S9.
 */
void S9ClearOK(void) {
    S9.OK = 0;
} // R�initialise OK pour S9

/**
 * @brief Annule le flag ESC du bouton S9.
 */
void S9ClearESC(void) {
    S9.ESC = 0;
} // R�initialise ESC pour S9
