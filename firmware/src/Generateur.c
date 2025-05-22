// =====================================================================================
// Canevas manipulation GenSig avec menu
// C. HUBER  09/02/2015
// Fichier Generateur.C
// Gestion du générateur
//
// Prévu pour signal de 40 échantillons
// Migration sur PIC32 30.04.2014 C. Huber
// =====================================================================================

#include "Generateur.h"     // Définitions des fonctions du générateur (prototypes)
#include "DefMenuGen.h"     // Définitions générales : structure S_ParamGen, etc.
#include "Mc32gestSpiDac.h" // Fonctions pour l'envoi de données au DAC (SPI_WriteToDac)
#include "Mc32NVMUtil.h"    // Fonctions de lecture/écriture mémoire (non utilisées ici)
#include "system_config.h"  // Configuration système (FREQU_SYS, PRESC_TIM3, etc.)
#include "Mc32DriverLcd.h"  // Pour l'affichage (facultatif)

// -------------------------------------------------------------------------------------
// T.P. 2016 : On a 100 échantillons par période (au lieu de 40)
// -------------------------------------------------------------------------------------

// -------------------------------------------------------------------------------------
// int32_t SaturateMv(int32_t valMv)
// -------------------------------------------------------------------------------------
// Description : Borne la valeur 'valMv' à l'intervalle [-10000, +10000] mV, 
//               soit -10 V à +10 V avant conversion en code DAC.
// Paramètres :  valMv : valeur (en millivolts) à saturer
// Retour     :  la valeur saturée dans [-10000, +10000]

int32_t SaturateMv(int32_t valMv) {
    if (valMv < AMPLITUDE_MIN) {
        return AMPLITUDE_MIN; // Valeur trop basse : on force à -10000 mV
    }
    if (valMv > AMPLITUDE_MAX) {
        return AMPLITUDE_MAX; // Valeur trop haute : on force à +10000 mV
    }
    return valMv; // Sinon, on la conserve telle quelle
}

// -------------------------------------------------------------------------------------
// Variables globales liées au générateur
// -------------------------------------------------------------------------------------
S_ParamGen valParaGen; // Paramètres du générateur (fréquence, amplitude, offset...)
int32_t tb_tabValSig[MAX_ECH]; // Tableau d'échantillons en mV (saturés à ±10000)
int32_t tb_tabValSig2[MAX_ECH]; // Tableau d'échantillons convertis pour le DAC [0..65535]

// -------------------------------------------------------------------------------------
// void GENSIG_Initialize(S_ParamGen *pParam)
// -------------------------------------------------------------------------------------
// Description : Initialise le générateur. 
//               Ici, pas de configuration spécifique : on se contente de savoir 
//               que l'ISR du Timer3 appellera GENSIG_Execute() pour sortir le signal.
// Paramètres :  pParam : pointeur vers la structure de param. du générateur (si besoin)
// Retour     :  aucun

void GENSIG_Initialize(S_ParamGen *pParam) {
    // Aucune initialisation particulière.
    // Si nécessaire, on pourrait configurer le Timer ou initialiser d'autres ressources.
}

// -------------------------------------------------------------------------------------
// void GENSIG_UpdatePeriode(S_ParamGen *pParam)
// -------------------------------------------------------------------------------------
// Description : Calcule la période du Timer3 pour obtenir la fréquence souhaitée.
//               Le nombre total d'échantillons (MAX_ECH) et le prescaler (PRESC_TIM3)
//               déterminent le débit d'envoi au DAC, et donc la fréquence de sortie.
// Paramètres :  pParam->Frequence : la fréquence du signal en Hz
// Retour     :  aucun

void GENSIG_UpdatePeriode(S_ParamGen *pParam) {
    // Calcul de la fréquence effective du Timer3 (après prescaler)
    uint32_t timerFreq = F_SYS / PRESCALER;

    // Fréquence d'échantillonnage nécessaire (nombre d'interruptions par seconde)
    uint32_t sampleRate = (uint32_t) pParam->Frequence * MAX_ECH;

    // Calcul de la période en ticks du Timer3 avec arrondi
    // On ajoute la moitié du sampleRate pour obtenir un arrondi classique
    uint16_t periode = (uint16_t) ((timerFreq + (sampleRate / 2)) / sampleRate) - 1;

    // Mise à jour du registre de période du Timer3 via la PLIB
    PLIB_TMR_Period16BitSet(TMR_ID_3, periode);
}


// -------------------------------------------------------------------------------------
// void GENSIG_UpdateSignal(S_ParamGen *pParam)
// -------------------------------------------------------------------------------------
// Description : En fonction de la forme du signal (sinus, triangle, dent de scie, carré),
//               et des paramètres amplitude / offset, on remplit un tableau 
//               d'échantillons en mV. Chaque échantillon est borné à [-10000..+10000],
//               puis converti en code DAC [0..65535] pour la sortie analogique.
// Paramètres :  pParam->Forme      : type de signal (SignalSinus, etc.)
//               pParam->Amplitude  : amplitude crête en mV
//               pParam->Offset     : offset en mV (positif => signal monté)
// Retour     :  aucun

