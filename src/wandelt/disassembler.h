/**
 * @file disassembler.h
 * @copyright Copyright (c) 2026 Tycjan Fortuna.
 *            All rights reserved.
 */
#pragma once

#include "wandelt/bytecode.h"
#include "wandelt/file.h"

void disassemble_chunk(Chunk* chunk, const char* name, const File* source);
void disassemble_instruction(Chunk* chunk, u32 offset);
bool disassemble_chunk_to_file(Chunk* chunk, const char* name, const File* source, const char* filepath);
