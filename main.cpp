#include <iostream>
#include <fstream>
#include <span>
#include <string>
#include <vector>

#include "MemoryAddress.hpp"

#include "extern/MachO.hpp"
using namespace MachO;

uint64_t cfStringISA = 0;

std::string read_file_to_string(std::ifstream& f)
{
    std::string fileData;
    f.seekg(0, std::ios::end);
    fileData.reserve(f.tellg());
    f.seekg(0, std::ios::beg);

    fileData.assign((std::istreambuf_iterator<char>(f)), std::istreambuf_iterator<char>());
    return fileData;
}

void convert_slice(const MemoryAddress d)
{
    auto header = d.GetValue<mach_header_64>();
    //swapStruct(header);
    std::cout << "Slice header: " << std::hex << header.magic << std::endl;
    std::cout << "Slice cputype: " << std::hex << header.cputype << std::endl;
    std::cout << "Slice cpusubtype: " << std::hex << header.cpusubtype << std::endl;

    if (header.magic == MH_MAGIC) { return; } // not caring about non-64bit

    if (header.magic != MH_MAGIC_64)
    {
        std::cerr << "ERROR: Unexpected mach header (expected MH_MAGIC_64), got:" << std::hex << header.magic << std::endl;
        return;
    }

    if (header.cputype != CPU_TYPE_ARM64) { return; } // not caring about others (like armv7)

    if (CPU_SUBTYPE_ARM64E_IS_VERSIONED_PTRAUTH_ABI(header.cpusubtype) && CPU_SUBTYPE_ARM64E_PTRAUTH_VERSION(header.cpusubtype) > 0)
    {
        std::cerr << "ERROR: Unsupported arm64e versioned ABI version: " << std::to_string(CPU_SUBTYPE_ARM64E_PTRAUTH_VERSION(header.cpusubtype)) << std::endl;
        return;
    }

    if (!(header.cpusubtype == CPU_SUBTYPE_ARM64E || header.cpusubtype == CPU_SUBTYPE_ARM64E_WITH_PTRAUTH_VERSION(0, false)))
    {
        return; // not arm64e that we care about
    }

    //////////////////////////////////////////////////////////

    uint64_t index = sizeof(mach_header_64);
    for (auto _ = 0; _ < header.ncmds; _++)
    {
        auto cmd_d = d.Offset(index);
        auto& cmd = cmd_d.GetValueRef<load_command>();

        if (cmd.cmd == LoadCommandType::LC_SEGMENT_64)
        {
            auto& segment_cmd = cmd_d.GetValueRef<segment_command_64>();
            printf("\n===== SEGMENT NAME: %s =====\n", segment_cmd.segname);

            for (int i = 0; i < segment_cmd.nsects; i++)
            {
                auto sec_d = cmd_d.Offset(sizeof(segment_command_64)).Offset(i * sizeof(section_64));
                auto& section = sec_d.GetValueRef<section_64>();
                printf("%s\n", section.sectname);
                std::string_view sectname{ section.sectname };

                if (sectname.starts_with("__objc_data"))
                {
                    struct objc_class
                    {
                        uint64_t isa;
                        uint64_t superclass;
                        uint64_t cache;
                        uint64_t vtable;
                        uint64_t data;
                    };
                    objc_class* classes = d.Offset(section.offset).Cast<objc_class*>();
                    for (int c = 0; c < (section.size / sizeof(objc_class)); c++)
                    {
                        auto& cls = classes[c];

                        printf("--- MAIN CLASS ---\n");
                        printf("objc_class isa:\t\t0x%02llX\n", cls.isa);
                        printf("objc_class superclass:\t0x%02llX\n", cls.superclass);
                        printf("objc_class cache:\t0x%02llX\n", cls.cache);
                        printf("objc_class vtable:\t0x%02llX\n", cls.vtable);
                        printf("objc_class data:\t0x%02llX\n", cls.data);

                        cls.isa = cls.isa | 0x800D6AE100000000;
                        cls.superclass = cls.superclass | 0xC00DB5AB00000000;

                        printf("--- MAIN CLASS AFTER ---\n");
                        printf("objc_class isa:\t\t0x%02llX\n", cls.isa);
                        printf("objc_class superclass:\t0x%02llX\n", cls.superclass);
                        printf("objc_class cache:\t0x%02llX\n", cls.cache);
                        printf("objc_class vtable:\t0x%02llX\n", cls.vtable);
                        printf("objc_class data:\t0x%02llX\n", cls.data);

                        struct class_ro_t
                        {
                            uint32_t flags;
                            uint32_t instanceStart;
                            uint32_t instanceSize;
                            uint32_t reserved; // 64-bit
                            uint64_t ivarLayoutOrNonMetaclass;
                            uint64_t name;
                            uint64_t baseMethodList;
                            uint64_t baseProtocols;
                            uint64_t ivars;
                            uint64_t weakIvarLayout;
                            uint64_t baseProperties;
                        };

                        // MARK: CLS DATA (ro_t)

                        auto& data = d.Offset(cls.data & 0x0000000fffffffff).GetValueRef<class_ro_t>();

                        printf("class_ro_t flags:\t0x%02lX\n", data.flags);
                        printf("class_ro_t instanceStart:\t0x%02lX\n", data.instanceStart);
                        printf("class_ro_t instanceSize:\t0x%02lX\n", data.instanceSize);
                        printf("class_ro_t reserved:\t0x%02lX\n", data.reserved);
                        printf("class_ro_t ivarLayoutOrNonMetaclass:\t0x%02llX\n", data.ivarLayoutOrNonMetaclass);
                        printf("class_ro_t name:\t%s\n", d.Offset(data.name & 0x0000000fffffffff).Cast<const char*>());
                        printf("class_ro_t baseMethodList:\t0x%02llX\n", data.baseMethodList);
                        printf("class_ro_t baseProtocols:\t0x%02llX\n", data.baseProtocols);
                        printf("class_ro_t ivars:\t0x%02llX\n", data.ivars);
                        printf("class_ro_t weakIvarLayout:\t0x%02llX\n", data.weakIvarLayout);
                        printf("class_ro_t weakIvarLayout:\t0x%02llX\n", data.weakIvarLayout);

                        if (data.baseMethodList != 0)
                            data.baseMethodList = data.baseMethodList | 0x8005c31000000000;

                    }
                }
            
                if (sectname.starts_with("__cfstring"))
                {
                    struct _cfstring
                    {
                        uint64_t isa;
                        int32_t flags;
                        uint64_t str;
                        int length;
                    };

                    _cfstring* strings = d.Offset(section.offset).Cast<_cfstring*>();
                    for (int s = 0; s < (section.size / sizeof(_cfstring)); s++)
                    {
                        auto& string = strings[s];
                        printf("cfstr ISA 0x%02llX\n", string.isa);

                        if (cfStringISA == 0 && (string.isa & 0x0000000fffffffff) > 100)
                        {
                            printf("Found cfstr isa 0x%02llX\n", string.isa);
                            cfStringISA = string.isa;
                        }

                        printf("cfstr offset: 0x%02llX\n", string.str & 0x0000000fffffffff);
                        printf("cfstr value: %s\n", d.Offset(string.str & 0x0000000fffffffff).Cast<const char*>());

                        string.isa = string.isa | 0xc0156ae100000000;

                        printf("cfstr ISA after 0x%02llX\n", string.isa);
                    }
                }

                if (sectname.starts_with("__data"))
                {
                    uint64_t* strings = d.Offset(section.offset).Cast<uint64_t*>();
                    for (int s = 0; s < (section.size / sizeof(uint64_t)); s++)
                    {
                        auto& string = strings[s];
                        printf("current str isa: 0x%02llX, hikari str: 0x%02llX, hikari stroff: 0x%02llX\n", cfStringISA, string, uint64_t(&string - d.GetPtr()));

                        if (string == cfStringISA && cfStringISA != 0x00 && string != 0x00)
                        {
                            printf("Found cfString header!!! hikari str: 0x%02llX\n", string);
                            auto newISA = string | 0xc0156ae100000000;
                            printf("cfstr ISA after 0x%02llX\n", newISA);
                            string = newISA;
                        }
                    }
                }
            }
        }

        index += cmd.cmdsize;
    }
}

