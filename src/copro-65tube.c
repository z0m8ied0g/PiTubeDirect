/*
 * 6502 Co Processor Emulation
 *
 * (c) 2016 David Banks and Ed Spittles
 * 
 * based on code by
 * - Reuben Scratton.
 * - Tom Walker
 *
 */

#include <stdio.h>
#include <string.h>
#include "tube-defs.h"
#include "tube.h"
#include "tube-ula.h"
#include "tuberom_6502.h"
#include "programs.h"
#include "copro-65tube.h"

#ifdef HISTOGRAM

unsigned int histogram_memory[0x100];

void copro_65tube_init_histogram() {
  int i;
  for (i = 0; i < 256; i++) {
    histogram_memory[i] = 0;
  }
}

void copro_65tube_dump_histogram() {
  int i;
  for (i = 0; i < 256; i++) {
    printf("%02x %u\r\n", i, histogram_memory[i]);
  }
}

#endif

static void copro_65tube_poweron_reset(unsigned char mpu_memory[]) {
   // Wipe memory
   memset(mpu_memory, 0, 0x10000);
   // Install test programs (like sphere)
   copy_test_programs(mpu_memory);
}

static void copro_65tube_reset(unsigned char mpu_memory[]) {
   // Re-instate the Tube ROM on reset
   memcpy(mpu_memory + 0xf800, tuberom_6502_orig, 0x800);
   // Wait for rst become inactive before continuing to execute
   tube_wait_for_rst_release();
}

void copro_65tube_emulator() {
   // Remember the current copro so we can exit if it changes
   int last_copro = copro;
   unsigned char *addr;
   unsigned char mpu_memory[128*1024]; // allocate 2x the amount of ram
   
   addr = &mpu_memory[0];
   
   addr += 64*1024; // move half way into ram
   
   addr -= ((unsigned int)(&mpu_memory[0]) % (64*1024)); // round down to 64K boundary
   
   copro_65tube_poweron_reset(addr);
   copro_65tube_reset(addr);

   while (copro == last_copro) {
#ifdef HISTOGRAM
      copro_65tube_init_histogram();
#endif
      tube_reset_performance_counters();
      exec_65tube(addr, copro == COPRO_65TUBE_1 ? 1 : 0);
      tube_log_performance_counters();
#ifdef HISTOGRAM
      copro_65tube_dump_histogram();
#endif
      copro_65tube_reset(addr);
   }
}

