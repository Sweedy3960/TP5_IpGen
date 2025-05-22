// =====================================================================================
// Canevas manipulation GenSig avec menu
// C. HUBER  09/02/2015
// Fichier Generateur.C
// Gestion du g�n�rateur
//
// Pr�vu pour signal de 40 �chantillons
// Migration sur PIC32 30.04.2014 C. Huber
// =====================================================================================

#include "Generateur.h"     // D�finitions des fonctions du g�n�rateur (prototypes)
#include "DefMenuGen.h"     // D�finitions g�n�rales : structure S_ParamGen, etc.
#include "Mc32gestSpiDac.h" // Fonctions pour l'envoi de donn�es au DAC (SPI_WriteToDac)
#include "Mc32NVMUtil.h"    // Fonctions de lecture/�criture m�moire (non utilis�es ici)
#include "system_config.h"  // Configuration syst�me (FREQU_SYS, PRESC_TIM3, etc.)
#include "Mc32DriverLcd.h"  // Pour l'affichage (facultatif)

// -------------------------------------------------------------------------------------
// T.P. 2016 : On a 100 �chantillons par p�riode (au lieu de 40)
// -------------------------------------------------------------------------------------

// -------------------------------------------------------------------------------------
// int32_t SaturateMv(int32_t valMv)
// -------------------------------------------------------------------------------------
// Description : Borne la valeur 'valMv' � l'intervalle [-10000, +10000] mV, 
//               soit -10 V � +10 V avant conversion en code DAC.
// Param�tres :  valMv : valeur (en millivolts) � saturer
// Retour     :  la valeur satur�e dans [-10000, +10000]

int32_t SaturateMv(int32_t valMv) {
    if (valMv < AMPLITUDE_MIN) {
        return AMPLITUDE_MIN; // Valeur trop basse : on force � -10000 mV
    }
    if (valMv > AMPLITUDE_MAX) {
        return AMPLITUDE_MAX; // Valeur trop haute : on force � +10000 mV
    }
    return valMv; // Sinon, on la conserve telle quelle
}

// -------------------------------------------------------------------------------------
// Variables globales li�es au g�n�rateur
// -------------------------------------------------------------------------------------
S_ParamGen valParaGen; // Param�tres du g�n�rateur (fr�quence, amplitude, offset...)
int32_t tb_tabValSig[MAX_ECH]; // Tableau d'�chantillons en mV (satur�s � �10000)
int32_t tb_tabValSig2[MAX_ECH]; // Tableau d'�chantillons convertis pour le DAC [0..65535]

// -------------------------------------------------------------------------------------
// void GENSIG_Initialize(S_ParamGen *pParam)
// -------------------------------------------------------------------------------------
// Description : Initialise le g�n�rateur. 
//               Ici, pas de configuration sp�cifique : on se contente de savoir 
//               que l'ISR du Timer3 appellera GENSIG_Execute() pour sortir le signal.
// Param�tres :  pParam : pointeur vers la structure de param. du g�n�rateur (si besoin)
// Retour     :  aucun

void GENSIG_Initialize(S_ParamGen *pParam) {
    // Aucune initialisation particuli�re.
    // Si n�cessaire, on pourrait configurer le Timer ou initialiser d'autres ressources.
}

// -------------------------------------------------------------------------------------
// void GENSIG_UpdatePeriode(S_ParamGen *pParam)
// -------------------------------------------------------------------------------------
// Description : Calcule la p�riode du Timer3 pour obtenir la fr�quence souhait�e.
//               Le nombre total d'�chantillons (MAX_ECH) et le prescaler (PRESC_TIM3)
//               d�terminent le d�bit d'envoi au DAC, et donc la fr�quence de sortie.
// Param�tres :  pParam->Frequence : la fr�quence du signal en Hz
// Retour     :  aucun

void GENSIG_UpdatePeriode(S_ParamGen *pParam) {
    // Calcul de la fr�quence effective du Timer3 (apr�s prescaler)
    uint32_t timerFreq = F_SYS / PRESCALER;

    // Fr�quence d'�chantillonnage n�cessaire (nombre d'interruptions par seconde)
    uint32_t sampleRate = (uint32_t) pParam->Frequence * MAX_ECH;

    // Calcul de la p�riode en ticks du Timer3 avec arrondi
    // On ajoute la moiti� du sampleRate pour obtenir un arrondi classique
    uint16_t periode = (uint16_t) ((timerFreq + (sampleRate / 2)) / sampleRate) - 1;

    // Mise � jour du registre de p�riode du Timer3 via la PLIB
    PLIB_TMR_Period16BitSet(TMR_ID_3, periode);
}


