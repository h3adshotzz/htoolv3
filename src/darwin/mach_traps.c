//===----------------------------------------------------------------------===//
//
//                         === The HTool Project ===
//
//  This  document  is the property of "Is This On?" It is considered to be
//  confidential and proprietary and may not be, in any form, reproduced or
//  transmitted, in whole or in part, without express permission of Is This
//  On?.
//
//  Copyright (C) 2023, Harry Moulton - Is This On? Holdings Ltd
//
//  Harry Moulton <me@h3adsh0tzz.com>
//
//===----------------------------------------------------------------------===//

#include "darwin/mach_traps.h"
#include "commands/macho.h"

#define MACH_TRAP_TABLE_COUNT               128

/**
 *  Table of mach traps publicly released as open-source by Apple, taken from
 *  the xnu/osfmk/kern/syscall_sw.c file. Not all the mach traps are here, as
 *  not all of the kernel is opensource. 
 *  
 */
const char * mach_syscall_name_table[MACH_TRAP_TABLE_COUNT] = {
/* 0 */		"kern_invalid",
/* 1 */		"kern_invalid",
/* 2 */		"kern_invalid",
/* 3 */		"kern_invalid",
/* 4 */		"kern_invalid",
/* 5 */		"kern_invalid",
/* 6 */		"kern_invalid",
/* 7 */		"kern_invalid",
/* 8 */		"kern_invalid",
/* 9 */		"kern_invalid",
/* 10 */	"_kernelrpc_mach_vm_allocate_trap",
/* 11 */	"kern_invalid",
/* 12 */	"_kernelrpc_mach_vm_deallocate_trap",
/* 13 */	"kern_invalid",
/* 14 */	"_kernelrpc_mach_vm_protect_trap",
/* 15 */	"_kernelrpc_mach_vm_map_trap",
/* 16 */	"_kernelrpc_mach_port_allocate_trap",
/* 17 */	"_kernelrpc_mach_port_destroy_trap",
/* 18 */	"_kernelrpc_mach_port_deallocate_trap",
/* 19 */	"_kernelrpc_mach_port_mod_refs_trap",
/* 20 */	"_kernelrpc_mach_port_move_member_trap",
/* 21 */	"_kernelrpc_mach_port_insert_right_trap",
/* 22 */	"_kernelrpc_mach_port_insert_member_trap",
/* 23 */	"_kernelrpc_mach_port_extract_member_trap",
/* 24 */	"_kernelrpc_mach_port_construct_trap",
/* 25 */	"_kernelrpc_mach_port_destruct_trap",
/* 26 */	"mach_reply_port",
/* 27 */	"thread_self_trap",
/* 28 */	"task_self_trap",
/* 29 */	"host_self_trap",
/* 30 */	"kern_invalid",
/* 31 */	"mach_msg_trap",
/* 32 */	"mach_msg_overwrite_trap",
/* 33 */	"semaphore_signal_trap",
/* 34 */	"semaphore_signal_all_trap",
/* 35 */	"semaphore_signal_thread_trap",
/* 36 */	"semaphore_wait_trap",
/* 37 */	"semaphore_wait_signal_trap",
/* 38 */	"semaphore_timedwait_trap",
/* 39 */	"semaphore_timedwait_signal_trap",
/* 40 */	"kern_invalid",
/* 41 */	"_kernelrpc_mach_port_guard_trap",
/* 42 */	"_kernelrpc_mach_port_unguard_trap",
/* 43 */	"mach_generate_activity_id",
/* 44 */	"task_name_for_pid",
/* 45 */ 	"task_for_pid",
/* 46 */	"pid_for_task",
/* 47 */	"kern_invalid",
/* 48 */	"macx_swapon",
/* 49 */	"macx_swapoff",
/* 50 */	"kern_invalid",
/* 51 */	"macx_triggers",
/* 52 */	"macx_backing_store_suspend",
/* 53 */	"macx_backing_store_recovery",
/* 54 */	"kern_invalid",
/* 55 */	"kern_invalid",
/* 56 */	"kern_invalid",
/* 57 */	"kern_invalid",
/* 58 */	"pfz_exit",
/* 59 */ 	"swtch_pri",
/* 60 */	"swtch",
/* 61 */	"thread_switch",
/* 62 */	"clock_sleep_trap",
/* 63 */	"kern_invalid",
/* traps 64 - 95 reserved (debo) */
/* 64 */	"kern_invalid",
/* 65 */	"kern_invalid",
/* 66 */	"kern_invalid",
/* 67 */	"kern_invalid",
/* 68 */	"kern_invalid",
/* 69 */	"kern_invalid",
/* 70 */	"host_create_mach_voucher_trap",
/* 71 */	"kern_invalid",
/* 72 */	"mach_voucher_extract_attr_recipe_trap",
/* 73 */	"kern_invalid",
/* 74 */	"kern_invalid",
/* 75 */	"kern_invalid",
/* 76 */	"kern_invalid",
/* 77 */	"kern_invalid",
/* 78 */	"kern_invalid",
/* 79 */	"kern_invalid",
/* 80 */	"kern_invalid",
/* 81 */	"kern_invalid",
/* 82 */	"kern_invalid",
/* 83 */	"kern_invalid",
/* 84 */	"kern_invalid",
/* 85 */	"kern_invalid",
/* 86 */	"kern_invalid",
/* 87 */	"kern_invalid",
/* 88 */	"kern_invalid",
/* 89 */	"mach_timebase_info_trap",
/* 90 */	"mach_wait_until_trap",
/* 91 */	"mk_timer_create_trap",
/* 92 */	"mk_timer_destroy_trap",
/* 93 */	"mk_timer_arm_trap",
/* 94 */	"mk_timer_cancel_trap",
/* 95 */	"kern_invalid",
/* traps 64 - 95 reserved (debo) */
/* 96 */	"kern_invalid",
/* 97 */	"kern_invalid",
/* 98 */	"kern_invalid",
/* 99 */	"kern_invalid",
/* traps 100-107 reserved for iokit (esb) */ 
/* 100 */	"iokit_user_client_trap",
/* 101 */	"kern_invalid",
/* 102 */	"kern_invalid",
/* 103 */	"kern_invalid",
/* 104 */	"kern_invalid",
/* 105 */	"kern_invalid",
/* 106 */	"kern_invalid",
/* 107 */	"kern_invalid",
/* traps 108-127 unused */			
/* 108 */	"kern_invalid",
/* 109 */	"kern_invalid",
/* 110 */	"kern_invalid",
/* 111 */	"kern_invalid",
/* 112 */	"kern_invalid",
/* 113 */	"kern_invalid",
/* 114 */	"kern_invalid",
/* 115 */	"kern_invalid",
/* 116 */	"kern_invalid",
/* 117 */	"kern_invalid",
/* 118 */	"kern_invalid",
/* 119 */	"kern_invalid",
/* 120 */	"kern_invalid",
/* 121 */	"kern_invalid",
/* 122 */	"kern_invalid",
/* 123 */	"kern_invalid",
/* 124 */	"kern_invalid",
/* 125 */	"kern_invalid",
/* 126 */	"kern_invalid",
/* 127 */	"kern_invalid",
};


