# allemande

A port of [allemand](https://github.com/evelyneee/allemand) by [evelyneee](https://github.com/evelyneee) to C++.

Converts old ABI arm64e Mach-O binaries to work on latest ABI.

Usage:

```
allemande file.dylib [out.dylib]
```

Invoke the above on fat binary, and then re-sign it with:
```
ldid -s out.dylib
ldid -S out.dylib
```

## Compilation

### Windows

MSVC:
```
cl /std:c++20 /EHsc /Feallemande.exe main.cpp
```

### Linux/Mac

GCC:
```
g++ -std=c++20 -o allemande main.cpp
```

Clang:
```
clang++ -std=c++20 -o allemande main.cpp
```

If you're compiling it on iOS itself, remember to also sign it with `ldid -S`
