#include "file.h"
#include "defines.h"
#include "wandelt/string.h"

#include <assert.h>
#include <sys/stat.h>

File file_create(Allocator* alloc, String path)
{
	File result;
	result.alloc      = alloc;
	result.name       = string_from_cstr(alloc, strrchr(path.data, '/') ? strrchr(path.data, '/') + 1 : path.data);
	result.path       = path;
	result.total_rows = 0;

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

	// Count the number of lines in the file
	u64 i = 0;
	for (; i + 4 <= file_size; i += 4)
	{
		result.total_rows += (result.content.data[i] == '\n') + (result.content.data[i + 1] == '\n') +
		                     (result.content.data[i + 2] == '\n') + (result.content.data[i + 3] == '\n');
	}
	for (; i < file_size; i++)
		if (result.content.data[i] == '\n')
			result.total_rows++;

	return result;
}

void file_print_info(const File* file)
{
	printf("File '%s' info:\n", file->name.data);
	printf("- Path: %s\n", file->path.data);
	printf("- Size: %llu bytes\n", file->content_size);
	printf("- Total rows: %u\n", file->total_rows);
}

void file_destroy(File* file)
{
	string_free(&file->content);
}

bool does_file_exist(String path)
{
	struct stat buffer;

	return stat(path.data, &buffer) == 0;
}
