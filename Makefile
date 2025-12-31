#
# OpenDMR - Open Source DMR (AMBE+2) Vocoder Library
#
# Makefile for building the library and test tools
#

# Compiler settings
CXX = g++
CC = gcc
CXXFLAGS = -O3 -std=c++11 -Wall -fPIC
CFLAGS = -O3 -Wall -fPIC

# Include paths
INCLUDES = -I. -Idecoder -Iencoder

# Library paths
LDFLAGS = -lm

# Platform detection
UNAME_S := $(shell uname -s)
ifeq ($(UNAME_S),Darwin)
    # macOS
    SHARED_EXT = dylib
    SHARED_FLAGS = -dynamiclib -install_name @rpath/libopendmr.$(SHARED_EXT)
else
    # Linux
    SHARED_EXT = so
    SHARED_FLAGS = -shared -Wl,-soname,libopendmr.$(SHARED_EXT).1
endif

# Output files
STATIC_LIB = libopendmr.a
SHARED_LIB = libopendmr.$(SHARED_EXT)
TEST_TOOL = dmr_codec

# Source files
OPENDMR_SRCS = opendmr.cpp

# Decoder sources (from mbelib-neo)
# DMR AMBE+2 (3600x2450) only
DECODER_SRCS = decoder/mbelib.c \
               decoder/mbe_adaptive.c \
               decoder/mbe_unvoiced_fft.c \
               decoder/ambe3600x2450.c \
               decoder/ambe_common.c \
               decoder/ecc.c \
               decoder/ecc_const.c \
               decoder/pffft.c \
               decoder/fftpack.c

# Encoder wrapper sources (from OP25 MBEEncoder)
ENCODER_SRCS = encoder/cgolay24128.cpp \
               encoder/mbeenc.cpp

# IMBE vocoder sources (encode path only, from OP25)
VOCODER_SRCS = encoder/aux_sub.cc \
               encoder/basicop2.cc \
               encoder/ch_encode.cc \
               encoder/dc_rmv.cc \
               encoder/dsp_sub.cc \
               encoder/encode.cc \
               encoder/imbe_vocoder.cc \
               encoder/imbe_vocoder_impl.cc \
               encoder/math_sub.cc \
               encoder/pe_lpf.cc \
               encoder/pitch_est.cc \
               encoder/pitch_ref.cc \
               encoder/qnt_sub.cc \
               encoder/rand_gen.cc \
               encoder/sa_encode.cc \
               encoder/tbls.cc \
               encoder/v_uv_det.cc

# Object files
OPENDMR_OBJS = $(OPENDMR_SRCS:.cpp=.o)
DECODER_OBJS = $(DECODER_SRCS:.c=.o)
ENCODER_OBJS = $(ENCODER_SRCS:.cpp=.o)
VOCODER_OBJS = $(VOCODER_SRCS:.cc=.o)

ALL_OBJS = $(OPENDMR_OBJS) $(DECODER_OBJS) $(ENCODER_OBJS) $(VOCODER_OBJS)

# Default target
all: $(STATIC_LIB) $(SHARED_LIB) $(TEST_TOOL)

# Static library
$(STATIC_LIB): $(ALL_OBJS)
	ar rcs $@ $^

# Shared library
$(SHARED_LIB): $(ALL_OBJS)
	$(CXX) $(SHARED_FLAGS) -o $@ $^ $(LDFLAGS)

# Test tool (statically linked)
$(TEST_TOOL): dmr_codec.cpp $(STATIC_LIB)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -o $@ $< $(STATIC_LIB) $(LDFLAGS)

# Compile rules
%.o: %.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

%.o: %.cc
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

%.o: %.c
	$(CC) $(CFLAGS) $(INCLUDES) -Wno-unused-but-set-variable -c $< -o $@

# Clean
clean:
	rm -f $(ALL_OBJS) $(STATIC_LIB) $(SHARED_LIB) $(TEST_TOOL)
	rm -f opendmr.o dmr_codec.o
	rm -f decoder/*.o encoder/*.o

# Install (to /usr/local by default)
PREFIX ?= /usr/local
install: $(STATIC_LIB) $(SHARED_LIB)
	install -d $(PREFIX)/lib
	install -d $(PREFIX)/include
	install -m 644 $(STATIC_LIB) $(PREFIX)/lib/
	install -m 755 $(SHARED_LIB) $(PREFIX)/lib/
	install -m 644 opendmr.h $(PREFIX)/include/

# Uninstall
uninstall:
	rm -f $(PREFIX)/lib/$(STATIC_LIB)
	rm -f $(PREFIX)/lib/$(SHARED_LIB)
	rm -f $(PREFIX)/include/opendmr.h

.PHONY: all clean install uninstall