void
_dump_mach_traps2 (char *mach)
{
    int thumb = 0;
    uint64_t kernInvalid = *((uint64_t *) mach);

    if (mach) printf ("Kern invalid should be %llx. Ignoring those\n", kernInvalid);;
    for (int i = 0; i < 128; i++) {
        uint64_t addr = *((int64_t *) (mach + 4*8*i));

        if (addr == *((int *) (mach + 4))) continue;
        if ((addr % 4) == 1 || (addr % 4) == -3) { addr--; thumb++; }
        if (addr % 4) { thumb = "-1"; }

        if (addr && (addr != kernInvalid)) {
            printf ("%3d %-40s 0x%llx %s\n", i, mach_syscall_name_table[i], addr, (thumb? "T": "-"));
        } else {
            if (addr) printf ("%3d %-40s 0x%llx %s\n", i, "__unknown_mach_trap__", addr, (thumb? "T": "-"));
        }
    }
}

typedef struct mach_trap_test {
    unsigned char a;
    unsigned char b;
    unsigned char c;
    unsigned char d;
    htool_return_t ptr;
    const char *trap_name;
};

void _dump_mach_traps (char *table_base)
{
    uint64_t kern_invalid_trap = *((uint64_t *) table_base);
    int thumb = 0;

    /* Check that the kern_invalid trap is at the base of the table */
    if (kern_invalid_trap)
        printf ("kern_invalid is at 0x%llx. Ignoring these\n", kern_invalid_trap);


    /**
     *  Loop through the mach_trap_table ... 
     */
    for (int i = 0; i < 128; i++) {
        
        /* Get the address of the next trap in the table */
        uint64_t addr = *((int64_t *) (table_base + 4*8*i));
        

        /* Checking the address is valid */
        if (addr == *((int *) (table_base + 4))) continue;
        if ((addr % 4) == 1 || (addr % 4) == -3) { addr--; thumb++; }
        if (addr % 4) { thumb = "-1"; }

        if (addr && (addr != kern_invalid_trap)) {

            if (!strcmp (mach_syscall_name_table[i], "kern_invalid"))
                printf ("NOTE: Found actual trap\n");

            printf ("%3d %-40s 0x%llx\n", i, mach_syscall_name_table[i], addr);
        }

        //if (addr && (addr != kern_invalid_trap))
          //  printf ("kern_invalid: %llx, addr: %llx: %s\n", kern_invalid_trap, addr, mach_syscall_name_table[i]);
    }
}

