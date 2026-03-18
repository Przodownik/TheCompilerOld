/**
 * @file disassembler.h
 * @copyright Copyright (c) 2026 Tycjan Fortuna.
 *            All rights reserved.
 */
#pragma once

#include "wandelt/bytecode.h"

void disassemble_chunk(Chunk* chunk, const char* name);
void disassemble_instruction(Chunk* chunk, u32 offset);