void GENSIG_UpdateSignal(S_ParamGen *pParam) {
    uint8_t echantillons; // Index pour parcourir tb_tabValSig
    uint16_t amplitude = pParam->Amplitude; // amplitude crête en mV
    int16_t offset = pParam->Offset; // offset en mV (positif => décale vers le haut)
    uint8_t compt_sig;

    // Tableau pré-calculé du sinus (100 points). Valeurs comprises entre -1.0 et +1.0.
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
                // On limite la valeur à [-10000..+10000]
                tb_tabValSig[echantillons] = SaturateMv(valMv);
            }
            break;

            // ------------------------------------------------------------------
            // 2) Forme triangle
            // ------------------------------------------------------------------
        case SignalTriangle:
            // Première moitié du triangle : montée de -amplitude à 0
            for (compt_sig = 0; compt_sig < (MAX_ECH / 2); compt_sig++) {
                // Equation pour la montée :
                //   valMv = (2 * amplitude * compt_sig)/(MAX_ECH/2) - amplitude - offset
                // * (compt_sig) varie de 0 à (MAX_ECH/2 - 1)
                // * le terme "2 * amplitude / (MAX_ECH/2)" est la pente
                // * on soustrait "amplitude" pour partir à -amplitude
                // * on soustrait offset pour décaler verticalement le signal
                int32_t valMv =
                        ((2 * (int32_t) amplitude * compt_sig) / (MAX_ECH / 2))
                        - amplitude
                        - offset;

                tb_tabValSig[compt_sig] = SaturateMv(valMv);
            }
            // Deuxième moitié du triangle : descente de +amplitude à 0
            for (compt_sig = (MAX_ECH / 2); compt_sig < MAX_ECH; compt_sig++) {
                // Equation pour la descente :
                //   valMv = [ -2 * amplitude * (compt_sig - (MAX_ECH/2)) / (MAX_ECH/2) ] + amplitude - offset
                //
                // * "compt_sig - (MAX_ECH/2)" permet de repartir de 0 à la seconde moitié
                //   (on "réinitialise" l'index pour cette partie)
                // * "-2 * amplitude" => pente négative
                // * "+ amplitude" => on part de +amplitude en début de seconde moitié
                // * "- offset" => décalage vertical
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
            // Génère un rampement linéaire de -amplitude à +amplitude
            // sur toute la période (compt_sig = 0..MAX_ECH-1).
            for (compt_sig = 0; compt_sig < MAX_ECH; compt_sig++) {
                // valMv = ((amplitude * 2) * compt_sig / MAX_ECH) - amplitude - offset
                // * (compt_sig) varie de 0..(MAX_ECH-1)
                // * "2 * amplitude / MAX_ECH" est la pente d'évolution
                // * "- amplitude" pour démarrer à -amplitude (lorsque compt_sig=0)
                // * "- offset" pour décaler verticalement
                int32_t valMv =
                        (((int32_t) amplitude * 2 * compt_sig) / MAX_ECH)
                        - amplitude
                        - offset;

                tb_tabValSig[compt_sig] = SaturateMv(valMv);
            }
            break;

            // ------------------------------------------------------------------
            // 4) Forme carré
            // ------------------------------------------------------------------
        case SignalCarre:
            // Dans un carré, la première moitié des échantillons (0..MAX_ECH/2-1)
            // est à un niveau haut (amplitude - offset), et la seconde moitié
            // (MAX_ECH/2..MAX_ECH-1) est à un niveau bas (-amplitude - offset).
            for (compt_sig = 0; compt_sig < MAX_ECH; compt_sig++) {
                if (compt_sig < (MAX_ECH / 2)) {
                    // Crête haute
                    tb_tabValSig[compt_sig] =
                            SaturateMv((int32_t) amplitude - offset);
                }
                else {
                    // Crête basse
                    tb_tabValSig[compt_sig] =
                            SaturateMv(-(int32_t) amplitude - offset);
                }
            }
            break;

            // ------------------------------------------------------------------
            // Forme non prévue 
            // ------------------------------------------------------------------
        default:
            // Au besoin, on pourrait forcer la sortie à zéro
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
// Description : Fonction généralement appelée dans l'ISR du Timer3. 
//               À chaque interruption, on envoie l'échantillon courant au DAC,
//               puis on incrémente l'index pour pointer vers l'échantillon suivant.
// Paramètres :  aucun
// Retour     :  aucun

void GENSIG_Execute(void) {
    static uint16_t EchNb = 0; // Index de l'échantillon en cours, incrémenté à chaque appel

    // Envoie la valeur tb_tabValSig2[EchNb] sur le DAC (canal 0)
    SPI_WriteToDac(0, (uint16_t) tb_tabValSig2[EchNb]);
    EchNb++; // Passe à l'échantillon suivant
    EchNb = EchNb % MAX_ECH; // Gére les débordements du nombre d'échantillon
}
