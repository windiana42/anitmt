# Makefile.am for test functionality
INCLUDES=		-I$(top_srcdir)/lib -I$(srcdir)

noinst_LIBRARIES=	libfunctionality.a
BUILT_SOURCES=		base_func.hpp base_func.cpp \
			scalar.hpp scalar.cpp \
			object.hpp object.cpp \
			functionality.hpp functionality.cpp 
libfunctionality_a_SOURCES= base_func.afd base_func.hpp base_func.cpp \
			scalar.afd scalar.hpp scalar.cpp \
			object.afd object.hpp object.cpp \
			functionality.afd functionality.hpp functionality.cpp \
			make_avail.cpp

check_PROGRAMS=		test_functionality
CXXFLAGS =		@CXXFLAGS@ -Wno-non-template-friend -DYYDEBUG \
			-D__CPLUSPLUS -DTHROW_NIL=throw\(\) \
			-ftemplate-depth-30

test_functionality_SOURCES= \
			test_functionality.cpp

test_functionality_LDADD= \
			../../lib/message/libmessage.a \
			../../lib/val/libval.a \
			../../lib/solve/libsolve.a \
			../../lib/proptree/libproptree.a 

FUNCGEN=../funcgen/funcgen
FUNCGEN_CODE=$(srcdir)/../funcgen/gen_cpp.cpp

clean:
	rm -f $(BUILT_SOURCES)

# exec funcgen...
# base_func isn't regenerated when funcgen is changed
base_func.cpp \
base_func.hpp: $(srcdir)/base_func.afd 
	if test -e $(FUNCGEN); then					\
	  $(FUNCGEN) -i $(srcdir)/base_func.afd -o base_func -n functionality	\
	    -I+=$(srcdir)/ ;						\
	else								\
	  echo "No funcgen availible, cannot create code.";		\
	fi

scalar.cpp \
scalar.hpp: $(srcdir)/scalar.afd $(FUNCGEN_CODE)
	if test -e $(FUNCGEN); then					    \
	  $(FUNCGEN) -i $(srcdir)/scalar.afd -o scalar -n functionality -I+=$(srcdir)/ ; \
	else								    \
	  echo "No funcgen availible, cannot create code.";		    \
	fi

object.cpp \
object.hpp: $(srcdir)/object.afd $(FUNCGEN_CODE)
	if test -e $(FUNCGEN); then					    \
	  $(FUNCGEN) -i $(srcdir)/object.afd -o object -n functionality -I+=$(srcdir)/ ; \
	else								    \
	  echo "No funcgen availible, cannot create code.";		    \
	fi

functionality.cpp \
functionality.hpp: $(srcdir)/functionality.afd $(FUNCGEN_CODE)
	if test -e $(FUNCGEN); then					     \
	  $(FUNCGEN) -i $(srcdir)/functionality.afd -o functionality -n functionality \
	    -I+=$(srcdir)/ ;						     \
	else								     \
	  echo "No funcgen availible, cannot create code.";		     \
	fi

