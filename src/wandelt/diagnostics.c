#include "diagnostics.h"

#include <stdarg.h>

#include "defines.h"

static u32 g_error_count   = 0;
static u32 g_warning_count = 0;

void diagnostics_vnote_along_span(Span span, const File* file, const char* message, ...)
{
	char formatted_message[1024];
	va_list args;
	va_start(args, message);
	vsnprintf(formatted_message, sizeof(formatted_message), message, args);
	_diagnostics_print_at_location(span, file, formatted_message, DIAGNOSTIC_PRINT_TYPE_NOTE);
	va_end(args);
}

void diagnostics_vwarning_along_span(Span span, const File* file, const char* message, ...)
{
	char formatted_message[1024];
	va_list args;
	va_start(args, message);
	vsnprintf(formatted_message, sizeof(formatted_message), message, args);
	_diagnostics_print_at_location(span, file, formatted_message, DIAGNOSTIC_PRINT_TYPE_WARN);
	va_end(args);
}

void diagnostics_verror_along_span(Span span, const File* file, const char* message, ...)
{
	char formatted_message[1024];
	va_list args;
	va_start(args, message);
	vsnprintf(formatted_message, sizeof(formatted_message), message, args);
	_diagnostics_print_at_location(span, file, formatted_message, DIAGNOSTIC_PRINT_TYPE_ERROR);
	va_end(args);
}

bool diagnostics_has_errors(void)
{
	return g_error_count > 0;
}

bool diagnostics_has_warnings(void)
{
	return g_warning_count > 0;
}

u32 diagnostics_get_error_count(void)
{
	return g_error_count;
}

u32 diagnostics_get_warning_count(void)
{
	return g_warning_count;
}

void _diagnostics_print_at_location(Span span, const File* file, const char* message, DiagnosticPrintType print_type)
{
	const char* print_type_str = "";
	switch (print_type)
	{
	case DIAGNOSTIC_PRINT_TYPE_NOTE:
		print_type_str = "note";
		break;
	case DIAGNOSTIC_PRINT_TYPE_WARN:
		print_type_str = "warning";
		g_warning_count++;
		break;
	case DIAGNOSTIC_PRINT_TYPE_ERROR:
		print_type_str = "error";
		g_error_count++;
		break;
	}

	const char* color_code = "";
	switch (print_type)
	{
	case DIAGNOSTIC_PRINT_TYPE_NOTE:
		color_code = ANSI_COLOR_BLUE;
		break;
	case DIAGNOSTIC_PRINT_TYPE_WARN:
		color_code = ANSI_COLOR_YELLOW;
		break;
	case DIAGNOSTIC_PRINT_TYPE_ERROR:
		color_code = ANSI_COLOR_RED;
		break;
	}

	FileLocation loc = file_resolve_location(file, span.begin);

	// Header line: main.wdt:1:1: error: message
	printf("%s%s%.*s:%u:%u: %s%s:%s %s%s%s\n", ANSI_COLOR_BOLD, ANSI_COLOR_WHITE, FMT_STR_ARG(file->name), loc.row,
	       loc.col, color_code, print_type_str, ANSI_COLOR_RESET, ANSI_COLOR_BOLD, message, ANSI_COLOR_RESET);

	// Find the start and end of the source line containing the error
	const char* src = file->content.data;
	u64 src_len     = file->content.len;
	u64 line_start  = span.begin;
	u64 line_end    = span.begin;

	while (line_start > 0 && src[line_start - 1] != '\n') line_start--;
	while (line_end < src_len && src[line_end] != '\n') line_end++;

	int gutter_width = snprintf(NULL, 0, "%u", loc.row);

	// If the error line is empty, print the previous line for context
	if (line_start == line_end && line_start > 0)
	{
		size_t prev_end   = line_start - 1; // skip the '\n'
		size_t prev_start = prev_end;
		while (prev_start > 0 && src[prev_start - 1] != '\n') prev_start--;

		printf(" %*u | %.*s\n", gutter_width, loc.row - 1, (int)(prev_end - prev_start), src + prev_start);
	}

	// Print the source line with line number gutter
	// 1 |
	printf(" %u | %.*s\n", loc.row, (int)(line_end - line_start), src + line_start);

	// Print the caret + tildes underline with matching gutter
	u64 col_offset = span.begin - line_start;
	u64 span_len   = span.end - span.begin;
	if (span_len == 0)
		span_len = 1;

	printf(" %*s | %s%s", gutter_width, "", color_code, ANSI_COLOR_BOLD);

	for (u64 i = 0; i < col_offset; i++) putchar(' ');

	putchar('^');

	for (u64 i = 1; i < span_len && (col_offset + i) < (line_end - line_start); i++) putchar('~');

	printf("%s\n", ANSI_COLOR_RESET);
}
