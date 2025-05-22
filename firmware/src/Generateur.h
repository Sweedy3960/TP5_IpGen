#ifndef Generateur_h
#define Generateur_h

// TP3 MenuGen 2016
// C. HUBER  03.02.2016
// Fichier Generateur.h
// Prototypes des fonctions du générateur  de signal

#include <math.h>
#include <stdint.h>
#include <stdbool.h>

#include "DefMenuGen.h"

//définition des constantes
#define ECHANTILLONS_MAX 100 //nombre max d'échantillons
#define AMPLITUDE_MAX 10000
#define AMPLITUDE_MIN -10000
#define AMPLITUDE_SUR_2 5000
#define PAS_MAXIMUM 65535
#define F_SYS       80000000  // Fréquence du CPU ou du Timer (ex: 80 MHz)
#define PRESCALER   32          // Prescaler sélectionné pour TMR3
#define MAX_ECH     100         // Nombre d'échantillons utilisé
#define MAX_ECH 100  // Nombre d'échantillons par période

// Borne la tension en mV dans l'intervalle [-10000, +10000] avant conversion DAC
#define DAC_MIN  0
#define DAC_MAX  65535


// Initialisation du  générateur
void GENSIG_Initialize(S_ParamGen *pParam);


// Mise à jour de la periode d'échantillonage
void GENSIG_UpdatePeriode(S_ParamGen *pParam);


// Mise à jour du signal (forme, amplitude, offset)
void GENSIG_UpdateSignal(S_ParamGen *pParam);

// A appeler dans int Timer3
void GENSIG_Execute(void);


#endif