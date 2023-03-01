/*
This is a modified version of the original opblib.h/opblib.c (https://github.com/Enichan/OPBinaryLib)
Basically, I have merged the two files into a stb-style single-file header-only C library.
Define OPBLIB_IMPLEMENTATION before you include this file in *one* C/C++ file to create the implementation.
I have also replaced the C initializers with initializing functions for compatibility with C++.
I place all my changes under the same license as the original code (see below).
                        / Mattias Gustavsson ( mattias@mattiasgustavsson.com )
*/

/*
//  MIT License
//
//  Copyright (c) 2021 Eniko Fox/Emma Maassen
//
//  Permission is hereby granted, free of charge, to any person obtaining a copy
//  of this software and associated documentation files (the "Software"), to deal
//  in the Software without restriction, including without limitation the rights
//  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
//  copies of the Software, and to permit persons to whom the Software is
//  furnished to do so, subject to the following conditions:
//
//  The above copyright notice and this permission notice shall be included in all
//  copies or substantial portions of the Software.
//
//  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
//  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
//  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
//  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
//  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
//  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
//  SOFTWARE.
*/

#ifndef opblib_h
#define opblib_h
//#pragma once
#include <stdint.h>

//#ifdef __cplusplus
//extern "C" {
//#endif

    // uncomment this for big endian architecture
    //#define OPB_BIG_ENDIAN

    #define OPBERR_LOGGED 1 // an error occurred and what error that was has been sent to OPB_Log
    #define OPBERR_WRITE_ERROR 2
    #define OPBERR_SEEK_ERROR 3
    #define OPBERR_TELL_ERROR 4
    #define OPBERR_READ_ERROR 5
    #define OPBERR_BUFFER_ERROR 6
    #define OPBERR_NOT_AN_OPB_FILE 7
    #define OPBERR_VERSION_UNSUPPORTED 8

    typedef struct OPB_Command {
        uint16_t Addr;
        uint8_t Data;
        double Time;
    } OPB_Command;

    typedef enum OPB_Format {
        OPB_Format_Default,
        OPB_Format_Raw,
    } OPB_Format;

    const char* OPB_GetErrorMessage(int errCode);

    const char* OPB_GetFormatName(OPB_Format fmt);

    // Custom write handler of the same form as stdio.h's fwrite for writing to memory
    // This function should write elementSize * elementCount bytes from buffer to the user-defined context object
    // Must return elementCount if successful
    typedef size_t(*OPB_StreamWriter)(const void* buffer, size_t elementSize, size_t elementCount, void* context);

    // Custom seek handler of the same form as stdio.h's fseek for writing to memory
    // This function should change the position to write to in the user-defined context object by the number of bytes
    // Specified by offset, relative to the specified origin which is one of 3 values:
    //
    // 1. Beginning of file (same as fseek's SEEK_SET)
    // 2. Current position of the file pointer (same as fseek's SEEK_CUR)
    // 3. End of file (same as fseek's SEEK_END)
    //
    // Must return 0 if successful
    typedef int (*OPB_StreamSeeker)(void* context, long offset, int origin);
    
    // Custom tell handler of the same form as stdio.h's ftell for writing to memory
    // This function must return the current write position for the user-defined context object
    // Must return -1L if unsuccessful
    typedef long (*OPB_StreamTeller)(void* context);

    // Custom read handler of the same form as stdio.h's fread for reading from memory
    // This function should read elementSize * elementCount bytes from the user-defined context object to buffer
    // Should return number of elements read
    typedef size_t(*OPB_StreamReader)(void* buffer, size_t elementSize, size_t elementCount, void* context);

    // Function that receives OPB_Command items read by OPB_BinaryToOpl and OPB_FileToOpl
    // This is where you copy the OPB_Command items into a data structure or the user-defined context object
    // Should return 0 if successful. Note that the array for `commandStream` is stack allocated and must be copied!
    typedef int(*OPB_BufferReceiver)(OPB_Command* commandStream, size_t commandCount, void* context);

    // OPL command stream to binary. Returns 0 if successful.
    int OPB_OplToBinary(OPB_Format format, OPB_Command* commandStream, size_t commandCount,
        OPB_StreamWriter write, OPB_StreamSeeker seek, OPB_StreamTeller tell, void* userData);

    // OPL command stream to file. Returns 0 if successful.
    int OPB_OplToFile(OPB_Format format, OPB_Command* commandStream, size_t commandCount, const char* file);

    // OPB binary to OPL command stream. Returns 0 if successful.
    int OPB_BinaryToOpl(OPB_StreamReader reader, void* readerData, OPB_BufferReceiver receiver, void* receiverData);

    // OPB file to OPL command stream. Returns 0 if successful.
    int OPB_FileToOpl(const char* file, OPB_BufferReceiver receiver, void* receiverData);

    // OPBLib log function
    typedef void (*OPB_LogHandler)(const char* s);
    extern OPB_LogHandler OPB_Log;

//#ifdef __cplusplus
//}
//#endif

#endif /* opblib_h */

#ifdef OPBLIB_IMPLEMENTATION
#undef OPBLIB_IMPLEMENTATION

/*
//  MIT License
//
//  Copyright (c) 2021 Eniko Fox/Emma Maassen
//
//  Permission is hereby granted, free of charge, to any person obtaining a copy
//  of this software and associated documentation files (the "Software"), to deal
//  in the Software without restriction, including without limitation the rights
//  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
//  copies of the Software, and to permit persons to whom the Software is
//  furnished to do so, subject to the following conditions:
//
//  The above copyright notice and this permission notice shall be included in all
//  copies or substantial portions of the Software.
//
//  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
//  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
//  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
//  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
//  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
//  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
//  SOFTWARE.
*/
#ifdef _WIN32
#define _CRT_SECURE_NO_DEPRECATE
#endif
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdbool.h>
#include <string.h>
#include "opblib.h"

#define OPB_HEADER_SIZE 7
// OPBin1\0
const char OPB_Header[OPB_HEADER_SIZE] = { 'O', 'P', 'B', 'i', 'n', '1', '\0' };

#define VECTOR_MIN_CAPACITY 8
#define VECTOR_PTR(vector, index) (void*)((uint8_t*)((vector)->Storage) + (index) * vector->ElementSize)
// this only exists to make type declarations clearer
#define VectorT(T) Vector

typedef struct Vector {
    size_t Count;
    size_t Capacity;
    size_t ElementSize;
    void* Storage;
} Vector;

Vector Vector_New(size_t elementSize) {
    Vector v = { 0 };
    v.ElementSize = elementSize;
    return v;
}

static void Vector_Free(Vector* v) {
    if (v->Storage != NULL) {
        free(v->Storage);
    }
    v->Storage = NULL;
    v->Capacity = 0;
    v->Count = 0;
}

static void* Vector_Get(Vector* v, int index) {
    if (index < 0 || index >= v->Count) {
        return NULL;
    }
    if (v->ElementSize <= 0) {
        return NULL;
    }
    return VECTOR_PTR(v, index);
}
#define Vector_GetT(T, vector, index) ((T*)Vector_Get(vector, index))

static int Vector_Set(Vector* v, void* item, int index) {
    if (index < 0 || index >= v->Count) {
        return -1;
    }
    if (v->ElementSize <= 0) {
        return -1;
    }
    memcpy(VECTOR_PTR(v, index), item, v->ElementSize);
    return 0;
}

