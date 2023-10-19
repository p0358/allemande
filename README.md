# allemande

A port of [allemand](https://github.com/evelyneee/allemand) by [evelyneee](https://github.com/evelyneee) to C++.

Converts old ABI arm64e Mach-O binaries to work on latest ABI.

**NOTE:** This is not a complete solution, and while it works well enough for many tweaks, nobody can guarantee it'll work. Especially it seems to **not work with tweaks using Swift**, which includes any tweak relying on the Cephei library.

Usage:

```
allemande file.dylib [out.dylib]
```

Invoke the above on fat (final) binary, and then re-sign it with:
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

## Usage with Theos

Edit the file `$THEOS/makefiles/instance/rules.mk`, find the line `$(ECHO_MERGING)$(ECHO_UNBUFFERED)$(TARGET_LIPO) $(foreach ARCH,$(TARGET_ARCHS),-arch $(ARCH) $(THEOS_OBJ_DIR)/$(ARCH)/$(1)) -create -output "$$@"$(ECHO_END)` and insert the following underneath:
```makefile
ifeq ($(THEOS_PACKAGE_SCHEME),rootless)
ifneq ($(filter arm64e,$(TARGET_ARCHS)),)
        /path/to/allemande "$$@"
endif
endif
```

(here it assumes that you only want to run it through allemande if building for rootless, and if target archs include arm64e – all rootful jailbreaks should not require the usage of new ABI...)
