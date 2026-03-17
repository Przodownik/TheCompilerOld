/**
 * @file diagnostics.h
 * @copyright Copyright (c) 2026 Tycjan Fortuna.
 *            All rights reserved.
 */
#pragma once

#include "wandelt/lexer.h"

typedef enum DiagnosticPrintType
{
	DIAGNOSTIC_PRINT_TYPE_NOTE,
	DIAGNOSTIC_PRINT_TYPE_WARN,
	DIAGNOSTIC_PRINT_TYPE_ERROR,
} DiagnosticPrintType;

void diagnostics_vnote_along_span(Span span, const File* file, const char* message, ...);
void diagnostics_vwarning_along_span(Span span, const File* file, const char* message, ...);
void diagnostics_verror_along_span(Span span, const File* file, const char* message, ...);

bool diagnostics_has_errors(void);
bool diagnostics_has_warnings(void);

u32 diagnostics_get_error_count(void);
u32 diagnostics_get_warning_count(void);

void _diagnostics_print_at_location(Span span, const File* file, const char* message, DiagnosticPrintType print_type);
