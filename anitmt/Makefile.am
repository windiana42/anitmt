SUBDIRS = lib hlib anitmt-calc anitmt-view mkavi rendview example
EXTRA_DIST = BUILD FAQ autogen.sh

# Make rules currently used by me (Wolfgang): 
COLUMNS=$(shell echo \"$$COLUMNS\")
# CFLAGS="-D__CPLUSPLUS -DTHROW_NIL=\"throw()\""
all-wolfgang: FORCE
	$(MAKE) -C hlib opt
	$(MAKE) -C rendview opt
	$(MAKE) -C lib CXXFLAGS="-Wno-non-template-friend -ftemplate-depth-33 -DYYDEBUG -DWOLFGANG $(ADDFLAGS)"
	$(MAKE) -C anitmt-calc CXXFLAGS="-Wno-non-template-friend -ftemplate-depth-33 -DYYDEBUG -DWOLFGANG $(ADDFLAGS)"
	$(MAKE) -C anitmt-view CFLAGS="" CXXFLAGS=""
	$(MAKE) -C mkavi CFLAGS="" CXXFLAGS=""
debug-wolfgang: FORCE
	$(MAKE) -C hlib CXXFLAGS="-g" CFLAGS="-g" 
	$(MAKE) -C rendview CXXFLAGS="-g"
	$(MAKE) -C lib CXXFLAGS="-g -Wno-non-template-friend -ftemplate-depth-33 -fmessage-length=$(COLUMNS) -DYYDEBUG -DWOLFGANG"
	$(MAKE) -C anitmt-calc CXXFLAGS="-O -g -Wno-non-template-friend -ftemplate-depth-33 -fmessage-length=$(COLUMNS) -DYYDEBUG -DWOLFGANG"
	$(MAKE) -C anitmt-view CFLAGS="-g" CXXFLAGS="-g"
	$(MAKE) -C mkavi CFLAGS="-g" CXXFLAGS="-g"

FORCE:

