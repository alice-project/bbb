.origin 0
.entrypoint USONIC_DETECT

#include "hm_pru.hp"

USONIC_DETECT:

    // Configure the block index register for PRU0 by setting c24_blk_index[7:0] and
    // c25_blk_index[7:0] field to 0x00 and 0x00, respectively.  This will make C24 point
    // to 0x00000000 (PRU0 DRAM) and C25 point to 0x00002000 (PRU1 DRAM).
    MOV       r0, 0x00000000
    MOV       r1, CTBIR_0
    SBBO      r0, r1, 0, 4

    //set ARM such that PRU can write to GPIO
    LBCO r0, C4, 4, 4
    CLR  r0, r0, 4
    SBCO r0, C4, 4, 4

USONIC_RUN:
    USONIC_PROC USONIC1_TRIGGER_BIT, USONIC1_ECHO_BIT, 1
    USONIC_PROC USONIC2_TRIGGER_BIT, USONIC2_ECHO_BIT, 2
    USONIC_PROC USONIC3_TRIGGER_BIT, USONIC3_ECHO_BIT, 3
    USONIC_PROC USONIC4_TRIGGER_BIT, USONIC4_ECHO_BIT, 4

    // Send notification to Host for program completion
    MOV R31.b0, PRU0_ARM_INTERRUPT+16

    JMP USONIC_RUN

    HALT
