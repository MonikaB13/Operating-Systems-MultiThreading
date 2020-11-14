#ifndef MULTI_LOOKUP_H
#define MULTI_LOOKUP_H

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <pthread.h>
#include <stdbool.h>
#include "util.h"
#include <semaphore.h>
#include <sys/time.h>

void* ReadDomainNames(void* args);
void* WriteIPAddr();

#endif
