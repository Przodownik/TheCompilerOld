/**
 * @file file.h
 * @copyright Copyright (c) 2026 Tycjan Fortuna.
 *            All rights reserved.
 */
#pragma once

#include "wandelt/string.h"

typedef struct File
{
	Allocator* alloc;
	String path;      // Path to the file (relative to the project root)
	String name;      // Name of the file (with extension)
	String content;   // Content of the file
	u64 content_size; // Size of the content in bytes
	u32 total_rows;   // Number of lines in the file
} File;

File file_create(Allocator* alloc, String path);
void file_print_info(const File* file);
void file_destroy(File* file);

bool does_file_exist(String path);
