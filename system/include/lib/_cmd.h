#ifndef _CMD_ENTRY_H_
#define _CMD_ENTRY_H_

#include <_list.h>

typedef struct __CMD_Entry {
    char *name;
    int	(*func)(int argc, char * const argv[]);
#if defined(__ARMCC_VERSION)
    struct list_head list;
#endif
} CMD_Entry;

int  CMD_Run(char *command);

#if !defined(__ARMCC_VERSION)
#define CMD_TYPE	static
#define CMD_DEFINE(_cmd, _func) \
	CMD_Entry _c_ent_##_func_##_cmd		\
	__attribute__((unused, section(".cmd_entry_2_"#_cmd""))) = { #_cmd, _func }
#else
void CMD_Register(CMD_Entry *entry);
#define CMD_TYPE
#define CMD_DEFINE(_cmd, _func) \
static CMD_Entry _c_ent_##_cmd	= { #_cmd, _func, LIST_HEAD_INIT(_c_ent_##_cmd.list) }; \
void do_cmd_##_cmd(void) __attribute__((constructor(10)));	\
void do_cmd_##_cmd(void) { \
	CMD_Register(&_c_ent_##_cmd); \
}
#endif

#endif
