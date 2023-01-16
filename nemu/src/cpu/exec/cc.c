#include "cpu/rtl.h"

/* Condition Code */

void rtl_setcc(rtlreg_t* dest, uint8_t subcode) {
  bool invert = subcode & 0x1;
  enum {
    CC_O, CC_NO, CC_B,  CC_NB,
    CC_E, CC_NE, CC_BE, CC_NBE,
    CC_S, CC_NS, CC_P,  CC_NP,
    CC_L, CC_NL, CC_LE, CC_NLE
  };

  // TODO: Query EFLAGS to determine whether the condition code is satisfied.
  // dest <- ( cc is satisfied ? 1 : 0)
  switch (subcode & 0xe) {
    case CC_O:
		{
				//printf("setcc: CC_O\n");
				rtl_get_OF(dest);
				break;
		}
    case CC_B:
		{
				//printf("setcc: CC_B\n");
				rtl_get_CF(dest);
				break;
		}
    case CC_E:
		{
				rtl_get_ZF(dest);
				//printf("dest: %08x\n", *dest);
				break;
		}
    case CC_BE:
		{
				//printf("setcc: CC_BE\n");
				rtl_get_CF(&t0);
				rtl_get_ZF(&t1);
				rtl_or(dest, &t0, &t1);
				//operand_write(dest, &t0);
				break;
		}
    case CC_S:
		{
				//printf("setcc: CC_S\n");
				rtl_get_SF(dest);
				break;
		}
    case CC_L:
		{
				//printf("setcc: CC_L\n");
				rtl_get_OF(&t0);
				rtl_get_SF(&t1);
				rtl_xor(dest, &t0, &t1);
				//operand_write(dest, &t0);
				break;
		}
    case CC_LE:
		{
				//printf("setcc: CC_LE\n");
				rtl_get_OF(&t0);
				rtl_get_SF(&t1);
				rtl_xor(&t0, &t0, &t1);//SF != OF
				rtl_get_ZF(&t1);
				rtl_or(dest, &t0, &t1);
				//operand_write(dest, &t0);
				break;
		}
    default: panic("should not reach here");
    case CC_P: panic("n86 does not have PF");
  }

  if (invert) {
    rtl_xori(dest, dest, 0x1);
    //printf("dest(after): %08x\n", *dest);
  }
}
