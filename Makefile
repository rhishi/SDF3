#
#   TU Eindhoven
#   Eindhoven, The Netherlands
#
#   Author          :   Sander Stuijk (sander@ics.ele.tue.nl)
#
#   Date            :   March 29, 2002
#   
#   $Id: Makefile,v 1.6 2008/03/06 12:06:46 sander Exp $

MAKE		= make
MAKEFLAGS	= --no-print-directory

MODULES 	= base sdf csdf sadf

export MODULE

.PHONY: doc $(MODULES)

# Default (build all)
default: all

# Build single module and its dependencies
base:   guard
	@for MODULE in $@; do\
		echo "";\
		echo "### Module "$(notdir $$MODULE); \
		cd $$MODULE && $(MAKE) $(MAKEFLAGS) && cd .. ; \
	done

sdf:	base
	@for MODULE in $^ $@; do\
		echo "";\
		echo "### Module "$(notdir $$MODULE); \
		cd $$MODULE && $(MAKE) $(MAKEFLAGS) && cd .. ; \
	done

csdf:	base sdf
	@for MODULE in $^ $@; do\
		echo "";\
		echo "### Module "$(notdir $$MODULE); \
		cd $$MODULE && $(MAKE) $(MAKEFLAGS) && cd .. ; \
	done

sadf:	base sdf csdf
	@for MODULE in $^ $@; do\
		echo "";\
		echo "### Module "$(notdir $$MODULE); \
		cd $$MODULE && $(MAKE) $(MAKEFLAGS) && cd .. ; \
	done

# Build all modules
all clean:	guard
	@for MODULE in $(MODULES); do\
		echo "";\
		echo "### Module "$(notdir $$MODULE); \
		cd $$MODULE && $(MAKE) $(MAKEFLAGS) $@ && cd .. ; \
	done

# Check basic build environment
guard:
	@if [ -z "$(PRIVATE_CM_ROOT)" ]; then \
		echo "Error: variable 'PRIVATE_CM_ROOT' not set";\
		exit 1; fi

# Documentation
doc:
	cd doc && doxygen doxygen.cfg && cd .. ;

