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

```
cl /std:c++latest /EHsc /Feallemande.exe main.cpp
```
