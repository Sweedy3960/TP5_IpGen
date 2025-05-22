// GesPec12.c - Gestion du codeur PEC12 et des boutons associés
// C. HUBER    09/02/2015
//
// Fonctionnalités :
// - Détection des rotations du codeur PEC12 (incrément, décrément).
// - Gestion des boutons poussoirs du PEC12 et du bouton S9.
// - Extinction automatique du rétroéclairage après inactivité.
//
// La fonction ScanBtn() doit être appelée cycliquement toutes les 1 ms.


// ================================
// Inclusion des bibliothèques nécessaires
// ================================
#include "GesPec12.h"        // Définitions des structures et prototypes liés au PEC12
#include "Mc32Debounce.h"    // Gestion de l'anti-rebond des boutons et codeurs
#include "Mc32DriverLcd.h"   // Fonctions pour l'affichage sur écran LCD

#define AFK_TIME 5000 // Temps d'inactivité avant extinction du rétroéclairage (5s)


// ================================
// Déclaration des structures et variables globales
// ================================

S_SwitchDescriptor DescrA; // Signal A du codeur PEC12 (pour l'anti-rebond)
S_SwitchDescriptor DescrB; // Signal B du codeur PEC12 (pour l'anti-rebond)
S_SwitchDescriptor DescrPB; // Bouton poussoir intégré au PEC12 (pour l'anti-rebond)
S_SwitchDescriptor DescrS9; // Bouton poussoir externe S9 (pour l'anti-rebond)

// Descripteur principal du PEC12 (incrément, décrément, OK, ESC, etc.)
S_Pec12_Descriptor Pec12;
// Descripteur pour le bouton S9 (OK, ESC, etc.)
S_PB_Descriptor S9;


// ================================
// Fonctions d'initialisation
// ================================

/**
 * @brief Initialise les descripteurs et réinitialise les variables d'état du PEC12 et du bouton S9.
 * @note Cette fonction doit être appelée au démarrage pour préparer l'anti-rebond et les compteurs d'événements.
 */
void Pec12Init(void) {
    // Initialisation des structures anti-rebond pour chaque signal
    DebounceInit(&DescrA); // Prépare le traitement anti-rebond pour le signal A
    DebounceInit(&DescrB); // Prépare le traitement anti-rebond pour le signal B
    DebounceInit(&DescrPB); // Prépare le traitement anti-rebond pour le bouton poussoir du PEC12
    DebounceInit(&DescrS9); // Prépare le traitement anti-rebond pour le bouton poussoir S9

    // Réinitialisation des variables du PEC12
    Pec12.Inc = 0; // Remet à zéro le flag d'incrément
    Pec12.Dec = 0; // Remet à zéro le flag de décrément
    Pec12.OK = 0; // Remet à zéro l'événement OK
    Pec12.ESC = 0; // Remet à zéro l'événement ESC
    Pec12.NoActivity = 0; // Remet à zéro le flag d'activité (0 = pas d'activité)
    Pec12.PressDuration = 0; // Remet à zéro la durée de pression
    Pec12.InactivityDuration = 0; // Remet à zéro la durée d'inactivité

    // Réinitialisation des variables du bouton S9
    S9.OK = 0; // Remet à zéro l'événement OK pour S9
    S9.ESC = 0; // Remet à zéro l'événement ESC pour S9
    S9.NoActivity = 0; // Remet à zéro le flag d'activité pour S9
    S9.PressDuration = 0; // Remet à zéro la durée de pression pour S9
    S9.InactivityDuration = 0; // Remet à zéro la durée d'inactivité pour S9
}


// ================================
// Gestion des événements du PEC12 et des boutons
// ================================

/**
 * @brief Scanne l'état du codeur PEC12 et des boutons associés, et met à jour les événements.
 * @param ValA  État logique du signal A du codeur
 * @param ValB  État logique du signal B du codeur
 * @param ValPB État du bouton poussoir du codeur (PEC12)
 * @param ValS9 État du bouton poussoir S9
 * @note Cette fonction doit être appelée toutes les 1 ms pour un traitement fiable de l'anti-rebond et de la détection d'événements.
 */
