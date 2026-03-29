/**
 * @file symbol_table.h
 * @copyright Copyright (c) 2026 Tycjan Fortuna.
 *            All rights reserved.
 */
#pragma once

#include "wandelt/ast.h"
#include "wandelt/file.h"
#include "wandelt/string.h"

#define SYMTAB_BUCKETS 128

typedef enum SymbolKind
{
	SYMBOL_KIND_VARIABLE,
} SymbolKind;

typedef struct Symbol
{
	StringView name;
	SymbolKind kind;
	Type* type;
	Declaration* declaration_ref;
	const File* source_file;
	u32 scope_depth;
	bool is_used;

	struct Symbol* next_in_bucket;
	struct Symbol* next_in_scope;
} Symbol;

typedef struct Scope
{
	struct Scope* parent;
	Symbol* first_symbol;
	u32 depth; // nesting depth (0 = global)
} Scope;

typedef struct SymbolTable
{
	Symbol* buckets[SYMTAB_BUCKETS];
	Scope* current_scope;
	u32 scope_depth;
	Allocator* allocator;
} SymbolTable;

SymbolTable symtab_create(Allocator* allocator);

void symtab_push_scope(SymbolTable* tab);
void symtab_pop_scope(SymbolTable* tab);

Symbol* symtab_insert(SymbolTable* tab, StringView name, SymbolKind kind, Type* type, Declaration* decl);
Symbol* symtab_lookup(SymbolTable* tab, StringView name, bool is_used);
