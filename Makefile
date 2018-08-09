TARGET ?= libEncryptedFileReader.so
OS := $(shell uname)
BUILD_DIR ?= ./build
LIB_DIR ?= ./lib
OS_ID := $(shell grep '^ID=' /etc/os-release | sed s/ID=//)

WIBU ?= on
ifeq ($(WIBU), on)
FLAG_NO_WIBU =
WIBU_LIB_FLAGS = -lwibucm
else
FLAG_NO_WIBU = -DNO_WIBU
WIBU_LIB_FLAGS = 
endif

ENCRYPTION ?= on
ifeq ($(ENCRYPTION), on)
FLAG_NO_ENCRYPTION = $(FLAG_NO_WIBU)
ENCRYPTION_LIB_FLAGS = -lcrypto $(WIBU_LIB_FLAGS)
else
FLAG_NO_ENCRYPTION = -DNO_ENCRYPTION $(WIBU_LIB_FLAGS)
ENCRYPTION_LIB_FLAGS = 
endif

SRC_DIRS = ./src \
	$(shell find $(LIB_DIR) -type d -path \*src -not -path \*test\*)

3DP_LIBS = $(shell find $(LIB_DIR) -name *.a)
3DP_DIRS = $(dir $(3DP_LIBS))

SRCS = $(shell find $(SRC_DIRS) -name *.cpp -or -name *.c -or -name *.s)

ifeq ($(ENCRYPTION), on)
ifneq ("$(wildcard ./private_src/decrypt.cpp)","")
SRCS := $(filter-out ./src/decrypt.cpp,$(SRCS))
SRCS += ./private_src/decrypt.cpp
endif
else
SRCS := $(filter-out ./src/decrypt.cpp,$(SRCS))
SRCS := $(filter-out ./src/cryptobuffer.cpp,$(SRCS))
SRCS := $(filter-out ./src/cryptohelpers.cpp,$(SRCS))
endif

OBJS = $(SRCS:%=$(BUILD_DIR)/%.o)
DEPS = $(OBJS:.o=.d)
INC_DIRS = $(shell find $(SRC_DIRS) -type d)
INC_DIRS += $(3DP_DIRS)
INC_DIRS += ./lib
INC_FLAGS = $(addprefix -I,$(INC_DIRS))

ifeq ($(OS), Darwin)
# Run MacOS commands 
#LDFLAGS := -g -L/usr/local/opt/openssl/lib -L/usr/local/Cellar/boost/1.63.0/lib/ -lcrypto -lboost_system -lboost_regex -lboost_program_options -framework IOKit -framework CoreFoundation -framework WibuCmMacX
#INC_DIRS += /usr/local/opt/openssl/include
#INC_DIRS += /usr/local/Cellar/boost/1.63.0/include/
else
# check for Linux and run other commands
TESTER_LDFLAGS := -g -lpthread -Wl,-E 
LDFLAGS := -shared -g -lpthread $(ENCRYPTION_LIB_FLAGS) -Wl,-E 
endif

DOWNLOAD_FILES := $(shell find $(LIB_DIR) -name *.download)
DOWNLOADED_FILES := $(DOWNLOAD_FILES:%.download=%.downloaded)

CPPFLAGS ?= $(INC_FLAGS) -MMD -MP -std=c++11 -Wall -g -DOS_$(OS_ID) -DELPP_THREAD_SAFE $(FLAG_NO_ENCRYPTION) $(FLAG_NO_REALDRIVERS) -fPIC

%.downloaded: %.download
	$(MKDIR_P) $(dir $<)/downloaded/$(basename $(notdir $<))/src
	curl  -L $(shell cat $<) --output $(dir $<)/downloaded/$(basename $(notdir $<))/src/$(notdir $(shell cat $<))
	touch $@

all: $(BUILD_DIR)/$(TARGET)

$(BUILD_DIR)/$(TARGET): $(OBJS)
	@echo SRC Dirs: $(SRC_DIRS)
	$(CXX) $(OBJS) -o $@ $(LDFLAGS)

# c source
$(BUILD_DIR)/%.c.o: %.c $(DOWNLOADED_FILES)
	$(MKDIR_P) $(dir $@)
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

# c++ source
$(BUILD_DIR)/%.cpp.o: %.cpp $(DOWNLOADED_FILES)
	$(MKDIR_P) $(dir $@)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c $< -o $@

.PHONY: realclean clean all list-srcs protect

realclean:
	$(RM) -r $(BUILD_DIR)
	$(RM) -r $(LIB_DIR)/downloaded
	$(RM) $(LIB_DIR)/*.downloaded

clean:
	$(RM) -r $(BUILD_DIR)

list-srcs:
	@echo $(SRCS)
	
protect: all ./private_src/encryptedfilereader.wbc
	AxProtectorLin @./private_src/encryptedfilereader.wbc

test: build/TestRunner

build/TestRunner: all ./test/testrunner.cpp ./test/TestRunner.py ./test/test.iunoum3
	$(MKDIR_P) $(dir $@)
	$(CXX) -I src ./test/testrunner.cpp -L$(BUILD_DIR) -lEncryptedFileReader -o $@ $(TESTER_LDFLAGS)
	ln -sf ../test/TestRunner.py build/
	ln -sf ../test/test.iunoum3 build/

.PHONY: importLibs
importLibs: $(DOWNLOADED_FILES)

build: $(BUILD_DIR)/$(TARGET)

-include $(DEPS)

MKDIR_P ?= mkdir -p
