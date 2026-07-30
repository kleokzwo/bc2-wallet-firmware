#ifndef BC2_BASE58CHECK_H
#define BC2_BASE58CHECK_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

enum
{
  BC2_BASE58CHECK_CHECKSUM_SIZE = 4,
  BC2_BASE58CHECK_MAX_PAYLOAD_SIZE = 128
};

/**
 * Berechnet die maximal benötigte Größe für einen
 * Base58Check-kodierten String.
 *
 * Der Rückgabewert enthält Platz für das abschließende '\0'.
 *
 * @return benötigte Größe oder 0 bei Overflow
 */
size_t bc2_base58check_encoded_size(size_t payload_length);

/**
 * Kodiert Binärdaten mit Base58Check.
 *
 * Ablauf:
 *
 * checksum = erste 4 Bytes von SHA256d(payload)
 * encoded  = Base58(payload || checksum)
 *
 * @param payload         zu kodierende Daten
 * @param payload_length  Länge der Daten
 * @param output          Zielpuffer für Base58Check-String
 * @param output_size     Größe des Zielpuffers
 *
 * @return true bei Erfolg, sonst false
 */
bool bc2_base58check_encode(
    const uint8_t *payload,
    size_t payload_length,
    char *output,
    size_t output_size);

/**
 * Dekodiert und validiert einen Base58Check-String.
 *
 * Die letzten vier dekodierten Bytes werden als Checksumme behandelt.
 * Die Checksumme wird gegen SHA256d(payload) geprüft.
 *
 * @param input           Base58Check-String
 * @param input_length    Länge ohne abschließendes '\0'
 * @param payload         Zielpuffer für dekodierte Nutzdaten
 * @param payload_size    Größe des Zielpuffers
 * @param payload_length  tatsächlich geschriebene Nutzdatenlänge
 *
 * @return true bei gültiger Checksumme, sonst false
 */
bool bc2_base58check_decode(
    const char *input,
    size_t input_length,
    uint8_t *payload,
    size_t payload_size,
    size_t *payload_length);

#endif