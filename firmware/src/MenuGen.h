#ifndef MenuGen_h
#define MenuGen_h

// MenuGen.h
// Auteur : C. HUBER
// Création : 03.02.2016
// Description : Gestion du menu pour le générateur de signal
// Traitement cyclique appelé toutes les 1 ms avec utilisation du PEC12

// Inclusion des types standardisés pour la portabilité du code
#include <stdbool.h>     // Définitions des booléens
#include <stdint.h>      // Définitions des types entiers standardisés
#include "DefMenuGen.h"  // Structure des paramètres du générateur (S_ParamGen) et états du menu


#define SELECTFORME       1
#define REGLAGEFORME      2
#define SELECTFREQUENCE   3
#define REGLAGEFREQUENCE  4
#define SELECTAMPLITUDE   5
#define REGLAGEAMPLITUDE  6
#define SELECTOFFSET      7
#define REGLAGEOFFSET     8
#define SAVEMODE          9  // Ã‰tat spÃ©cial pour gÃ©rer la sauvegarde
#define SAVEUSB           10

//========================================================
// ParamÃ©trage de la frÃ©quence
//========================================================
#define PAS_FREQUENCE  20    // IncrÃ©ment ou dÃ©crÃ©ment en Hz
#define FREQUENCE_MAX  2000  // FrÃ©quence maximale en Hz
#define FREQUENCE_MIN  20    // FrÃ©quence minimale en Hz

//========================================================
// ParamÃ©trage de lâ€™amplitude
//========================================================
#define PAS_AMPLITUDE  100   // IncrÃ©ment ou dÃ©crÃ©ment en mV
#define AMPLITUDE_MAX  10000 // Amplitude maximale en mV


//========================================================
// ParamÃ©trage de lâ€™offset
//========================================================
#define PAS_OFFSET     100   // IncrÃ©ment ou dÃ©crÃ©ment en mV
#define OFFSET_MAX     5000  // Offset maximal en mV
#define OFFSET_MIN    -5000  // Offset minimal en mV


// structure 


 
//typedef enum  { SignalSinus, SignalTriangle, SignalDentDeScie, SignalCarre } E_FormesSignal;
 
// Structure des paramètres du générateur

void MENU_DemandeSave(void);
 
/**
 * @name MENU_Initialize
 * @brief Initialise les paramètres et affiche l'état initial du menu.
 *
 * Cette fonction est appelée une seule fois au démarrage du programme afin d'initialiser
 * la machine d'état du menu et les paramètres liés au générateur de signal.
 *
 * @param pParam Pointeur vers la structure contenant les paramètres initiaux du générateur.
 */
void MENU_Initialize(S_ParamGen *pParam, bool local);

/**
 * @name MENU_Execute
 * @brief Exécute la machine d'état du menu (appel cyclique, non-bloquant).
 *
 * Cette fonction est appelée cycliquement (toutes les 1 ms) et gère :
 * - La navigation dans les différentes rubriques du menu (forme, fréquence, amplitude, offset).
 * - La modification interactive des valeurs sélectionnées via le PEC12 (bouton rotatif et poussoir).
 * - Le déclenchement de la sauvegarde des paramètres en mémoire non volatile.
 *
 * @param pParam Pointeur vers la structure de paramètres courants du générateur.
 */
void MENU_Execute(S_ParamGen *pParam, bool local);

/**
 * @name MENU_Display
 * @brief Gère l'affichage des paramètres sur l'écran LCD selon l'état actuel du menu.
 *
 * Affiche sur l'écran LCD l'état courant du menu et les paramètres modifiables (forme,
 * fréquence, amplitude et offset). Indique clairement la ligne active par un astérisque (*)
 * en mode sélection ou par un point d'interrogation (?) en mode édition.
 *
 * @param pParam Pointeur vers la structure contenant les valeurs courantes à afficher.
 * @param menu Valeur correspondant à l'état courant de la machine d'état (voir enum MenuState_t).
 */
void MENU_Display(S_ParamGen *pParam, uint8_t menu);

#endif