void
_mach_trap_debug (xnu_t *xnu)
{
    macho_t *macho;
    if (xnu->flags & HTOOL_XNU_FLAG_FILESET_ENTRY) macho = xnu->kern;
    else macho = xnu->macho;
    char *zeros = calloc (24, 1);

    /* Find either __DATA.__const or __CONST */
    mach_section_64_t *sect64 = calloc (1, sizeof (mach_section_64_t));
    //mach_section_64_t *sect64 = mach_section_64_search (macho->scmds, "__DATA", "__const");
    //if (!sect64) sect64 = mach_section_64_search (macho->scmds, "__CONST", "__constdata");
    //if (!sect64) sect64 = mach_section_64_search (macho->scmds, "__DATA_CONST", "__const");
    
    htool_return_t find_sect = macho_search_section (macho, &sect64, "__DATA", "__const");
    if (find_sect == HTOOL_RETURN_FAILURE) macho_search_section (macho, &sect64, "__CONST", "__constdata");
    if (find_sect == HTOOL_RETURN_FAILURE) macho_search_section (macho, &sect64, "__DATA_CONST", "__const");

    if (!sect64) {
        errorf ("No __DATA.__const, __CONST.__constdata, or __DATA_CONST.__const\n");
        return;
    }

    uint32_t offset = sect64->offset;
    int adv = 8;
    char *mach_data = NULL;
    char *pos = macho->data + offset;

    uint64_t sect_addr = sect64->addr;
    uint32_t sect_offset = sect64->offset;
    int sect_size = sect64->size;
    int skip = 3;

    for (int i = 0; i < sect_size; i += adv) {
        if (( ((memcmp(&pos[i], zeros, skip*adv) == 0) &&
	(memcmp(&pos[i+(skip+1)*adv], zeros, skip*adv) == 0) &&
	(memcmp(&pos[i+2*(skip+1)*adv], zeros, skip*adv) == 0) &&
	(memcmp(&pos[i+3*(skip+1)*adv], zeros, skip*adv) == 0) &&
	(memcmp(&pos[i+4*(skip+1)*adv], zeros, skip *adv) == 0) &&
       (  (*((uint64_t *) &pos[i-adv])) &&  *((int64_t *) &pos[i+skip*adv]))))
      ) {
        printf ("mach_trap_table offset in file/memory: 0x%x/0x%llx\n", sect_offset + i , sect_addr + i);
        mach_data = &pos[i] - adv;
        _dump_mach_traps (mach_data);
        break;
      }
    }
}

htool_return_t
mach_parse_trap_table (xnu_t *xnu)
{
    htool_return_t ret;
    macho_t *macho = xnu_select_macho (xnu);
    char *zeros = calloc (24, 1);

    /**
     *  The mach_traps_table is contained either in __DATA.__const, __CONST.__constdata
     *  or __DATA_CONST.__const. Find whichever is valid.
     */
    mach_section_64_t *sect64 = calloc (1, sizeof (mach_section_64_t));
    ret = macho_search_section (macho, &sect64, "__DATA", "__const");
    if (ret == HTOOL_RETURN_FAILURE) macho_search_section (macho, &sect64, "__CONST", "__constdata");
    if (ret == HTOOL_RETURN_FAILURE) macho_search_section (macho, &sect64, "__DATA_CONST", "__const");

    if (!sect64) {
        errorf ("No __DATA.__const, __CONST.__constdata, or __DATA_CONST.__const\n");
        return HTOOL_RETURN_FAILURE;
    }
    debugf ("mach_traps_table section: %s.%s\n", sect64->segname, sect64->sectname);

    uint32_t offset = sect64->offset;
    int adv = 8;
    char *mach_data = NULL;
    char *pos = macho->data + offset;

    uint64_t sect_addr = sect64->addr;
    uint32_t sect_offset = sect64->offset;
    int sect_size = sect64->size;
    int skip = 3;


    for (int i = 0; i < sect64->size; i += adv) {

        // check that 5 memory chunks are zero.
        if (memcmp (&pos[i], zeros, 3 * adv) == 0)

    }

    
    return HTOOL_RETURN_SUCCESS;
}