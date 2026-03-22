/**
 * @file hash.h
 * @copyright Copyright (c) 2026 Tycjan Fortuna.
 *            All rights reserved.
 */
#pragma once

#include "wandelt/string.h"

u64 fnv1a_hash(StringView str, u64 count);
