.origin 0
.entrypoint USONIC_DETECT

#include "hm_pru.hp"

USONIC_DETECT:

    // Enable OCP master port
    LBCO      r0, CONST_PRUCFG, 4, 4
    CLR       r0, r0, 4         // Clear SYSCFG[STANDBY_INIT] to enable OCP master port
    SBCO      r0, CONST_PRUCFG, 4, 4

    // Configure the programmable pointer register for PRU0 by setting c28_pointer[15:0]
    // field to 0x0100.  This will make C28 point to 0x00010000 (PRU shared RAM).
    MOV       r0, 0x00000100
    MOV       r1, CTBIR_0
    SBBO      r0, r1, 0, 4

    // Configure the programmable pointer register for PRU0 by setting c31_pointer[15:0]
    // field to 0x0010.  This will make C31 point to 0x80001000 (DDR memory).
    MOV       r0, 0x00100000
    MOV       r1, CTPPR_1
    SBBO      r0, r1, 0, 4

    // Enable Cycle Counter
    MOV       r1, CONTROL_0
    LBBO      r0, r1, 0, 4
    OR        r0, r0, 0xa
    SBBO      r0, r1, 0, 4

    //set ARM such that PRU can write to GPIO
    LBCO      r0, C4, 4, 4
    CLR       r0, r0, 4
    SBCO      r0, C4, 4, 4

USONIC_RUN:

    USONIC_PROC USONIC1_TRIGGER_BIT, USONIC1_ECHO_BIT, 1, 3
    USONIC_PROC USONIC2_TRIGGER_BIT, USONIC2_ECHO_BIT, 2, 1
    USONIC_PROC USONIC3_TRIGGER_BIT, USONIC3_ECHO_BIT, 3, 2
    USONIC_PROC USONIC4_TRIGGER_BIT, USONIC4_ECHO_BIT, 4, 0

    JMP USONIC_RUN

    HALT

