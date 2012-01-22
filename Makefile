# Global defines
GCC := gcc
GXX := g++
COMMON_CPPFLAGS := -g3 -fno-strict-aliasing -march=nocona
CPPFLAGS := -Wall $(COMMON_CPPFLAGS)
PARSER_CPPFLAGS := $(COMMON_CPPFLAGS)

# Defines for Core
SCC_OBJS := $(patsubst %.cpp,Obj/%.o,$(wildcard *.cpp))
SCC_LEX_OBJS := $(patsubst %.lex,Obj/%Lexer.o,$(wildcard *.lex))
SCC_PARSE_OBJS := $(patsubst %.y,Obj/%Parser.o,$(wildcard *.y))
ASMX86_OBJS := Obj/asmx86/asmx86.o
ASMX86_HEADERS := asmx86/asmx86.h asmx86/asmx86str.h

RUNTIME_LIBS := linux_x86.lib linux_x64.lib freebsd_x86.lib freebsd_x64.lib mach_x86.lib mach_x64.lib windows_x86.lib windows_x64.lib x86.lib x64.lib
RUNTIME_SOURCES := $(patsubst %.lib,Obj/%.cpp,$(RUNTIME_LIBS))
RUNTIME_OBJS := $(patsubst %.lib,Obj/Obj/%.o,$(RUNTIME_LIBS))

# Default targets
all : scc
.PHONY : all clean test

# Object directories
Obj/:
	if [ ! -d Obj ]; then mkdir Obj; fi
Obj/Obj/: | Obj/
	if [ ! -d Obj/Obj ]; then mkdir Obj/Obj; fi
Obj/asmx86/: | Obj/
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

