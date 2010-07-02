#
#   TU Eindhoven
#   Eindhoven, The Netherlands
#
#   Author          :   Sander Stuijk (sander@ics.ele.tue.nl)
#
#   Date            :   March 29, 2002
#   
#   $Id: Makefile,v 1.1.1.1 2007/10/02 10:59:45 sander Exp $

include $(PRIVATE_CM_ROOT)/sdf3/etc/Makefile.inc

#-------------------------------------------------------------------------------
# Variables
#-------------------------------------------------------------------------------

# Source files that need to be handled (e.g. foo.cc)
SRCS = $(wildcard *.cc)

# Name of the static library (e.g. libfoo.a)
PROG_LIB = 

# Name of include files needed in distribution (e.g. foo.h)
PROG_INC = 

# Name of the binary in distribution (e.g. foo)
PROG_BIN = sdf3ctg-$(MODULE)
PROG_REL_BIN = sdf3ctg-$(MODULE)

# Link with external libraries
USR_LIB_EXT  = $(LIB)sdf.$(LIBSO) $(LIB)base.$(LIBSO)

#-------------------------------------------------------------------------------
# Rules
#-------------------------------------------------------------------------------

include $(PRIVATE_CM_ROOT)/sdf3/etc/Makefile.rules

#-------------------------------------------------------------------------------
# Additional rules
#-------------------------------------------------------------------------------
