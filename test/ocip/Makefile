BUILDTYPE := Debug
TARGET_DIR := Build/$(BUILDTYPE)
CC := gcc
ORACLE_HOME := /home/doberkofler/oracle/instantclient_12_1

# Defines
DEFS_Debug := \
	'-D_LARGEFILE_SOURCE' \
	'-D_FILE_OFFSET_BITS=64' \
	'-DUSE_LINUX=1' \
	'-DUSE_ORACLE_DUMMY=0' \
	'-DDEBUG' \
	'-D_DEBUG'

# Flags passed to all source files.
CFLAGS_Debug := \
	-fPIC \
	-Wall \
	-Wextra \
	-Wno-unused-parameter \
	-pthread \
	-m64 \
	-g \
	-O0

# Flags passed to only C++ files.
CFLAGS_CC_Debug := \
	-fno-rtti

INCS_Debug := \
	-I../../src \
	-I$(ORACLE_HOME)/sdk/include

DEFS_Release := \
	'-D_LARGEFILE_SOURCE' \
	'-D_FILE_OFFSET_BITS=64' \
	'-DUSE_LINUX=1' \
	'-DUSE_ORACLE_DUMMY=0'

# Flags passed to all source files.
CFLAGS_Release := \
	-fPIC \
	-Wall \
	-Wextra \
	-Wno-unused-parameter \
	-pthread \
	-m64 \
	-O2 \
	-fno-strict-aliasing \
	-fno-tree-vrp \
	-fno-omit-frame-pointer

# Flags passed to only C++ files.
CFLAGS_CC_Release := \
	-fno-rtti

INCS_Release := \
	-I../../src \
	-I$(ORACLE_HOME)/sdk/include

# Add to the list of files we specially track dependencies for.
all_deps += $(OBJS)

# CFLAGS et al overrides must be target-local.
# See "Target-specific Variable Values" in the GNU Make manual.
CFLAGS := $(DEFS_$(BUILDTYPE)) $(INCS_$(BUILDTYPE))  $(CFLAGS_$(BUILDTYPE)) $(CFLAGS_C_$(BUILDTYPE))
CXXFLAGS := $(DEFS_$(BUILDTYPE)) $(INCS_$(BUILDTYPE))  $(CFLAGS_$(BUILDTYPE)) $(CFLAGS_CC_$(BUILDTYPE))

### Rules for final target.
LDFLAGS_Debug := -pthread -rdynamic -m64 -lstdc++
LDFLAGS_Release := -pthread -rdynamic -m64 -lstdc++

LIBS := -lclntsh -L$(ORACLE_HOME)

LDFLAGS := $(LDFLAGS_$(BUILDTYPE))

OBJS := $(TARGET_DIR)/ocip.o $(TARGET_DIR)/oci_text.o $(TARGET_DIR)/oci_interface.o $(TARGET_DIR)/ocip_interface.o $(TARGET_DIR)/oracleError.o $(TARGET_DIR)/global.o
DEPS := ../../src/oci_text.h ../../src/oci_interface.h ../../src/ocip_interface.h ../../src/oracleError.h ../../src/global.h

# Compile
$(TARGET_DIR)/ocip.o: ocip.cc $(DEPS)
	$(CC) -c -o $@ $< $(CXXFLAGS)

$(TARGET_DIR)/oci_text.o: ../../src/oci_text.cc $(DEPS)
	$(CC) -c -o $@ $< $(CXXFLAGS)

$(TARGET_DIR)/oci_interface.o: ../../src/oci_interface.cc $(DEPS)
	$(CC) -c -o $@ $< $(CXXFLAGS)

$(TARGET_DIR)/ocip_interface.o: ../../src/ocip_interface.cc $(DEPS)
	$(CC) -c -o $@ $< $(CXXFLAGS)

$(TARGET_DIR)/oracleError.o: ../../src/oracleError.cc $(DEPS)
	$(CC) -c -o $@ $< $(CXXFLAGS)

$(TARGET_DIR)/global.o: ../../src/global.cc $(DEPS)
	$(CC) -c -o $@ $< $(CXXFLAGS)

# Link
ocip: $(OBJS)
	$(CC) -o ocip $(OBJS) $(CXXFLAGS) $(LDFLAGS) $(LIBS)

.PHONY: clean

clean:
	rm -f $(TARGET_DIR)/*.o
