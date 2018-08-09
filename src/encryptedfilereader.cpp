#include "encryptedfilereader.h"

#include "cryptohelpers.h"

#include <fstream>
#include <iostream>
#include <cstring>
#include <sstream>
#include <stdexcept>
#include <openssl/conf.h>
#include <openssl/evp.h>
#include <openssl/err.h>
#include <openssl/buffer.h>

using namespace std;

EncryptedFileReader::EncryptedFileReader(const char* file_path) : product_id_(0), offset_header_(0), offset_body_(0), bytes_read_(0), decrypt_prepared_(false){
    ERR_load_crypto_strings();
    OpenSSL_add_all_algorithms();
    OPENSSL_config(NULL);

    crypto_header_.set_size(crypto_header_size_);
    {
        ifstream inf(file_path, ios::binary | ios::ate);
        if(!inf.good()){
            stringstream ss;
            ss << "File '" << file_path << "' could not be opened for read.";
            throw invalid_argument(ss.str());
        }
        size_t file_size = inf.tellg();
        inf.seekg(0);

        unsigned char product_id_bytes[4];
        inf.read(reinterpret_cast<char*>(product_id_bytes), sizeof(product_id_bytes));
        // manually combining bytes from LE representation ... ;-)
        product_id_ = product_id_bytes[3];
        product_id_ = (product_id_ << 8) + product_id_bytes[2];
        product_id_ = (product_id_ << 8) + product_id_bytes[1];
        product_id_ = (product_id_ << 8) + product_id_bytes[0];

        inf.read(reinterpret_cast<char*>(static_cast<unsigned char*>(crypto_header_)), crypto_header_size_);

        unsigned char gcode_hdr_len_bytes[4];
        inf.read(reinterpret_cast<char*>(gcode_hdr_len_bytes), sizeof(gcode_hdr_len_bytes));
        // manually combining bytes from LE representation ... ;-)
        size_t gcode_hdr_len = gcode_hdr_len_bytes[3];
        gcode_hdr_len = (gcode_hdr_len << 8) + gcode_hdr_len_bytes[2];
        gcode_hdr_len = (gcode_hdr_len << 8) + gcode_hdr_len_bytes[1];
        gcode_hdr_len = (gcode_hdr_len << 8) + gcode_hdr_len_bytes[0];

        gcode_header_.set_size(gcode_hdr_len);
        inf.read(reinterpret_cast<char*>(static_cast<unsigned char*>(gcode_header_)), gcode_hdr_len);

        size_t body_size = file_size - sizeof(product_id_bytes) - crypto_header_size_ - sizeof(gcode_hdr_len_bytes) - gcode_hdr_len;
        gcode_body_enc_.set_size(body_size);
        inf.read(reinterpret_cast<char*>(static_cast<unsigned char*>(gcode_body_enc_)), body_size);
    }
}

EncryptedFileReader::~EncryptedFileReader() {
    EVP_cleanup();
    ERR_free_strings();
}

void EncryptedFileReader::prepare_decrypt(){
    CryptoHelpers::CmDecrypt(firmcode_, product_id_, crypto_header_);

    CryptoBuffer enc_aes_key;
    enc_aes_key.set(crypto_header_, size_of_enc_aes_key_);
    PrepareDecryptPrivate(enc_aes_key);

    iv_.set(crypto_header_+size_of_enc_aes_key_, size_of_iv_);

    decrypt_prepared_=true;
}

bool EncryptedFileReader::ReadLine(char* buffer, size_t& size) {

    bool eof_found = false;

    if(current_line_.size() == 0){
        size_t eol_size = 1;
        bool eol_found = false;

        while(!eol_found && !eof_found)
        {
            size_t i=0;
            size_t last_block_size = last_block_.size();
            for(; i<last_block_size; i++){
                if(last_block_[i] == '\n'){
                    if(((i+1) < last_block_size) && (last_block_[i+1] == '\r')){
                        eol_size = 2;
                        i++;
                    }
                    eol_found = true;
                    break;
                } else if (last_block_[i] == '\r') {
                    if(((i+1) < last_block_size) && (last_block_[i+1] == '\n')){
                        eol_size = 2;
                        i++;
                    }
                    eol_found = true;
                    break;
                } else {
                    // nothing
                }
            }

            if(eol_found){
                current_line_.append(last_block_, i+1-eol_size);
                CryptoBuffer tmp;
                tmp.set(last_block_+i+1, last_block_size-(i+1));
                last_block_ = tmp;
                eol_size = 1;
            } else {
                current_line_.append(last_block_);
                eof_found = !DecryptNextBlock();
            }
        }
    }

    bool rv = false;

    if(size >= current_line_.size()+1){
        if(current_line_.size()){
            memcpy(buffer, current_line_, current_line_.size()+1);
        } else {
            buffer[0]=0;
        }
        size = current_line_.size();
        current_line_.clear();
        rv = (size>0) || !eof_found;
    } else {
        size = current_line_.size()+1;
        rv = false;
    }

    return rv;
}

size_t EncryptedFileReader::GetBytesRead(){
    return bytes_read_ - last_block_.size();
}

size_t EncryptedFileReader::GetBytesTotal(){
    return gcode_body_enc_.size();
}

EncryptedFileReader* create_efr(const char* file_path, char* error_text, size_t error_text_size){
    EncryptedFileReader* rv = NULL;
    try{
        rv = new EncryptedFileReader(file_path);
    }catch(exception& e){
        if(error_text_size){
            strncpy(error_text, e.what(), error_text_size);
            error_text[error_text_size-1]=0;
        }
    }
    return rv;
}
bool read_line(EncryptedFileReader* efr, char* buffer, size_t* size){
    return efr->ReadLine(buffer, *size);
}
size_t get_bytes_read(EncryptedFileReader* efr){
    return efr->GetBytesRead();
}
size_t get_bytes_total(EncryptedFileReader* efr){
    return efr->GetBytesTotal();
}
void destroy_efr(EncryptedFileReader* efr){
    delete efr;
}

