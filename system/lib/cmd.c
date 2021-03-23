#include <string.h>
#include <cmsis_device.h>
#include <config.h>

#ifdef CMD_ENABLED

#ifndef CMD_BUFFER_SIZE
#define CMD_BUFFER_SIZE		128
#endif
#ifndef CMD_MAXARGS
#define CMD_MAXARGS		16
#endif

#ifdef __DEBUG__
#include <console.h>
#define dbg(m...)		Printf(m)
#else
#define dbg(m...)		do {} while(0)
#endif

#if !defined(__ARMCC_VERSION)
#define CMD_ENTRY_START \
({ \
        static char _cmd_start[0] __attribute__((unused, section(".cmd_entry_1"))); \
        (CMD_Entry *)&_cmd_start; \
})

#define CMD_ENTRY_END \
({ \
        static char _cmd_end[0] __attribute__((unused, section(".cmd_entry_3"))); \
        (CMD_Entry *)&_cmd_end; \
})

static CMD_Entry *CMD_FindEntry(const char *cmd)
{
	CMD_Entry *start = CMD_ENTRY_START;
	CMD_Entry *end = CMD_ENTRY_END;

	dbg("Entry: cmd:%s, 0x%x-0x%x, %dEa\n", cmd, start, end, (end - start));
	if (!cmd || (unsigned int)(end - start) == 0)
		return NULL;

	for (CMD_Entry *p = start; p < end; p++) {
		if (strcmp(cmd, p->name) == 0) {
			if (strlen(cmd) == strlen(p->name)) {
				dbg("Found [%s:%s]\n", cmd, p->name);
				return p;
			}
		}
	}

	return NULL;
}

#else
static LIST_HEAD(cmd_entry_list);

void CMD_Register(CMD_Entry *entry)
{
	list_add(&entry->list, &cmd_entry_list);
}

CMD_Entry *CMD_FindEntry(const char *cmd)
{
	struct list_head *pos;

	if (!cmd || list_empty(&cmd_entry_list))
		return NULL;

	list_for_each(pos, &cmd_entry_list) {
		CMD_Entry *p = list_entry(pos, CMD_Entry, list);
		if (strcmp(cmd, p->name) == 0) {
			if (strlen(cmd) == strlen(p->name)) {
				dbg("Found [%s:%s]\n", cmd, p->name);
				return p;
			}
		}
	}

	return NULL;
}
#endif

#define isblank(c)      (c == ' ' || c == '\t')

static int LineToArgs(char *line, char *argv[])
{
	int nargs = 0;

	while (nargs < CMD_MAXARGS) {
		/* skip any white space */
		while (isblank(*line))
			++line;

		if (*line == '\0') {	/* end of line, no more args	*/
			argv[nargs] = NULL;
			return nargs;
		}

		argv[nargs++] = line;	/* begin of argument string	*/

		/* find end of string */
		while (*line && !isblank(*line))
			++line;

		if (*line == '\0') {	/* end of line, no more args	*/
			argv[nargs] = NULL;
			return nargs;
		}

		*line++ = '\0';		/* terminate current arg	 */
	}

	return nargs;
}

static int CMD_RunEntry(int argc, char * const argv[])
{
	CMD_Entry *entry = CMD_FindEntry(argv[0]);

	if (!entry)
		return -1;

	return (entry->func)(argc, argv);
}

int CMD_Run(char *cmd)
{
	char cmdbuf[CMD_BUFFER_SIZE]; /* working copy of cmd */
	char *str = cmdbuf;
	char *argv[CMD_MAXARGS + 1];	/* NULL terminated	*/
	char *token;			/* start of token in cmdbuf	*/
	char *sep;			/* end of token (separator) in cmdbuf */
	int argc, inquotes;
	int rc = 0;

	if (!cmd || !*cmd)
		return -1;

	if (strlen(cmd) >= CMD_BUFFER_SIZE)
		return -1;

	strcpy(cmdbuf, cmd);

	while (*str) {
		/*
		 * Find separator, or string end
		 * Allow simple escape of ';' by writing "\;"
		 */
		for (inquotes = 0, sep = str; *sep; sep++) {
			if (!inquotes &&
			    (*sep == ';') &&	/* separator		*/
			    (sep != str) &&	/* past string start	*/
			    (*(sep - 1) != '\\'))	/* and NOT escaped */
				break;
		}

		/*
		 * Limit the token to data between separators
		 */
		token = str;
		if (*sep) {
			str = sep + 1;	/* start of command for next pass */
			*sep = '\0';
		} else {
			str = sep;	/* no more commands for next pass */
		}

		argc = LineToArgs(token, argv);
		if (argc == 0) {
			rc = -1;	/* no command at all */
			continue;
		}

		rc = CMD_RunEntry(argc, argv);
	}

	return rc;
}

#endif
