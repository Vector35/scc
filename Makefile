# Global defines
HOST := $(shell uname)
CC := gcc
CXX := g++
LINK := g++
PYTHON := python3
ifeq ($(CONFIG),release)
	COMMON_CPPFLAGS := -O3 -fno-strict-aliasing -march=nocona -DRELEASE
else
	COMMON_CPPFLAGS := -g3 -fno-strict-aliasing -march=nocona
endif
	CFLAGS := $(COMMON_CPPFLAGS) -std=gnu99
	CPPFLAGS := -Wall $(COMMON_CPPFLAGS)
	PARSER_CPPFLAGS := $(COMMON_CPPFLAGS)
LDFLAGS :=
define OUTPUT_EXE
-o $1
endef
define OUTPUT_OBJ
-o $1
endef

ifeq ($(CONFIG),release)
STRIP := yes
else
STRIP := no
endif

ifeq ($(HOST),Linux)
	TARGET := scc
	CODEGEN := Obj/makecodegen
	BOOTSTRAP := Obj/scc-bootstrap
	MAKEOPSTR := Obj/makeopstr
	MAKE_VERSION = echo "const char* g_versionString = \"$(MAJOR).$(MINOR).$(BUILD)\";" > Obj/Version.cpp
else
ifeq ($(HOST),FreeBSD)
	TARGET := scc
	CODEGEN := Obj/makecodegen
	BOOTSTRAP := Obj/scc-bootstrap
	MAKEOPSTR := Obj/makeopstr
	MAKE_VERSION = echo -e "const char* g_versionString = \"$(MAJOR).$(MINOR).$(BUILD)\";" > Obj/Version.cpp
else
ifeq ($(HOST),Darwin)
	TARGET := scc
	CODEGEN := Obj/makecodegen
	BOOTSTRAP := Obj/scc-bootstrap
	MAKEOPSTR := Obj/makeopstr
	MAKE_VERSION = echo "const char* g_versionString = \"$(MAJOR).$(MINOR).$(BUILD)\";" > Obj/Version.cpp
else
	CC := cl
	CXX := cl
	LINK := link
	PYTHON := /c/python27/python.exe
ifeq ($(CONFIG),release)
	COMMON_CPPFLAGS := -O2 -EHsc -nologo -DWIN32 -DRELEASE -D_CRT_SECURE_NO_WARNINGS
else
	COMMON_CPPFLAGS := -EHsc -nologo -DWIN32 -D_CRT_SECURE_NO_WARNINGS
endif
	CFLAGS := $(COMMON_CPPFLAGS)
	CPPFLAGS := -W3 $(COMMON_CPPFLAGS)
	PARSER_CPPFLAGS := $(COMMON_CPPFLAGS) -wd4005
	CXXFLAGS := $(COMMON_CPPFLAGS)
	LDFLAGS := -nologo
	TARGET := scc.exe
	CODEGEN := Obj/makecodegen.exe
	BOOTSTRAP := Obj/scc-bootstrap.exe
	MAKEOPSTR := Obj/makeopstr.exe
	MAKE_VERSION = echo -e "const char* g_versionString = \"$(MAJOR).$(MINOR).$(BUILD)\";\n" > Obj/Version.cpp
	STRIP := no
define OUTPUT_OBJ
-Fo$1
endef
define OUTPUT_EXE
-OUT:$1
endef
endif
endif
endif

# Defines for Core
SCC_OBJS := $(patsubst %.cpp,Obj/%.o,$(wildcard *.cpp))
SCC_LEX_OBJS := $(patsubst %.lex,Obj/%Lexer.o,$(wildcard *.lex))
SCC_PARSE_OBJS := $(patsubst %.y,Obj/%Parser.o,$(wildcard *.y))
SCC_CODEGEN_OBJS := $(patsubst %.cgen,Obj/%CodeGen.o,$(wildcard *.cgen))
ASMX86_OBJS := Obj/asmx86/asmx86.o
ASMX86_HEADERS := asmx86/asmx86.h asmx86/asmx86str.h

RUNTIME_LIBS := linux_x86.lib linux_x64.lib linux_quark.lib linux_mips.lib linux_mipsel.lib linux_arm.lib linux_armeb.lib linux_aarch64.lib linux_ppc.lib linux_ppcel.lib freebsd_x86.lib freebsd_x64.lib freebsd_quark.lib mac_x86.lib mac_x64.lib mac_quark.lib windows_x86.lib windows_x64.lib windows_quark.lib windows_arm.lib x86.lib x64.lib quark.lib mips.lib mipsel.lib arm.lib armeb.lib aarch64.lib ppc.lib ppcel.lib
RUNTIME_SOURCES := $(patsubst %.lib,Obj/%.cpp,$(RUNTIME_LIBS))
RUNTIME_OBJS := $(patsubst %.lib,Obj/Obj/%.o,$(RUNTIME_LIBS))

ifeq ($(CONFIG),release)
REV := $(shell git describe --tags HEAD)
MAJOR := $(shell $(PYTHON) -c "print('$(REV)'[1:].replace('-','.').split('.')[0])")
MINOR := $(shell $(PYTHON) -c "print('$(REV)'[1:].replace('-','.').split('.')[1])")
BUILD := $(shell $(PYTHON) -c "print('$(REV)'[1:].replace('-','.').split('.')[2])")
VERSION_OBJ := Obj/Obj/Version.o
else
VERSION_OBJ :=
endif

