#include "platform.h"

#ifdef _WIN32

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

void platform_timer_start(PlatformTimer* timer)
{
	LARGE_INTEGER freq, counter;
	QueryPerformanceFrequency(&freq);
	QueryPerformanceCounter(&counter);
	timer->frequency = (u64)freq.QuadPart;
	timer->start     = (u64)counter.QuadPart;
}

double platform_timer_elapsed_ms(PlatformTimer* timer)
{
	LARGE_INTEGER counter;
	QueryPerformanceCounter(&counter);
	return (double)((u64)counter.QuadPart - timer->start) / (double)timer->frequency * 1000.0;
}

int platform_get_terminal_width(void)
{
	CONSOLE_SCREEN_BUFFER_INFO csbi;
	if (GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi))
		return csbi.srWindow.Right - csbi.srWindow.Left + 1;
	return 80;
}

#else

#include <sys/ioctl.h>
#include <unistd.h>
#include <time.h>

void platform_timer_start(PlatformTimer* timer)
{
	struct timespec ts;
	clock_gettime(CLOCK_MONOTONIC, &ts);
	timer->start     = (u64)ts.tv_sec * 1000000000ULL + (u64)ts.tv_nsec;
	timer->frequency = 1000000000ULL;
}

double platform_timer_elapsed_ms(PlatformTimer* timer)
{
	struct timespec ts;
	clock_gettime(CLOCK_MONOTONIC, &ts);
	u64 now = (u64)ts.tv_sec * 1000000000ULL + (u64)ts.tv_nsec;
	return (double)(now - timer->start) / (double)timer->frequency * 1000.0;
}

int platform_get_terminal_width(void)
{
	struct winsize w;
	if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &w) == 0 && w.ws_col > 0)
		return (int)w.ws_col;
	return 80;
}

#endif
