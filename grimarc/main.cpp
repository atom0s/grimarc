
/**
 * Copyright (c) 2014 atom0s [atom0s@live.com]
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see http://www.gnu.org/licenses/
 *
 * This file is part of grimarc source code.
 */

#pragma comment(lib, "Shlwapi.lib")

#include <algorithm>
#include <Shlobj.h>
#include <Shlwapi.h>
#include <vector>

#include "ARCFile.h"
#include "lz4\lz4.h"

#define GRIMARC_VERSION_STRING "1.0.1 [B21]"

/**
 * @brief Application entry point.
 *
 * @param argc      Number of arguments.
 * @param argv      Pointer to the array of arguments.
 *
 * @return Non-important return value.
 */
int __cdecl main(int argc, char* argv[])
{
    printf_s("============================================================\n");
    printf_s("Grim Dawn ARC Extractor (grimarc) by atom0s\nContact : atom0s@live.com\nVersion : "GRIMARC_VERSION_STRING"\n");
    printf_s("============================================================\n\n");

    // To allow drag and drop we must alter the working folder..
    char szWorkingDirectory[MAX_PATH] = { 0 };
    strcpy_s(szWorkingDirectory, argv[0]);
    ::PathRemoveFileSpec(szWorkingDirectory);
    ::SetCurrentDirectoryA(szWorkingDirectory);

    // Validate the arguments..
    if (argc < 2)
    {
        printf_s("[ERROR] Missing argument(s); cannot continue.\nUsage: grimarc.exe [file.arc] [file.arc] [file.arc]\n");
        return 0;
    }

    // Process each given file..
    for (auto x = 1; x < argc; x++)
    {
        printf_s("[INFO] Processing file: %s\n", argv[x]);

        // Open the file for reading..
        FILE* f = NULL;
        if (!(fopen_s(&f, argv[x], "rb") == ERROR_SUCCESS))
        {
            printf_s("[ERROR] Failed to open the given file for reading.\n");
            continue;
        }

        // Obtain the file size..
        fseek(f, 0, SEEK_END);
        auto size = ftell(f);
        fseek(f, 0, SEEK_SET);

        // Read the file data into local memory..
        auto buffer = new unsigned char[size + 1];
        memset(buffer, 0x00, size);
        fread(buffer, size, 1, f);
        fclose(f);

        // Attempt to dump the file..
        auto dump_file = [&size, &buffer, &szWorkingDirectory]() -> bool
        {
            // Read and validate this file header..
            auto header = (ARC_V3_HEADER*)buffer;
            if (header->Magic != 0x435241 || header->Version != 3)
            {
                printf_s("[ERROR] Invalid file header, cannot read given file as an ARC archive file!\n");
                return false;
            }

            printf_s("[INFO] --> Opened the file successfully! Processing file..\n");
            printf_s("[INFO] --> File count in archive: %d\n", header->NumberOfFileEntries);

            // Read the archive table of contents..
            for (auto x = 0; x < header->NumberOfFileEntries; x++)
            {
                // Obtain the current toc entry..
                auto tocEntry = (ARC_V3_FILE_TOC_ENTRY*)&buffer[header->RecordTableOffset + header->RecordTableSize + header->StringTableSize + (x * 44)];

                // Build path to the output file..
                char szOutputFile[MAX_PATH] = { 0 };
                strcpy_s(szOutputFile, szWorkingDirectory);
                strcat_s(szOutputFile, "\\extracted\\");
                strcat_s(szOutputFile, (const char*)&buffer[header->RecordTableOffset + header->RecordTableSize + tocEntry->StringEntryOffset]);

                // Obtain the proper formatted path..
                char szOutputPath[MAX_PATH] = { 0 };
                ::GetFullPathName(szOutputFile, MAX_PATH, szOutputFile, NULL);
                strcpy_s(szOutputPath, szOutputFile);

                // Strip the path and create all required folders to file..
                PathRemoveFileSpec(szOutputPath);
                SHCreateDirectoryEx(NULL, szOutputPath, NULL);

                // Attempt to open this file for writing..
                FILE* f = NULL;
                if (!(fopen_s(&f, szOutputFile, "wb+") == ERROR_SUCCESS))
                {
                    printf_s("[ERROR] --> Failed to open the file for extraction:\n%s\n", szOutputFile);
                    continue;
                }

                // Process the file for dumping..
                if (tocEntry->EntryType == 1 && tocEntry->CompressedSize == tocEntry->DecompressedSize)
                    fwrite(buffer + tocEntry->FileOffset, 1, tocEntry->DecompressedSize, f);
                else
                {
                    // The data is compressed, process each file part individually..
                    for (auto y = 0; y < tocEntry->FileParts; y++)
                    {
                        // Obtain the part entry for this toc file..
                        auto part = (ARC_V3_FILE_PART*)&buffer[header->RecordTableOffset + ((tocEntry->FirstPartIndex + y) * sizeof(ARC_V3_FILE_PART))];

                        // If the part is not compressed, dump the part data directly..
                        if (part->CompressedSize == part->DecompressedSize)
                            fwrite(buffer + part->PartOffset, part->DecompressedSize, 1, f);
                        else
                        {
                            // The part is decompressed, we need to decompress it..
                            auto compressed = new unsigned char[part->CompressedSize];
                            memcpy(compressed, buffer + part->PartOffset, part->CompressedSize);

                            // Decompress the data..
                            auto decompressed = new unsigned char[part->DecompressedSize];
                            auto ret = LZ4_decompress_safe((const char*)compressed, (char*)decompressed, part->CompressedSize, part->DecompressedSize);
                            if (ret < 0)
                                printf_s("[ERROR] --> Failed to decompress a file part!\n");
                            else
                                fwrite(decompressed, part->DecompressedSize, 1, f);

                            delete[] compressed;
                            delete[] decompressed;
                        }
                    }
                }

                fclose(f);
            }

            return true;

        }();

        // Cleanup the file buffer..
        printf_s((dump_file == true) ? "[INFO] Successfully dumped file!\n\n" : "[ERROR] There was an error while dumping the file!\n\n");
        delete[] buffer;
    }

    printf_s("File queue processed!");
    return 0;
}
