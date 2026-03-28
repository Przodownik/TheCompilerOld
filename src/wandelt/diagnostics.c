#include "diagnostics.h"

#include <stdarg.h>

#include "defines.h"
#include "platform.h"

static u32 g_error_count      = 0;
static u32 g_warning_count    = 0;
static bool g_capture_enabled = false;
static DiagnosticEntry g_captured[MAX_CAPTURED_DIAGNOSTICS];
static u32 g_captured_count = 0;

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

void diagnostics_reset(void)
{
	g_error_count   = 0;
	g_warning_count = 0;
}

void diagnostics_enable_capture(void)
{
	g_capture_enabled = true;
	g_captured_count  = 0;
}

void diagnostics_disable_capture(void)
{
	g_capture_enabled = false;
}

u32 diagnostics_captured_count(void)
{
	return g_captured_count;
}

DiagnosticEntry* diagnostics_get_captured(u32 index)
{
	ASSERT(index < g_captured_count);
	return &g_captured[index];
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

	// we either capture the diagnostic for later query or print it immediately to the console
	if (g_capture_enabled)
	{
		if (g_captured_count < MAX_CAPTURED_DIAGNOSTICS)
		{
			DiagnosticEntry* entry = &g_captured[g_captured_count++];
			entry->type            = print_type;
			FileLocation loc       = file_resolve_location(file, span.begin);
			entry->line            = loc.row;
			entry->col             = loc.col;
			strncpy_s(entry->message, sizeof(entry->message), message, _TRUNCATE);
			entry->message[sizeof(entry->message) - 1] = '\0';
		}
	}
	else
	{
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
			u64 prev_end   = line_start - 1; // skip the '\n'
			u64 prev_start = prev_end;
			while (prev_start > 0 && src[prev_start - 1] != '\n') prev_start--;

			printf(" %*u | %.*s\n", gutter_width, loc.row - 1, (int)(prev_end - prev_start), src + prev_start);
		}

		// Truncate long source lines to fit terminal width
		u64 col_offset = span.begin - line_start;
		u64 span_len   = span.end - span.begin;
		if (span_len == 0)
			span_len = 1;

		u64 line_len     = line_end - line_start;
		int gutter_chars = gutter_width + 4; // " %u | " = width + space + space + pipe + space
		int term_width   = platform_get_terminal_width();
		int avail        = term_width - gutter_chars;
		if (avail < 20)
			avail = 20; // minimum usable width

		u64 view_start  = 0;
		u64 view_end    = line_len;
		bool clip_left  = false;
		bool clip_right = false;

		if ((int)line_len > avail)
		{
			// Center the window on the span, with some margin
			int margin = (avail - (int)span_len) / 2;
			if (margin < 8)
				margin = 8;

			i64 desired_start = (i64)col_offset - margin;
			if (desired_start < 0)
				desired_start = 0;

			view_start = (u64)desired_start;
			view_end   = view_start + (u64)avail;

			// Account for "..." markers (3 chars each)
			if (view_start > 0)
			{
				view_start += 3;
				clip_left = true;
			}
			if (view_end < line_len)
			{
				view_end -= 3;
				clip_right = true;
			}
			if (view_end > line_len)
				view_end = line_len;
		}

		// Print the source line with line number gutter
		printf(" %u | ", loc.row);
		if (clip_left)
			printf("...");
		printf("%.*s", (int)(view_end - view_start), src + line_start + view_start);
		if (clip_right)
			printf("...");
		printf("\n");

		// Print the caret + tildes underline with matching gutter
		u64 display_offset = col_offset - view_start;

		printf(" %*s | ", gutter_width, "");
		if (clip_left)
			printf("   "); // match "..." width
		printf("%s%s", color_code, ANSI_COLOR_BOLD);

		for (u64 i = 0; i < display_offset; i++) putchar(' ');

		putchar('^');

		for (u64 i = 1; i < span_len && (display_offset + i) < (view_end - view_start); i++) putchar('~');

		printf("%s\n", ANSI_COLOR_RESET);
	}
}
