/* The log extension for JOS
     Author: Li Mingyang & Wu Kaidong @ EECS.PKU */

#include "fs.h"

#define debug 0

// --------------------------------------------------------------
// Log Initialization
// --------------------------------------------------------------

// Initialize the log system.
void
log_init(void)
{
	struct log_header lh;
	struct log_content lc;

	// Read all the log blocks from disk
	memmove(&lh, (void *)LOGMAP, 1);

	int i;
	for (i = LOGMAP + BLKSIZE; i < LOGLIM; i += BLKSIZE){
		memmove(&lc, (void *)i, 1);
	}

	// Map the log_header and log_content
	Log_header = (struct log_header*)LOGMAP;
	Log_content = (struct log_content*)(LOGMAP + BLKSIZE);

	// Recover from log
	recover_from_log();
}

// --------------------------------------------------------------
// Log operations
// --------------------------------------------------------------

// Write a log record into memory log blocks.
// There are two kinds of log item: Commit Item & Write Item.
// NULL logdata indicates a Commit Item.
void
log_record(uint32_t blockno, int offset, char *logdata, int size)
{
	void *addr = diskaddr(blockno);
	if (log_pointer == LOGNUM){
		log_submit();
		log_commit();
	} // If there is no free log item, commit all logs to disk
	Log_header[log_pointer].lh_blkaddr = addr;
	if (logdata){ // Write Item
		Log_header[log_pointer].lh_status = offset;
		Log_header[log_pointer].lh_status |= (size << 12);
		Log_header[log_pointer].lh_status |= LOG_HC;
		Log_header[log_pointer].lh_status |= LOG_D;
		memmove(Log_content[log_pointer].lc_log, logdata, size);
	}
	else { // Commit Item
		Log_header[log_pointer].lh_status = 0;
		int i;
		for (i = 0; i < log_pointer; i ++){
			if (Log_header[i].lh_blkaddr == addr){
				Log_header[i].lh_status &= ~LOG_D;

				// Check the log system works
				if (debug)
					print_log(i, "clear");

			}
		} // Clear all logs with the same block number.
	}

	// Check the log system works
	if (debug > 1)
		print_log(log_pointer, "record");


	log_pointer ++;
}

// Submit the log blocks to disk.
// Operate the whole log region.
void
log_submit(void)
{
	int i;
	for (i = LOGMAP; i < LOGLIM; i += BLKSIZE){
		flush_block((void *)i);
	}
}

// Commit the whole log and clear it.
// Operate reversely to minimize the flush_block times.
void
log_commit(void)
{
	int i, j;
	for (i = LOGNUM; i >= 0; i --){
		if (Log_header[i].lh_status & LOG_D){
			for (j = 0; j < i; j ++){
				if (Log_header[j].lh_blkaddr == Log_header[i].lh_blkaddr)
					Log_header[j].lh_status &= ~LOG_D;
			} // Modifications to the same block only need to be flushed once
			Log_header[i].lh_status &= ~LOG_D;

			// Check the log system works
			if (debug)
				print_log(i, "clear");

			flush_block(Log_header[i].lh_blkaddr);
		}
	}
	// Clear all bog items by set the pointer to 0
	log_pointer = 0;
}

// Recover according to disk logs.
// Install all the modifications in order.
void
recover_from_log(void)
{
	int i, offset, size;
	for (i = 0; i < LOGNUM; i++){
		if (Log_header[i].lh_status & LOG_D){
			offset = LOG_OFF(Log_header[i].lh_status);
			size = LOG_SIZE(Log_header[i].lh_status);
			memmove(Log_header[i].lh_blkaddr + offset, Log_content[i].lc_log, size);
			Log_header[i].lh_status &= ~LOG_D;

			// Check the log system works
			if (debug)
				print_log(i, "recover");
			
			flush_block(Log_header[i].lh_blkaddr);
			cprintf("Address 0x%x is recovered from log.\n", Log_header[i].lh_blkaddr);
		}
	}
	log_submit();
	log_pointer = 0;
}

// --------------------------------------------------------------
// Log Checking
// --------------------------------------------------------------

// Check the log region.
void
check_log(void)
{
	int i;
	for (i = 0; i < LOGBLKS; i ++){
		assert(!block_is_free(addr2blockno((void *)LOGMAP + i * BLKSIZE)));
	}
	cprintf("log region is good\n");
}

// Print the log
void
print_log(int logno, char *info) {
	cprintf("log-%s: {\n\tlog no: %d,\n\tblock no: %d,\n\tstatus: %08x", info, logno, ((unsigned int)Log_header[logno].lh_blkaddr - DISKMAP) / BLKSIZE, Log_header[logno].lh_status);
	if (Log_header[logno].lh_status & LOG_HC)
		cprintf(",\n\tcontent: {\n%512.512s\n}", Log_content[logno].lc_log);
	cprintf("\n}\n");
}