/**
 * @file platform.h
 * @copyright Copyright (c) 2026 Tycjan Fortuna.
 *            All rights reserved.
 */
#pragma once

#include "wandelt/defines.h"

typedef struct PlatformTimer
{
	u64 start;
	u64 frequency;
} PlatformTimer;

void platform_timer_start(PlatformTimer* timer);
double platform_timer_elapsed_ms(PlatformTimer* timer);

int platform_get_terminal_width(void);
