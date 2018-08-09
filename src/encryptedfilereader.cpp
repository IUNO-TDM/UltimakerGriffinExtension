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

EncryptedFileReader::EncryptedFileReader(const char* file_path) : product_id_(0), offset_(0), bytes_read_(0){
    ERR_load_crypto_strings();
    OpenSSL_add_all_algorithms();
    OPENSSL_config(NULL);

    unsigned char product_id_bytes[4];
    const size_t size_of_enc_aes_key = 512;
    const size_t size_of_iv = 16;
    const size_t head_size = size_of_enc_aes_key + size_of_iv;
    CryptoBuffer head(head_size);
    {
        ifstream inf(file_path, ios::binary | ios::ate);
        if(!inf.good()){
            stringstream ss;
            ss << "File '" << file_path << "' could not be opened for read.";
            throw invalid_argument(ss.str());
        }
        size_t file_size = inf.tellg();
        size_t body_size = file_size - sizeof(product_id_bytes) - head_size;
        intermediate_.set_size(body_size);
        inf.seekg(0);
        inf.read(reinterpret_cast<char*>(product_id_bytes), sizeof(product_id_bytes));
        inf.read(reinterpret_cast<char*>(static_cast<unsigned char*>(head)), head_size);
        inf.read(reinterpret_cast<char*>(static_cast<unsigned char*>(intermediate_)), body_size);
    }

    // manually combining bytes from LE representation ... ;-)
    product_id_ = product_id_bytes[3];
    product_id_ = (product_id_ << 8) + product_id_bytes[2];
    product_id_ = (product_id_ << 8) + product_id_bytes[1];
    product_id_ = (product_id_ << 8) + product_id_bytes[0];

    CryptoHelpers::CmDecrypt(firmcode_, product_id_, head);

    CryptoBuffer enc_aes_key;
    enc_aes_key.set(head, size_of_enc_aes_key);
    PrepareDecryptPrivate(enc_aes_key);

    iv_.set(head+size_of_enc_aes_key, size_of_iv);
}

EncryptedFileReader::~EncryptedFileReader() {
    EVP_cleanup();
    ERR_free_strings();
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
    return intermediate_.size();
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

