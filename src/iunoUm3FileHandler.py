from .gcodeFileHandler import GCodeFileHandler

import logging
import os

import subprocess

from .encryptedFileReader import *

log = logging.getLogger(__name__.split(".")[-1])
        
class IunoUm3FileHandler(GCodeFileHandler):

    # Public

    # @brief Open the file stream
    # @param codec_error defines how codec errors are handled. 
    #     'None' is the default and means strict error handling with raising a UnicodeError.
    #     'ignore' will ignore the error and continue.
    #     'replace' will replace the error character by the official U+FFFD REPLACEMENT CHARACTER
    #     See the python codecs documentation for more options and more details.  
    # @return Returns the filestream object
    def openFileStream(self, codec_error=None):
        return self.__efr

    # Protected

    # @brief Determines the filesize
    # @return Returns the size of the file in bytes
    def getFileSize(self):
        return self.__efr.size()

    # Private

    def __init__(self, filename):
        super().__init__(filename)
        self.__efr = EncryptedFileReader(filename)

    @classmethod
    def getSupportedFileExtensions(cls):
        return [".iunoum3"]
