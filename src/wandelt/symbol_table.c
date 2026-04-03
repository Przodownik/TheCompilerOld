#include "symbol_table.h"

#include <stdio.h>

#include "wandelt/defines.h"
#include "wandelt/hash.h"
#include "wandelt/string.h"
#include "wandelt/type.h"

SymbolTable symtab_create(Allocator* allocator)
{
	SymbolTable tab   = {0};
	tab.allocator     = allocator;
	tab.scope_depth   = 0;
	tab.current_scope = nullptr;
	return tab;
}

void symtab_push_scope(SymbolTable* tab)
{
	Scope* scope        = tab->allocator->alloc(tab->allocator->ctx, sizeof(Scope));
	scope->parent       = tab->current_scope;
	scope->first_symbol = nullptr;
	scope->depth        = ++tab->scope_depth;

	tab->current_scope = scope;
}

void symtab_pop_scope(SymbolTable* tab)
{
	Scope* scope = tab->current_scope;
	ASSERT(scope != nullptr, "Cannot pop global scope");

	Symbol* sym = scope->first_symbol;
	while (sym != nullptr)
	{
		u64 bucket = fnv1a_hash(sym->name, SYMTAB_BUCKETS);
		ASSERT(tab->buckets[bucket] == sym, "Symbol table corruption: symbol not at bucket head");

		tab->buckets[bucket] = sym->next_in_bucket;
		sym                  = sym->next_in_scope;
	}

	tab->current_scope = scope->parent;
	tab->scope_depth--;
}

Symbol* symtab_insert(SymbolTable* tab, StringView name, SymbolKind kind, Type* type, Declaration* decl)
{
	u64 bucket = fnv1a_hash(name, SYMTAB_BUCKETS);

	Symbol* existing = tab->buckets[bucket];
	while (existing != nullptr)
	{
		if (string_view_equals(existing->name, name) && existing->scope_depth == tab->scope_depth)
		{
			// if declared in the same scope or from outer scope, it's an error
			if (existing->scope_depth == tab->scope_depth || existing->scope_depth < tab->scope_depth)
				return nullptr;
		}

		existing = existing->next_in_bucket;
	}

	Symbol* sym          = tab->allocator->alloc(tab->allocator->ctx, sizeof(Symbol));
	sym->name            = name;
	sym->kind            = kind;
	sym->type            = type;
	sym->declaration_ref = decl;
	sym->scope_depth     = tab->scope_depth;
	sym->is_used         = false;

	sym->next_in_bucket  = tab->buckets[bucket];
	tab->buckets[bucket] = sym;

	sym->next_in_scope               = tab->current_scope->first_symbol;
	tab->current_scope->first_symbol = sym;

	return sym;
}

Symbol* symtab_lookup(SymbolTable* tab, StringView name, bool is_used)
{
	u64 bucket  = fnv1a_hash(name, SYMTAB_BUCKETS);
	Symbol* sym = tab->buckets[bucket];

	while (sym != nullptr)
	{
		if (string_view_equals(sym->name, name))
		{
			sym->is_used = is_used;
			return sym;
		}

		sym = sym->next_in_bucket;
	}

	symtab_debug_print(tab);

	return nullptr;
}

void symtab_debug_print(const SymbolTable* tab)
{
	printf("Symbol Table ");
	for (int i = 0; i < 51; i++) putchar('=');
	putchar('\n');
	printf("  Scope depth : %u\n", tab->scope_depth);

	const Scope* scope = tab->current_scope;
	while (scope != nullptr)
	{
		printf("\n  Scope (depth %u)\n", scope->depth);
		printf("  ");
		for (int i = 0; i < 66; i++) putchar('-');
		putchar('\n');

		const Symbol* sym = scope->first_symbol;
		if (sym == nullptr)
		{
			printf("    (empty)\n");
		}

		while (sym != nullptr)
		{
			const char* kind_str = sym->kind == SYMBOL_KIND_VARIABLE ? "var" : "???";
			const char* type_str = sym->type ? type_kind_to_cstr(sym->type->kind) : "???";

			printf("    %-4s %-8s %.*s", kind_str, type_str, FMT_STR_ARG(sym->name));

			if (sym->is_used)
				printf("  (used)");
			else
				printf("  (unused)");

			putchar('\n');
			sym = sym->next_in_scope;
		}

		scope = scope->parent;
	}

	printf("\n");
	for (int i = 0; i < 68; i++) putchar('=');
	putchar('\n');
}
