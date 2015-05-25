#include <inc/log.h>
#include <inc/lib.h>

// Logs layout in memory
// The log blocks lie in the end of disk.
#define LOGLIM	(DISKMAP + DISKSIZE)
#define LOGMAP	(LOGLIM - LOGSIZE)