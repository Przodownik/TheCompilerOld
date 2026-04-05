/**
 * @file defines.h
 * @copyright Copyright (c) 2026 Tycjan Fortuna.
 *            All rights reserved.
 */
#pragma once

#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Unsigned int types.
typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;
typedef unsigned long long u64;

// Signed int types.
typedef signed char i8;
typedef signed short i16;
typedef signed int i32;
typedef signed long long i64;

// Floating point types
typedef float f32;
typedef double f64;

static_assert(sizeof(u8) == 1, "u8 is not 1 byte");
static_assert(sizeof(u16) == 2, "u16 is not 2 bytes");
static_assert(sizeof(u32) == 4, "u32 is not 4 bytes");
static_assert(sizeof(u64) == 8, "u64 is not 8 bytes");

static_assert(sizeof(i8) == 1, "i8 is not 1 byte");
static_assert(sizeof(i16) == 2, "i16 is not 2 bytes");
static_assert(sizeof(i32) == 4, "i32 is not 4 bytes");
static_assert(sizeof(i64) == 8, "i64 is not 8 bytes");

static_assert(sizeof(f32) == 4, "f32 is not 4 bytes");
static_assert(sizeof(f64) == 8, "f64 is not 8 bytes");

#define nullptr NULL

#define KB(x) ((x) * 1024)
#define MB(x) (KB(x) * 1024)
#define GB(x) (MB(x) * 1024)

#define ANSI_COLOR_BOLD       "\x1b[1m"
#define ANSI_COLOR_BLACK      "\x1b[30m"
#define ANSI_COLOR_RED        "\x1b[31m"
#define ANSI_COLOR_GREEN      "\x1b[32m"
#define ANSI_COLOR_YELLOW     "\x1b[33m"
#define ANSI_COLOR_ORANGE     "\033[38;5;208m"
#define ANSI_COLOR_BLUE       "\x1b[34m"
#define ANSI_COLOR_MAGENTA    "\x1b[35m"
#define ANSI_COLOR_CYAN       "\x1b[36m"
#define ANSI_COLOR_WHITE      "\x1b[37m"
#define ANSI_BG_COLOR_BLACK   "\x1b[40m"
#define ANSI_BG_COLOR_RED     "\x1b[41m"
#define ANSI_BG_COLOR_GREEN   "\x1b[42m"
#define ANSI_BG_COLOR_YELLOW  "\x1b[43m"
#define ANSI_BG_COLOR_BLUE    "\x1b[44m"
#define ANSI_BG_COLOR_MAGENTA "\x1b[45m"
#define ANSI_BG_COLOR_CYAN    "\x1b[46m"
#define ANSI_BG_COLOR_WHITE   "\x1b[47m"
#define ANSI_COLOR_DIM        "\x1b[2m"
#define ANSI_COLOR_RESET      "\x1b[0m"

#define ASSERT_NO_MSG(condition)                                                                             \
	do                                                                                                       \
	{                                                                                                        \
		if (!(condition))                                                                                    \
		{                                                                                                    \
			fprintf(stderr,                                                                                  \
			        ANSI_COLOR_MAGENTA "Assertion failed: %s\n"                                              \
			                           "  File: %s\n"                                                        \
			                           "  Line: %d\n"                                                        \
			                           "\n"                                                                  \
			                           "The compiler encountered an unexpected error!\n"                     \
			                           "Please consider filing an issue on GitHub, including the possibly\n" \
			                           "erroneous source code and this error message, to help to diagnose\n" \
			                           "and fix the issue.\n" ANSI_COLOR_RESET,                              \
			        #condition, __FILE__, __LINE__);                                                         \
			abort();                                                                                         \
		}                                                                                                    \
	} while (0)

#define ASSERT_MSG(condition, ...)                                                                \
	do                                                                                            \
	{                                                                                             \
		if (!(condition))                                                                         \
		{                                                                                         \
			fprintf(stderr,                                                                       \
			        ANSI_COLOR_MAGENTA "Assertion failed: %s\n"                                   \
			                           "  File: %s\n"                                             \
			                           "  Line: %d\n"                                             \
			                           "  Message: ",                                             \
			        #condition, __FILE__, __LINE__);                                              \
			fprintf(stderr, __VA_ARGS__);                                                         \
			fprintf(stderr, "\n\n"                                                                \
			                "The compiler encountered an unexpected error!\n"                     \
			                "Please consider filing an issue on GitHub, including the possibly\n" \
			                "erroneous source code and this error message, to help to diagnose\n" \
			                "and fix the issue.\n" ANSI_COLOR_RESET);                             \
			abort();                                                                              \
		}                                                                                         \
	} while (0)

#define ASSERT_PICK(_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, NAME, ...) NAME
#define ASSERT(...)                                                                                              \
	ASSERT_PICK(__VA_ARGS__, ASSERT_MSG, ASSERT_MSG, ASSERT_MSG, ASSERT_MSG, ASSERT_MSG, ASSERT_MSG, ASSERT_MSG, \
	            ASSERT_MSG, ASSERT_MSG, ASSERT_NO_MSG, dummy)(__VA_ARGS__)