# Runtime files
COMMON_RUNTIME_C := $(wildcard runtime/*.c)
COMMON_RUNTIME_H := $(wildcard runtime/*.h)
X86_RUNTIME_C := $(wildcard runtime/x86/*.c)
X86_RUNTIME_H := $(wildcard runtime/x86/*.h)
X64_RUNTIME_C := $(wildcard runtime/x64/*.c)
X64_RUNTIME_H := $(wildcard runtime/x64/*.h)
POSIX_RUNTIME_C := $(wildcard runtime/posix/*.c)
POSIX_RUNTIME_H := $(wildcard runtime/posix/*.h)
LINUX_COMMON_RUNTIME_C := $(wildcard runtime/linux/*.c)
LINUX_COMMON_RUNTIME_H := $(wildcard runtime/linux/*.h)
LINUX_X86_RUNTIME_C := $(wildcard runtime/linux/x86/*.c)
LINUX_X86_RUNTIME_H := $(wildcard runtime/linux/x86/*.h)
LINUX_X64_RUNTIME_C := $(wildcard runtime/linux/x64/*.c)
LINUX_X64_RUNTIME_H := $(wildcard runtime/linux/x64/*.h)
FREEBSD_COMMON_RUNTIME_C := $(wildcard runtime/freebsd/*.c)
FREEBSD_COMMON_RUNTIME_H := $(wildcard runtime/freebsd/*.h)
FREEBSD_X86_RUNTIME_C := $(wildcard runtime/freebsd/x86/*.c)
FREEBSD_X86_RUNTIME_H := $(wildcard runtime/freebsd/x86/*.h)
FREEBSD_X64_RUNTIME_C := $(wildcard runtime/freebsd/x64/*.c)
FREEBSD_X64_RUNTIME_H := $(wildcard runtime/freebsd/x64/*.h)
MACH_COMMON_RUNTIME_C := $(wildcard runtime/mach/*.c)
MACH_COMMON_RUNTIME_H := $(wildcard runtime/mach/*.h)
MACH_X86_RUNTIME_C := $(wildcard runtime/mach/x86/*.c)
MACH_X86_RUNTIME_H := $(wildcard runtime/mach/x86/*.h)
MACH_X64_RUNTIME_C := $(wildcard runtime/mach/x64/*.c)
MACH_X64_RUNTIME_H := $(wildcard runtime/mach/x64/*.h)
WINDOWS_COMMON_RUNTIME_C := $(wildcard runtime/windows/*.c)
WINDOWS_COMMON_RUNTIME_H := $(wildcard runtime/windows/*.h)
WINDOWS_X86_RUNTIME_C := $(wildcard runtime/windows/x86/*.c)
WINDOWS_X86_RUNTIME_H := $(wildcard runtime/windows/x86/*.h)
WINDOWS_X64_RUNTIME_C := $(wildcard runtime/windows/x64/*.c)
WINDOWS_X64_RUNTIME_H := $(wildcard runtime/windows/x64/*.h)

COMMON_RUNTIME_SRC := $(COMMON_RUNTIME_H) $(COMMON_RUNTIME_C)
X86_RUNTIME_SRC := $(X86_RUNTIME_H) $(X86_RUNTIME_C)
X64_RUNTIME_SRC := $(X64_RUNTIME_H) $(X64_RUNTIME_C)
POSIX_RUNTIME_SRC := $(COMMON_RUNTIME_SRC) $(POSIX_RUNTIME_H) $(POSIX_RUNTIME_C)
LINUX_COMMON_RUNTIME_SRC := $(POSIX_RUNTIME_SRC) $(LINUX_COMMON_RUNTIME_H) $(LINUX_COMMON_RUNTIME_C)
LINUX_X86_RUNTIME_SRC := $(X86_RUNTIME_SRC) $(LINUX_COMMON_RUNTIME_SRC) $(LINUX_X86_RUNTIME_H) $(LINUX_X86_RUNTIME_C)
LINUX_X64_RUNTIME_SRC := $(X64_RUNTIME_SRC) $(LINUX_COMMON_RUNTIME_SRC) $(LINUX_X64_RUNTIME_H) $(LINUX_X64_RUNTIME_C)
FREEBSD_COMMON_RUNTIME_SRC := $(POSIX_RUNTIME_SRC) $(FREEBSD_COMMON_RUNTIME_H) $(FREEBSD_COMMON_RUNTIME_C)
FREEBSD_X86_RUNTIME_SRC := $(X86_RUNTIME_SRC) $(FREEBSD_COMMON_RUNTIME_SRC) $(FREEBSD_X86_RUNTIME_H) $(FREEBSD_X86_RUNTIME_C)
FREEBSD_X64_RUNTIME_SRC := $(X64_RUNTIME_SRC) $(FREEBSD_COMMON_RUNTIME_SRC) $(FREEBSD_X64_RUNTIME_H) $(FREEBSD_X64_RUNTIME_C)
MACH_COMMON_RUNTIME_SRC := $(POSIX_RUNTIME_SRC) $(MACH_COMMON_RUNTIME_H) $(MACH_COMMON_RUNTIME_C)
MACH_X86_RUNTIME_SRC := $(X86_RUNTIME_SRC) $(MACH_COMMON_RUNTIME_SRC) $(MACH_X86_RUNTIME_H) $(MACH_X86_RUNTIME_C)
MACH_X64_RUNTIME_SRC := $(X64_RUNTIME_SRC) $(MACH_COMMON_RUNTIME_SRC) $(MACH_X64_RUNTIME_H) $(MACH_X64_RUNTIME_C)
WINDOWS_COMMON_RUNTIME_SRC := $(COMMON_RUNTIME_SRC) $(WINDOWS_COMMON_RUNTIME_H) $(WINDOWS_COMMON_RUNTIME_C)
WINDOWS_X86_RUNTIME_SRC := $(X86_RUNTIME_SRC) $(WINDOWS_COMMON_RUNTIME_SRC) $(WINDOWS_X86_RUNTIME_H) $(WINDOWS_X86_RUNTIME_C)
WINDOWS_X64_RUNTIME_SRC := $(X64_RUNTIME_SRC) $(WINDOWS_COMMON_RUNTIME_SRC) $(WINDOWS_X64_RUNTIME_H) $(WINDOWS_X64_RUNTIME_C)

COMMON_RUNTIME := $(foreach header,$(COMMON_RUNTIME_H),--header $(header)) $(COMMON_RUNTIME_C)
X86_RUNTIME := $(foreach header,$(X86_RUNTIME_H),--header $(header)) $(X86_RUNTIME_C)
X64_RUNTIME := $(foreach header,$(X64_RUNTIME_H),--header $(header)) $(X64_RUNTIME_C)
POSIX_RUNTIME := $(COMMON_RUNTIME) $(foreach header,$(POSIX_RUNTIME_H),--header $(header)) $(POSIX_RUNTIME_C)
LINUX_COMMON_RUNTIME := $(POSIX_RUNTIME) $(foreach header,$(LINUX_COMMON_RUNTIME_H),--header $(header)) $(LINUX_COMMON_RUNTIME_C)
LINUX_X86_RUNTIME := $(X86_RUNTIME) $(LINUX_COMMON_RUNTIME) $(foreach header,$(LINUX_X86_RUNTIME_H),--header $(header)) $(LINUX_X86_RUNTIME_C)
LINUX_X64_RUNTIME := $(X64_RUNTIME) $(LINUX_COMMON_RUNTIME) $(foreach header,$(LINUX_X64_RUNTIME_H),--header $(header)) $(LINUX_X64_RUNTIME_C)
FREEBSD_COMMON_RUNTIME := $(POSIX_RUNTIME) $(foreach header,$(FREEBSD_COMMON_RUNTIME_H),--header $(header)) $(FREEBSD_COMMON_RUNTIME_C)
FREEBSD_X86_RUNTIME := $(X86_RUNTIME) $(FREEBSD_COMMON_RUNTIME) $(foreach header,$(FREEBSD_X86_RUNTIME_H),--header $(header)) $(FREEBSD_X86_RUNTIME_C)
FREEBSD_X64_RUNTIME := $(X64_RUNTIME) $(FREEBSD_COMMON_RUNTIME) $(foreach header,$(FREEBSD_X64_RUNTIME_H),--header $(header)) $(FREEBSD_X64_RUNTIME_C)
MACH_COMMON_RUNTIME := $(POSIX_RUNTIME) $(foreach header,$(MACH_COMMON_RUNTIME_H),--header $(header)) $(MACH_COMMON_RUNTIME_C)
MACH_X86_RUNTIME := $(X86_RUNTIME) $(MACH_COMMON_RUNTIME) $(foreach header,$(MACH_X86_RUNTIME_H),--header $(header)) $(MACH_X86_RUNTIME_C)
MACH_X64_RUNTIME := $(X64_RUNTIME) $(MACH_COMMON_RUNTIME) $(foreach header,$(MACH_X64_RUNTIME_H),--header $(header)) $(MACH_X64_RUNTIME_C)
WINDOWS_COMMON_RUNTIME := $(COMMON_RUNTIME) $(foreach header,$(WINDOWS_COMMON_RUNTIME_H),--header $(header)) $(WINDOWS_COMMON_RUNTIME_C)
WINDOWS_X86_RUNTIME := $(X86_RUNTIME) $(WINDOWS_COMMON_RUNTIME) $(foreach header,$(WINDOWS_X86_RUNTIME_H),--header $(header)) $(WINDOWS_X86_RUNTIME_C)
WINDOWS_X64_RUNTIME := $(X64_RUNTIME) $(WINDOWS_COMMON_RUNTIME) $(foreach header,$(WINDOWS_X64_RUNTIME_H),--header $(header)) $(WINDOWS_X64_RUNTIME_C)

-include $(SCC_OBJS:.o=.d)
-include $(SCC_LEX_OBJS:.o=.d)
-include $(SCC_PARSE_OBJS:.o=.d)

asmx86/makeopstr: asmx86/makeopstr.cpp Makefile
	$(CXX) $(CXXFLAGS) -o asmx86/makeopstr asmx86/makeopstr.cpp

asmx86/asmx86str.h: asmx86/makeopstr asmx86/asmx86.h Makefile
	asmx86/makeopstr asmx86/asmx86.h asmx86/asmx86str.h

$(SCC_OBJS): Obj/%.o: %.cpp $(SCC_LEX_OBJS) $(ASMX86_HEADERS) Makefile | Obj/
	$(call COMPILE,$(GXX),$(CPPFLAGS) -IObj -Iasmx86,$<,$*)

$(RUNTIME_SOURCES): %.cpp: %.lib Makefile | Obj/
	xxd -i $< > $@

$(RUNTIME_OBJS): Obj/%.o: %.cpp Makefile | Obj/ Obj/Obj/
	$(call COMPILE,$(GXX),$(CPPFLAGS),$<,$*)

Obj/x86.lib: Obj/scc-bootstrap $(X86_RUNTIME_SRC) $(COMMON_RUNTIME_SRC) Makefile | Obj/
	Obj/scc-bootstrap $(X86_RUNTIME) $(COMMON_RUNTIME) --arch x86 --platform none -f lib -o $@
Obj/x64.lib: Obj/scc-bootstrap $(X64_RUNTIME_SRC) $(COMMON_RUNTIME_SRC) Makefile | Obj/
	Obj/scc-bootstrap $(X64_RUNTIME) $(COMMON_RUNTIME) --arch x64 --platform none -f lib -o $@
Obj/linux_x86.lib: Obj/scc-bootstrap $(LINUX_X86_RUNTIME_SRC) Makefile | Obj/
	Obj/scc-bootstrap $(LINUX_X86_RUNTIME) --arch x86 --platform linux -f lib -o $@
Obj/linux_x64.lib: Obj/scc-bootstrap $(LINUX_X64_RUNTIME_SRC) Makefile | Obj/
	Obj/scc-bootstrap $(LINUX_X64_RUNTIME) --arch x64 --platform linux -f lib -o $@
Obj/freebsd_x86.lib: Obj/scc-bootstrap $(FREEBSD_X86_RUNTIME_SRC) Makefile | Obj/
	Obj/scc-bootstrap $(FREEBSD_X86_RUNTIME) --arch x86 --platform freebsd -f lib -o $@
Obj/freebsd_x64.lib: Obj/scc-bootstrap $(FREEBSD_X64_RUNTIME_SRC) Makefile | Obj/
	Obj/scc-bootstrap $(FREEBSD_X64_RUNTIME) --arch x64 --platform freebsd -f lib -o $@
Obj/mach_x86.lib: Obj/scc-bootstrap $(MACH_X86_RUNTIME_SRC) Makefile | Obj/
	Obj/scc-bootstrap $(MACH_X86_RUNTIME) --arch x86 --platform mach -f lib -o $@
Obj/mach_x64.lib: Obj/scc-bootstrap $(MACH_X64_RUNTIME_SRC) Makefile | Obj/
	Obj/scc-bootstrap $(MACH_X64_RUNTIME) --arch x64 --platform mach -f lib -o $@
Obj/windows_x86.lib: Obj/scc-bootstrap $(WINDOWS_X86_RUNTIME_SRC) Makefile | Obj/
	Obj/scc-bootstrap $(WINDOWS_X86_RUNTIME) --arch x86 --platform windows -f lib -o $@
Obj/windows_x64.lib: Obj/scc-bootstrap $(WINDOWS_X64_RUNTIME_SRC) Makefile | Obj/
	Obj/scc-bootstrap $(WINDOWS_X64_RUNTIME) --arch x64 --platform windows -f lib -o $@

$(ASMX86_OBJS): Obj/%.o: %.c $(ASMX86_HEADERS) Makefile | Obj/asmx86/
	$(call COMPILE,$(GCC),$(CPPFLAGS) -std=gnu99,$<,$*)

$(SCC_LEX_OBJS): Obj/%Lexer.o: %.lex $(SCC_PARSE_OBJS) $(ASMX86_HEADERS) Makefile | Obj/
	flex -R --prefix=$(lastword $(subst /, ,$*))_ -o Obj/$*Lexer.cpp --header=Obj/$*Lexer.h $<
	$(call COMPILE,$(GXX),$(PARSER_CPPFLAGS) -I. -IObj -Iasmx86,Obj/$*Lexer.cpp,$*Lexer)

$(SCC_PARSE_OBJS): Obj/%Parser.o: %.y $(ASMX86_HEADERS) Makefile | Obj/
	bison --name-prefix=$(lastword $(subst /, ,$*))_ -o Obj/$*Parser.cpp --defines=Obj/$*Parser.h $<
	$(call COMPILE,$(GXX),$(PARSER_CPPFLAGS) -I. -IObj -Iasmx86,Obj/$*Parser.cpp,$*Parser)

Obj/scc-bootstrap: $(SCC_OBJS) $(SCC_LEX_OBJS) $(SCC_PARSE_OBJS) $(ASMX86_OBJS) Makefile
	$(GXX) -o Obj/scc-bootstrap $(SCC_OBJS) $(SCC_LEX_OBJS) $(SCC_PARSE_OBJS) $(ASMX86_OBJS)

scc: $(SCC_OBJS) $(SCC_LEX_OBJS) $(SCC_PARSE_OBJS) $(ASMX86_OBJS) $(RUNTIME_OBJS) Makefile
	$(GXX) -o scc $(SCC_OBJS) $(SCC_LEX_OBJS) $(SCC_PARSE_OBJS) $(ASMX86_OBJS) $(RUNTIME_OBJS)

# Cleaning rule
clean :
	rm -f scc
	rm -rf Obj
	rm -f asmx86/asmx86str.h
	rm -f asmx86/makeopstr

test: scc
	python tests/test.py

