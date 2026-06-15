USE_CCACHE ?= 1
CCACHE ?= ccache
CCACHE_TMPDIR ?= $(CURDIR)/.cache/ccache-tmp
CCACHE_DIR ?= $(CURDIR)/.cache/ccache

ifeq ($(USE_CCACHE),1)
export CCACHE_TEMPDIR := $(CCACHE_TMPDIR)
export CCACHE_DIR := $(CCACHE_DIR)
CC := $(CCACHE) avr-gcc
CXX := $(CCACHE) avr-g++
TEST_CXX := $(CCACHE) g++
else
CC := avr-gcc
CXX := avr-g++
TEST_CXX := g++
endif

MCU := atmega328p
F_CPU := 16000000UL


