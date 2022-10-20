#_*_ Makefile _*_

# System independent config
OUTPUT 		= a

INCLUDE_DIR = include
SRC_DIR 	= src
OUTPUT_DIR 	= out

OBJS = main.o database.o symboltable.o bitset.o dbms_networking.o parser.o


# System specific makefile
ifeq ($(OS),Windows_NT)
    detected_OS := Windows
else
    detected_OS := $(shell uname)
endif


ifeq ($(detected_OS), Windows)
	include make/windows.mk
endif

ifeq ($(detected_OS), Linux)
	include make/linux.mk
endif