static int Vector_Add(Vector* v, void* item) {
    if (v->ElementSize <= 0) {
        return -1;
    }
    if (v->Count >= v->Capacity) {
        size_t newCapacity = v->Capacity * 2;
        if (newCapacity < VECTOR_MIN_CAPACITY) newCapacity = VECTOR_MIN_CAPACITY;

        void* newStorage = malloc(newCapacity * v->ElementSize);
        if (newStorage == NULL) {
            return -1;
        }

        if (v->Storage != NULL) {
            memcpy(newStorage, v->Storage, v->Count * v->ElementSize);
            free(v->Storage);
        }

        v->Storage = newStorage;
        v->Capacity = newCapacity;
    }

    v->Count++;
    return Vector_Set(v, item, (size_t)((int)v->Count - 1));
}

static int Vector_AddRange(Vector* v, void* items, size_t count) {
    if (v->ElementSize <= 0) {
        return -1;
    }
    uint8_t* itemBytes = (uint8_t*)items;
    for (size_t i = 0; i < count; i++, itemBytes += v->ElementSize) {
        int ret;
        ret = Vector_Add(v, (void*)itemBytes);
        if (ret) return ret;
    }
    return 0;
}

typedef int(*VectorSortFunc)(const void* a, const void* b);

static void Vector_Clear(Vector* v, bool keepStorage) {
    v->Count = 0;
    if (!keepStorage && v->Storage != NULL) {
        free(v->Storage);
        v->Storage = NULL;
        v->Capacity = 0;
    }
}

static void Vector_Sort(Vector* v, VectorSortFunc sortFunc) {
    qsort(v->Storage, v->Count, v->ElementSize, sortFunc);
}

static const char* GetFilename(const char* path) {
    const char* lastFwd = strrchr(path, '/');
    const char* lastBck = strrchr(path, '\\');
    if (lastFwd == NULL && lastBck == NULL) {
        return path;
    }
    return lastFwd > lastBck ? lastFwd + 1 : lastBck + 1;
}

static const char* GetSourceFilename(void) {
    return GetFilename(__FILE__);
}

#define CONCAT_IMPL(x, y) x##y
#define MACRO_CONCAT(x, y) CONCAT_IMPL(x, y)

#define NUM_CHANNELS 18
#define NUM_TRACKS (NUM_CHANNELS + 1)

#define WRITE(buffer, size, count, context) \
    if (context->Write(buffer, size, count, context->UserData) != count) { \
        Log("OPB write error occurred in '%s' at line %d\n", GetSourceFilename(), __LINE__); \
        return OPBERR_WRITE_ERROR; \
    }

#define WRITE_UINT7(context, value) \
    if (WriteUint7(context, value)) { \
        Log("OPB write error occurred in '%s' at line %d\n", GetSourceFilename(), __LINE__); \
        return OPBERR_WRITE_ERROR; \
    }

#define SEEK(context, offset, origin) \
    if (context->Seek(context->UserData, offset, origin)) { \
        Log("OPB seek error occurred in '%s' at line %d\n", GetSourceFilename(), __LINE__); \
        return OPBERR_SEEK_ERROR; \
    }
#define TELL(context, var) \
    var = context->Tell(context->UserData); \
    if (var == -1L) { \
        Log("OPB file position error occurred in '%s' at line %d\n", GetSourceFilename(), __LINE__); \
        return OPBERR_TELL_ERROR; \
    }

#define READ(buffer, size, count, context) \
    if (context->Read(buffer, size, count, context->UserData) != count) { \
        Log("OPB read error occurred in '%s' at line %d\n", GetSourceFilename(), __LINE__); \
        return OPBERR_READ_ERROR; \
    }
#define READ_UINT7(var, context) \
    if ((var = ReadUint7(context)) < 0) { \
        Log("OPB read error occurred in '%s' at line %d\n", GetSourceFilename(), __LINE__); \
        return OPBERR_READ_ERROR; \
    }

#define SUBMIT(stream, count, context) \
    if (context->Submit(stream, count, context->ReceiverData)) return OPBERR_BUFFER_ERROR

typedef struct Context Context;
typedef struct Command Command;
typedef struct OpbData OpbData;
typedef struct Instrument Instrument;

struct Context {
    VectorT(Command) CommandStream;
    OPB_StreamWriter Write;
    OPB_StreamSeeker Seek;
    OPB_StreamTeller Tell;
    OPB_StreamReader Read;
    OPB_BufferReceiver Submit;
    OPB_Format Format;
    VectorT(OpbData) DataMap;
    VectorT(Instrument) Instruments;
    VectorT(Command) Tracks[NUM_TRACKS];
    double Time;
    void* UserData;
    void* ReceiverData;
};

static void Context_Free(Context* context) {
    if (context->CommandStream.Storage != NULL) { Vector_Free(&context->CommandStream); }
    if (context->Instruments.Storage != NULL) { Vector_Free(&context->Instruments); }
    if (context->DataMap.Storage != NULL) { Vector_Free(&context->DataMap); }
    for (int i = 0; i < NUM_TRACKS; i++) {
        if (context->Tracks[i].Storage != NULL) { Vector_Free(&context->Tracks[i]); }
    }
}

OPB_LogHandler OPB_Log;

static inline size_t BufferSize(const char* format, ...) {
    va_list args;
    va_start(args, format);
    size_t result = vsnprintf(NULL, 0, format, args);
    va_end(args);
    return (size_t)(result + 1); // safe byte for \0
}

static void Log(const char* format, ...) {
    if (!OPB_Log) return;

    va_list args;

    va_start(args, format);
    size_t size = BufferSize(format, args);
    va_end(args);

    if (size == 0) return;

    va_start(args, format);
    char* s = NULL;
    if (size < 0 || (s = (char*)malloc(size)) == NULL) {
        vprintf(format, args);
    }
    else {
        vsprintf(s, format, args);
    }
    va_end(args);

    if (s != NULL) {
        OPB_Log(s);
        free(s);
    }
}

OPB_Command MakeCommand(uint16_t Addr, uint8_t Data, double Time) {
    OPB_Command cmd;
    cmd.Addr = Addr;
    cmd.Data = Data;
    cmd.Time = Time;
    return cmd;
}

const char* OPB_GetFormatName(OPB_Format fmt) {
    switch (fmt) {
    default:
        return "Default";
    case OPB_Format_Raw:
        return "Raw";
    }
}

static inline uint32_t FlipEndian32(uint32_t val) {
#ifdef OPB_BIG_ENDIAN
    return val;
#else
    val = ((val << 8) & 0xFF00FF00) | ((val >> 8) & 0xFF00FF);
    return (val << 16) | (val >> 16);
#endif
}

static inline uint16_t FlipEndian16(uint16_t val) {
#ifdef OPB_BIG_ENDIAN
    return val;
#else
    return (val << 8) | ((val >> 8) & 0xFF);
#endif
}

static size_t Uint7Size(uint32_t value) {
    if (value >= 2097152) {
        return 4;
    }
    else if (value >= 16384) {
        return 3;
    }
    else if (value >= 128) {
        return 2;
    }
    else {
        return 1;
    }
}

struct Command {
    uint16_t Addr;
    uint8_t Data;
    double Time;
    int OrderIndex;
    int DataIndex;
};

struct OpbData {
    uint32_t Count;
    uint8_t Args[16];
};

static void OpbData_WriteUint7(OpbData* data, uint32_t value) {
    if (value >= 2097152) {
        uint8_t b0 = (value & 0b01111111) | 0b10000000;
        uint8_t b1 = ((value & 0b011111110000000) >> 7) | 0b10000000;
        uint8_t b2 = ((value & 0b0111111100000000000000) >> 14) | 0b10000000;
        uint8_t b3 = (value & 0b11111111000000000000000000000) >> 21;
        data->Args[data->Count] = b0; data->Count++;
        data->Args[data->Count] = b1; data->Count++;
        data->Args[data->Count] = b2; data->Count++;
        data->Args[data->Count] = b3; data->Count++;
    }
    else if (value >= 16384) {
        uint8_t b0 = (value & 0b01111111) | 0b10000000;
        uint8_t b1 = ((value & 0b011111110000000) >> 7) | 0b10000000;
        uint8_t b2 = (value & 0b0111111100000000000000) >> 14;
        data->Args[data->Count] = b0; data->Count++;
        data->Args[data->Count] = b1; data->Count++;
        data->Args[data->Count] = b2; data->Count++;
    }
    else if (value >= 128) {
        uint8_t b0 = (value & 0b01111111) | 0b10000000;
        uint8_t b1 = (value & 0b011111110000000) >> 7;
        data->Args[data->Count] = b0; data->Count++;
        data->Args[data->Count] = b1; data->Count++;
    }
    else {
        uint8_t b0 = value & 0b01111111;
        data->Args[data->Count] = b0; data->Count++;
    }
}

static void OpbData_WriteU8(OpbData* data, uint32_t value) {
    data->Args[data->Count] = (uint8_t)value;
    data->Count++;
}

#define OPB_CMD_SETINSTRUMENT 0xD0
#define OPB_CMD_PLAYINSTRUMENT 0xD1
#define OPB_CMD_NOTEON 0xD7

#define NUM_OPERATORS 36
static int OperatorOffsets[NUM_OPERATORS] = {
    0x0, 0x1, 0x2, 0x3, 0x4, 0x5, 0x8, 0x9, 0xA, 0xB, 0xC, 0xD, 0x10, 0x11, 0x12, 0x13, 0x14, 0x15,
    0x100, 0x101, 0x102, 0x103, 0x104, 0x105, 0x108, 0x109, 0x10A, 0x10B, 0x10C, 0x10D, 0x110, 0x111, 0x112, 0x113, 0x114, 0x115,
};

static int ChannelToOp[NUM_CHANNELS] = {
    0, 1, 2, 6, 7, 8, 12, 13, 14, 18, 19, 20, 24, 25, 26, 30, 31, 32,
};

static int ChannelToOffset[NUM_CHANNELS] = {
    0, 1, 2, 3, 4, 5, 6, 7, 8,
    0x100, 0x101, 0x102, 0x103, 0x104, 0x105, 0x106, 0x107, 0x108,
};

static int RegisterOffsetToChannel(uint32_t offset) {
    uint32_t baseoff = offset & 0xFF;
    int chunk = baseoff / 8;
    int suboff = baseoff % 8;

    if (chunk >= 3 || suboff >= 6) {
        return -1;
    }
    return chunk * 3 + (suboff % 3) + ((offset & 0x100) != 0 ? NUM_CHANNELS / 2 : 0);
}

static int RegisterOffsetToOpIndex(uint32_t offset) {
    uint32_t baseoff = offset & 0xFF;
    uint32_t suboff = baseoff % 8;
    if (suboff >= 6) {
        return -1;
    }
    return suboff >= 3;
}

#define REG_FEEDCONN 0xC0
#define REG_CHARACTER 0x20
#define REG_LEVELS 0x40
#define REG_ATTACK 0x60
#define REG_SUSTAIN 0x80
#define REG_WAVE 0xE0
#define REG_FREQUENCY 0xA0
#define REG_NOTE 0xB0

typedef struct Operator {
    int16_t Characteristic;
    int16_t AttackDecay;
    int16_t SustainRelease;
    int16_t WaveSelect;
} Operator;

Operator MakeOperator(int16_t Characteristic, int16_t AttackDecay, int16_t SustainRelease, int16_t WaveSelect) {
    Operator op;
    op.Characteristic = Characteristic;
    op.AttackDecay = AttackDecay;
    op.SustainRelease = SustainRelease;
    op.WaveSelect = WaveSelect;
    return op;
}

struct Instrument {
    int16_t FeedConn;
    Operator Modulator;
    Operator Carrier;
    int Index;
};

Instrument MakeInstrument(int16_t FeedConn, Operator Modulator, Operator Carrier, int Index) {
    Instrument instrument;
    instrument.FeedConn = FeedConn;
    instrument.Modulator = Modulator;
    instrument.Carrier = Carrier;
    instrument.Index = Index;
    return instrument;
}

static Context Context_New(void) {
    Context context = { 0 };

    context.CommandStream = Vector_New(sizeof(Command));
    context.Instruments = Vector_New(sizeof(Instrument));
    context.DataMap = Vector_New(sizeof(OpbData));
    for (int i = 0; i < NUM_TRACKS; i++) {
        context.Tracks[i] = Vector_New(sizeof(Command));
    }

    return context;
}

static Instrument GetInstrument(Context* context, Command* feedconn,
    Command* modChar, Command* modAttack, Command* modSustain, Command* modWave,
    Command* carChar, Command* carAttack, Command* carSustain, Command* carWave) {
    // find a matching instrument
    for (int i = 0; i < context->Instruments.Count; i++) {
        Instrument* instr = Vector_GetT(Instrument, &context->Instruments, i);

        if ((feedconn == NULL || instr->FeedConn == feedconn->Data) &&
            (modChar == NULL || instr->Modulator.Characteristic == modChar->Data) &&
            (modAttack == NULL || instr->Modulator.AttackDecay == modAttack->Data) &&
            (modSustain == NULL || instr->Modulator.SustainRelease == modSustain->Data) &&
            (modWave == NULL || instr->Modulator.WaveSelect == modWave->Data) &&
            (carChar == NULL || instr->Carrier.Characteristic == carChar->Data) &&
            (carAttack == NULL || instr->Carrier.AttackDecay == carAttack->Data) &&
            (carSustain == NULL || instr->Carrier.SustainRelease == carSustain->Data) &&
            (carWave == NULL || instr->Carrier.WaveSelect == carWave->Data)) {
            return *instr;
        }
    }

    // no instrument found, create and store new instrument
    Instrument instr = {
        feedconn == NULL ? -1 : feedconn->Data,
        {
            modChar == NULL ? -1 : modChar->Data,
            modAttack == NULL ? -1 : modAttack->Data,
            modSustain == NULL ? -1 : modSustain->Data,
            modWave == NULL ? -1 : modWave->Data,
        },
        {
            carChar == NULL ? -1 : carChar->Data,
            carAttack == NULL ? -1 : carAttack->Data,
            carSustain == NULL ? -1 : carSustain->Data,
            carWave == NULL ? -1 : carWave->Data,
        },
        (int)context->Instruments.Count
    };
    Vector_Add(&context->Instruments, &instr);
    return instr;
}

static int WriteInstrument(Context* context, const Instrument* instr) {
    uint8_t feedConn = (uint8_t)(instr->FeedConn >= 0 ? instr->FeedConn : 0);
    uint8_t modChr = (uint8_t)(instr->Modulator.Characteristic >= 0 ? instr->Modulator.Characteristic : 0);
    uint8_t modAtk = (uint8_t)(instr->Modulator.AttackDecay >= 0 ? instr->Modulator.AttackDecay : 0);
    uint8_t modSus = (uint8_t)(instr->Modulator.SustainRelease >= 0 ? instr->Modulator.SustainRelease : 0);
    uint8_t modWav = (uint8_t)(instr->Modulator.WaveSelect >= 0 ? instr->Modulator.WaveSelect : 0);
    uint8_t carChr = (uint8_t)(instr->Carrier.Characteristic >= 0 ? instr->Carrier.Characteristic : 0);
    uint8_t carAtk = (uint8_t)(instr->Carrier.AttackDecay >= 0 ? instr->Carrier.AttackDecay : 0);
    uint8_t carSus = (uint8_t)(instr->Carrier.SustainRelease >= 0 ? instr->Carrier.SustainRelease : 0);
    uint8_t carWav = (uint8_t)(instr->Carrier.WaveSelect >= 0 ? instr->Carrier.WaveSelect : 0);

    WRITE(&feedConn, sizeof(uint8_t), 1, context);
    WRITE(&modChr, sizeof(uint8_t), 1, context);
    WRITE(&modAtk, sizeof(uint8_t), 1, context);
    WRITE(&modSus, sizeof(uint8_t), 1, context);
    WRITE(&modWav, sizeof(uint8_t), 1, context);
    WRITE(&carChr, sizeof(uint8_t), 1, context);
    WRITE(&carAtk, sizeof(uint8_t), 1, context);
    WRITE(&carSus, sizeof(uint8_t), 1, context);
    WRITE(&carWav, sizeof(uint8_t), 1, context);

    return 0;
}

static int WriteUint7(Context* context, uint32_t value) {
    if (value >= 2097152) {
        uint8_t b0 = (value & 0b01111111) | 0b10000000;
        uint8_t b1 = ((value & 0b011111110000000) >> 7) | 0b10000000;
        uint8_t b2 = ((value & 0b0111111100000000000000) >> 14) | 0b10000000;
        uint8_t b3 = (value & 0b11111111000000000000000000000) >> 21;
        if (context->Write(&b0, sizeof(uint8_t), 1, context->UserData) < 1) return -1;
        if (context->Write(&b1, sizeof(uint8_t), 1, context->UserData) < 1) return -1;
        if (context->Write(&b2, sizeof(uint8_t), 1, context->UserData) < 1) return -1;
        if (context->Write(&b3, sizeof(uint8_t), 1, context->UserData) < 1) return -1;
    }
    else if (value >= 16384) {
        uint8_t b0 = (value & 0b01111111) | 0b10000000;
        uint8_t b1 = ((value & 0b011111110000000) >> 7) | 0b10000000;
        uint8_t b2 = (value & 0b0111111100000000000000) >> 14;
        if (context->Write(&b0, sizeof(uint8_t), 1, context->UserData) < 1) return -1;
        if (context->Write(&b1, sizeof(uint8_t), 1, context->UserData) < 1) return -1;
        if (context->Write(&b2, sizeof(uint8_t), 1, context->UserData) < 1) return -1;
    }
    else if (value >= 128) {
        uint8_t b0 = (value & 0b01111111) | 0b10000000;
        uint8_t b1 = (value & 0b011111110000000) >> 7;
        if (context->Write(&b0, sizeof(uint8_t), 1, context->UserData) < 1) return -1;
        if (context->Write(&b1, sizeof(uint8_t), 1, context->UserData) < 1) return -1;
    }
    else {
        uint8_t b0 = value & 0b01111111;
        if (context->Write(&b0, sizeof(uint8_t), 1, context->UserData) < 1) return -1;
    }
    return 0;
}

// returns channel for note event or -1 if not a note event
static int IsNoteEvent(int addr) {
    int baseAddr = addr & 0xFF;
    if (baseAddr >= 0xB0 && baseAddr <= 0xB8) {
        return (baseAddr - 0xB0) * ((addr & 0x100) == 0 ? 1 : 2);
    }
    else if (baseAddr >= OPB_CMD_NOTEON && baseAddr < OPB_CMD_NOTEON + NUM_CHANNELS / 2) {
        return (baseAddr - OPB_CMD_NOTEON) * ((addr & 0x100) == 0 ? 1 : 2);
    }
    return -1;
}

static bool IsChannelNoteEvent(int addr, int channel) {
    return
        (addr == 0xB0 + (channel % 9) + (channel >= 9 ? 0x100 : 0)) ||
        (addr == OPB_CMD_NOTEON + (channel % 9) + (channel >= 9 ? 0x100 : 0));
}

static int ChannelFromRegister(int reg) {
    int baseReg = reg & 0xFF;
    if ((baseReg >= 0x20 && baseReg <= 0x95) || (baseReg >= 0xE0 && baseReg <= 0xF5)) {
        int offset = baseReg % 0x20;
        if (offset < 0 || offset >= 0x16) {
            return -1;
        }
        if ((reg & 0x100) != 0) {
            offset |= 0x100;
        }
        int ch;
        if ((ch = RegisterOffsetToChannel(offset)) >= 0) {
            return ch;
        }
    }
    else if ((baseReg >= 0xA0 && baseReg <= 0xB8) || (baseReg >= 0xC0 && baseReg <= 0xC8)) {
        int ch = baseReg % 0x10;
        if (ch < 0 || ch >= 0x9) {
            return -1;
        }
        if ((reg & 0x100) != 0) {
            ch += 9;
        }
        return ch;
    }
    return -1;
}

// 0 for modulator, 1 for carrier, -1 otherwise
static int RegisterToOpIndex(int reg) {
    int baseReg = reg & 0xFF;
    if ((baseReg >= 0x20 && baseReg <= 0x95) || (baseReg >= 0xE0 && baseReg <= 0xF5)) {
        int offset = baseReg % 0x20;
        if (offset < 0 || offset >= 0x16) {
            return -1;
        }
        int op;
        if ((op = RegisterOffsetToOpIndex(offset)) >= 0) {
            return op;
        }
    }
    return -1;
}

static void SeparateTracks(Context* context) {
    for (int i = 0; i < context->CommandStream.Count; i++) {
        Command* cmd = Vector_GetT(Command, &context->CommandStream, i);

        int channel = ChannelFromRegister(cmd->Addr);
        if (channel < 0) channel = NUM_TRACKS - 1;

        Vector_Add(&context->Tracks[channel], cmd);
    }
}

static int CountInstrumentChanges(Command* feedconn,
    Command* modChar, Command* modAttack, Command* modSustain, Command* modWave,
    Command* carChar, Command* carAttack, Command* carSustain, Command* carWave) {
    int count = 0;
    if (feedconn != NULL) count++;
    if (modChar != NULL) count++;
    if (modAttack != NULL) count++;
    if (modSustain != NULL) count++;
    if (modWave != NULL) count++;
    if (carChar != NULL) count++;
    if (carAttack != NULL) count++;
    if (carSustain != NULL) count++;
    if (carWave != NULL) count++;
    return count;
}

static int ProcessRange(Context* context, int channel, double time, Command* commands, int cmdCount, Vector* range, 
    int _debug_start, int _debug_end // these last two are only for logging in case of error
) {
    for (int i = 0; i < cmdCount; i++) {
        Command* cmd = commands + i;

        if (cmd->Time != time) {
            int timeMs = (int)(time * 1000);
            Log("A timing error occurred at %d ms on channel %d in range %d-%d\n", timeMs, channel, _debug_start, _debug_end);
            return OPBERR_LOGGED;
        }
    }

    Command* modChar = NULL, * modLevel = NULL, * modAttack = NULL, * modSustain = NULL, * modWave = NULL;
    Command* carChar = NULL, * carLevel = NULL, * carAttack = NULL, * carSustain = NULL, * carWave = NULL;
    Command* freq = NULL, * note = NULL, * feedconn = NULL;

    for (int i = 0; i < cmdCount; i++) {
        Command* cmd = commands + i;

        int baseAddr = cmd->Addr & 0xFF;
        int op;

        if ((op = RegisterToOpIndex(cmd->Addr)) > -1) {
            // command affects modulator or carrier
            if (op == 0) {
                if (baseAddr >= 0x20 && baseAddr <= 0x35)
                    modChar = cmd;
                else if (baseAddr >= 0x40 && baseAddr <= 0x55)
                    modLevel = cmd;
                else if (baseAddr >= 0x60 && baseAddr <= 0x75)
                    modAttack = cmd;
                else if (baseAddr >= 0x80 && baseAddr <= 0x95)
                    modSustain = cmd;
                else if (baseAddr >= 0xE0 && baseAddr <= 0xF5)
                    modWave = cmd;
            }
            else {
                if (baseAddr >= 0x20 && baseAddr <= 0x35)
                    carChar = cmd;
                else if (baseAddr >= 0x40 && baseAddr <= 0x55)
                    carLevel = cmd;
                else if (baseAddr >= 0x60 && baseAddr <= 0x75)
                    carAttack = cmd;
                else if (baseAddr >= 0x80 && baseAddr <= 0x95)
                    carSustain = cmd;
                else if (baseAddr >= 0xE0 && baseAddr <= 0xF5)
                    carWave = cmd;
            }
        }
        else {
            if (baseAddr >= 0xA0 && baseAddr <= 0xA8)
                freq = cmd;
            else if (baseAddr >= 0xB0 && baseAddr <= 0xB8) {
                if (note != NULL) {
                    int timeMs = (int)(time * 1000);
                    Log("A decoding error occurred at %d ms on channel %d in range %d-%d\n", timeMs, channel, _debug_start, _debug_end);
                    return OPBERR_LOGGED;
                }
                note = cmd;
            }
            else if (baseAddr >= 0xC0 && baseAddr <= 0xC8)
                feedconn = cmd;
            else {
                Vector_Add(range, cmd);
            }
        }
    }

    // combine instrument data
    int instrChanges;
    if ((instrChanges = CountInstrumentChanges(feedconn, modChar, modAttack, modSustain, modWave, carChar, carAttack, carSustain, carWave)) > 0) {
        Instrument instr = GetInstrument(context, feedconn, modChar, modAttack, modSustain, modWave, carChar, carAttack, carSustain, carWave);

        size_t size = Uint7Size(instr.Index) + 3;

        if (modLevel != NULL) {
            size++;
            instrChanges++;
        }
        if (carLevel != NULL) {
            size++;
            instrChanges++;
        }

        // combine with frequency and note command if present
        if (freq != NULL && note != NULL) {
            size += 2;
            instrChanges += 2;
        }

        if ((int)size < instrChanges * 2) {
            OpbData data = { 0 };
            OpbData_WriteUint7(&data, instr.Index);

            uint8_t channelMask = channel |
                (modLevel != NULL ? 0b00100000 : 0) |
                (carLevel != NULL ? 0b01000000 : 0) |
                (feedconn != NULL ? 0b10000000 : 0);
            OpbData_WriteU8(&data, channelMask);

            int mask =
                (modChar != NULL ? 0b00000001 : 0) |
                (modAttack != NULL ? 0b00000010 : 0) |
                (modSustain != NULL ? 0b00000100 : 0) |
                (modWave != NULL ? 0b00001000 : 0) |
                (carChar != NULL ? 0b00010000 : 0) |
                (carAttack != NULL ? 0b00100000 : 0) |
                (carSustain != NULL ? 0b01000000 : 0) |
                (carWave != NULL ? 0b10000000 : 0);
            OpbData_WriteU8(&data, mask);

            // instrument command is 0xD0
            int reg = OPB_CMD_SETINSTRUMENT;

            if (freq != NULL && note != NULL) {
                OpbData_WriteU8(&data, freq->Data);
                OpbData_WriteU8(&data, note->Data);

                // play command is 0xD1
                reg = OPB_CMD_PLAYINSTRUMENT;
            }

            if (modLevel != NULL) OpbData_WriteU8(&data, modLevel->Data);
            if (carLevel != NULL) OpbData_WriteU8(&data, carLevel->Data);

            int opbIndex = (int32_t)context->DataMap.Count + 1;
            Vector_Add(&context->DataMap, &data);

            Command cmd = {
                (uint16_t)(reg + (channel >= 9 ? 0x100 : 0)), // register
                0, // data
                time,
                commands[0].OrderIndex,
                opbIndex
            };

            Vector_Add(range, &cmd);
            feedconn = modChar = modLevel = modAttack = modSustain = modWave = carChar = carLevel = carAttack = carSustain = carWave = NULL;

            if (freq != NULL && note != NULL) {
                freq = note = NULL;
            }
        }
    }

    // combine frequency/note and modulator and carrier level data
    if (freq != NULL && note != NULL) {
        // note on command is 0xD7 through 0xDF (and 0x1D7 through 0x1DF for channels 10-18)
        int reg = OPB_CMD_NOTEON + (channel % 9) + (channel >= 9 ? 0x100 : 0);

        OpbData data = { 0 };
        OpbData_WriteU8(&data, freq->Data);

        int noteLevels = note->Data & 0b00111111;

        // encode modulator and carrier levels data in the note data's upper 2 (unused) bits
        if (modLevel != NULL) {
            noteLevels |= 0b01000000;
        }
        if (carLevel != NULL) {
            noteLevels |= 0b10000000;
        }

        OpbData_WriteU8(&data, noteLevels);

        if (modLevel != NULL) {
            OpbData_WriteU8(&data, modLevel->Data);
        }
        if (carLevel != NULL) {
            OpbData_WriteU8(&data, carLevel->Data);
        }

        int opbIndex = (int32_t)context->DataMap.Count + 1;
        Vector_Add(&context->DataMap, &data);

        Command cmd = {
            (uint16_t)reg, // register
            0, // data
            time,
            note->OrderIndex,
            opbIndex
        };

        Vector_Add(range, &cmd);
        freq = note = modLevel = carLevel = NULL;
    }

    if (modChar != NULL) Vector_Add(range, modChar);
    if (modLevel != NULL) Vector_Add(range, modLevel);
    if (modAttack != NULL) Vector_Add(range, modAttack);
    if (modSustain != NULL) Vector_Add(range, modSustain);
    if (modWave != NULL) Vector_Add(range, modWave);

    if (carChar != NULL) Vector_Add(range, carChar);
    if (carLevel != NULL) Vector_Add(range, carLevel);
    if (carAttack != NULL) Vector_Add(range, carAttack);
    if (carSustain != NULL) Vector_Add(range, carSustain);
    if (carWave != NULL) Vector_Add(range, carWave);

    if (feedconn != NULL) Vector_Add(range, feedconn);
    if (freq != NULL) Vector_Add(range, freq);
    if (note != NULL) Vector_Add(range, note);

    return 0;
}

static int ProcessTrack(Context* context, int channel, Vector* chOut) {
    Vector* commands = &context->Tracks[channel];

    if (commands->Count == 0) {
        return 0;
    }

    int lastOrder = Vector_GetT(Command, commands, 0)->OrderIndex;
    int i = 0;

    while (i < commands->Count) {
        double time = Vector_GetT(Command, commands, i)->Time;

        int start = i;
        // sequences must be all in the same time block and in order
        // sequences are capped by a note command (write to register B0-B8 or 1B0-1B8)
        while (i < commands->Count && Vector_GetT(Command, commands, i)->Time <= time && (Vector_GetT(Command, commands, i)->OrderIndex - lastOrder) <= 1) {
            Command* cmd = Vector_GetT(Command, commands, i);

            lastOrder = cmd->OrderIndex;
            i++;

            if (IsChannelNoteEvent(cmd->Addr, channel)) {
                break;
            }
        }
        int end = i;

        VectorT(Command) range = Vector_New(sizeof(Command));
        int ret = ProcessRange(context, channel, time, Vector_GetT(Command, commands, start), end - start, &range, start, end);
        if (ret) {
            Vector_Free(&range);
            return ret;
        }
        
        Vector_AddRange(chOut, range.Storage, range.Count);
        Vector_Free(&range);

        if (i < commands->Count) {
            lastOrder = Vector_GetT(Command, commands, i)->OrderIndex;
        }
    }

    return 0;
}

static int WriteChunk(Context* context, double elapsed, int start, int count) {
    uint32_t elapsedMs = (uint32_t)((elapsed * 1000) + 0.5);
    int loCount = 0;
    int hiCount = 0;

    for (int i = start; i < start + count; i++) {
        Command* cmd = Vector_GetT(Command, &context->CommandStream, i);

        if ((cmd->Addr & 0x100) == 0) {
            loCount++;
        }
        else {
            hiCount++;
        }
    }

    // write header
    WRITE_UINT7(context, elapsedMs);
    WRITE_UINT7(context, loCount);
    WRITE_UINT7(context, hiCount);

    // write low and high register writes
    bool isLow = true;
    while (true) {
        for (int i = start; i < start + count; i++) {
            Command* cmd = Vector_GetT(Command, &context->CommandStream, i);

            if (((cmd->Addr & 0x100) == 0) == isLow) {
                uint8_t baseAddr = cmd->Addr & 0xFF;
                WRITE(&baseAddr, sizeof(uint8_t), 1, context);

                if (cmd->DataIndex) {
                    // opb command
                    OpbData* data = Vector_GetT(OpbData, &context->DataMap, cmd->DataIndex - 1);
                    WRITE(data->Args, sizeof(uint8_t), data->Count, context);
                }
                else {
                    // regular write
                    WRITE(&(cmd->Data), sizeof(uint8_t), 1, context);
                }
            }
        }

        if (!isLow) {
            break;
        }

        isLow = !isLow;
    }

    return 0;
}

static int SortCommands(const void* a, const void* b) {
    return ((Command*)a)->OrderIndex - ((Command*)b)->OrderIndex;
}

static int ConvertToOpb(Context* context) {
    if (context->Format < OPB_Format_Default || context->Format > OPB_Format_Raw) {
        context->Format = OPB_Format_Default;
    }

    WRITE(OPB_Header, sizeof(char), OPB_HEADER_SIZE, context);

    Log("OPB format %d (%s)\n", context->Format, OPB_GetFormatName(context->Format));

    uint8_t fmt = (uint8_t)context->Format;
    WRITE(&fmt, sizeof(uint8_t), 1, context);

    if (context->Format == OPB_Format_Raw) {
        Log("Writing raw OPL data stream\n");

        double lastTime = 0.0;
        for (int i = 0; i < context->CommandStream.Count; i++) {
            Command* cmd = Vector_GetT(Command, &context->CommandStream, i);

            uint16_t elapsed = FlipEndian16((uint16_t)((cmd->Time - lastTime) * 1000.0));
            uint16_t addr = FlipEndian16(cmd->Addr);

            WRITE(&elapsed, sizeof(uint16_t), 1, context);
            WRITE(&addr, sizeof(uint16_t), 1, context);
            WRITE(&(cmd->Data), sizeof(uint8_t), 1, context);
            lastTime = cmd->Time;
        }
        return 0;
    }

    // separate command stream into tracks
    Log("Separating OPL data stream into channels\n");
    SeparateTracks(context);

    // process each track into its own output vector
    VectorT(Command) chOut[NUM_TRACKS];

    for (int i = 0; i < NUM_TRACKS; i++) {
        Log("Processing channel %d\n", i);
        chOut[i] = Vector_New(sizeof(Command));

        int ret = ProcessTrack(context, i, chOut + i);
        if (ret) {
            for (int j = 0; j < NUM_TRACKS; j++) {
                Vector_Free(chOut + j);
            }
            return ret;
        }
    }

    // combine all output back into command stream
    Log("Combining processed data into linear stream\n");
    Vector_Clear(&context->CommandStream, true);
    for (int i = 0; i < NUM_TRACKS; i++) {
        Vector_AddRange(&context->CommandStream, chOut[i].Storage, chOut[i].Count);
    }

    for (int j = 0; j < NUM_TRACKS; j++) {
        Vector_Free(chOut + j);
    }

    // sort by received order
    Vector_Sort(&context->CommandStream, SortCommands);

    // write instruments table
    SEEK(context, 12, SEEK_CUR); // skip header

    Log("Writing instrument table\n");
    for (int i = 0; i < context->Instruments.Count; i++) {
        int ret = WriteInstrument(context, Vector_GetT(Instrument, &context->Instruments, i));
        if (ret) return ret;
    }

    // write chunks
    {
        int chunks = 0;
        double lastTime = 0;
        int i = 0;

        Log("Writing chunks\n");
        while (i < context->CommandStream.Count) {
            double chunkTime = Vector_GetT(Command, &context->CommandStream, i)->Time;

            int start = i;
            while (i < context->CommandStream.Count && Vector_GetT(Command, &context->CommandStream, i)->Time <= chunkTime) {
                i++;
            }
            int end = i;

            int ret = WriteChunk(context, chunkTime - lastTime, start, end - start);
            if (ret) return ret;
            chunks++;

            lastTime = chunkTime;
        }

        // write header
        Log("Writing header\n");

        long fpos;
        TELL(context, fpos);

        uint32_t length = FlipEndian32(fpos);
        uint32_t instrCount = FlipEndian32((uint32_t)context->Instruments.Count);
        uint32_t chunkCount = FlipEndian32(chunks);

        SEEK(context, OPB_HEADER_SIZE + 1, SEEK_SET);
        WRITE(&length, sizeof(uint32_t), 1, context);
        WRITE(&instrCount, sizeof(uint32_t), 1, context);
        WRITE(&chunkCount, sizeof(uint32_t), 1, context);
    }

    return 0;
}

static size_t WriteToFile(const void* buffer, size_t elementSize, size_t elementCount, void* context) {
    return fwrite(buffer, elementSize, elementCount, (FILE*)context);
}

static int SeekInFile(void* context, long offset, int origin) {
    return fseek((FILE*)context, offset, origin);
}

static long TellInFile(void* context) {
    return ftell((FILE*)context);
}

int OPB_OplToFile(OPB_Format format, OPB_Command* commandStream, size_t commandCount, const char* file) {
    FILE* outFile;
    if ((outFile = fopen(file, "wb")) == NULL) {
        Log("Couldn't open file '%s' for writing\n", file);
        return OPBERR_LOGGED;
    }
    int ret = OPB_OplToBinary(format, commandStream, commandCount, WriteToFile, SeekInFile, TellInFile, outFile);
    if (fclose(outFile)) {
        Log("Error while closing file '%s'\n", file);
        return OPBERR_LOGGED;
    }
    return ret;
}

int OPB_OplToBinary(OPB_Format format, OPB_Command* commandStream, size_t commandCount, OPB_StreamWriter write, OPB_StreamSeeker seek, OPB_StreamTeller tell, void* userData) {
    Context context = Context_New();

    context.Write = write;
    context.Seek = seek;
    context.Tell = tell;
    context.UserData = userData;
    context.Format = format;

    // convert stream to internal format
    for (int i = 0; i < commandCount; i++) {
        const OPB_Command* src = commandStream + i;

        Command cmd = {
            src->Addr,   // OPL register
            src->Data,   // OPL data
            src->Time,   // Time in seconds
            i,           // Stream index
            0            // Data index
        };

        Vector_Add(&context.CommandStream, &cmd);
    }

    int ret = ConvertToOpb(&context);
    Context_Free(&context);

    if (ret) {
        Log("%s\n", OPB_GetErrorMessage(ret));
    }

    return ret;
}

static int ReadInstrument(Context* context, Instrument* instr) {
    uint8_t buffer[9];
    READ(buffer, sizeof(uint8_t), 9, context);
    *instr = MakeInstrument(
        buffer[0], // feedconn
        MakeOperator(
            buffer[1], // modulator characteristic
            buffer[2], // modulator attack/decay
            buffer[3], // modulator sustain/release
            buffer[4]  // modulator wave select
        ),
        MakeOperator(
            buffer[5], // carrier characteristic
            buffer[6], // carrier attack/decay
            buffer[7], // carrier sustain/release
            buffer[8]  // carrier wave select
        ),
        (int)context->Instruments.Count // instrument index
    );
    return 0;
}

static int ReadUint7(Context* context) {
    uint8_t b0 = 0, b1 = 0, b2 = 0, b3 = 0;

    if (context->Read(&b0, sizeof(uint8_t), 1, context->UserData) != 1) return -1;
    if (b0 >= 128) {
        b0 &= 0b01111111;
        if (context->Read(&b1, sizeof(uint8_t), 1, context->UserData) != 1) return -1;
        if (b1 >= 128) {
            b1 &= 0b01111111;
            if (context->Read(&b2, sizeof(uint8_t), 1, context->UserData) != 1) return -1;
            if (b2 >= 128) {
                b2 &= 0b01111111;
                if (context->Read(&b3, sizeof(uint8_t), 1, context->UserData) != 1) return -1;
            }
        }
    }

    return b0 | (b1 << 7) | (b2 << 14) | (b3 << 21);
}

#define DEFAULT_READBUFFER_SIZE 256

static inline int AddToBuffer(Context* context, OPB_Command* buffer, int* index, OPB_Command cmd) {
    buffer[*index] = cmd;
    (*index)++;

    if (*index >= DEFAULT_READBUFFER_SIZE) {
        SUBMIT(buffer, DEFAULT_READBUFFER_SIZE, context);
        *index = 0;
    }

    return 0;
}

#define ADD_TO_BUFFER_IMPL(retvar, context, buffer, index, command) \
    { int retvar; \
    if ((retvar = AddToBuffer(context, buffer, bufferIndex, command))) return retvar; }
#define ADD_TO_BUFFER(context, buffer, index, ...) ADD_TO_BUFFER_IMPL(MACRO_CONCAT(__ret, __LINE__), context, buffer, index, __VA_ARGS__)

static int ReadCommand(Context* context, OPB_Command* buffer, int* bufferIndex, int mask) {
    uint8_t baseAddr;
    READ(&baseAddr, sizeof(uint8_t), 1, context);

    int addr = baseAddr | mask;

    switch (baseAddr) {
        default: {
            uint8_t data;
            READ(&data, sizeof(uint8_t), 1, context);
            ADD_TO_BUFFER(context, buffer, bufferIndex, MakeCommand( (uint16_t)addr, data, context->Time ));
            break;
        }
        
        case OPB_CMD_PLAYINSTRUMENT:
        case OPB_CMD_SETINSTRUMENT: {
            int instrIndex;
            READ_UINT7(instrIndex, context);

            uint8_t channelMask[2];
            READ(channelMask, sizeof(uint8_t), 2, context);

            int channel = channelMask[0];
            bool modLvl = (channel & 0b00100000) != 0;
            bool carLvl = (channel & 0b01000000) != 0;
            bool feedconn = (channel & 0b10000000) != 0;
            channel &= 0b00011111;

            if (channel < 0 || channel >= NUM_CHANNELS) {
                Log("Error reading OPB command: channel %d out of range\n", channel);
                return OPBERR_LOGGED;
            }

            int chmask = channelMask[1];
            bool modChr = (chmask & 0b00000001) != 0;
            bool modAtk = (chmask & 0b00000010) != 0;
            bool modSus = (chmask & 0b00000100) != 0;
            bool modWav = (chmask & 0b00001000) != 0;
            bool carChr = (chmask & 0b00010000) != 0;
            bool carAtk = (chmask & 0b00100000) != 0;
            bool carSus = (chmask & 0b01000000) != 0;
            bool carWav = (chmask & 0b10000000) != 0;

            uint8_t freq = 0, note = 0;
            bool isPlay = baseAddr == OPB_CMD_PLAYINSTRUMENT;
            if (isPlay) {
                READ(&freq, sizeof(uint8_t), 1, context);
                READ(&note, sizeof(uint8_t), 1, context);
            }

            uint8_t modLvlData = 0, carLvlData = 0;
            if (modLvl) READ(&modLvlData, sizeof(uint8_t), 1, context);
            if (carLvl) READ(&carLvlData, sizeof(uint8_t), 1, context);

            if (instrIndex < 0 || instrIndex >= context->Instruments.Count) {
                Log("Error reading OPB command: instrument %d out of range\n", instrIndex);
                return OPBERR_LOGGED;
            }

            Instrument* instr = Vector_GetT(Instrument, &context->Instruments, instrIndex);
            int conn = ChannelToOffset[channel];
            int mod = OperatorOffsets[ChannelToOp[channel]];
            int car = mod + 3;
            int playOffset = ChannelToOffset[channel];

            if (feedconn) ADD_TO_BUFFER(context, buffer, bufferIndex, MakeCommand( (uint16_t)(REG_FEEDCONN + conn), (uint8_t)instr->FeedConn, context->Time ));
            if (modChr)   ADD_TO_BUFFER(context, buffer, bufferIndex, MakeCommand( (uint16_t)(REG_CHARACTER + mod), (uint8_t)instr->Modulator.Characteristic, context->Time ));
            if (modLvl)   ADD_TO_BUFFER(context, buffer, bufferIndex, MakeCommand( (uint16_t)(REG_LEVELS + mod), modLvlData, context->Time ));
            if (modAtk)   ADD_TO_BUFFER(context, buffer, bufferIndex, MakeCommand( (uint16_t)(REG_ATTACK + mod), (uint8_t)instr->Modulator.AttackDecay, context->Time ));
            if (modSus)   ADD_TO_BUFFER(context, buffer, bufferIndex, MakeCommand( (uint16_t)(REG_SUSTAIN + mod), (uint8_t)instr->Modulator.SustainRelease, context->Time ));
            if (modWav)   ADD_TO_BUFFER(context, buffer, bufferIndex, MakeCommand( (uint16_t)(REG_WAVE + mod), (uint8_t)instr->Modulator.WaveSelect, context->Time ));
            if (carChr)   ADD_TO_BUFFER(context, buffer, bufferIndex, MakeCommand( (uint16_t)(REG_CHARACTER + car), (uint8_t)instr->Carrier.Characteristic, context->Time ));
            if (carLvl)   ADD_TO_BUFFER(context, buffer, bufferIndex, MakeCommand( (uint16_t)(REG_LEVELS + car), carLvlData, context->Time ));
            if (carAtk)   ADD_TO_BUFFER(context, buffer, bufferIndex, MakeCommand( (uint16_t)(REG_ATTACK + car), (uint8_t)instr->Carrier.AttackDecay, context->Time ));
            if (carSus)   ADD_TO_BUFFER(context, buffer, bufferIndex, MakeCommand( (uint16_t)(REG_SUSTAIN + car), (uint8_t)instr->Carrier.SustainRelease, context->Time ));
            if (carWav)   ADD_TO_BUFFER(context, buffer, bufferIndex, MakeCommand( (uint16_t)(REG_WAVE + car), (uint8_t)instr->Carrier.WaveSelect, context->Time ));
            if (isPlay) {
                ADD_TO_BUFFER(context, buffer, bufferIndex, MakeCommand( (uint16_t)(REG_FREQUENCY + playOffset), freq, context->Time ));
                ADD_TO_BUFFER(context, buffer, bufferIndex, MakeCommand( (uint16_t)(REG_NOTE + playOffset), note, context->Time ));
            }

            break;
        }

        case OPB_CMD_NOTEON:
        case OPB_CMD_NOTEON + 1:
        case OPB_CMD_NOTEON + 2:
        case OPB_CMD_NOTEON + 3:
        case OPB_CMD_NOTEON + 4:
        case OPB_CMD_NOTEON + 5:
        case OPB_CMD_NOTEON + 6:
        case OPB_CMD_NOTEON + 7:
        case OPB_CMD_NOTEON + 8: {
            int channel = (baseAddr - 0xD7) + (mask != 0 ? 9 : 0);

            if (channel < 0 || channel >= NUM_CHANNELS) {
                Log("Error reading OPB command: channel %d out of range\n", channel);
                return OPBERR_LOGGED;
            }

            uint8_t freqNote[2];
            READ(freqNote, sizeof(uint8_t), 2, context);

            uint8_t freq = freqNote[0];
            uint8_t note = freqNote[1];

            ADD_TO_BUFFER(context, buffer, bufferIndex, MakeCommand( (uint16_t)(addr - (OPB_CMD_NOTEON - REG_FREQUENCY)), freq, context->Time ));
            ADD_TO_BUFFER(context, buffer, bufferIndex, MakeCommand( (uint16_t)(addr - (OPB_CMD_NOTEON - REG_NOTE)), (uint8_t)(note & 0b00111111), context->Time ));

            if ((note & 0b01000000) != 0) {
                // set modulator volume
                uint8_t vol;
                READ(&vol, sizeof(uint8_t), 1, context);
                int reg = REG_LEVELS + OperatorOffsets[ChannelToOp[channel]];
                ADD_TO_BUFFER(context, buffer, bufferIndex, MakeCommand( (uint16_t)reg, vol, context->Time ));
            }
            if ((note & 0b10000000) != 0) {
                // set carrier volume
                uint8_t vol;
                READ(&vol, sizeof(uint8_t), 1, context);
                int reg = REG_LEVELS + 3 + OperatorOffsets[ChannelToOp[channel]];
                ADD_TO_BUFFER(context, buffer, bufferIndex, MakeCommand( (uint16_t)reg, vol, context->Time ));
            }
            break;
        }
    }

    return 0;
}

