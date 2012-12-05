#include <stdio.h>

#include "z80.h"
#include "z80_op_priv.h"


/*
LD A, s
Operation: A ← s
Condition Bits Affected:
S is set if Register is negative; reset otherwise
Z is set if Register is zero; reset otherwise
H is reset
P/V contains contents of IFF2
N is reset
C is not affected
T States:
LD A, I     - 9 (4, 5)
LD A, R     - 9 (4, 5)
*/
#define Z80_LD_A_IR(c,s) Z80_OP2(c, ld, A, s) \
    {   \
        SET_A( GET_ ##s ); \
        SET_F( (GET_F & ((~FLAG_MASK) | FLAG_C)) | \
               (z80_general_flags[GET_A] & ~FLAG_PV) | \
               (cpu->iff2 ? FLAG_PV : 0) ); \
    }
Z80_LD_A_IR( ED57, I );
Z80_LD_A_IR( ED5F, R );

/*
LD d, s
Operation: s ← d
Condition Bits Affected: None
T States:
LD r, r'    - 4
LD r, n     - 7 (4, 3)
LD r, (HL)  - 7 (4, 3)
LD (HL), r  - 7 (4, 3)
LD (HL), n  - 10 (4, 3, 3)
LD A, (rp)  - 7 (4, 3)
LD A, (nn)  - 13 (4, 3, 3, 3)
LD (rp), A  - 7 (4, 3)
LD (nn), A  - 13 (4, 3, 3, 3)
LD I, A     - 9 (4, 5)
LD R, A     - 9 (4, 5)
LD dd, nn   - 10 (4, 3, 3)
*/
#define Z80_LD(c,d,s) Z80_OP2(c, ld, d, s) \
    {   \
        int tmp = GET_ ##s; \
        SET_ ##d ( tmp ); \
    }
Z80_LD( ED47, I, A );
Z80_LD( ED4F, R, A );


#define OP(c) [0x ##c] = { z80_op_ED ##c, &z80_op_dis_ED ##c }
struct z80_op z80_op_ed_cb[256] =
    {
        OP(47),
        OP(4F),
        OP(57),
        OP(5F)
    };
