SUBDIRS = lib hlib anitmt-calc anitmt-view mkavi 
EXTRA_DIST = BUILD FAQ autogen.sh

# Make rules currently used by me (Wolfgang): 
COLUMNS:=$(shell echo \"$$COLUMNS\")
all-wolfgang: FORCE
	$(MAKE) -C hlib opt
	$(MAKE) -C lib CXXFLAGS="-Wno-non-template-friend -ftemplate-depth-33 -fmessage-length=$(COLUMNS) -D__CPLUSPLUS -DTHROW_NIL=\"throw()\" -DYYDEBUG -DWOLFGANG"
	$(MAKE) -C anitmt-calc CXXFLAGS="-Wno-non-template-friend -ftemplate-depth-33 -fmessage-length=$(COLUMNS) -D__CPLUSPLUS -DTHROW_NIL=\"throw()\" -DYYDEBUG -DWOLFGANG"
debug-wolfgang: FORCE
	$(MAKE) -C hlib opt
	$(MAKE) -C lib CXXFLAGS="-g -Wno-non-template-friend -ftemplate-depth-33 -fmessage-length=$(COLUMNS) -D__CPLUSPLUS -DTHROW_NIL=\"throw()\" -DYYDEBUG -DWOLFGANG"
	$(MAKE) -C anitmt-calc CXXFLAGS="-O -g -Wno-non-template-friend -ftemplate-depth-33 -fmessage-length=$(COLUMNS) -D__CPLUSPLUS -DTHROW_NIL=\"throw()\" -DYYDEBUG -DWOLFGANG"

FORCE:

