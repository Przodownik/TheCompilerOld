/**
 * @file disassembler.h
 * @copyright Copyright (c) 2026 Tycjan Fortuna.
 *            All rights reserved.
 */
#pragma once

#include "wandelt/bytecode.h"
#include "wandelt/file.h"

void disassemble_program(Chunk* main_chunk, CompiledFunction* functions, u8 function_count, const char* name,
                         const File* source);
bool disassemble_program_to_file(Chunk* main_chunk, CompiledFunction* functions, u8 function_count, const char* name,
                                 const File* source, const char* filepath);