// -------------------------------------------------------------------------------------
// void GENSIG_UpdateSignal(S_ParamGen *pParam)
// -------------------------------------------------------------------------------------
// Description : En fonction de la forme du signal (sinus, triangle, dent de scie, carr�),
//               et des param�tres amplitude / offset, on remplit un tableau 
//               d'�chantillons en mV. Chaque �chantillon est born� � [-10000..+10000],
//               puis converti en code DAC [0..65535] pour la sortie analogique.
// Param�tres :  pParam->Forme      : type de signal (SignalSinus, etc.)
//               pParam->Amplitude  : amplitude cr�te en mV
//               pParam->Offset     : offset en mV (positif => signal mont�)
// Retour     :  aucun

void GENSIG_UpdateSignal(S_ParamGen *pParam) {
    uint8_t echantillons; // Index pour parcourir tb_tabValSig
    uint16_t amplitude = pParam->Amplitude; // amplitude cr�te en mV
    int16_t offset = pParam->Offset; // offset en mV (positif => d�cale vers le haut)
    uint8_t compt_sig;

    // Tableau pr�-calcul� du sinus (100 points). Valeurs comprises entre -1.0 et +1.0.
    const float tbSignalSinus[MAX_ECH] = {
        0.0000, 0.0628, 0.1253, 0.1874, 0.2487, 0.3090, 0.3681, 0.4258, 0.4818, 0.5358,
        0.5878, 0.6374, 0.6845, 0.7290, 0.7705, 0.8090, 0.8443, 0.8763, 0.9048, 0.9298,
        0.9511, 0.9686, 0.9823, 0.9921, 0.9980, 1.0000, 0.9980, 0.9921, 0.9823, 0.9686,
        0.9511, 0.9298, 0.9048, 0.8763, 0.8443, 0.8090, 0.7705, 0.7290, 0.6845, 0.6374,
        0.5878, 0.5358, 0.4818, 0.4258, 0.3681, 0.3090, 0.2487, 0.1874, 0.1253, 0.0628,
        0.0000, -0.0628, -0.1253, -0.1874, -0.2487, -0.3090, -0.3681, -0.4258, -0.4818, -0.5358,
        -0.5878, -0.6374, -0.6845, -0.7290, -0.7705, -0.8090, -0.8443, -0.8763, -0.9048, -0.9298,
        -0.9511, -0.9686, -0.9823, -0.9921, -0.9980, -1.0000, -0.9980, -0.9921, -0.9823, -0.9686,
        -0.9511, -0.9298, -0.9048, -0.8763, -0.8443, -0.8090, -0.7705, -0.7290, -0.6845, -0.6374,
        -0.5878, -0.5358, -0.4818, -0.4258, -0.3681, -0.3090, -0.2487, -0.1874, -0.1253, -0.0628
    };

    // Choix de la forme du signal en fonction de pParam->Forme
    switch (pParam->Forme) {
            // ------------------------------------------------------------------
            // 1) Forme sinus
            // ------------------------------------------------------------------
        case SignalSinus:
            for (echantillons = 0; echantillons < MAX_ECH; echantillons++) {
                // Valeur brute en mV : sinus * amplitude - offset
                // tbSignalSinus[echantillons] est dans [-1..+1]
                int32_t valMv = (int32_t) (tbSignalSinus[echantillons] * amplitude)
                        - offset;
                // On limite la valeur � [-10000..+10000]
                tb_tabValSig[echantillons] = SaturateMv(valMv);
            }
            break;

            // ------------------------------------------------------------------
            // 2) Forme triangle
            // ------------------------------------------------------------------
        case SignalTriangle:
            // Premi�re moiti� du triangle : mont�e de -amplitude � 0
            for (compt_sig = 0; compt_sig < (MAX_ECH / 2); compt_sig++) {
                // Equation pour la mont�e :
                //   valMv = (2 * amplitude * compt_sig)/(MAX_ECH/2) - amplitude - offset
                // * (compt_sig) varie de 0 � (MAX_ECH/2 - 1)
                // * le terme "2 * amplitude / (MAX_ECH/2)" est la pente
                // * on soustrait "amplitude" pour partir � -amplitude
                // * on soustrait offset pour d�caler verticalement le signal
                int32_t valMv =
                        ((2 * (int32_t) amplitude * compt_sig) / (MAX_ECH / 2))
                        - amplitude
                        - offset;

                tb_tabValSig[compt_sig] = SaturateMv(valMv);
            }
            // Deuxi�me moiti� du triangle : descente de +amplitude � 0
            for (compt_sig = (MAX_ECH / 2); compt_sig < MAX_ECH; compt_sig++) {
                // Equation pour la descente :
                //   valMv = [ -2 * amplitude * (compt_sig - (MAX_ECH/2)) / (MAX_ECH/2) ] + amplitude - offset
                //
                // * "compt_sig - (MAX_ECH/2)" permet de repartir de 0 � la seconde moiti�
                //   (on "r�initialise" l'index pour cette partie)
                // * "-2 * amplitude" => pente n�gative
                // * "+ amplitude" => on part de +amplitude en d�but de seconde moiti�
                // * "- offset" => d�calage vertical
                int32_t valMv =
                        ((-2 * (int32_t) amplitude
                        * (compt_sig - (MAX_ECH / 2)))
                        / (MAX_ECH / 2))
                        + amplitude
                        - offset;

                tb_tabValSig[compt_sig] = SaturateMv(valMv);
            }
            break;

            // ------------------------------------------------------------------
            // 3) Forme dent de scie
            // ------------------------------------------------------------------
        case SignalDentDeScie:
            // G�n�re un rampement lin�aire de -amplitude � +amplitude
            // sur toute la p�riode (compt_sig = 0..MAX_ECH-1).
            for (compt_sig = 0; compt_sig < MAX_ECH; compt_sig++) {
                // valMv = ((amplitude * 2) * compt_sig / MAX_ECH) - amplitude - offset
                // * (compt_sig) varie de 0..(MAX_ECH-1)
                // * "2 * amplitude / MAX_ECH" est la pente d'�volution
                // * "- amplitude" pour d�marrer � -amplitude (lorsque compt_sig=0)
                // * "- offset" pour d�caler verticalement
                int32_t valMv =
                        (((int32_t) amplitude * 2 * compt_sig) / MAX_ECH)
                        - amplitude
                        - offset;

                tb_tabValSig[compt_sig] = SaturateMv(valMv);
            }
            break;

            // ------------------------------------------------------------------
            // 4) Forme carr�
            // ------------------------------------------------------------------
        case SignalCarre:
            // Dans un carr�, la premi�re moiti� des �chantillons (0..MAX_ECH/2-1)
            // est � un niveau haut (amplitude - offset), et la seconde moiti�
            // (MAX_ECH/2..MAX_ECH-1) est � un niveau bas (-amplitude - offset).
            for (compt_sig = 0; compt_sig < MAX_ECH; compt_sig++) {
                if (compt_sig < (MAX_ECH / 2)) {
                    // Cr�te haute
                    tb_tabValSig[compt_sig] =
                            SaturateMv((int32_t) amplitude - offset);
                }
                else {
                    // Cr�te basse
                    tb_tabValSig[compt_sig] =
                            SaturateMv(-(int32_t) amplitude - offset);
                }
            }
            break;

            // ------------------------------------------------------------------
            // Forme non pr�vue 
            // ------------------------------------------------------------------
        default:
            // Au besoin, on pourrait forcer la sortie � z�ro
            break;
    }

    // -----------------------------
    // Conversion mV -> DAC
    // -----------------------------
    // On mappe l'intervalle [-10000..+10000] mV dans [0..65535].
    // AMPLITUDE_MIN = -10000, PAS_MAXIMUM = 65535.
    // Ex. valMv = -10000 => code DAC = 0
    //     valMv = +10000 => code DAC = 65535
    for (echantillons = 0; echantillons < MAX_ECH; echantillons++) {
        tb_tabValSig2[echantillons] =
                (((tb_tabValSig[echantillons] - AMPLITUDE_MIN) * PAS_MAXIMUM) / 20000);
    }
}

// -------------------------------------------------------------------------------------
// void GENSIG_Execute(void)
// -------------------------------------------------------------------------------------
// Description : Fonction g�n�ralement appel�e dans l'ISR du Timer3. 
//               � chaque interruption, on envoie l'�chantillon courant au DAC,
//               puis on incr�mente l'index pour pointer vers l'�chantillon suivant.
// Param�tres :  aucun
// Retour     :  aucun

void GENSIG_Execute(void) {
    static uint16_t EchNb = 0; // Index de l'�chantillon en cours, incr�ment� � chaque appel

    // Envoie la valeur tb_tabValSig2[EchNb] sur le DAC (canal 0)
    SPI_WriteToDac(0, (uint16_t) tb_tabValSig2[EchNb]);
    EchNb++; // Passe � l'�chantillon suivant
    EchNb = EchNb % MAX_ECH; // G�re les d�bordements du nombre d'�chantillon
}
