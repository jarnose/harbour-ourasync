// g++ -std=c++17 -Wall -Wextra -I ../src ../src/ring/aes128.cpp test_aes128.cpp -o /tmp/aes && /tmp/aes
#include "hex.h"
#include "ring/aes128.h"

using namespace Oura;

static AesKey key(const std::string &hex)
{
    AesKey k;
    auto v = fromHex(hex);
    for (size_t i = 0; i < 16; ++i)
        k[i] = v[i];
    return k;
}

int main()
{
    // FIPS-197 appendix B.
    {
        auto p = fromHex("3243f6a8885a308d313198a2e0370734");
        AesBlock in;
        for (size_t i = 0; i < 16; ++i)
            in[i] = p[i];
        CHECK_EQ(toHex(aes128EncryptBlock(key("2b7e151628aed2a6abf7158809cf4f3c"), in)),
                 std::string("3925841d02dc09fbdc118597196a0b32"));
    }
    // FIPS-197 appendix C.1.
    {
        auto p = fromHex("00112233445566778899aabbccddeeff");
        AesBlock in;
        for (size_t i = 0; i < 16; ++i)
            in[i] = p[i];
        CHECK_EQ(toHex(aes128EncryptBlock(key("000102030405060708090a0b0c0d0e0f"), in)),
                 std::string("69c4e0d86a7b0430d8cdb78070b4c55a"));
    }
    // The open_oura known-answer vector: a real 15-byte ring nonce, PKCS5 padded.
    {
        auto nonce = fromHex("0e2d6a0a08c99b4365f458e6e97382");
        CHECK_EQ(nonce.size(), size_t(15));
        CHECK_EQ(toHex(encryptNonce(key("4431967d8bacc2659743142b68391d9a"), nonce.data(), nonce.size())),
                 std::string("a38a8772d3acb6db5c2b516dd56987c8"));
    }
    return finish("aes128");
}
