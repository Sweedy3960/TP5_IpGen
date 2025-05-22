#ifndef Mc32SpiUtil_H
#define Mc32SpiUtil_H
/*--------------------------------------------------------*/
// Mc32SpiUtil.h
/*--------------------------------------------------------*/
//	Description :	Utilitaire gestion SPI CCS like
//
//	Auteur 		: 	C. Huber
//	Version		:	V1.1
//	Compilateur	:	XC32 V1.40
//
/*--------------------------------------------------------*/
#include <stdint.h>
/*--------------------------------------------------------*/
// Prototypes des fonctions pour Mc32SpiUtil.h
/*--------------------------------------------------------*/

/**
 * @brief Envoie un octet via SPI1 sans lecture.
 * @param Val Octet à transmettre.
 */
void spi_write1(uint8_t Val);

/**
 * @brief Envoie un octet via SPI2 sans lecture.
 * @param Val Octet à transmettre.
 */
void spi_write2(uint8_t Val);

/**
 * @brief Envoie un octet via SPI1 et lit la réponse.
 * @param Val Octet à transmettre.
 * @return Octet reçu sur SPI1.
 */
uint8_t spi_read1(uint8_t Val);

/**
 * @brief Envoie un octet via SPI2 et lit la réponse.
 * @param Val Octet à transmettre.
 * @return Octet reçu sur SPI2.
 */
uint8_t spi_read2(uint8_t Val);


#endif
