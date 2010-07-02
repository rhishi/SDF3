#
#   TU Eindhoven
#   Eindhoven, The Netherlands
#
#   Author          :   Sander Stuijk (sander@ics.ele.tue.nl)
#
#   Date            :   March 29, 2002
#   
#   $Id: Makefile.inc,v 1.3 2008/09/18 07:38:21 sander Exp $

#-------------------------------------------------------------------------------
# Project structure
#-------------------------------------------------------------------------------

PROJECT_ROOT   = $(PRIVATE_CM_ROOT)/sdf3

#-------------------------------------------------------------------------------
# Component
#-------------------------------------------------------------------------------

COMPONENT_BASE = $(join $(subst /,_, $(subst $(subst //,/, $(join $(PROJECT_ROOT), /)),, $(PWD))), _)

#-------------------------------------------------------------------------------
# External packages
#-------------------------------------------------------------------------------

LIBXML2     = xml2
LIBXML2_INC = /usr/include/libxml2
LIBXML2_LIB = /usr/include

#-------------------------------------------------------------------------------
# Variables
#-------------------------------------------------------------------------------

UNAME       = $(shell uname)
PLATFORM    = $(UNAME:CYGWIN%=Cygwin)

ifeq ($(PLATFORM),Linux)
TARGET      = LINUX
endif
ifeq ($(PLATFORM),Cygwin)
TARGET      = CYGWIN
endif

#-------------------------------------------------------------------------------
# Directories
#-------------------------------------------------------------------------------

BLD_DIR     = $(PROJECT_ROOT)/build

WRK_DIR     = $(BLD_DIR)/work/$(PLATFORM)
REL_DIR     = $(BLD_DIR)/release/$(PLATFORM)

BIN_DIR     = $(WRK_DIR)/bin
BIN_DIR_REL = $(REL_DIR)/bin
DEP_DIR     = $(WRK_DIR)/dep
INC_DIR     = $(REL_DIR)/include
LIB_DIR     = $(WRK_DIR)/lib
LIB_DIR_REL = $(REL_DIR)/lib
OBJ_DIR     = $(WRK_DIR)/obj
EXA_DIR_REL = $(REL_DIR)/example

#-------------------------------------------------------------------------------
# Miscellaneous commands / tools
#-------------------------------------------------------------------------------
RM          = rm -rf
CP          = cp
CXX         = g++
LD          = $(CXX)
LIBTOOL     = libtool
AR          = ar
RANLIB      = ranlib
MAKE		= make

CXXFLAGS    = -g -O3 -Wall -D_GNU_SOURCE -D_$(TARGET)_ -pipe -fpic
LDFLAGS     = 
ARFLAGS     = cru
MAKEFLAGS	= --no-print-directory
 
OBJ         = o
CPP         = cc
DEP         = d
LIB         = libsdf3-
LIBA        = a
LIBSO       = so

OBJS        = $(SRCS:%.$(CPP)=$(COMPONENT_BASE)%.$(OBJ))
DEPS        = $(SRCS:%.$(CPP)=$(COMPONENT_BASE)%.$(DEP))
DIRS        = $(BLD_DIR)/work $(BLD_DIR)/release $(WRK_DIR) $(REL_DIR) \
			$(BIN_DIR) $(DEP_DIR) $(INC_DIR) $(LIB_DIR) $(OBJ_DIR) \
			$(EXA_DIR_REL) $(LIB_DIR_REL) $(BIN_DIR_REL)

VPATH       += $(OBJ_DIR) $(DEP_DIR) $(BIN_DIR) $(LIB_DIR)

INCLUDES    += $(PROJECT_ROOT)
