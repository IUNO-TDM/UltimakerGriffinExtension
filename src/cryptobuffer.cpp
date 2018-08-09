#include "cryptobuffer.h"

#include <string.h>

using namespace std;

CryptoBuffer::CryptoBuffer(): buffer_(NULL), size_(0){
}

CryptoBuffer::CryptoBuffer(size_t size){
    buffer_ = new unsigned char[size+1]; // +1 .. space for forced zero termination
    size_= size;
    memset(buffer_, 0, size_+1);
}

CryptoBuffer::CryptoBuffer(const CryptoBuffer& other): buffer_(NULL), size_(0){
	set(other.buffer_, other.size_);
}

CryptoBuffer::~CryptoBuffer(){
    clear();
}

CryptoBuffer& CryptoBuffer::operator = (const CryptoBuffer& other){
	set(other.buffer_, other.size_);
	return *this;
}

void CryptoBuffer::clear() {
    if(size_){
        memset(buffer_, 0, size_+1);
        delete[] buffer_;
        buffer_ = NULL;
        size_ = 0;
    }
}

void CryptoBuffer::set(const unsigned char* buffer, size_t size){
    if(size_){
        memset(buffer_, 0, size_+1);
        delete[] buffer_;
        buffer_ = NULL;
        size_ = 0;
    }

    if(size){
        buffer_ = new unsigned char[size+1]; // +1 .. space for forced zero termination
        size_= size;
        memcpy(buffer_, buffer, size_);
        buffer_[size_] = 0;
    }
}

void CryptoBuffer::set(const string& in){
    set(reinterpret_cast<const unsigned char*>(in.c_str()), in.length());
}

void CryptoBuffer::set_size(size_t size){
    unsigned char* tmp = buffer_;
    buffer_ = new unsigned char[size+1];
    if(size>size_){
        if(size_){
            memcpy(buffer_, tmp, size_);
        }
        memset(buffer_+size_, 0, size-size_+1);
    }else{
        if(size){
            memcpy(buffer_, tmp, size);
        }
        buffer_[size_] = 0;
    }
    if(size_){
        memset(tmp, 0, size_+1);
        delete[] tmp;
    }
    size_=size;
}

void CryptoBuffer::append(const unsigned char* buffer, size_t size){
    if(size){
        size_t old_size = size_;
        set_size(size_ + size);
        memcpy(buffer_+old_size, buffer, size);
    }
}

void CryptoBuffer::append(const CryptoBuffer& other){
    append(other.buffer_, other.size_);
}