void ScanBtn(bool ValA, bool ValB, bool ValPB, bool ValS9) {
    // Applique l'anti-rebond sur toutes les entrées
    DoDebounce(&DescrA, ValA); // Met à jour l'état débouncé du signal A
    DoDebounce(&DescrB, ValB); // Met à jour l'état débouncé du signal B
    DoDebounce(&DescrPB, ValPB); // Met à jour l'état débouncé du bouton PEC12
    DoDebounce(&DescrS9, ValS9); // Met à jour l'état débouncé du bouton S9

    // ================================
    // Détection de la rotation du codeur PEC12
    // ================================
    if (DebounceIsPressed(&DescrB)) { // Vérifie si le signal B vient d'être pressé (transition détectée)
        DebounceClearPressed(&DescrB); // Efface l'événement pressé pour éviter toute redondance
        Pec12.NoActivity = 1; // Indique qu'une activité a eu lieu (pas d'inactivité)

        if (DebounceGetInput(&DescrA) == 0) {
            Pec12.Inc = 1; // Si A est à 0, on considère que le mouvement est un incrément
            Pec12.Dec = 0; // Annule tout décrément éventuel
        } else {
            Pec12.Dec = 1; // Sinon, c'est un décrément
            Pec12.Inc = 0; // Annule tout incrément éventuel
        }
    }

    // ================================
    // Gestion du bouton poussoir du PEC12
    // ================================
    if (DebounceIsPressed(&DescrPB)) { // Vérifie si le bouton PEC12 vient d'être pressé
        Pec12.NoActivity = 1; // Signale une activité pour réinitialiser l'inactivité
        DebounceClearPressed(&DescrPB); // Efface l'événement pressé
        Pec12.PressDuration = 0; // Réinitialise la durée de pression à chaque nouvel appui
    }
    else if (DebounceGetInput(&DescrPB) == 0) { // Vérifie si le bouton PEC12 est maintenu enfoncé
        Pec12.PressDuration++; // Incrémente la durée de pression tant que c'est maintenu
    }
    else if (DebounceIsReleased(&DescrPB)) { // Vérifie si le bouton PEC12 vient d'être relâché
        DebounceClearReleased(&DescrPB); // Efface l'événement relâché

        if (Pec12.PressDuration < 500) {
            Pec12.OK = 1; // Pression courte (moins de 500 ms) = validation OK
        } else {
            Pec12.ESC = 1; // Pression longue (500 ms ou plus) = ESC
        }

        Pec12.PressDuration = 0; // Réinitialise la durée de pression après relâchement
    }

    // ================================
    // Gestion du bouton S9
    // ================================
    if (DebounceIsPressed(&DescrS9)) { // Vérifie si le bouton S9 vient d'être pressé
        Pec12.NoActivity = 1; // Signale une activité (utilisateur)
        DebounceClearPressed(&DescrS9); // Efface l'événement pressé
        S9.PressDuration = 0; // Réinitialise la durée de pression pour S9
    } else if (DebounceGetInput(&DescrS9) == 0) { // Vérifie si le bouton S9 est maintenu enfoncé
        S9.PressDuration++; // Incrémente la durée de pression pour S9
    } else if (DebounceIsReleased(&DescrS9)) { // Vérifie si le bouton S9 vient d'être relâché
        DebounceClearReleased(&DescrS9); // Efface l'événement relâché

        if (S9.PressDuration < 500) {
            S9.OK = 1; // Pression courte = OK
        } else {
            S9.ESC = 1; // Pression longue = ESC
        }

        S9.PressDuration = 0; // Réinitialise la durée de pression après relâchement
    }

    // ================================
    // Gestion de l'inactivité et extinction du rétroéclairage
    // ================================
    if (Pec12.NoActivity == 0) { // Vérifie s'il n'y a eu aucune activité lors de ce cycle
        if (Pec12.InactivityDuration >= AFK_TIME) {
            lcd_bl_off(); // Éteint le rétroéclairage si l'inactivité dépasse AFK_TIME
        } else {
            Pec12.InactivityDuration++; // Incrémente le compteur d'inactivité
        }
    } else {
        Pec12ClearInactivity(); // Remet à zéro l'inactivité si une activité est détectée
        lcd_bl_on(); // Rallume le rétroéclairage en cas d'activité
    }
}


