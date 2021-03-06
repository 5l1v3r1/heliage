#pragma once

#define TEAL "\033[36m"
#define WHITE "\033[1;37m"
#define YELLOW "\033[1;33m"
#define RED "\033[1;31m"
#define PURPLE "\033[1;35m"

#define LTRACE(format, ...) printf("AF=%04X BC=%04X DE=%04X HL=%04X SP=%04X PC=%04X  " format "\n", af, bc, de, hl, sp, pc_at_opcode, ##__VA_ARGS__)
#define LDEBUG(format, ...) printf(TEAL "debug: " format "\033[0m\n", ##__VA_ARGS__)
#define LINFO(format, ...) printf(WHITE "info: " format "\033[0m\n", ##__VA_ARGS__)
#define LWARN(format, ...) printf(YELLOW "warning: " format "\033[0m\n", ##__VA_ARGS__)
#define LERROR(format, ...) printf(RED "error: " format "\033[0m\n", ##__VA_ARGS__)
#define LFATAL(format, ...) printf(PURPLE "fatal: " format "\033[0m\n", ##__VA_ARGS__)
