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
    void prepare_decrypt();
    void PrepareDecryptPrivate(const CryptoBuffer& enc_aes_key);
    bool DecryptNextBlock();

    unsigned long product_id_;
    CryptoBuffer crypto_header_;
    CryptoBuffer gcode_header_;
    CryptoBuffer gcode_body_enc_;
    CryptoBuffer aes_key_;
    CryptoBuffer iv_;
    size_t offset_header_;
    size_t offset_body_;
    CryptoBuffer last_block_;
    CryptoBuffer current_line_;
    size_t bytes_read_;
    bool decrypt_prepared_;

    static const unsigned firmcode_ = 6000274;
    //static const unsigned firmcode_ = 6000010;
    static const size_t size_of_enc_aes_key_ = 512;
    static const size_t size_of_iv_ = 16;
    static const size_t crypto_header_size_ = size_of_enc_aes_key_ + size_of_iv_;
};

extern "C"{
    EncryptedFileReader* create_efr(const char* file_path, char* error_text, size_t error_text_size);
    bool read_line(EncryptedFileReader* efr, char* buffer, size_t* size);
    size_t get_bytes_read(EncryptedFileReader* efr);
    size_t get_bytes_total(EncryptedFileReader* efr);
    void destroy_efr(EncryptedFileReader* efr);
}

#endif /* SRC_ENCRYPTEDFILEREADER_H_ */
