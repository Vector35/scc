# Global defines
GCC := gcc
GXX := g++
COMMON_CPPFLAGS := -g3 -Wstrict-aliasing=2 -fstrict-aliasing -march=nocona
CPPFLAGS := -Wall $(COMMON_CPPFLAGS)
PARSER_CPPFLAGS := $(COMMON_CPPFLAGS)

# Defines for Core
SCC_OBJS := $(patsubst %.cpp,Obj/%.o,$(wildcard *.cpp))
SCC_LEX_OBJS := $(patsubst %.lex,Obj/%Lexer.o,$(wildcard *.lex))
SCC_PARSE_OBJS := $(patsubst %.y,Obj/%Parser.o,$(wildcard *.y))
ASMX86_OBJS := Obj/asmx86/asmx86.o
ASMX86_HEADERS := asmx86/asmx86.h asmx86/asmx86str.h

# Default targets
all : scc
.PHONY : all clean

# Object directories
Obj/:
	if [ ! -d Obj ]; then mkdir Obj; fi
Obj/asmx86/:
	if [ ! -d Obj/asmx86 ]; then mkdir Obj/asmx86; fi

# Main build targets
define COMPILE
	$1 -c $2 $3 -o $@
	@$1 -MM $2 $3 > Obj/$4.d
	@mv -f Obj/$4.d Obj/$4.d.tmp
	@sed -e 's|.*:|Obj/$4.o:|' < Obj/$4.d.tmp > Obj/$4.d
	@sed -e 's/.*://' -e 's/\\$$//' < Obj/$4.d.tmp | fmt -1 | sed -e 's/^ *//' -e 's/$$/:/' >> Obj/$4.d
	@rm -f Obj/$4.d.tmp
endef

-include $(SCC_OBJS:.o=.d)
-include $(SCC_LEX_OBJS:.o=.d)
-include $(SCC_PARSE_OBJS:.o=.d)

asmx86/makeopstr: asmx86/makeopstr.cpp Makefile
	$(CXX) $(CXXFLAGS) -o asmx86/makeopstr asmx86/makeopstr.cpp

asmx86/asmx86str.h: asmx86/makeopstr asmx86/asmx86.h Makefile
	asmx86/makeopstr asmx86/asmx86.h asmx86/asmx86str.h

$(SCC_OBJS): Obj/%.o: %.cpp $(SCC_LEX_OBJS) $(ASMX86_HEADERS) Makefile | Obj/
	$(call COMPILE,$(GXX),$(CPPFLAGS) -IObj -Iasmx86,$<,$*)

$(ASMX86_OBJS): Obj/%.o: %.c $(ASMX86_HEADERS) Makefile | Obj/asmx86/
	$(call COMPILE,$(GCC),$(CPPFLAGS) -std=gnu99,$<,$*)

$(SCC_LEX_OBJS): Obj/%Lexer.o: %.lex $(SCC_PARSE_OBJS) $(ASMX86_HEADERS) Makefile | Obj/
	flex -R --prefix=$(lastword $(subst /, ,$*))_ -o Obj/$*Lexer.cpp --header=Obj/$*Lexer.h $<
	$(call COMPILE,$(GXX),$(PARSER_CPPFLAGS) -I. -IObj -Iasmx86,Obj/$*Lexer.cpp,$*Lexer)

$(SCC_PARSE_OBJS): Obj/%Parser.o: %.y $(ASMX86_HEADERS) Makefile | Obj/
	bison --name-prefix=$(lastword $(subst /, ,$*))_ -o Obj/$*Parser.cpp --defines=Obj/$*Parser.h $<
	$(call COMPILE,$(GXX),$(PARSER_CPPFLAGS) -I. -IObj -Iasmx86,Obj/$*Parser.cpp,$*Parser)

scc: $(SCC_OBJS) $(SCC_LEX_OBJS) $(SCC_PARSE_OBJS) $(ASMX86_OBJS) Makefile
	$(GXX) -o scc $(SCC_OBJS) $(SCC_LEX_OBJS) $(SCC_PARSE_OBJS) $(ASMX86_OBJS)

# Cleaning rule
clean :
	rm -f scc
	rm -rf Obj
	rm -f asmx86/asmx86str.h
	rm -f asmx86/makeopstr

