// The log extension for JOS
// Author: Li Mingyang & Wu Kaidong @ EECS.PKU
#ifndef JOS_INC_LOG_H
#define JOS_INC_LOG_H

#include <inc/fs.h>

// Basic logging parameters
#define LOGBLKS	64
#define LOGSIZE	(LOGBLKS * BLKSIZE)

// Log header information in lg_info
#define LOG_V		0x01	// valid ?
#define LOG_S		0x02	// logs submit to disk logs
#define LOG_C		0x04	// logs commit to disk blocks
#define LOG_P		0x08	// priority of writing back
#define LOG_A		0x10	// log is active in writing

#define LOG_R		0x03	// recovered logs ?

struct {
	struct Log_block_head {
		int lg_dst_blkaddr;
		int lg_info;
		int lg_sbmt_t;
		int lg_wrt_t;
	} log_head[LOGBLKS];
	int wb_time;

	char _pad[PGSIZE - sizeof(log_head)];
} Log;

#endif /* !JOS_INC_LOG_H */