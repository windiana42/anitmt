# Makefile.am for test functionality
INCLUDES=		-I$(top_srcdir)/lib -I$(srcdir)

noinst_LIBRARIES=	libfunctionality.a
BUILT_SOURCES=		base_func.hpp base_func.cpp \
			scene_base.hpp scene_base.cpp \
			scalar_base.hpp scalar_base.cpp \
			scalar.hpp scalar.cpp \
			object_base.hpp object_base.cpp \
			object.hpp object.cpp 
libfunctionality_a_SOURCES= base_func.afd base_func.hpp base_func.cpp \
			scene_base.afd scene_base.hpp scene_base.cpp \
			scalar_base.afd scalar_base.hpp scalar_base.cpp \
			scalar.afd scalar.hpp scalar.cpp \
			object_base.afd object_base.hpp object_base.cpp \
			object.afd object.hpp object.cpp \
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

scene_base.cpp \
scene_base.hpp: $(srcdir)/scene_base.afd $(FUNCGEN_CODE)
	if test -e $(FUNCGEN); then					     \
	  $(FUNCGEN) -i $(srcdir)/scene_base.afd -o scene_base -n functionality \
	    -I+=$(srcdir)/ ;						     \
	else								     \
	  echo "No funcgen availible, cannot create code.";		     \
	fi

scalar_base.cpp \
scalar_base.hpp: $(srcdir)/scalar_base.afd $(FUNCGEN_CODE)
	if test -e $(FUNCGEN); then					    \
	  $(FUNCGEN) -i $(srcdir)/scalar_base.afd -o scalar_base -n functionality -I+=$(srcdir)/ ; \
	else								    \
	  echo "No funcgen availible, cannot create code.";		    \
	fi

scalar.cpp \
scalar.hpp: $(srcdir)/scalar.afd $(FUNCGEN_CODE)
	if test -e $(FUNCGEN); then					    \
	  $(FUNCGEN) -i $(srcdir)/scalar.afd -o scalar -n functionality -I+=$(srcdir)/ ; \
	else								    \
	  echo "No funcgen availible, cannot create code.";		    \
	fi

object_base.cpp \
object_base.hpp: $(srcdir)/object_base.afd $(FUNCGEN_CODE)
	if test -e $(FUNCGEN); then					    \
	  $(FUNCGEN) -i $(srcdir)/object_base.afd -o object_base -n functionality -I+=$(srcdir)/ ; \
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


