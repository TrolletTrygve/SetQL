#ifndef __DBMS_NETWORKING_H__
#define __DBMS_NETWORKING_H__

#include <stdint.h>

// Function declarations
int dbms_networking_initialize(uint16_t port);
int dbms_start(void);
int dbms_networking_kill(void);

#endif