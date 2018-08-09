import sys
import logging

from ctypes import *

log = logging.getLogger(__name__.split(".")[-1])

efrso=cdll.LoadLibrary("libEncryptedFileReader.so")

class EncryptedFileReader:
    def __init__(self, filename):
        self.__filename = filename
        
    def __enter__(self):
        log.info("Opening file '" + self.__filename + "'...")
        error_text_size=1024
        error_text = create_string_buffer(error_text_size)
        self.__efr = efrso.create_efr(self.__filename.encode("utf-8"), error_text, error_text_size);
        if self.__efr==0:
            raise RuntimeError (error_text.value.decode("utf-8"))
        self.__buffer_size=8
        self.__buffer = create_string_buffer(self.__buffer_size)
        log.info("Opened file '" + self.__filename + "' successfully.")
        return self

    def __exit__(self, exc_type, exc_val, exc_tb):
        log.info("Closing file '" + self.__filename + "'...")
        efrso.destroy_efr(self.__efr)
        log.info("Closed file '" + self.__filename + "' successfully.")
        
    def __iter__(self):
        return self

    def __next__(self):
        line = self.readline()
        if len(line)==0:
            raise StopIteration
        else:
            return line
        
    def readline(self):
        line = ""
        while True:
            size = c_size_t(self.__buffer_size)
            success = efrso.read_line(self.__efr, self.__buffer, byref(size))
            if success:
                line = self.__buffer.value.decode("utf-8")
                log.info("Read line: '" + line + "'")
                line = line + "\n"
                break
            else:
                if size.value>0:
                    while self.__buffer_size < size.value:
                        self.__buffer_size = 2 * self.__buffer_size
                    self.__buffer = create_string_buffer(self.__buffer_size)
                else:
                    log.info("End of file reached.")
                    break
        return line
    
    def tell(self):
        return efrso.get_bytes_read(self.__efr)
    
    def size(self):
        return efrso.get_bytes_total(self.__efr)
