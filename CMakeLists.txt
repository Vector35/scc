cmake_minimum_required(VERSION 3.9 FATAL_ERROR)

project(scc)

set(INTERMEDIATE ${PROJECT_BINARY_DIR}/Obj)
file(MAKE_DIRECTORY ${INTERMEDIATE})
file(MAKE_DIRECTORY ${INTERMEDIATE}/codegen)

file(GLOB CODEGEN_SOURCES
	${PROJECT_SOURCE_DIR}/codegen/*.cpp
	${PROJECT_SOURCE_DIR}/codegen/*.h)

file(GLOB COMPILER_SOURCES
	${PROJECT_SOURCE_DIR}/*.cpp
	${PROJECT_SOURCE_DIR}/*.h
	${PROJECT_SOURCE_DIR}/asmx86/asmx86.c
	${PROJECT_SOURCE_DIR}/asmx86/asmx86.h)

add_executable(codegen ${CODEGEN_SOURCES}
	${INTERMEDIATE}/codegen/PreprocessLexer.cpp
	${INTERMEDIATE}/codegen/PreprocessLexer.h
	${INTERMEDIATE}/codegen/PreprocessParser.cpp
	${INTERMEDIATE}/codegen/PreprocessParser.h
	${INTERMEDIATE}/codegen/CodegenLexer.cpp
	${INTERMEDIATE}/codegen/CodegenLexer.h
	${INTERMEDIATE}/codegen/CodegenParser.cpp
	${INTERMEDIATE}/codegen/CodegenParser.h)

add_custom_command(OUTPUT ${INTERMEDIATE}/codegen/PreprocessLexer.cpp ${INTERMEDIATE}/codegen/PreprocessLexer.h
	DEPENDS ${PROJECT_SOURCE_DIR}/codegen/Preprocess.lex ${INTERMEDIATE}/codegen/PreprocessParser.h
	COMMAND flex -R --prefix=Preprocess_ -o ${INTERMEDIATE}/codegen/PreprocessLexer.cpp --header=${INTERMEDIATE}/codegen/PreprocessLexer.h ${PROJECT_SOURCE_DIR}/codegen/Preprocess.lex)
add_custom_command(OUTPUT ${INTERMEDIATE}/codegen/PreprocessParser.cpp ${INTERMEDIATE}/codegen/PreprocessParser.h
	DEPENDS ${PROJECT_SOURCE_DIR}/codegen/Preprocess.y
	COMMAND bison --name-prefix=Preprocess_ -o ${INTERMEDIATE}/codegen/PreprocessParser.cpp --defines=${INTERMEDIATE}/codegen/PreprocessParser.h ${PROJECT_SOURCE_DIR}/codegen/Preprocess.y)
add_custom_command(OUTPUT ${INTERMEDIATE}/codegen/CodegenLexer.cpp ${INTERMEDIATE}/codegen/CodegenLexer.h
	DEPENDS ${PROJECT_SOURCE_DIR}/codegen/Codegen.lex ${INTERMEDIATE}/codegen/CodegenParser.h
	COMMAND flex -R --prefix=Codegen_ -o ${INTERMEDIATE}/codegen/CodegenLexer.cpp --header=${INTERMEDIATE}/codegen/CodegenLexer.h ${PROJECT_SOURCE_DIR}/codegen/Codegen.lex)
add_custom_command(OUTPUT ${INTERMEDIATE}/codegen/CodegenParser.cpp ${INTERMEDIATE}/codegen/CodegenParser.h
	DEPENDS ${PROJECT_SOURCE_DIR}/codegen/Codegen.y
	COMMAND bison --name-prefix=Codegen_ -o ${INTERMEDIATE}/codegen/CodegenParser.cpp --defines=${INTERMEDIATE}/codegen/CodegenParser.h ${PROJECT_SOURCE_DIR}/codegen/Codegen.y)

target_include_directories(codegen PRIVATE ${INTERMEDIATE}/codegen ${PROJECT_SOURCE_DIR}/codegen)

set_target_properties(codegen PROPERTIES
    CXX_STANDARD 11
	CXX_STANDARD_REQUIRED ON
	BUILD_WITH_INSTALL_RPATH OFF
	RUNTIME_OUTPUT_DIRECTORY ${INTERMEDIATE}/codegen)

execute_process(COMMAND git describe --tags HEAD WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
	OUTPUT_VARIABLE REV OUTPUT_STRIP_TRAILING_WHITESPACE)

if(WIN32)
	execute_process(COMMAND py -3 -c "print('${REV}'[1:].replace('-','.').split('.')[0])" OUTPUT_VARIABLE MAJOR
		OUTPUT_STRIP_TRAILING_WHITESPACE)
	execute_process(COMMAND py -3 -c "print('${REV}'[1:].replace('-','.').split('.')[1])" OUTPUT_VARIABLE MINOR
		OUTPUT_STRIP_TRAILING_WHITESPACE)
	execute_process(COMMAND py -3 -c "print('${REV}'[1:].replace('-','.').split('.')[2])" OUTPUT_VARIABLE BUILD
		OUTPUT_STRIP_TRAILING_WHITESPACE)
else()
	execute_process(COMMAND python3 -c "print('${REV}'[1:].replace('-','.').split('.')[0])" OUTPUT_VARIABLE MAJOR
		OUTPUT_STRIP_TRAILING_WHITESPACE)
	execute_process(COMMAND python3 -c "print('${REV}'[1:].replace('-','.').split('.')[1])" OUTPUT_VARIABLE MINOR
		OUTPUT_STRIP_TRAILING_WHITESPACE)
	execute_process(COMMAND python3 -c "print('${REV}'[1:].replace('-','.').split('.')[2])" OUTPUT_VARIABLE BUILD
		OUTPUT_STRIP_TRAILING_WHITESPACE)
endif()

add_custom_command(OUTPUT ${INTERMEDIATE}/Version.cpp
	COMMAND ${CMAKE_COMMAND} -E echo \"const char* g_versionString = \\\"${MAJOR}.${MINOR}.${BUILD}\\\"\;\" > ${INTERMEDIATE}/Version.cpp)

add_executable(makeopstr ${PROJECT_SOURCE_DIR}/asmx86/makeopstr.cpp)

set_target_properties(makeopstr PROPERTIES
    CXX_STANDARD 11
	CXX_STANDARD_REQUIRED ON
	CXX_VISIBILITY_PRESET hidden
	VISIBILITY_INLINES_HIDDEN ON
	BUILD_WITH_INSTALL_RPATH OFF
	RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR})

add_custom_command(OUTPUT ${PROJECT_SOURCE_DIR}/asmx86/asmx86str.h
	DEPENDS ${PROJECT_SOURCE_DIR}/asmx86/asmx86.h $<TARGET_FILE:makeopstr>
	COMMAND $<TARGET_FILE:makeopstr> ${PROJECT_SOURCE_DIR}/asmx86/asmx86.h ${PROJECT_SOURCE_DIR}/asmx86/asmx86str.h
)

add_library(scc_common ${COMPILER_SOURCES}
	${INTERMEDIATE}/PreprocessLexer.cpp
	${INTERMEDIATE}/PreprocessLexer.h
	${INTERMEDIATE}/PreprocessParser.cpp
	${INTERMEDIATE}/PreprocessParser.h
	${INTERMEDIATE}/CodeLexer.cpp
	${INTERMEDIATE}/CodeLexer.h
	${INTERMEDIATE}/CodeParser.cpp
	${INTERMEDIATE}/CodeParser.h
	${INTERMEDIATE}/PpcCodeGen.cpp
	${INTERMEDIATE}/AArch64CodeGen.cpp
	${INTERMEDIATE}/ArmCodeGen.cpp
	${INTERMEDIATE}/QuarkCodeGen.cpp
	${INTERMEDIATE}/MipsCodeGen.cpp
	${INTERMEDIATE}/Version.cpp
	${PROJECT_SOURCE_DIR}/asmx86/asmx86str.h)

add_custom_command(OUTPUT ${INTERMEDIATE}/PreprocessLexer.cpp ${INTERMEDIATE}/PreprocessLexer.h
	DEPENDS ${PROJECT_SOURCE_DIR}/Preprocess.lex ${INTERMEDIATE}/PreprocessParser.h
	COMMAND flex -R --prefix=Preprocess_ -o ${INTERMEDIATE}/PreprocessLexer.cpp --header=${INTERMEDIATE}/PreprocessLexer.h ${PROJECT_SOURCE_DIR}/Preprocess.lex)
add_custom_command(OUTPUT ${INTERMEDIATE}/PreprocessParser.cpp ${INTERMEDIATE}/PreprocessParser.h
	DEPENDS ${PROJECT_SOURCE_DIR}/Preprocess.y
	COMMAND bison --name-prefix=Preprocess_ -o ${INTERMEDIATE}/PreprocessParser.cpp --defines=${INTERMEDIATE}/PreprocessParser.h ${PROJECT_SOURCE_DIR}/Preprocess.y)
add_custom_command(OUTPUT ${INTERMEDIATE}/CodeLexer.cpp ${INTERMEDIATE}/CodeLexer.h
	DEPENDS ${PROJECT_SOURCE_DIR}/Code.lex ${INTERMEDIATE}/CodeParser.h
	COMMAND flex -R --prefix=Code_ -o ${INTERMEDIATE}/CodeLexer.cpp --header=${INTERMEDIATE}/CodeLexer.h ${PROJECT_SOURCE_DIR}/Code.lex)
add_custom_command(OUTPUT ${INTERMEDIATE}/CodeParser.cpp ${INTERMEDIATE}/CodeParser.h
	DEPENDS ${PROJECT_SOURCE_DIR}/Code.y
	COMMAND bison --name-prefix=Code_ -o ${INTERMEDIATE}/CodeParser.cpp --defines=${INTERMEDIATE}/CodeParser.h ${PROJECT_SOURCE_DIR}/Code.y)

add_custom_command(OUTPUT ${INTERMEDIATE}/PpcCodeGen.cpp
	DEPENDS codegen ${PROJECT_SOURCE_DIR}/Ppc.cgen
	COMMAND ${CMAKE_COMMAND} -E env ASAN_OPTIONS=detect_leaks=0 $<TARGET_FILE:codegen> -o ${INTERMEDIATE}/PpcCodeGen.cpp ${PROJECT_SOURCE_DIR}/Ppc.cgen)
add_custom_command(OUTPUT ${INTERMEDIATE}/AArch64CodeGen.cpp
	DEPENDS codegen ${PROJECT_SOURCE_DIR}/AArch64.cgen
	COMMAND ${CMAKE_COMMAND} -E env ASAN_OPTIONS=detect_leaks=0 $<TARGET_FILE:codegen> -o ${INTERMEDIATE}/AArch64CodeGen.cpp ${PROJECT_SOURCE_DIR}/AArch64.cgen)
add_custom_command(OUTPUT ${INTERMEDIATE}/ArmCodeGen.cpp
	DEPENDS codegen ${PROJECT_SOURCE_DIR}/Arm.cgen
	COMMAND ${CMAKE_COMMAND} -E env ASAN_OPTIONS=detect_leaks=0 $<TARGET_FILE:codegen> -o ${INTERMEDIATE}/ArmCodeGen.cpp ${PROJECT_SOURCE_DIR}/Arm.cgen)
add_custom_command(OUTPUT ${INTERMEDIATE}/QuarkCodeGen.cpp
	DEPENDS codegen ${PROJECT_SOURCE_DIR}/Quark.cgen
	COMMAND ${CMAKE_COMMAND} -E env ASAN_OPTIONS=detect_leaks=0 $<TARGET_FILE:codegen> -o ${INTERMEDIATE}/QuarkCodeGen.cpp ${PROJECT_SOURCE_DIR}/Quark.cgen)
add_custom_command(OUTPUT ${INTERMEDIATE}/MipsCodeGen.cpp
	DEPENDS codegen ${PROJECT_SOURCE_DIR}/Mips.cgen
	COMMAND ${CMAKE_COMMAND} -E env ASAN_OPTIONS=detect_leaks=0 $<TARGET_FILE:codegen> -o ${INTERMEDIATE}/MipsCodeGen.cpp ${PROJECT_SOURCE_DIR}/Mips.cgen)

target_include_directories(scc_common PRIVATE ${INTERMEDIATE} ${PROJECT_SOURCE_DIR} ${PROJECT_SOURCE_DIR}/asmx86)
target_compile_definitions(scc_common PRIVATE RELEASE)

set_target_properties(scc_common PROPERTIES
    CXX_STANDARD 11
	CXX_STANDARD_REQUIRED ON
	CXX_VISIBILITY_PRESET hidden
	C_STANDARD 99
	C_STANDARD_REQUIRED ON
	C_VISIBILITY_PRESET hidden
	VISIBILITY_INLINES_HIDDEN ON
	BUILD_WITH_INSTALL_RPATH OFF
	ARCHIVE_OUTPUT_DIRECTORY ${INTERMEDIATE})

add_executable(scc_bootstrap ${INTERMEDIATE}/Bootstrap.cpp)

add_custom_command(OUTPUT ${INTERMEDIATE}/Bootstrap.cpp
	DEPENDS ${PROJECT_SOURCE_DIR}/Bootstrap.inc
	COMMAND ${CMAKE_COMMAND} -E copy ${PROJECT_SOURCE_DIR}/Bootstrap.inc ${INTERMEDIATE}/Bootstrap.cpp)

target_link_libraries(scc_bootstrap PRIVATE scc_common)

set_target_properties(scc_bootstrap PROPERTIES
    CXX_STANDARD 11
	CXX_STANDARD_REQUIRED ON
	CXX_VISIBILITY_PRESET hidden
	VISIBILITY_INLINES_HIDDEN ON
	BUILD_WITH_INSTALL_RPATH OFF
	RUNTIME_OUTPUT_DIRECTORY ${INTERMEDIATE})

macro(runtime_options OUT_VAR LIB_PATH OS_PARENT ARCH_PARENT)
	file(GLOB H_FILES ${LIB_PATH}/*.h)
	file(GLOB C_FILES ${LIB_PATH}/*.c)
	set(${OUT_VAR} ${${ARCH_PARENT}} ${${OS_PARENT}})
	foreach(HEADER ${H_FILES})
		list(APPEND ${OUT_VAR} --header ${HEADER})
	endforeach()
	list(APPEND ${OUT_VAR} ${C_FILES})
endmacro()

set(NONE "")
runtime_options(COMMON_RUNTIME runtime NONE NONE)
runtime_options(X86_RUNTIME runtime/x86 COMMON_RUNTIME NONE)
runtime_options(X64_RUNTIME runtime/x64 COMMON_RUNTIME NONE)
runtime_options(QUARK_RUNTIME runtime/quark COMMON_RUNTIME NONE)
runtime_options(MIPS_RUNTIME runtime/mips COMMON_RUNTIME NONE)
runtime_options(ARM_RUNTIME runtime/arm COMMON_RUNTIME NONE)
runtime_options(AARCH64_RUNTIME runtime/aarch64 COMMON_RUNTIME NONE)
runtime_options(PPC_RUNTIME runtime/ppc COMMON_RUNTIME NONE)
runtime_options(POSIX_RUNTIME runtime/posix NONE NONE)
runtime_options(LINUX_COMMON_RUNTIME runtime/linux POSIX_RUNTIME NONE)
runtime_options(LINUX_X86_RUNTIME runtime/linux/x86 LINUX_COMMON_RUNTIME X86_RUNTIME)
runtime_options(LINUX_X64_RUNTIME runtime/linux/x64 LINUX_COMMON_RUNTIME X64_RUNTIME)
runtime_options(LINUX_QUARK_RUNTIME runtime/linux/x86 LINUX_COMMON_RUNTIME QUARK_RUNTIME)
runtime_options(LINUX_MIPS_RUNTIME runtime/linux/mips LINUX_COMMON_RUNTIME MIPS_RUNTIME)
runtime_options(LINUX_ARM_RUNTIME runtime/linux/arm LINUX_COMMON_RUNTIME ARM_RUNTIME)
runtime_options(LINUX_AARCH64_RUNTIME runtime/linux/aarch64 LINUX_COMMON_RUNTIME AARCH64_RUNTIME)
runtime_options(LINUX_PPC_RUNTIME runtime/linux/ppc LINUX_COMMON_RUNTIME PPC_RUNTIME)
runtime_options(FREEBSD_COMMON_RUNTIME runtime/freebsd POSIX_RUNTIME NONE)
runtime_options(FREEBSD_X86_RUNTIME runtime/freebsd/x86 FREEBSD_COMMON_RUNTIME X86_RUNTIME)
runtime_options(FREEBSD_X64_RUNTIME runtime/freebsd/x64 FREEBSD_COMMON_RUNTIME X64_RUNTIME)
runtime_options(FREEBSD_QUARK_RUNTIME runtime/freebsd/x86 FREEBSD_COMMON_RUNTIME QUARK_RUNTIME)
runtime_options(MAC_COMMON_RUNTIME runtime/mac POSIX_RUNTIME NONE)
runtime_options(MAC_X86_RUNTIME runtime/mac/x86 MAC_COMMON_RUNTIME X86_RUNTIME)
runtime_options(MAC_X64_RUNTIME runtime/mac/x64 MAC_COMMON_RUNTIME X64_RUNTIME)
runtime_options(MAC_QUARK_RUNTIME runtime/mac/x86 MAC_COMMON_RUNTIME QUARK_RUNTIME)
runtime_options(WINDOWS_COMMON_RUNTIME runtime/windows NONE NONE)
runtime_options(WINDOWS_X86_RUNTIME runtime/windows/x86 WINDOWS_COMMON_RUNTIME X86_RUNTIME)
runtime_options(WINDOWS_X64_RUNTIME runtime/windows/x64 WINDOWS_COMMON_RUNTIME X64_RUNTIME)
runtime_options(WINDOWS_QUARK_RUNTIME runtime/windows/x86 WINDOWS_COMMON_RUNTIME QUARK_RUNTIME)
runtime_options(WINDOWS_ARM_RUNTIME runtime/windows/arm WINDOWS_COMMON_RUNTIME ARM_RUNTIME)

set(LIBRARIES "")
macro(generate_library LIB_NAME ARCH PLATFORM RUNTIME)
	add_custom_command(OUTPUT ${INTERMEDIATE}/${LIB_NAME}.lib
		DEPENDS scc_bootstrap
		COMMAND ${CMAKE_COMMAND} -E env ASAN_OPTIONS=detect_leaks=0 $<TARGET_FILE:scc_bootstrap> ${${RUNTIME}} -f lib -o ${INTERMEDIATE}/${LIB_NAME}.lib
		WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
		COMMENT "Generating runtime library ${LIB_NAME}")

	if(WIN32)
		add_custom_command(OUTPUT ${INTERMEDIATE}/${LIB_NAME}.cpp
			DEPENDS ${INTERMEDIATE}/${LIB_NAME}.lib
			COMMAND py -3 ${PROJECT_SOURCE_DIR}/genlibrary.py ${INTERMEDIATE}/${LIB_NAME}.lib ${INTERMEDIATE}/${LIB_NAME}.cpp Obj/${LIB_NAME}.lib)
	else()
		add_custom_command(OUTPUT ${INTERMEDIATE}/${LIB_NAME}.cpp
			DEPENDS ${INTERMEDIATE}/${LIB_NAME}.lib
			COMMAND python3 ${PROJECT_SOURCE_DIR}/genlibrary.py ${INTERMEDIATE}/${LIB_NAME}.lib ${INTERMEDIATE}/${LIB_NAME}.cpp Obj/${LIB_NAME}.lib)
	endif()

	list(APPEND LIBRARIES ${INTERMEDIATE}/${LIB_NAME}.cpp)
endmacro()

generate_library(x86 x86 none X86_RUNTIME)
generate_library(x64 x64 none X64_RUNTIME)
generate_library(quark quark none QUARK_RUNTIME)
generate_library(mips mips none MIPS_RUNTIME)
generate_library(mipsel mipsel none MIPS_RUNTIME)
generate_library(arm arm none ARM_RUNTIME)
generate_library(armeb armeb none ARM_RUNTIME)
generate_library(aarch64 aarch64 none AARCH64_RUNTIME)
generate_library(ppc ppc none PPC_RUNTIME)
generate_library(ppcel ppcel none PPC_RUNTIME)
generate_library(linux_x86 x86 linux LINUX_X86_RUNTIME)
generate_library(linux_x64 x64 linux LINUX_X64_RUNTIME)
generate_library(linux_quark quark linux LINUX_QUARK_RUNTIME)
generate_library(linux_mips mips linux LINUX_MIPS_RUNTIME)
generate_library(linux_mipsel mipsel linux LINUX_MIPS_RUNTIME)
generate_library(linux_arm arm linux LINUX_ARM_RUNTIME)
generate_library(linux_armeb armeb linux LINUX_ARM_RUNTIME)
generate_library(linux_aarch64 aarch64 linux LINUX_AARCH64_RUNTIME)
generate_library(linux_ppc ppc linux LINUX_PPC_RUNTIME)
generate_library(linux_ppcel ppcel linux LINUX_PPC_RUNTIME)
generate_library(freebsd_x86 x86 freebsd FREEBSD_X86_RUNTIME)
generate_library(freebsd_x64 x64 freebsd FREEBSD_X64_RUNTIME)
generate_library(freebsd_quark quark freebsd FREEBSD_QUARK_RUNTIME)
generate_library(mac_x86 x86 mac MAC_X86_RUNTIME)
generate_library(mac_x64 x64 mac MAC_X64_RUNTIME)
generate_library(mac_quark quark mac MAC_QUARK_RUNTIME)
generate_library(windows_x86 x86 windows WINDOWS_X86_RUNTIME)
generate_library(windows_x64 x64 windows WINDOWS_X64_RUNTIME)
generate_library(windows_quark quark windows WINDOWS_QUARK_RUNTIME)
generate_library(windows_arm arm windows WINDOWS_ARM_RUNTIME)
generate_library(windows_armeb armeb windows WINDOWS_ARM_RUNTIME)

add_executable(scc ${LIBRARIES})

target_link_libraries(scc PRIVATE scc_common)

set_target_properties(scc PROPERTIES
    CXX_STANDARD 11
	CXX_STANDARD_REQUIRED ON
	CXX_VISIBILITY_PRESET hidden
	VISIBILITY_INLINES_HIDDEN ON
	BUILD_WITH_INSTALL_RPATH OFF
	RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR})

if(BN_INTERNAL_BUILD)
	add_custom_command(TARGET scc POST_BUILD
		COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:scc> ${BN_CORE_PLUGIN_DIR}/scc)
endif()
