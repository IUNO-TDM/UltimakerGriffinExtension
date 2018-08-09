#ifndef SRC_CRYPTOHELPERS_H_
#define SRC_CRYPTOHELPERS_H_

#include "cryptobuffer.h"
#include <string>

class CryptoHelpers {
    public:
        static void Unbase64(const std::string& in, CryptoBuffer& out);
        static std::string CmErrorCodeAsString();
        static void CmDecrypt(unsigned long firm_code, unsigned long product_id, CryptoBuffer& buffer);
        static void RsaDecrypt(const char* rsa_private_key, const CryptoBuffer& in, size_t expected_size, CryptoBuffer& out);
        static void AesDecrypt(const CryptoBuffer& aes_key, CryptoBuffer& iv, CryptoBuffer& buffer, bool finalize);

    private:
        CryptoHelpers(){}
        ~CryptoHelpers(){}
};

#endif
