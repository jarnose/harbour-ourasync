#pragma once

#include <array>
#include <cstddef>
#include <cstdint>

// AES-128, encrypt direction only, one block at a time.
//
// The ring authenticates the app with a challenge: it sends a 15-byte nonce,
// the app encrypts it with the shared 16-byte key (AES/ECB/PKCS5Padding, which
// for 15 bytes of input is exactly one block) and returns the ciphertext. That
// is the only cryptography this protocol asks of the client, so a 100-line
// table implementation is used rather than a dependency on OpenSSL, whose
// development headers are not part of every Sailfish build target.
//
// Not constant-time. The key never leaves the phone and the ring is the only
// party that ever sees ciphertexts, so a timing channel has nobody to leak to.
namespace Oura {

using AesKey = std::array<uint8_t, 16>;
using AesBlock = std::array<uint8_t, 16>;

AesBlock aes128EncryptBlock(const AesKey &key, const AesBlock &plain);

// The ring's nonce encryption: PKCS#7-pad `nonce` (at most 16 bytes) into one
// block, then encrypt it. Mirrors the Android app's AES/ECB/PKCS5Padding.
AesBlock encryptNonce(const AesKey &key, const uint8_t *nonce, size_t length);

} // namespace Oura
