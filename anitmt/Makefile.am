SUBDIRS = lib hlib anitmt-calc utils example
EXTRA_DIST = BUILD FAQ autogen.sh

# Special make rules building only a subset: 
# Mainly interesting for developers. 
# NOTE: _Intentionally_ without dependencies. 
hlib: FORCE
	$(MAKE) -C hlib
rendview: FORCE
	$(MAKE) -C rendview
lib: FORCE
	$(MAKE) -C lib
utils: FORCE
	$(MAKE) -C utils
anitmt-calc: FORCE
	$(MAKE) -C anitmt-calc
FORCE:

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
	$(MAKE) -C hlib CXXFLAGS="-g" CFLAGS="-g $(ADDFLAGS)" 
	$(MAKE) -C rendview CXXFLAGS="-g $(ADDFLAGS)"
	$(MAKE) -C lib CXXFLAGS="-g -Wno-non-template-friend -ftemplate-depth-33 -fmessage-length=$(COLUMNS) -DYYDEBUG -DWOLFGANG"
	$(MAKE) -C anitmt-calc CXXFLAGS="-O -g -Wno-non-template-friend -ftemplate-depth-33 -fmessage-length=$(COLUMNS) -DYYDEBUG -DWOLFGANG"
	$(MAKE) -C anitmt-view CFLAGS="-g" CXXFLAGS="-g"
	$(MAKE) -C mkavi CFLAGS="-g" CXXFLAGS="-g"

FORCE:

