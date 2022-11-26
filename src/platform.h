#ifndef INCLUDED_PLATFORM_H
#define INCLUDED_PLATFORM_H

#include <stdint.h>

/* All platform-specific code goes here. */

int get_single_char();

typedef void (*getch_loop_proc)();

void get_terminal_size(uint32_t* width, uint32_t* height);

void create_global_mutex();

void wait_for_mutex();

void release_mutex();

void start_getch_loop(getch_loop_proc proc);

void join_getch_loop();

void sleep(uint64_t milliseconds);

#endif