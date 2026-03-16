#include "file.h"
#include "defines.h"
#include "wandelt/string.h"

#include <assert.h>
#include <sys/stat.h>

File file_create(Allocator* alloc, String path)
{
	File result;
	result.alloc = alloc;
	result.name  = string_from_cstr(alloc, strrchr(path.data, '/') ? strrchr(path.data, '/') + 1 : path.data);
	result.path  = path;

	FILE* file  = nullptr;
	errno_t err = fopen_s(&file, path.data, "rb");
	ASSERT(err == 0 && file, "Failed to open the file at path: %.*s", FMT_STR_ARG(path));

	fseek(file, 0, SEEK_END);

	const u64 file_size = (u64)ftell(file);
	result.content_size = file_size;
	rewind(file);

	result.content = string_from_buffer(alloc, file_size);

	const u64 bytes_read = fread(result.content.data, 1, file_size, file);
	ASSERT(bytes_read == file_size, "Failed to read file at path: %.*s", FMT_STR_ARG(path));

	result.content.data[file_size] = '\0';

	fclose(file);

	return result;
}

void file_print_info(const File* file)
{
	printf("File '%s' info:\n", file->name.data);
	printf("- Path: %s\n", file->path.data);
	printf("- Size: %llu bytes\n", file->content_size);
}

void file_destroy(File* file)
{
	string_free(&file->content);
}

StringView file_get_part_of_content(const File* file, u32 start, u32 length)
{
	ASSERT(start + length <= file->content_size, "Requested part of content is out of bounds");

	return string_view_from_cstr_part(file->content.data + start, length);
}

FileLocation file_resolve_location(const File* file, u32 offset)
{
	u32 row = 1;
	u32 col = 1;

	for (u32 i = 0; i < offset && i < file->content_size; i++)
	{
		if (file->content.data[i] == '\n')
		{
			row++;
			col = 1;
		}
		else
		{
			col++;
		}
	}

	return (FileLocation){.row = row, .col = col};
}

bool does_file_exist(String path)
{
	struct stat buffer;

	return stat(path.data, &buffer) == 0;
}
