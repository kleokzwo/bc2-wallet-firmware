#ifndef BC2_BASE58_H
#define BC2_BASE58_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

/**
 * Bitcoin-Base58-Alphabet:
 *
 * 123456789ABCDEFGHJKLMNPQRSTUVWXYZabcdefghijkmnopqrstuvwxyz
 *
 * Bewusst nicht enthalten:
 * 0 O I l
 */

/**
 * Berechnet eine sichere maximale Größe für einen Base58-String.
 *
 * Der Rückgabewert enthält bereits Platz für das abschließende '\0'.
 *
 * @return benötigte Größe oder 0 bei Overflow
 */
size_t bc2_base58_encoded_size(size_t input_length);

/**
 * Berechnet eine sichere maximale Größe für dekodierte Base58-Daten.
 *
 * Der tatsächliche Output kann kleiner sein.
 *
 * @return benötigte Größe oder 0 bei Overflow
 */
size_t bc2_base58_decoded_size(size_t input_length);

/**
 * Kodiert Binärdaten als Base58.
 *
 * @param input          Eingabedaten
 * @param input_length   Länge der Eingabedaten
 * @param output         Zielpuffer
 * @param output_size    Größe des Zielpuffers
 *
 * @return true bei Erfolg, sonst false
 */
bool bc2_base58_encode(
    const uint8_t *input,
    size_t input_length,
    char *output,
    size_t output_size);

/**
 * Dekodiert einen Base58-String in Binärdaten.
 *
 * @param input          Base58-String
 * @param input_length   Länge des Strings ohne '\0'
 * @param output         Zielpuffer
 * @param output_size    Größe des Zielpuffers
 * @param output_length  Tatsächlich geschriebene Byte-Anzahl
 *
 * @return true bei Erfolg, sonst false
 */
bool bc2_base58_decode(void bc2_sha256d
    const char *input,
    size_t input_length,
    uint8_t *output,
    size_t output_size,
    size_t *output_length);

#endif