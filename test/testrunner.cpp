#include "encryptedfilereader.h"
#include <iostream>

using namespace std;

int main(int argc, const char* argv[]){

    int rv = 0;

    const char* filename = "test.iunoum3";
    if(argc >= 2){
        filename = argv[1];
    }
    cout << "Input file: '" << filename << "'." << endl;

    size_t buffer_size = 8;
    char error_text[1024];
    EncryptedFileReader* efr = create_efr(filename, error_text, sizeof(error_text));
    if(efr){
        char* buffer = new char[buffer_size];
        cout << "=======================================" << endl;
        bool success = true;
        while(success){
            size_t size = buffer_size;
            success = read_line(efr, buffer, &size);
            if(success){
                cout << ">>>" << buffer << "<<<" <<endl;
            }else{
                if(size){
                    while(buffer_size < size){
                        buffer_size = buffer_size * 2;
                    }
                    delete[] buffer;
                    buffer = new char[buffer_size];
                    success = true;
                }
            }
        }
        cout << "=======================================" << endl;
        delete[] buffer;
        destroy_efr(efr);
    }else{
        cerr << error_text << endl;
        rv = 1;
    }

    return 0;
}
