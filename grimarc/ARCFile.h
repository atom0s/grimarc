
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

#ifndef __ARCFILE_H_INCLUDED__
#define __ARCFILE_H_INCLUDED__

#pragma once

/**
 * @brief ARC File Header (v3)
 */
struct ARC_V3_HEADER
{
    unsigned int        Magic; // ARC
    unsigned int        Version;
    unsigned int        NumberOfFileEntries;
    unsigned int        NumberOfDataRecords; // (NumberOfDataRecords / 12) = RecordTableSize
    unsigned int        RecordTableSize;
    unsigned int        StringTableSize;
    unsigned int        RecordTableOffset;
};

/**
 * @brief ARC File Part Entry (v3)
 */
struct ARC_V3_FILE_PART
{
    unsigned int        PartOffset;
    unsigned int        CompressedSize;
    unsigned int        DecompressedSize;
};

/**
 * @brief ARC File Table Of Contents Entry (v3)
 */
#pragma pack(1)
struct ARC_V3_FILE_TOC_ENTRY
{
    unsigned int        EntryType;
    unsigned int        FileOffset;
    unsigned int        CompressedSize;
    unsigned int        DecompressedSize;
    unsigned int        DecompressedHash; // Adler32 hash of the decompressed file bytes
    unsigned __int64    FileTime;
    unsigned int        FileParts;
    unsigned int        FirstPartIndex;
    unsigned int        StringEntryLength;
    unsigned int        StringEntryOffset;
};
#pragma pop

#endif // __ARCFILE_H_INCLUDED__