void convert(const std::span<uint8_t>& data)
{
    const MemoryAddress d{ data.data() };
    printf("convert len:%llu\n", data.size());

    auto machHeaderPointer = d.GetValue<fat_header>();
    swapStruct(machHeaderPointer);

    if (machHeaderPointer.magic == FAT_MAGIC)
    {
        // we have a fat binary
        auto nslices = machHeaderPointer.nfat_arch;
        for (int i = 0; i < nslices; i++)
        {
            auto slice = d.Offset(sizeof(fat_header)).Offset(i * sizeof(fat_arch)).GetValue<fat_arch>();
            swapStruct(slice);
            convert_slice(d.Offset(slice.offset));
        }
    }

    if (machHeaderPointer.magic == MH_CIGAM_64 || machHeaderPointer.magic == MH_CIGAM)
    {
        // we have a non-fat mach binary
        convert_slice(d);
    }
}

int main(int argc, char* argv[])
{
    if (argc <= 1)
    {
        std::cout << "Allemand iOS 13->14 arm64e ABI converter" << std::endl;
        std::cout << "Original code by evelynnn, ported to C++ by p0358." << std::endl;
        std::cout << std::endl;
        std::cout << "Usage: allemande file.dylib [out.dylib]" << std::endl;
        std::cout << std::endl;
        return 0;
    }

    std::string data;

    {
        printf("Reading file...\n\n");
        auto file = std::ifstream(argv[1], std::ios::binary);
        data = read_file_to_string(file); //std::vector<char> data = std::vector<char>(std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>());
    }

    printf("Patching\n\n");
    convert({ reinterpret_cast<uint8_t*>(data.data()), data.size() });
    printf("End\n\n");

    {
        auto dst = argc >= 3 ? argv[2] : argv[1];
        auto file = std::ofstream(dst, std::ios::binary | std::ios::trunc);
        file << data;
        printf("Wrote out patched file to: %s\n", dst);
    }
}