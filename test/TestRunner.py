#!/usr/bin/python3 

import sys
from ctypes import *

efrso=cdll.LoadLibrary("libEncryptedFileReader.so")
    
rv = 0

filename = "test.iunoum3"
if len(sys.argv) >= 2:
    filename = sys.argv[1]
print("Input file: '" + filename + "'.")

error_text_size=1024
error_text = create_string_buffer(error_text_size)
efr = efrso.create_efr(filename.encode("utf-8"), error_text, error_text_size);
if efr!=0:
    buffer_size=8
    p = create_string_buffer(buffer_size)
    print("=======================================")
    success = True
    while success:
        size = c_size_t(buffer_size)
        success = efrso.read_line(efr, p, byref(size))
        if success:
            print(">>>" + p.value.decode("utf-8") + "<<<")
        else:
            if size.value>0:
                while buffer_size < size.value:
                    buffer_size = 2 * buffer_size
                p = create_string_buffer(buffer_size)
                success = True
    print("=======================================")
    efrso.destroy_efr(efr)
else:
    print(error_text.value.decode("utf-8"))
    rv = 1

exit(rv)
