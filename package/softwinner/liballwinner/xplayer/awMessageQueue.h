
#ifndef MESSAGE_QUEUE_H
#define MESSAGE_QUEUE_H

#include <semaphore.h>
#include <time.h>

typedef void* AwMessageQueue;

typedef struct AwMessage
{
    int          messageId;
    unsigned int params[8];
}AwMessage;

AwMessageQueue* AwMessageQueueCreate(int nMaxMessageNum);

void AwMessageQueueDestroy(AwMessageQueue* mq);

int AwMessageQueuePostMessage(AwMessageQueue* mq, AwMessage* m);

int AwMessageQueueGetMessage(AwMessageQueue* mq, AwMessage* m);

int AwMessageQueueTryGetMessage(AwMessageQueue* mq, AwMessage* m, int64_t timeout);

int AwMessageQueueFlush(AwMessageQueue* mq);

int AwMessageQueueGetCount(AwMessageQueue* mq);

//* define a semaphore timedwait method for common use.
int SemTimedWait(sem_t* sem, int64_t time_ms);

void setMessage(AwMessage* msg,
                int        cmd,
                int        param0 = 0,
                int        param1 = 0,
                int        param2 = 0,
                int        param3 = 0,
                int        param4 = 0,
                int        param5 = 0,
                int        param6 = 0,
                int        param7 = 0);

#endif

