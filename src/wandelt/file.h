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
} File;

File file_create(Allocator* alloc, String path);
void file_print_info(const File* file);
void file_destroy(File* file);

StringView file_get_part_of_content(const File* file, u32 start, u32 length);

typedef struct FileLocation
{
	u32 row;
	u32 col;
} FileLocation;

FileLocation file_resolve_location(const File* file, u32 offset);

bool does_file_exist(String path);
