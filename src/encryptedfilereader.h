#ifndef SRC_ENCRYPTEDFILEREADER_H_
#define SRC_ENCRYPTEDFILEREADER_H_

#include "cryptobuffer.h"

class EncryptedFileReader {
public:
    EncryptedFileReader(const char* file_path);
    virtual ~EncryptedFileReader();
    bool ReadLine(char* buffer, size_t& size);
    size_t GetBytesRead();
    size_t GetBytesTotal();
private:
    void PrepareDecryptPrivate(const CryptoBuffer& enc_aes_key);
    bool DecryptNextBlock();

    unsigned long product_id_;
    CryptoBuffer intermediate_;
    CryptoBuffer aes_key_;
    CryptoBuffer iv_;
    size_t offset_;
    CryptoBuffer last_block_;
    CryptoBuffer current_line_;
    size_t bytes_read_;
    static const unsigned firmcode_ = 6000274;
    //static const unsigned firmcode_ = 6000010;
};

extern "C"{
    EncryptedFileReader* create_efr(const char* file_path, char* error_text, size_t error_text_size);
    bool read_line(EncryptedFileReader* efr, char* buffer, size_t* size);
    size_t get_bytes_read(EncryptedFileReader* efr);
    size_t get_bytes_total(EncryptedFileReader* efr);
    void destroy_efr(EncryptedFileReader* efr);
}

#endif /* SRC_ENCRYPTEDFILEREADER_H_ */
