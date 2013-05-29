#include <stdio.h>
#include <stdlib.h>
#include <setjmp.h>

#define MAXTHREAD 4
#define STACKDIR - // set to + for upwards and - for downwards
#define STACKSIZE (1<<18)

struct threadStruct {
    jmp_buf stackBuf;
    char *stackPtr;
};

typedef struct threadStruct *coThread;

static char *tos; // top of stack
static void *coarg;
static struct threadStruct coThreads[MAXTHREAD];
static int coNumThreads = 0;

static inline coThread coThreadAlloc(void) { return coThreads + coNumThreads++; }

void *coSwitchToThread(coThread currentThread, coThread newThread) {
    if (setjmp(currentThread->stackBuf)) {
        return(coarg);
    }
    longjmp(newThread->stackBuf, 1);
}

void *coNewThread(coThread currentThread, void (*fun)(coThread)) {
    coThread newThread = coThreadAlloc();

    if (tos == NULL) {
        tos = (char*)&newThread;
    }
    tos += STACKDIR STACKSIZE;
    char n[STACKDIR (tos - (char*)&newThread)];
    coarg = n; // ensure optimizer keeps n
    if (setjmp(currentThread->stackBuf)) {
        return(coarg);
    }
    fun(newThread);
    abort();
}

static void comain(coThread thread) {
    int i = thread - coThreads;

    for (;;) {
        printf("coroutine %d\n", i, i);
        int n = rand() % coNumThreads;
        printf("jumping to %d\n", n);
        coSwitchToThread(coThreads + i, coThreads + n);
    }
}

/*
int main(void) {
    coThread mainThread = coThreadAlloc();
    int i;

    for(i = 0; i < MAXTHREAD - 1; i++) {
        printf("spawning %d\n", i);
        coStartThread(mainThread, comain);
    }
    return 0;
}
*/

static char text[100]

static void produceText(coThread mainThread)
{
    static int counter = 1;

    while(true) {
        sprintf(text, "Count = %d", counter++);
        coSwitchToThread(mainThread);
    }
}

static void consumeText(coThread mainthread)
{
    while(true) {
        printf("%s\n", text);
        coSwitchToThread(mainThread);
    }
}

int main(void) {
    coThread mainThread = coThreadAlloc();
    coThread producer = coNewThread(mainThread, produceText);
    coThread consumer = coNewThread(mainThread, consumeText);

    while(true) {
        coSwitchToThread(producer);
        coSwitchToThread(consumer);
    }
}
