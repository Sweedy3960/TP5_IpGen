#ifndef MenuGen_h
#define MenuGen_h

// MenuGen.h
// Auteur : C. HUBER
// Cr�ation : 03.02.2016
// Description : Gestion du menu pour le g�n�rateur de signal
// Traitement cyclique appel� toutes les 1 ms avec utilisation du PEC12

// Inclusion des types standardis�s pour la portabilit� du code
#include <stdbool.h>     // D�finitions des bool�ens
#include <stdint.h>      // D�finitions des types entiers standardis�s
#include "DefMenuGen.h"  // Structure des param�tres du g�n�rateur (S_ParamGen) et �tats du menu


#define SELECTFORME       1
#define REGLAGEFORME      2
#define SELECTFREQUENCE   3
#define REGLAGEFREQUENCE  4
#define SELECTAMPLITUDE   5
#define REGLAGEAMPLITUDE  6
#define SELECTOFFSET      7
#define REGLAGEOFFSET     8
#define SAVEMODE          9  // État spécial pour gérer la sauvegarde
#define SAVEUSB           10

//========================================================
// Paramétrage de la fréquence
//========================================================
#define PAS_FREQUENCE  20    // Incrément ou décrément en Hz
#define FREQUENCE_MAX  2000  // Fréquence maximale en Hz
#define FREQUENCE_MIN  20    // Fréquence minimale en Hz

//========================================================
// Paramétrage de l’amplitude
//========================================================
#define PAS_AMPLITUDE  100   // Incrément ou décrément en mV
#define AMPLITUDE_MAX  10000 // Amplitude maximale en mV


//========================================================
// Paramétrage de l’offset
//========================================================
#define PAS_OFFSET     100   // Incrément ou décrément en mV
#define OFFSET_MAX     5000  // Offset maximal en mV
#define OFFSET_MIN    -5000  // Offset minimal en mV


// structure 


 
//typedef enum  { SignalSinus, SignalTriangle, SignalDentDeScie, SignalCarre } E_FormesSignal;
 
// Structure des param�tres du g�n�rateur

void MENU_DemandeSave(void);
 
/**
 * @name MENU_Initialize
 * @brief Initialise les param�tres et affiche l'�tat initial du menu.
 *
 * Cette fonction est appel�e une seule fois au d�marrage du programme afin d'initialiser
 * la machine d'�tat du menu et les param�tres li�s au g�n�rateur de signal.
 *
 * @param pParam Pointeur vers la structure contenant les param�tres initiaux du g�n�rateur.
 */
void MENU_Initialize(S_ParamGen *pParam, bool local);

/**
 * @name MENU_Execute
 * @brief Ex�cute la machine d'�tat du menu (appel cyclique, non-bloquant).
 *
 * Cette fonction est appel�e cycliquement (toutes les 1 ms) et g�re :
 * - La navigation dans les diff�rentes rubriques du menu (forme, fr�quence, amplitude, offset).
 * - La modification interactive des valeurs s�lectionn�es via le PEC12 (bouton rotatif et poussoir).
 * - Le d�clenchement de la sauvegarde des param�tres en m�moire non volatile.
 *
 * @param pParam Pointeur vers la structure de param�tres courants du g�n�rateur.
 */
void MENU_Execute(S_ParamGen *pParam, bool local);

/**
 * @name MENU_Display
 * @brief G�re l'affichage des param�tres sur l'�cran LCD selon l'�tat actuel du menu.
 *
 * Affiche sur l'�cran LCD l'�tat courant du menu et les param�tres modifiables (forme,
 * fr�quence, amplitude et offset). Indique clairement la ligne active par un ast�risque (*)
 * en mode s�lection ou par un point d'interrogation (?) en mode �dition.
 *
 * @param pParam Pointeur vers la structure contenant les valeurs courantes � afficher.
 * @param menu Valeur correspondant � l'�tat courant de la machine d'�tat (voir enum MenuState_t).
 */
void MENU_Display(S_ParamGen *pParam, uint8_t menu);

#endif
