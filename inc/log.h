// The log extension for JOS
// Author: Li Mingyang & Wu Kaidong @ EECS.PKU
#ifndef JOS_INC_LOG_H
#define JOS_INC_LOG_H

#include <inc/fs.h>

// Basic logging parameters
#define LOGBLKS	65				// log region blocks
#define LOGSIZE	(LOGBLKS * BLKSIZE)		// log region size
#define LOGCONTSIZE 	512				// log content size
#define BLKCONTS	(BLKSIZE / LOGCONTSIZE)	// number of log contents in a block
#define LOGNUM	(BLKCONTS * LOGBLKS)		// numbers of log headers/contents in log region

// Log header information in lh_status
#define LOG_D		0x1000000		// logs dirty, not commit to disk
#define LOG_HC		0x2000000		// logs have content

#define LOG_OFF(x)	(x & 0xfff)		// use this to get the offset
#define LOG_SIZE(x)	((x & 0xfff000) >> 12)	// use this to get the size

struct log_header
{
	void* lh_blkaddr;
	int lh_status;
};

struct log_content
{
	char lc_log[LOGCONTSIZE];
};

#endif /* !JOS_INC_LOG_H */