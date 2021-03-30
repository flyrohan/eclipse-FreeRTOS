#ifndef _CLI_H_
#define _CLI_H_

void CLI_RunLoop(void);
int CLI_RunLoopThread(uint32_t stacksize, int priority);

#endif