static int ReadChunk(Context* context, OPB_Command* buffer, int* bufferIndex) {
    int elapsed, loCount, hiCount;

    READ_UINT7(elapsed, context);
    READ_UINT7(loCount, context);
    READ_UINT7(hiCount, context);

    context->Time += elapsed / 1000.0;

    for (int i = 0; i < loCount; i++) {
        int ret = ReadCommand(context, buffer, bufferIndex, 0x0);
        if (ret) return ret;
    }
    for (int i = 0; i < hiCount; i++) {
        int ret = ReadCommand(context, buffer, bufferIndex, 0x100);
        if (ret) return ret;
    }

    return 0;
}

static int ReadOpbDefault(Context* context) {
    uint32_t header[3];
    READ(header, sizeof(uint32_t), 3, context);
    for (int i = 0; i < 3; i++) header[i] = FlipEndian32(header[i]);

    uint32_t instrumentCount = header[1];
    uint32_t chunkCount = header[2];

    for (uint32_t i = 0; i < instrumentCount; i++) {
        Instrument instr;
        int ret = ReadInstrument(context, &instr);
        if (ret) return ret;
        Vector_Add(&context->Instruments, &instr);
    }

    OPB_Command buffer[DEFAULT_READBUFFER_SIZE];
    int bufferIndex = 0;

    for (uint32_t i = 0; i < chunkCount; i++) {
        int ret = ReadChunk(context, buffer, &bufferIndex);
        if (ret) return ret;
    }

    if (bufferIndex > 0) {
        SUBMIT(buffer, bufferIndex, context);
    }

    return 0;
}

#define RAW_READBUFFER_SIZE 256
#define RAW_ENTRY_SIZE 5

static int ReadOpbRaw(Context* context) {
    double time = 0;
    uint8_t buffer[RAW_READBUFFER_SIZE * RAW_ENTRY_SIZE];
    OPB_Command commandStream[RAW_READBUFFER_SIZE];

    size_t itemsRead;
    while ((itemsRead = context->Read(buffer, RAW_ENTRY_SIZE, RAW_READBUFFER_SIZE, context->UserData)) > 0) {
        uint8_t* value = buffer;

        for (int i = 0; i < itemsRead; i++, value += RAW_ENTRY_SIZE) {
            uint16_t elapsed = (value[0] << 8) | value[1];
            uint16_t addr = (value[2] << 8) | value[3];
            uint8_t data = value[4];

            time += elapsed / 1000.0;
            
            OPB_Command cmd = {
                addr,
                data,
                time
            };
            commandStream[i] = cmd;
        }
        SUBMIT(commandStream, itemsRead, context);
    }

    return 0;
}

static int ConvertFromOpb(Context* context) {
    char id[OPB_HEADER_SIZE + 1] = { 0 };
    READ(id, sizeof(char), OPB_HEADER_SIZE, context);

    if (strncmp(id, "OPBin", 5)) {
        return OPBERR_NOT_AN_OPB_FILE;
    }

    switch (id[5]) {
    case '1':
        break;
    default:
        return OPBERR_VERSION_UNSUPPORTED;
    }

    if (id[6] != '\0') {
        return OPBERR_NOT_AN_OPB_FILE;
    }

    uint8_t fmt;
    READ(&fmt, sizeof(uint8_t), 1, context);

    switch (fmt) {
    default:
        Log("Error reading OPB file: unknown format %d\n", fmt);
        return OPBERR_LOGGED;
    case OPB_Format_Default:
        return ReadOpbDefault(context);
    case OPB_Format_Raw:
        return ReadOpbRaw(context);
    }
}

static size_t ReadFromFile(void* buffer, size_t elementSize, size_t elementCount, void* context) {
    return fread(buffer, elementSize, elementCount, (FILE*)context);
}

int OPB_FileToOpl(const char* file, OPB_BufferReceiver receiver, void* receiverData) {
    FILE* inFile;
    if ((inFile = fopen(file, "rb")) == NULL) {
        Log("Couldn't open file '%s' for reading\n", file);
        return OPBERR_LOGGED;
    }
    int ret = OPB_BinaryToOpl(ReadFromFile, inFile, receiver, receiverData);
    fclose(inFile);
    return ret;
}

int OPB_BinaryToOpl(OPB_StreamReader reader, void* readerData, OPB_BufferReceiver receiver, void* receiverData) {
    Context context = { 0 };

    context.Read = reader;
    context.Submit = receiver;
    context.UserData = readerData;
    context.ReceiverData = receiverData;
    context.Instruments = Vector_New(sizeof(Instrument));

    int ret = ConvertFromOpb(&context);
    Context_Free(&context);

    if (ret) {
        Log("%s\n", OPB_GetErrorMessage(ret));
    }

    return ret;
}

const char* OPB_GetErrorMessage(int errCode) {
    switch (errCode) {
    case OPBERR_WRITE_ERROR:
        return "A write error occurred while converting OPB";
        break;
    case OPBERR_SEEK_ERROR:
        return "A seek error occurred while converting OPB";
        break;
    case OPBERR_TELL_ERROR:
        return "A file position error occurred while converting OPB";
        break;
    case OPBERR_READ_ERROR:
        return "A read error occurred while converting OPB";
        break;
    case OPBERR_BUFFER_ERROR:
        return "A buffer error occurred while converting OPB";
        break;
    case OPBERR_NOT_AN_OPB_FILE:
        return "Couldn't parse OPB file; not a valid OPB file";
        break;
    case OPBERR_VERSION_UNSUPPORTED:
        return "Couldn't parse OPB file; invalid version or version unsupported";
        break;
    default:
        return "Unknown OPB error";
    }
}


#endif /* OPBLIB_IMPLEMENTATION */