// ================================
// Fonctions de test et annulation des événements
// ================================

/**
 * @brief Vérifie si l'événement d'incrément est actif sur le codeur PEC12.
 * @return true si l'événement d'incrément (Inc) est actif, false sinon.
 */
bool Pec12IsPlus(void) {
    return (Pec12.Inc);
} // Retourne l'état du flag Inc

/**
 * @brief Vérifie si l'événement de décrément est actif sur le codeur PEC12.
 * @return true si l'événement de décrément (Dec) est actif, false sinon.
 */
bool Pec12IsMinus(void) {
    return (Pec12.Dec);
} // Retourne l'état du flag Dec

/**
 * @brief Vérifie si l'événement OK (pression courte) est actif sur le codeur PEC12.
 * @return true si l'événement OK est actif, false sinon.
 */
bool Pec12IsOK(void) {
    return (Pec12.OK);
} // Retourne l'état du flag OK

/**
 * @brief Vérifie si l'événement ESC (pression longue) est actif sur le codeur PEC12.
 * @return true si l'événement ESC est actif, false sinon.
 */
bool Pec12IsESC(void) {
    return (Pec12.ESC);
} // Retourne l'état du flag ESC

/**
 * @brief Vérifie s'il y a eu de l'activité sur le codeur PEC12 durant le cycle actuel.
 * @return true si une activité a été détectée, false sinon.
 */
bool Pec12NoActivity(void) {
    return (Pec12.NoActivity);
} // Retourne l'état du flag NoActivity

/**
 * @brief Vérifie si l'événement OK est actif sur le bouton S9.
 * @return true si l'événement OK est actif, false sinon.
 */
bool S9IsOK(void) {
    return (S9.OK);
} // Retourne l'état du flag OK pour S9

/**
 * @brief Vérifie si l'événement ESC est actif sur le bouton S9.
 * @return true si l'événement ESC est actif, false sinon.
 */
bool S9IsESC(void) {
    return (S9.ESC);
} // Retourne l'état du flag ESC pour S9

/**
 * @brief Annule les flags d'incrément et de décrément du codeur PEC12 (les remet à zéro).
 */
void Pec12ClearPlus(void) {
    Pec12.Inc = 0;
    Pec12.Dec = 0;
} // Réinitialise Inc et Dec

/**
 * @brief Annule les flags de décrément et d'incrément du codeur PEC12 (les remet à zéro).
 */
void Pec12ClearMinus(void) {
    Pec12.Dec = 0;
    Pec12.Inc = 0;
} // Réinitialise Dec et Inc

/**
 * @brief Annule le flag OK du codeur PEC12.
 */
void Pec12ClearOK(void) {
    Pec12.OK = 0;
} // Réinitialise OK

/**
 * @brief Annule le flag ESC du codeur PEC12.
 */
void Pec12ClearESC(void) {
    Pec12.ESC = 0;
} // Réinitialise ESC

/**
 * @brief Réinitialise l'état d'activité et la durée d'inactivité du codeur PEC12.
 */
void Pec12ClearInactivity(void) {
    Pec12.NoActivity = 0;
    Pec12.InactivityDuration = 0;
} // Réinitialise NoActivity et InactivityDuration

/**
 * @brief Annule le flag OK du bouton S9.
 */
void S9ClearOK(void) {
    S9.OK = 0;
} // Réinitialise OK pour S9

/**
 * @brief Annule le flag ESC du bouton S9.
 */
void S9ClearESC(void) {
    S9.ESC = 0;
} // Réinitialise ESC pour S9
