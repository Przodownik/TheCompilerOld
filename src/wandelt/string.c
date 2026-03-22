#include "string.h"

String string_from_cstr(Allocator* alloc, const char* str)
{
	u64 len   = strlen(str);
	char* buf = (char*)alloc->alloc(alloc->ctx, len + 1);

	memcpy(buf, str, len + 1);

	return (String){.alloc = alloc, .data = buf, .len = len};
}

String string_from_buffer(Allocator* alloc, u64 len)
{
	char* buf = (char*)alloc->alloc(alloc->ctx, len + 1);

	buf[len] = '\0';

	return (String){.alloc = alloc, .data = buf, .len = len};
}

void string_free(String* s)
{
	s->alloc->free(s->alloc->ctx, s->data, s->len + 1);
	s->data = NULL;
	s->len  = 0;
}

StringView string_view_from_cstr(const char* str)
{
	return (StringView){.data = str, .len = strlen(str)};
}

StringView string_view_from_cstr_part(const char* str, u64 len)
{
	return (StringView){.data = str, .len = len};
}

bool string_view_equals(StringView a, StringView b)
{
	if (a.len != b.len)
		return false;

	return memcmp(a.data, b.data, a.len) == 0;
}
