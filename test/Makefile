# module name
MODULE = libspl

# dtest executable to build
DTEST = dtest-cxx17

# add any include directories
INCLUDES = -I../include

# add any library directories and files
LIB_DIRS = -L../lib/$(shell uname -s)-$(shell uname -m)
LIBS = -lspl
LIB_DEPEND = ../lib/$(shell uname -s)-$(shell uname -m)/libspl.so

CXX = g++
CPPFLAGS = -Werror -Wall -Winline -Wpedantic
CXXFLAGS = -std=c++17 -march=native -fPIC -fopenmp -pthread

################################################################################

LDFLAGS = -Wl,-E -Wl,-export-dynamic

DEPFLAGS = -MM

BUILD_DIR = build/$(shell uname -s)-$(shell uname -m)

SOURCES = $(wildcard *.dtest.cpp)
OBJ_FILES = $(SOURCES:%.dtest.cpp=$(BUILD_DIR)/%.dtest.so)

.PHONY : all clean clean-dep dtest

################################################################################

all : dtest $(OBJ_FILES)

ifndef nodep
include $(SOURCES:%.cpp=.dep/%.d)
else
ifneq ($(nodep), true)
include $(SOURCES:%.cpp=.dep/%.d)
endif
endif

# cleanup

clean :
	@rm -rf build
	@echo "Cleaned $(MODULE)/test/build/"

clean-dep :
	@rm -rf .dep
	@echo "Cleaned $(MODULE)/test/.dep/"

# dirs

.dep $(BUILD_DIR):
	@echo "MKDIR     $(MODULE)/test/$@/"
	@mkdir -p $@

# test

dtest:
	@[ ! -d "dtest" ] && git clone -q https://github.com/noahorensa/dtest.git || :
	@git -C dtest pull -q
	@$(MAKE) -C dtest --no-print-directory $(DTEST)

.dep/%.d : %.cpp | .dep
	@echo "DEP       $(MODULE)/test/$@"
	@set -e; rm -f $@; \
	$(CXX) $(DEPFLAGS) $(INCLUDES) $< > $@.$$$$; \
	sed 's,\($*\)\.o[ :]*,$(BUILD_DIR)/\1.so $@ : ,g' < $@.$$$$ > $@; \
	rm -f $@.$$$$

$(BUILD_DIR)/%.dtest.so : %.dtest.cpp | $(BUILD_DIR) dtest $(LIB_DEPEND)
	@echo "CXX       $(MODULE)/test/$@"
	@$(CXX) -shared $(CPPFLAGS) $(CXXFLAGS) $(EXTRACXXFLAGS) -Idtest/include $(INCLUDES) $< $(LD_FLAGS) $(LIB_DIRS) $(LIBS) -o $@