# Defines for codegen generator
CODEGEN_OBJS := $(patsubst %.cpp,Obj/%.o,$(wildcard codegen/*.cpp))
CODEGEN_LEX_OBJS := $(patsubst %.lex,Obj/%Lexer.o,$(wildcard codegen/*.lex))
CODEGEN_PARSE_OBJS := $(patsubst %.y,Obj/%Parser.o,$(wildcard codegen/*.y))

# Default targets
all : $(TARGET)
.PHONY : all clean test

# Object directories
Obj/:
	if [ ! -d Obj ]; then mkdir Obj; fi
Obj/Obj/: | Obj/
	if [ ! -d Obj/Obj ]; then mkdir Obj/Obj; fi
Obj/asmx86/: | Obj/
	if [ ! -d Obj/asmx86 ]; then mkdir Obj/asmx86; fi
Obj/Obj/asmx86/: | Obj/Obj/
	if [ ! -d Obj/Obj/asmx86 ]; then mkdir Obj/Obj/asmx86; fi
Obj/codegen/: | Obj/
	if [ ! -d Obj/codegen ]; then mkdir Obj/codegen; fi
Obj/Obj/codegen/: | Obj/Obj/
	if [ ! -d Obj/Obj/codegen ]; then mkdir Obj/Obj/codegen; fi

# Main build targets
define COMPILE
	$1 -c $2 $3 $4 $(call OUTPUT_OBJ,$@)
	@gcc -MM $3 $4 > Obj/$5.d
	@mv -f Obj/$5.d Obj/$5.d.tmp
	@sed -e 's|.*:|Obj/$5.o:|' < Obj/$5.d.tmp > Obj/$5.d
	@sed -e 's/.*://' -e 's/\\$$//' -e 's/^ *//' -e 's/ *$$//' -e 's/ /\n/g' < Obj/$5.d.tmp | sed -e 's/^ *//' -e 's/$$/:/' >> Obj/$5.d
	@rm -f Obj/$5.d.tmp
endef

define COMPILE_ONLY
	$1 -c $2 $3 $4 $(call OUTPUT_OBJ,$@)
endef

# Runtime files
COMMON_RUNTIME_C := $(wildcard runtime/*.c)
COMMON_RUNTIME_H := $(wildcard runtime/*.h)
X86_RUNTIME_C := $(wildcard runtime/x86/*.c)
X86_RUNTIME_H := $(wildcard runtime/x86/*.h)
X64_RUNTIME_C := $(wildcard runtime/x64/*.c)
X64_RUNTIME_H := $(wildcard runtime/x64/*.h)
QUARK_RUNTIME_C := $(wildcard runtime/quark/*.c)
QUARK_RUNTIME_H := $(wildcard runtime/quark/*.h)
MIPS_RUNTIME_C := $(wildcard runtime/mips/*.c)
MIPS_RUNTIME_H := $(wildcard runtime/mips/*.h)
ARM_RUNTIME_C := $(wildcard runtime/arm/*.c)
ARM_RUNTIME_H := $(wildcard runtime/arm/*.h)
AARCH64_RUNTIME_C := $(wildcard runtime/aarch64/*.c)
AARCH64_RUNTIME_H := $(wildcard runtime/aarch64/*.h)
PPC_RUNTIME_C := $(wildcard runtime/ppc/*.c)
PPC_RUNTIME_H := $(wildcard runtime/ppc/*.h)
POSIX_RUNTIME_C := $(wildcard runtime/posix/*.c)
POSIX_RUNTIME_H := $(wildcard runtime/posix/*.h)
LINUX_COMMON_RUNTIME_C := $(wildcard runtime/linux/*.c)
LINUX_COMMON_RUNTIME_H := $(wildcard runtime/linux/*.h)
LINUX_X86_RUNTIME_C := $(wildcard runtime/linux/x86/*.c)
LINUX_X86_RUNTIME_H := $(wildcard runtime/linux/x86/*.h)
LINUX_X64_RUNTIME_C := $(wildcard runtime/linux/x64/*.c)
LINUX_X64_RUNTIME_H := $(wildcard runtime/linux/x64/*.h)
LINUX_QUARK_RUNTIME_C := $(wildcard runtime/linux/x86/*.c)
LINUX_QUARK_RUNTIME_H := $(wildcard runtime/linux/x86/*.h)
LINUX_MIPS_RUNTIME_C := $(wildcard runtime/linux/mips/*.c)
LINUX_MIPS_RUNTIME_H := $(wildcard runtime/linux/mips/*.h)
LINUX_ARM_RUNTIME_C := $(wildcard runtime/linux/arm/*.c)
LINUX_ARM_RUNTIME_H := $(wildcard runtime/linux/arm/*.h)
LINUX_AARCH64_RUNTIME_C := $(wildcard runtime/linux/aarch64/*.c)
LINUX_AARCH64_RUNTIME_H := $(wildcard runtime/linux/aarch64/*.h)
LINUX_PPC_RUNTIME_C := $(wildcard runtime/linux/ppc/*.c)
LINUX_PPC_RUNTIME_H := $(wildcard runtime/linux/ppc/*.h)
FREEBSD_COMMON_RUNTIME_C := $(wildcard runtime/freebsd/*.c)
FREEBSD_COMMON_RUNTIME_H := $(wildcard runtime/freebsd/*.h)
FREEBSD_X86_RUNTIME_C := $(wildcard runtime/freebsd/x86/*.c)
FREEBSD_X86_RUNTIME_H := $(wildcard runtime/freebsd/x86/*.h)
FREEBSD_X64_RUNTIME_C := $(wildcard runtime/freebsd/x64/*.c)
FREEBSD_X64_RUNTIME_H := $(wildcard runtime/freebsd/x64/*.h)
FREEBSD_QUARK_RUNTIME_C := $(wildcard runtime/freebsd/x86/*.c)
FREEBSD_QUARK_RUNTIME_H := $(wildcard runtime/freebsd/x86/*.h)
MAC_COMMON_RUNTIME_C := $(wildcard runtime/mac/*.c)
MAC_COMMON_RUNTIME_H := $(wildcard runtime/mac/*.h)
MAC_X86_RUNTIME_C := $(wildcard runtime/mac/x86/*.c)
MAC_X86_RUNTIME_H := $(wildcard runtime/mac/x86/*.h)
MAC_X64_RUNTIME_C := $(wildcard runtime/mac/x64/*.c)
MAC_X64_RUNTIME_H := $(wildcard runtime/mac/x64/*.h)
MAC_QUARK_RUNTIME_C := $(wildcard runtime/mac/x86/*.c)
MAC_QUARK_RUNTIME_H := $(wildcard runtime/mac/x86/*.h)
WINDOWS_COMMON_RUNTIME_C := $(wildcard runtime/windows/*.c)
WINDOWS_COMMON_RUNTIME_H := $(wildcard runtime/windows/*.h)
WINDOWS_X86_RUNTIME_C := $(wildcard runtime/windows/x86/*.c)
WINDOWS_X86_RUNTIME_H := $(wildcard runtime/windows/x86/*.h)
WINDOWS_X64_RUNTIME_C := $(wildcard runtime/windows/x64/*.c)
WINDOWS_X64_RUNTIME_H := $(wildcard runtime/windows/x64/*.h)
WINDOWS_QUARK_RUNTIME_C := $(wildcard runtime/windows/x86/*.c)
WINDOWS_QUARK_RUNTIME_H := $(wildcard runtime/windows/x86/*.h)
WINDOWS_ARM_RUNTIME_C := $(wildcard runtime/windows/arm/*.c)
WINDOWS_ARM_RUNTIME_H := $(wildcard runtime/windows/arm/*.h)

COMMON_RUNTIME_SRC := $(COMMON_RUNTIME_H) $(COMMON_RUNTIME_C)
X86_RUNTIME_SRC := $(X86_RUNTIME_H) $(X86_RUNTIME_C)
X64_RUNTIME_SRC := $(X64_RUNTIME_H) $(X64_RUNTIME_C)
QUARK_RUNTIME_SRC := $(QUARK_RUNTIME_H) $(QUARK_RUNTIME_C)
MIPS_RUNTIME_SRC := $(MIPS_RUNTIME_H) $(MIPS_RUNTIME_C)
ARM_RUNTIME_SRC := $(ARM_RUNTIME_H) $(ARM_RUNTIME_C)
AARCH64_RUNTIME_SRC := $(AARCH64_RUNTIME_H) $(AARCH64_RUNTIME_C)
PPC_RUNTIME_SRC := $(PPC_RUNTIME_H) $(PPC_RUNTIME_C)
POSIX_RUNTIME_SRC := $(COMMON_RUNTIME_SRC) $(POSIX_RUNTIME_H) $(POSIX_RUNTIME_C)
LINUX_COMMON_RUNTIME_SRC := $(POSIX_RUNTIME_SRC) $(LINUX_COMMON_RUNTIME_H) $(LINUX_COMMON_RUNTIME_C)
LINUX_X86_RUNTIME_SRC := $(X86_RUNTIME_SRC) $(LINUX_COMMON_RUNTIME_SRC) $(LINUX_X86_RUNTIME_H) $(LINUX_X86_RUNTIME_C)
LINUX_X64_RUNTIME_SRC := $(X64_RUNTIME_SRC) $(LINUX_COMMON_RUNTIME_SRC) $(LINUX_X64_RUNTIME_H) $(LINUX_X64_RUNTIME_C)
LINUX_QUARK_RUNTIME_SRC := $(QUARK_RUNTIME_SRC) $(LINUX_COMMON_RUNTIME_SRC) $(LINUX_QUARK_RUNTIME_H) $(LINUX_QUARK_RUNTIME_C)
LINUX_MIPS_RUNTIME_SRC := $(MIPS_RUNTIME_SRC) $(LINUX_COMMON_RUNTIME_SRC) $(LINUX_MIPS_RUNTIME_H) $(LINUX_MIPS_RUNTIME_C)
LINUX_ARM_RUNTIME_SRC := $(ARM_RUNTIME_SRC) $(LINUX_COMMON_RUNTIME_SRC) $(LINUX_ARM_RUNTIME_H) $(LINUX_ARM_RUNTIME_C)
LINUX_AARCH64_RUNTIME_SRC := $(AARCH64_RUNTIME_SRC) $(LINUX_COMMON_RUNTIME_SRC) $(LINUX_AARCH64_RUNTIME_H) $(LINUX_AARCH64_RUNTIME_C)
LINUX_PPC_RUNTIME_SRC := $(PPC_RUNTIME_SRC) $(LINUX_COMMON_RUNTIME_SRC) $(LINUX_PPC_RUNTIME_H) $(LINUX_PPC_RUNTIME_C)
FREEBSD_COMMON_RUNTIME_SRC := $(POSIX_RUNTIME_SRC) $(FREEBSD_COMMON_RUNTIME_H) $(FREEBSD_COMMON_RUNTIME_C)
FREEBSD_X86_RUNTIME_SRC := $(X86_RUNTIME_SRC) $(FREEBSD_COMMON_RUNTIME_SRC) $(FREEBSD_X86_RUNTIME_H) $(FREEBSD_X86_RUNTIME_C)
FREEBSD_X64_RUNTIME_SRC := $(X64_RUNTIME_SRC) $(FREEBSD_COMMON_RUNTIME_SRC) $(FREEBSD_X64_RUNTIME_H) $(FREEBSD_X64_RUNTIME_C)
FREEBSD_QUARK_RUNTIME_SRC := $(QUARK_RUNTIME_SRC) $(FREEBSD_COMMON_RUNTIME_SRC) $(FREEBSD_QUARK_RUNTIME_H) $(FREEBSD_QUARK_RUNTIME_C)
MAC_COMMON_RUNTIME_SRC := $(POSIX_RUNTIME_SRC) $(MAC_COMMON_RUNTIME_H) $(MAC_COMMON_RUNTIME_C)
MAC_X86_RUNTIME_SRC := $(X86_RUNTIME_SRC) $(MAC_COMMON_RUNTIME_SRC) $(MAC_X86_RUNTIME_H) $(MAC_X86_RUNTIME_C)
MAC_X64_RUNTIME_SRC := $(X64_RUNTIME_SRC) $(MAC_COMMON_RUNTIME_SRC) $(MAC_X64_RUNTIME_H) $(MAC_X64_RUNTIME_C)
MAC_QUARK_RUNTIME_SRC := $(QUARK_RUNTIME_SRC) $(MAC_COMMON_RUNTIME_SRC) $(MAC_QUARK_RUNTIME_H) $(MAC_QUARK_RUNTIME_C)
WINDOWS_COMMON_RUNTIME_SRC := $(COMMON_RUNTIME_SRC) $(WINDOWS_COMMON_RUNTIME_H) $(WINDOWS_COMMON_RUNTIME_C)
WINDOWS_X86_RUNTIME_SRC := $(X86_RUNTIME_SRC) $(WINDOWS_COMMON_RUNTIME_SRC) $(WINDOWS_X86_RUNTIME_H) $(WINDOWS_X86_RUNTIME_C)
WINDOWS_X64_RUNTIME_SRC := $(X64_RUNTIME_SRC) $(WINDOWS_COMMON_RUNTIME_SRC) $(WINDOWS_X64_RUNTIME_H) $(WINDOWS_X64_RUNTIME_C)
WINDOWS_QUARK_RUNTIME_SRC := $(QUARK_RUNTIME_SRC) $(WINDOWS_COMMON_RUNTIME_SRC) $(WINDOWS_QUARK_RUNTIME_H) $(WINDOWS_QUARK_RUNTIME_C)
WINDOWS_ARM_RUNTIME_SRC := $(ARM_RUNTIME_SRC) $(WINDOWS_COMMON_RUNTIME_SRC) $(WINDOWS_ARM_RUNTIME_H) $(WINDOWS_ARM_RUNTIME_C)

COMMON_RUNTIME := $(foreach header,$(COMMON_RUNTIME_H),--header $(header)) $(COMMON_RUNTIME_C)
X86_RUNTIME := $(foreach header,$(X86_RUNTIME_H),--header $(header)) $(X86_RUNTIME_C)
X64_RUNTIME := $(foreach header,$(X64_RUNTIME_H),--header $(header)) $(X64_RUNTIME_C)
QUARK_RUNTIME := $(foreach header,$(QUARK_RUNTIME_H),--header $(header)) $(QUARK_RUNTIME_C)
MIPS_RUNTIME := $(foreach header,$(MIPS_RUNTIME_H),--header $(header)) $(MIPS_RUNTIME_C)
ARM_RUNTIME := $(foreach header,$(ARM_RUNTIME_H),--header $(header)) $(ARM_RUNTIME_C)
AARCH64_RUNTIME := $(foreach header,$(AARCH64_RUNTIME_H),--header $(header)) $(AARCH64_RUNTIME_C)
PPC_RUNTIME := $(foreach header,$(PPC_RUNTIME_H),--header $(header)) $(PPC_RUNTIME_C)
POSIX_RUNTIME := $(COMMON_RUNTIME) $(foreach header,$(POSIX_RUNTIME_H),--header $(header)) $(POSIX_RUNTIME_C)
LINUX_COMMON_RUNTIME := $(POSIX_RUNTIME) $(foreach header,$(LINUX_COMMON_RUNTIME_H),--header $(header)) $(LINUX_COMMON_RUNTIME_C)
LINUX_X86_RUNTIME := $(X86_RUNTIME) $(LINUX_COMMON_RUNTIME) $(foreach header,$(LINUX_X86_RUNTIME_H),--header $(header)) $(LINUX_X86_RUNTIME_C)
LINUX_X64_RUNTIME := $(X64_RUNTIME) $(LINUX_COMMON_RUNTIME) $(foreach header,$(LINUX_X64_RUNTIME_H),--header $(header)) $(LINUX_X64_RUNTIME_C)
LINUX_QUARK_RUNTIME := $(QUARK_RUNTIME) $(LINUX_COMMON_RUNTIME) $(foreach header,$(LINUX_QUARK_RUNTIME_H),--header $(header)) $(LINUX_QUARK_RUNTIME_C)
LINUX_MIPS_RUNTIME := $(MIPS_RUNTIME) $(LINUX_COMMON_RUNTIME) $(foreach header,$(LINUX_MIPS_RUNTIME_H),--header $(header)) $(LINUX_MIPS_RUNTIME_C)
LINUX_ARM_RUNTIME := $(ARM_RUNTIME) $(LINUX_COMMON_RUNTIME) $(foreach header,$(LINUX_ARM_RUNTIME_H),--header $(header)) $(LINUX_ARM_RUNTIME_C)
LINUX_AARCH64_RUNTIME := $(AARCH64_RUNTIME) $(LINUX_COMMON_RUNTIME) $(foreach header,$(LINUX_AARCH64_RUNTIME_H),--header $(header)) $(LINUX_AARCH64_RUNTIME_C)
LINUX_PPC_RUNTIME := $(PPC_RUNTIME) $(LINUX_COMMON_RUNTIME) $(foreach header,$(LINUX_PPC_RUNTIME_H),--header $(header)) $(LINUX_PPC_RUNTIME_C)
FREEBSD_COMMON_RUNTIME := $(POSIX_RUNTIME) $(foreach header,$(FREEBSD_COMMON_RUNTIME_H),--header $(header)) $(FREEBSD_COMMON_RUNTIME_C)
FREEBSD_X86_RUNTIME := $(X86_RUNTIME) $(FREEBSD_COMMON_RUNTIME) $(foreach header,$(FREEBSD_X86_RUNTIME_H),--header $(header)) $(FREEBSD_X86_RUNTIME_C)
FREEBSD_X64_RUNTIME := $(X64_RUNTIME) $(FREEBSD_COMMON_RUNTIME) $(foreach header,$(FREEBSD_X64_RUNTIME_H),--header $(header)) $(FREEBSD_X64_RUNTIME_C)
FREEBSD_QUARK_RUNTIME := $(QUARK_RUNTIME) $(FREEBSD_COMMON_RUNTIME) $(foreach header,$(FREEBSD_QUARK_RUNTIME_H),--header $(header)) $(FREEBSD_QUARK_RUNTIME_C)
MAC_COMMON_RUNTIME := $(POSIX_RUNTIME) $(foreach header,$(MAC_COMMON_RUNTIME_H),--header $(header)) $(MAC_COMMON_RUNTIME_C)
MAC_X86_RUNTIME := $(X86_RUNTIME) $(MAC_COMMON_RUNTIME) $(foreach header,$(MAC_X86_RUNTIME_H),--header $(header)) $(MAC_X86_RUNTIME_C)
MAC_X64_RUNTIME := $(X64_RUNTIME) $(MAC_COMMON_RUNTIME) $(foreach header,$(MAC_X64_RUNTIME_H),--header $(header)) $(MAC_X64_RUNTIME_C)
MAC_QUARK_RUNTIME := $(QUARK_RUNTIME) $(MAC_COMMON_RUNTIME) $(foreach header,$(MAC_QUARK_RUNTIME_H),--header $(header)) $(MAC_QUARK_RUNTIME_C)
WINDOWS_COMMON_RUNTIME := $(COMMON_RUNTIME) $(foreach header,$(WINDOWS_COMMON_RUNTIME_H),--header $(header)) $(WINDOWS_COMMON_RUNTIME_C)
WINDOWS_X86_RUNTIME := $(X86_RUNTIME) $(WINDOWS_COMMON_RUNTIME) $(foreach header,$(WINDOWS_X86_RUNTIME_H),--header $(header)) $(WINDOWS_X86_RUNTIME_C)
WINDOWS_X64_RUNTIME := $(X64_RUNTIME) $(WINDOWS_COMMON_RUNTIME) $(foreach header,$(WINDOWS_X64_RUNTIME_H),--header $(header)) $(WINDOWS_X64_RUNTIME_C)
WINDOWS_QUARK_RUNTIME := $(QUARK_RUNTIME) $(WINDOWS_COMMON_RUNTIME) $(foreach header,$(WINDOWS_QUARK_RUNTIME_H),--header $(header)) $(WINDOWS_QUARK_RUNTIME_C)
WINDOWS_ARM_RUNTIME := $(ARM_RUNTIME) $(WINDOWS_COMMON_RUNTIME) $(foreach header,$(WINDOWS_ARM_RUNTIME_H),--header $(header)) $(WINDOWS_ARM_RUNTIME_C)

-include $(SCC_OBJS:.o=.d)
-include $(SCC_LEX_OBJS:.o=.d)
-include $(SCC_PARSE_OBJS:.o=.d)
-include $(CODEGEN_OBJS:.o=.d)
-include $(CODEGEN_LEX_OBJS:.o=.d)
-include $(CODEGEN_PARSE_OBJS:.o=.d)

Obj/asmx86/makeopstr.o: asmx86/makeopstr.cpp Makefile | Obj/ Obj/asmx86/ Obj/Obj/asmx86/
	$(call COMPILE,$(CXX),$(CPPFLAGS),-Iasmx86,$<,$*)

$(MAKEOPSTR): Obj/asmx86/makeopstr.o Makefile | Obj/ Obj/asmx86/
	$(LINK) $(LDFLAGS) $(call OUTPUT_EXE,$(MAKEOPSTR)) Obj/asmx86/makeopstr.o

asmx86/asmx86str.h: $(MAKEOPSTR) asmx86/asmx86.h Makefile
	$(MAKEOPSTR) asmx86/asmx86.h asmx86/asmx86str.h

$(SCC_OBJS): Obj/%.o: %.cpp $(SCC_LEX_OBJS) $(ASMX86_HEADERS) Makefile | Obj/
	$(call COMPILE,$(CXX),$(CPPFLAGS),-IObj -Iasmx86,$<,$*)

$(CODEGEN_OBJS): Obj/%.o: %.cpp $(CODEGEN_LEX_OBJS) Makefile | Obj/codegen/
	$(call COMPILE,$(CXX),$(CPPFLAGS),-Icodegen -IObj/codegen,$<,$*)

$(RUNTIME_SOURCES): %.cpp: %.lib genlibrary.py Makefile | Obj/
	$(PYTHON) genlibrary.py $< $@

$(RUNTIME_OBJS): Obj/%.o: %.cpp Makefile | Obj/ Obj/Obj/
	$(call COMPILE,$(CXX),$(CPPFLAGS),,$<,$*)

Obj/x86.lib: $(BOOTSTRAP) $(X86_RUNTIME_SRC) $(COMMON_RUNTIME_SRC) Makefile | Obj/
	$(BOOTSTRAP) $(X86_RUNTIME) $(COMMON_RUNTIME) --arch x86 --platform none -f lib -o $@
Obj/x64.lib: $(BOOTSTRAP) $(X64_RUNTIME_SRC) $(COMMON_RUNTIME_SRC) Makefile | Obj/
	$(BOOTSTRAP) $(X64_RUNTIME) $(COMMON_RUNTIME) --arch x64 --platform none -f lib -o $@
Obj/quark.lib: $(BOOTSTRAP) $(QUARK_RUNTIME_SRC) $(COMMON_RUNTIME_SRC) Makefile | Obj/
	$(BOOTSTRAP) $(QUARK_RUNTIME) $(COMMON_RUNTIME) --arch quark --platform none -f lib -o $@
Obj/mips.lib: $(BOOTSTRAP) $(MIPS_RUNTIME_SRC) $(COMMON_RUNTIME_SRC) Makefile | Obj/
	$(BOOTSTRAP) $(MIPS_RUNTIME) $(COMMON_RUNTIME) --arch mips --platform none -f lib -o $@
Obj/mipsel.lib: $(BOOTSTRAP) $(MIPS_RUNTIME_SRC) $(COMMON_RUNTIME_SRC) Makefile | Obj/
	$(BOOTSTRAP) $(MIPS_RUNTIME) $(COMMON_RUNTIME) --arch mipsel --platform none -f lib -o $@
Obj/arm.lib: $(BOOTSTRAP) $(ARM_RUNTIME_SRC) $(COMMON_RUNTIME_SRC) Makefile | Obj/
	$(BOOTSTRAP) $(ARM_RUNTIME) $(COMMON_RUNTIME) --arch arm --platform none -f lib -o $@
Obj/armeb.lib: $(BOOTSTRAP) $(ARM_RUNTIME_SRC) $(COMMON_RUNTIME_SRC) Makefile | Obj/
	$(BOOTSTRAP) $(ARM_RUNTIME) $(COMMON_RUNTIME) --arch armeb --platform none -f lib -o $@
Obj/aarch64.lib: $(BOOTSTRAP) $(AARCH64_RUNTIME_SRC) $(COMMON_RUNTIME_SRC) Makefile | Obj/
	$(BOOTSTRAP) $(AARCH64_RUNTIME) $(COMMON_RUNTIME) --arch aarch64 --platform none -f lib -o $@
Obj/ppc.lib: $(BOOTSTRAP) $(PPC_RUNTIME_SRC) $(COMMON_RUNTIME_SRC) Makefile | Obj/
	$(BOOTSTRAP) $(PPC_RUNTIME) $(COMMON_RUNTIME) --arch ppc --platform none -f lib -o $@
Obj/ppcel.lib: $(BOOTSTRAP) $(PPC_RUNTIME_SRC) $(COMMON_RUNTIME_SRC) Makefile | Obj/
	$(BOOTSTRAP) $(PPC_RUNTIME) $(COMMON_RUNTIME) --arch ppcel --platform none -f lib -o $@
Obj/linux_x86.lib: $(BOOTSTRAP) $(LINUX_X86_RUNTIME_SRC) Makefile | Obj/
	$(BOOTSTRAP) $(LINUX_X86_RUNTIME) --arch x86 --platform linux -f lib -o $@
Obj/linux_x64.lib: $(BOOTSTRAP) $(LINUX_X64_RUNTIME_SRC) Makefile | Obj/
	$(BOOTSTRAP) $(LINUX_X64_RUNTIME) --arch x64 --platform linux -f lib -o $@
Obj/linux_quark.lib: $(BOOTSTRAP) $(LINUX_QUARK_RUNTIME_SRC) Makefile | Obj/
	$(BOOTSTRAP) $(LINUX_QUARK_RUNTIME) --arch quark --platform linux -f lib -o $@
Obj/linux_mips.lib: $(BOOTSTRAP) $(LINUX_MIPS_RUNTIME_SRC) Makefile | Obj/
	$(BOOTSTRAP) $(LINUX_MIPS_RUNTIME) --arch mips --platform linux -f lib -o $@
Obj/linux_mipsel.lib: $(BOOTSTRAP) $(LINUX_MIPS_RUNTIME_SRC) Makefile | Obj/
	$(BOOTSTRAP) $(LINUX_MIPS_RUNTIME) --arch mipsel --platform linux -f lib -o $@
Obj/linux_arm.lib: $(BOOTSTRAP) $(LINUX_ARM_RUNTIME_SRC) Makefile | Obj/
	$(BOOTSTRAP) $(LINUX_ARM_RUNTIME) --arch arm --platform linux -f lib -o $@
Obj/linux_armeb.lib: $(BOOTSTRAP) $(LINUX_ARM_RUNTIME_SRC) Makefile | Obj/
	$(BOOTSTRAP) $(LINUX_ARM_RUNTIME) --arch armeb --platform linux -f lib -o $@
Obj/linux_aarch64.lib: $(BOOTSTRAP) $(LINUX_AARCH64_RUNTIME_SRC) Makefile | Obj/
	$(BOOTSTRAP) $(LINUX_AARCH64_RUNTIME) --arch aarch64 --platform linux -f lib -o $@
Obj/linux_ppc.lib: $(BOOTSTRAP) $(LINUX_PPC_RUNTIME_SRC) Makefile | Obj/
	$(BOOTSTRAP) $(LINUX_PPC_RUNTIME) --arch ppc --platform linux -f lib -o $@
Obj/linux_ppcel.lib: $(BOOTSTRAP) $(LINUX_PPC_RUNTIME_SRC) Makefile | Obj/
	$(BOOTSTRAP) $(LINUX_PPC_RUNTIME) --arch ppcel --platform linux -f lib -o $@
Obj/freebsd_x86.lib: $(BOOTSTRAP) $(FREEBSD_X86_RUNTIME_SRC) Makefile | Obj/
	$(BOOTSTRAP) $(FREEBSD_X86_RUNTIME) --arch x86 --platform freebsd -f lib -o $@
Obj/freebsd_x64.lib: $(BOOTSTRAP) $(FREEBSD_X64_RUNTIME_SRC) Makefile | Obj/
	$(BOOTSTRAP) $(FREEBSD_X64_RUNTIME) --arch x64 --platform freebsd -f lib -o $@
Obj/freebsd_quark.lib: $(BOOTSTRAP) $(FREEBSD_QUARK_RUNTIME_SRC) Makefile | Obj/
	$(BOOTSTRAP) $(FREEBSD_QUARK_RUNTIME) --arch quark --platform freebsd -f lib -o $@
Obj/mac_x86.lib: $(BOOTSTRAP) $(MAC_X86_RUNTIME_SRC) Makefile | Obj/
	$(BOOTSTRAP) $(MAC_X86_RUNTIME) --arch x86 --platform mac -f lib -o $@
Obj/mac_x64.lib: $(BOOTSTRAP) $(MAC_X64_RUNTIME_SRC) Makefile | Obj/
	$(BOOTSTRAP) $(MAC_X64_RUNTIME) --arch x64 --platform mac -f lib -o $@
Obj/mac_quark.lib: $(BOOTSTRAP) $(MAC_QUARK_RUNTIME_SRC) Makefile | Obj/
	$(BOOTSTRAP) $(MAC_QUARK_RUNTIME) --arch quark --platform mac -f lib -o $@
Obj/windows_x86.lib: $(BOOTSTRAP) $(WINDOWS_X86_RUNTIME_SRC) Makefile | Obj/
	$(BOOTSTRAP) $(WINDOWS_X86_RUNTIME) --arch x86 --platform windows -f lib -o $@
Obj/windows_x64.lib: $(BOOTSTRAP) $(WINDOWS_X64_RUNTIME_SRC) Makefile | Obj/
	$(BOOTSTRAP) $(WINDOWS_X64_RUNTIME) --arch x64 --platform windows -f lib -o $@
Obj/windows_quark.lib: $(BOOTSTRAP) $(WINDOWS_QUARK_RUNTIME_SRC) Makefile | Obj/
	$(BOOTSTRAP) $(WINDOWS_QUARK_RUNTIME) --arch quark --platform windows -f lib -o $@
Obj/windows_arm.lib: $(BOOTSTRAP) $(WINDOWS_ARM_RUNTIME_SRC) Makefile | Obj/
	$(BOOTSTRAP) $(WINDOWS_QUARK_RUNTIME) --arch arm --platform windows -f lib -o $@

$(ASMX86_OBJS): Obj/%.o: %.c $(ASMX86_HEADERS) Makefile | Obj/asmx86/
	$(call COMPILE,$(CC),$(CFLAGS),,$<,$*)

$(SCC_LEX_OBJS): Obj/%Lexer.o: %.lex $(SCC_PARSE_OBJS) $(ASMX86_HEADERS) Makefile | Obj/
	flex -R --prefix=$(lastword $(subst /, ,$*))_ -o Obj/$*Lexer.cpp --header=Obj/$*Lexer.h $<
	$(call COMPILE,$(CXX),$(PARSER_CPPFLAGS),-I. -IObj -Iasmx86,Obj/$*Lexer.cpp,$*Lexer)

$(SCC_PARSE_OBJS): Obj/%Parser.o: %.y $(ASMX86_HEADERS) Makefile | Obj/
	bison --name-prefix=$(lastword $(subst /, ,$*))_ -o Obj/$*Parser.cpp --defines=Obj/$*Parser.h $<
	$(call COMPILE,$(CXX),$(PARSER_CPPFLAGS),-I. -IObj -Iasmx86,Obj/$*Parser.cpp,$*Parser)

$(CODEGEN_LEX_OBJS): Obj/%Lexer.o: %.lex $(CODEGEN_PARSE_OBJS) Makefile | Obj/codegen/
	flex -R --prefix=$(lastword $(subst /, ,$*))_ -o Obj/$*Lexer.cpp --header=Obj/$*Lexer.h $<
	$(call COMPILE,$(CXX),$(PARSER_CPPFLAGS),-Icodegen -IObj/codegen,Obj/$*Lexer.cpp,$*Lexer)

$(CODEGEN_PARSE_OBJS): Obj/%Parser.o: %.y Makefile | Obj/codegen/
	bison --name-prefix=$(lastword $(subst /, ,$*))_ -o Obj/$*Parser.cpp --defines=Obj/$*Parser.h $<
	$(call COMPILE,$(CXX),$(PARSER_CPPFLAGS),-Icodegen -IObj/codegen,Obj/$*Parser.cpp,$*Parser)

$(SCC_CODEGEN_OBJS): Obj/%CodeGen.o: %.cgen $(CODEGEN) Makefile | Obj/
	$(CODEGEN) -o Obj/$*CodeGen.cpp $<
	$(call COMPILE_ONLY,$(CXX),$(CPPFLAGS),-I. -IObj -Iasmx86,Obj/$*CodeGen.cpp,$*CodeGen)

Obj/Bootstrap.cpp: Bootstrap.inc Makefile | Obj/
	cp Bootstrap.inc Obj/Bootstrap.cpp

Obj/Bootstrap.o: Obj/Bootstrap.cpp Makefile | Obj/ Obj/Obj/
	$(call COMPILE,$(CXX),$(CPPFLAGS),,$<,$*)

ifeq ($(CONFIG),release)
Obj/Version.cpp: Makefile | Obj/
	$(MAKE_VERSION)

$(VERSION_OBJ): Obj/%.o: %.cpp Makefile | Obj/ Obj/Obj/
	$(call COMPILE,$(CXX),$(CPPFLAGS),,$<,$*)
endif

$(CODEGEN): $(CODEGEN_OBJS) $(CODEGEN_LEX_OBJS) $(CODEGEN_PARSE_OBJS) Makefile | Obj/codegen/
	$(LINK) $(LDFLAGS) $(call OUTPUT_EXE,$(CODEGEN)) $(CODEGEN_OBJS) $(CODEGEN_LEX_OBJS) $(CODEGEN_PARSE_OBJS)

$(BOOTSTRAP): $(SCC_OBJS) $(SCC_LEX_OBJS) $(SCC_PARSE_OBJS) $(SCC_CODEGEN_OBJS) $(ASMX86_OBJS) $(VERSION_OBJ) Obj/Bootstrap.o Makefile | Obj/
	$(LINK) $(LDFLAGS) $(call OUTPUT_EXE,$(BOOTSTRAP)) $(SCC_OBJS) $(SCC_LEX_OBJS) $(SCC_PARSE_OBJS) $(SCC_CODEGEN_OBJS) $(ASMX86_OBJS) $(VERSION_OBJ) Obj/Bootstrap.o

$(TARGET): $(SCC_OBJS) $(SCC_LEX_OBJS) $(SCC_PARSE_OBJS) $(SCC_CODEGEN_OBJS) $(ASMX86_OBJS) $(RUNTIME_OBJS) $(VERSION_OBJ) Makefile
	$(LINK) $(LDFLAGS) $(call OUTPUT_EXE,$(TARGET)) $(SCC_OBJS) $(SCC_LEX_OBJS) $(SCC_PARSE_OBJS) $(SCC_CODEGEN_OBJS) $(ASMX86_OBJS) $(RUNTIME_OBJS) $(VERSION_OBJ)
ifeq ($(STRIP),yes)
	strip $(TARGET)
endif

# Cleaning rule
clean :
	rm -f $(TARGET)
	rm -rf Obj
	rm -f asmx86/asmx86str.h

test: scc
	$(PYTHON) tests/test.py
