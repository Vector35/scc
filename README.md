# Shellcode Compiler

The Shellcode Compiler started its life as an internal CTF tool before it was re-purposed to be the compiler integrated into Binary Ninja.

With the 5.0 release of [Binary Ninja](https://binary.ninja/), this repository was open-sourced. In the future, it's likely that SCC may be migrated into the main [binaryninja-api](https://github.com/Vector35/binaryninja-api/) repository.

Long-term our plan is to replace scc with a version of LLVM using the appropriate compiler flags for minimal shellcode-style codegen. (We're already embedding multiple copies of LLVM -- one for the type parse and one for the debugger, so this need not be as much of a burden as it might sound.)

Note that scc is not being actively maintained, however pull-requests and [issues](https://github.com/Vector35/binaryninja-api/issues?q=is%3Aissue%20state%3Aopen%20label%3A%22Component%3A%20SCC%22) are welcome.

## Documentation

Online documentation is available at: [https://scc.binary.ninja/](https://scc.binary.ninja/)

## Usage and Build Instructions

The build system uses cmake:

```
$ git clone https://github.com/vector35/scc
$ cd scc
$ cmake -S . -B build
...
$ cmake --build build
```

## Licensing

Some components may be released under compatible but slightly different open source licenses and should have their own LICENSE file as appropriate.

Remaining components are released under an [MIT](https://github.com/Vector35/scc/blob/dev/LICENSE.txt) license.
