#include "bytecode.h"
#include "wandelt/ast.h"
#include "wandelt/vector.h"

const char* op_code_to_cstr(OpCode op)
{
	switch (op)
	{
	case OP_CODE_LOAD_CONST:
		return "LOAD_CONST";
	case OP_CODE_LOAD_INT:
		return "LOAD_INT";
	case OP_CODE_MOVE:
		return "MOVE";
	case OP_CODE_ADD:
		return "ADD";
	case OP_CODE_SUB:
		return "SUB";
	case OP_CODE_MUL:
		return "MUL";
	case OP_CODE_DIV:
		return "DIV";
	case OP_CODE_RETURN:
		return "RETURN";
	case OP_CODE_HALT:
		return "HALT";
	default:
		break;
	}

	ASSERT(false, "Invalid OpCode");
}

static u8 bytecode_compiler_compile_node(BytecodeCompiler* c, Statement* statement);
static u8 bytecode_compiler_compile_expr(BytecodeCompiler* c, Expression* expression);

BytecodeCompiler bytecode_compiler_create(Allocator* alloc, const File* source)
{
	return (BytecodeCompiler){
	    .alloc    = alloc,
	    .source   = source,
	    .chunk    = chunk_create(alloc),
	    .next_reg = 0,
	    .max_reg  = 0,
	};
}

Chunk chunk_create(Allocator* alloc)
{
	return (Chunk){
	    .instructions     = (Instruction*)vector_create(alloc, 32, sizeof(Instruction)),
	    .constants        = (Value*)vector_create(alloc, 8, sizeof(Value)),
	    .lines            = (u32*)vector_create(alloc, 32, sizeof(u32)),
	    .current_line     = 0,
	    .registers_needed = 0,
	};
}
u32 chunk_emit(Chunk* chunk, Instruction inst)
{
	u32 offset = (u32)vector_get_length(chunk->instructions);
	vector_push(chunk->instructions, inst);
	vector_push(chunk->lines, chunk->current_line);
	return offset;
}
u32 chunk_add_constant(Chunk* chunk, Value val)
{
	u32 index = (u32)vector_get_length(chunk->constants);
	vector_push(chunk->constants, val);
	return index;
}

Chunk bytecode_compiler_compile(BytecodeCompiler* compiler, Statement** program_statements)
{
	for (u64 i = 0; i < vector_get_length(program_statements); i++)
	{
		Statement* stmt = program_statements[i];
		bytecode_compiler_compile_node(compiler, stmt);
	}

	chunk_emit(&compiler->chunk, ENCODE_ABx(OP_CODE_HALT, 0, 0));

	compiler->chunk.registers_needed = compiler->max_reg;

	return compiler->chunk;
}

static void set_line_from_span(BytecodeCompiler* c, Span span)
{
	if (c->source)
	{
		FileLocation loc    = file_resolve_location(c->source, span.begin);
		c->chunk.current_line = loc.row;
	}
}

static u8 bytecode_compiler_compile_node(BytecodeCompiler* c, Statement* statement)
{
	set_line_from_span(c, statement->span);

	switch (statement->type)
	{
	case STATEMENT_TYPE_RETURN: {
		u8 reg = bytecode_compiler_compile_expr(c, statement->return_stmt.expression);
		chunk_emit(&c->chunk, ENCODE_ABx(OP_CODE_RETURN, reg, 0));
		return reg;
	}
	default:
		ASSERT(false, "Cannot compile node kind %d", statement->type);
		return 0;
	}
}

static u8 bytecode_compiler_compile_expr(BytecodeCompiler* c, Expression* expression)
{
	set_line_from_span(c, expression->span);

	switch (expression->type)
	{
	case EXPRESSION_TYPE_CONSTANT: {
		u8 dest = c->next_reg++;
		if (c->next_reg > c->max_reg) c->max_reg = c->next_reg;
		u32 k = chunk_add_constant(&c->chunk, value_int((i64)expression->constant.integer));
		chunk_emit(&c->chunk, ENCODE_ABx(OP_CODE_LOAD_CONST, dest, k));
		return dest;
	}
	case EXPRESSION_TYPE_BINARY: {
		u8 left  = bytecode_compiler_compile_expr(c, expression->binary.left);
		u8 right = bytecode_compiler_compile_expr(c, expression->binary.right);
		u8 dest  = left; // reuse left register

		OpCode op;

		switch (expression->binary.operator)
		{
		case BINARY_OPERATOR_ADD:
			op = OP_CODE_ADD;
			break;
		case BINARY_OPERATOR_SUB:
			op = OP_CODE_SUB;
			break;
		case BINARY_OPERATOR_MUL:
			op = OP_CODE_MUL;
			break;
		case BINARY_OPERATOR_DIV:
			op = OP_CODE_DIV;
			break;
		default:
			ASSERT(false, "Invalid binary operator");
		}

		chunk_emit(&c->chunk, ENCODE_ABC(op, dest, left, right));

		// Free the right register (reclaim for future use)
		c->next_reg = (u8)(dest + 1);

		return dest;
	}
	default:
		break;
	}

	ASSERT(false, "Cannot compile expression kind %d", expression->type);
}
