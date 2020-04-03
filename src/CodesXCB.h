/** Z80: portable Z80 emulator *******************************/
/**                                                         **/
/**                         CodesXCB.h                      **/
/**                                                         **/
/** This file contains implementation for FD/DD-CB tables   **/
/** of Z80 commands. It is included from Z80.c.             **/
/**                                                         **/
/** Copyright (C) Marat Fayzullin 1994,1995,1996            **/
/**     You are not allowed to distribute this software     **/
/**     commercially. Please, notify me, if you make any    **/
/**     changes to this file.                               **/
/*************************************************************/

case RLC_B:   I=M_RDMEM(J.W);M_RLC(I);R.BC.B.h = I; M_WRMEM(J.W,I); break;	//Undefined op code
case RLC_C:   I=M_RDMEM(J.W);M_RLC(I);R.BC.B.l = I; M_WRMEM(J.W,I); break;	//Undefined op code
case RLC_D:   I=M_RDMEM(J.W);M_RLC(I);R.DE.B.h = I; M_WRMEM(J.W,I); break;	//Undefined op code
case RLC_E:   I=M_RDMEM(J.W);M_RLC(I);R.DE.B.l = I; M_WRMEM(J.W,I); break;	//Undefined op code
case RLC_H:   I=M_RDMEM(J.W);M_RLC(I);R.HL.B.h = I; M_WRMEM(J.W,I); break;	//Undefined op code
case RLC_L:   I=M_RDMEM(J.W);M_RLC(I);R.HL.B.l = I; M_WRMEM(J.W,I); break;	//Undefined op code
case RLC_A:   I=M_RDMEM(J.W);M_RLC(I);R.AF.B.h = I; M_WRMEM(J.W,I); break;	//Undefined op code

case RRC_B:   I=M_RDMEM(J.W);M_RRC(I);R.BC.B.h = I; M_WRMEM(J.W,I); break;	//Undefined op code
case RRC_C:   I=M_RDMEM(J.W);M_RRC(I);R.BC.B.l = I; M_WRMEM(J.W,I); break;	//Undefined op code
case RRC_D:   I=M_RDMEM(J.W);M_RRC(I);R.DE.B.h = I; M_WRMEM(J.W,I); break;	//Undefined op code
case RRC_E:   I=M_RDMEM(J.W);M_RRC(I);R.DE.B.l = I; M_WRMEM(J.W,I); break;	//Undefined op code
case RRC_H:   I=M_RDMEM(J.W);M_RRC(I);R.HL.B.h = I; M_WRMEM(J.W,I); break;	//Undefined op code
case RRC_L:   I=M_RDMEM(J.W);M_RRC(I);R.HL.B.l = I; M_WRMEM(J.W,I); break;	//Undefined op code
case RRC_A:   I=M_RDMEM(J.W);M_RRC(I);R.AF.B.h = I; M_WRMEM(J.W,I); break;	//Undefined op code

case RL_B:    I=M_RDMEM(J.W);M_RL(I); R.BC.B.h = I; M_WRMEM(J.W,I); break;	//Undefined op code
case RL_C:    I=M_RDMEM(J.W);M_RL(I); R.BC.B.l = I; M_WRMEM(J.W,I); break;	//Undefined op code
case RL_D:    I=M_RDMEM(J.W);M_RL(I); R.DE.B.h = I; M_WRMEM(J.W,I); break;	//Undefined op code
case RL_E:    I=M_RDMEM(J.W);M_RL(I); R.DE.B.l = I; M_WRMEM(J.W,I); break;	//Undefined op code
case RL_H:    I=M_RDMEM(J.W);M_RL(I); R.HL.B.h = I; M_WRMEM(J.W,I); break;	//Undefined op code
case RL_L:    I=M_RDMEM(J.W);M_RL(I); R.HL.B.l = I; M_WRMEM(J.W,I); break;	//Undefined op code
case RL_A:    I=M_RDMEM(J.W);M_RL(I); R.AF.B.h = I; M_WRMEM(J.W,I); break;	//Undefined op code

case RR_B:    I=M_RDMEM(J.W);M_RR(I); R.BC.B.h = I; M_WRMEM(J.W,I); break;	//Undefined op code
case RR_C:    I=M_RDMEM(J.W);M_RR(I); R.BC.B.l = I; M_WRMEM(J.W,I); break;	//Undefined op code
case RR_D:    I=M_RDMEM(J.W);M_RR(I); R.DE.B.h = I; M_WRMEM(J.W,I); break;	//Undefined op code
case RR_E:    I=M_RDMEM(J.W);M_RR(I); R.DE.B.l = I; M_WRMEM(J.W,I); break;	//Undefined op code
case RR_H:    I=M_RDMEM(J.W);M_RR(I); R.HL.B.h = I; M_WRMEM(J.W,I); break;	//Undefined op code
case RR_L:    I=M_RDMEM(J.W);M_RR(I); R.HL.B.l = I; M_WRMEM(J.W,I); break;	//Undefined op code
case RR_A:    I=M_RDMEM(J.W);M_RR(I); R.AF.B.h = I; M_WRMEM(J.W,I); break;	//Undefined op code

case SLA_B:   I=M_RDMEM(J.W);M_SLA(I);R.BC.B.h = I; M_WRMEM(J.W,I); break;	//Undefined op code
case SLA_C:   I=M_RDMEM(J.W);M_SLA(I);R.BC.B.l = I; M_WRMEM(J.W,I); break;	//Undefined op code
case SLA_D:   I=M_RDMEM(J.W);M_SLA(I);R.DE.B.h = I; M_WRMEM(J.W,I); break;	//Undefined op code
case SLA_E:   I=M_RDMEM(J.W);M_SLA(I);R.DE.B.l = I; M_WRMEM(J.W,I); break;	//Undefined op code
case SLA_H:   I=M_RDMEM(J.W);M_SLA(I);R.HL.B.h = I; M_WRMEM(J.W,I); break;	//Undefined op code
case SLA_L:   I=M_RDMEM(J.W);M_SLA(I);R.HL.B.l = I; M_WRMEM(J.W,I); break;	//Undefined op code
case SLA_A:   I=M_RDMEM(J.W);M_SLA(I);R.AF.B.h = I; M_WRMEM(J.W,I); break;	//Undefined op code

case SRA_B:   I=M_RDMEM(J.W);M_SRA(I);R.BC.B.h = I; M_WRMEM(J.W,I); break;	//Undefined op code
case SRA_C:   I=M_RDMEM(J.W);M_SRA(I);R.BC.B.l = I; M_WRMEM(J.W,I); break;	//Undefined op code
case SRA_D:   I=M_RDMEM(J.W);M_SRA(I);R.DE.B.h = I; M_WRMEM(J.W,I); break;	//Undefined op code
case SRA_E:   I=M_RDMEM(J.W);M_SRA(I);R.DE.B.l = I; M_WRMEM(J.W,I); break;	//Undefined op code
case SRA_H:   I=M_RDMEM(J.W);M_SRA(I);R.HL.B.h = I; M_WRMEM(J.W,I); break;	//Undefined op code
case SRA_L:   I=M_RDMEM(J.W);M_SRA(I);R.HL.B.l = I; M_WRMEM(J.W,I); break;	//Undefined op code
case SRA_A:   I=M_RDMEM(J.W);M_SRA(I);R.AF.B.h = I; M_WRMEM(J.W,I); break;	//Undefined op code

case SRL_B:   I=M_RDMEM(J.W);M_SRL(I);R.BC.B.h = I; M_WRMEM(J.W,I); break;	//Undefined op code
case SRL_C:   I=M_RDMEM(J.W);M_SRL(I);R.BC.B.l = I; M_WRMEM(J.W,I); break;	//Undefined op code
case SRL_D:   I=M_RDMEM(J.W);M_SRL(I);R.DE.B.h = I; M_WRMEM(J.W,I); break;	//Undefined op code
case SRL_E:   I=M_RDMEM(J.W);M_SRL(I);R.DE.B.l = I; M_WRMEM(J.W,I); break;	//Undefined op code
case SRL_H:   I=M_RDMEM(J.W);M_SRL(I);R.HL.B.h = I; M_WRMEM(J.W,I); break;	//Undefined op code
case SRL_L:   I=M_RDMEM(J.W);M_SRL(I);R.HL.B.l = I; M_WRMEM(J.W,I); break;	//Undefined op code
case SRL_A:   I=M_RDMEM(J.W);M_SRL(I);R.AF.B.h = I; M_WRMEM(J.W,I); break;	//Undefined op code

case RES0_B:   I=M_RDMEM(J.W);M_RES(0,I);R.BC.B.h =I; M_WRMEM(J.W,I); break;	// Undefined op code
case RES0_C:   I=M_RDMEM(J.W);M_RES(0,I);R.BC.B.l =I; M_WRMEM(J.W,I); break;	// Undefined op code
case RES0_D:   I=M_RDMEM(J.W);M_RES(0,I);R.DE.B.h =I; M_WRMEM(J.W,I); break;	// Undefined op code
case RES0_E:   I=M_RDMEM(J.W);M_RES(0,I);R.DE.B.l =I; M_WRMEM(J.W,I); break;	// Undefined op code
case RES0_H:   I=M_RDMEM(J.W);M_RES(0,I);R.HL.B.h =I; M_WRMEM(J.W,I); break;	// Undefined op code
case RES0_L:   I=M_RDMEM(J.W);M_RES(0,I);R.HL.B.l =I; M_WRMEM(J.W,I); break;	// Undefined op code
case RES0_A:   I=M_RDMEM(J.W);M_RES(0,I);R.AF.B.h =I; M_WRMEM(J.W,I); break;	// Undefined op code

case RES1_B:   I=M_RDMEM(J.W);M_RES(1,I);R.BC.B.h =I; M_WRMEM(J.W,I); break;	// Undefined op code
case RES1_C:   I=M_RDMEM(J.W);M_RES(1,I);R.BC.B.l =I; M_WRMEM(J.W,I); break;	// Undefined op code
case RES1_D:   I=M_RDMEM(J.W);M_RES(1,I);R.DE.B.h =I; M_WRMEM(J.W,I); break;	// Undefined op code
case RES1_E:   I=M_RDMEM(J.W);M_RES(1,I);R.DE.B.l =I; M_WRMEM(J.W,I); break;	// Undefined op code
case RES1_H:   I=M_RDMEM(J.W);M_RES(1,I);R.HL.B.h =I; M_WRMEM(J.W,I); break;	// Undefined op code
case RES1_L:   I=M_RDMEM(J.W);M_RES(1,I);R.HL.B.l =I; M_WRMEM(J.W,I); break;	// Undefined op code
case RES1_A:   I=M_RDMEM(J.W);M_RES(1,I);R.AF.B.h =I; M_WRMEM(J.W,I); break;	// Undefined op code

case RES2_B:   I=M_RDMEM(J.W);M_RES(2,I);R.BC.B.h =I; M_WRMEM(J.W,I); break;	// Undefined op code
case RES2_C:   I=M_RDMEM(J.W);M_RES(2,I);R.BC.B.l =I; M_WRMEM(J.W,I); break;	// Undefined op code
case RES2_D:   I=M_RDMEM(J.W);M_RES(2,I);R.DE.B.h =I; M_WRMEM(J.W,I); break;	// Undefined op code
case RES2_E:   I=M_RDMEM(J.W);M_RES(2,I);R.DE.B.l =I; M_WRMEM(J.W,I); break;	// Undefined op code
case RES2_H:   I=M_RDMEM(J.W);M_RES(2,I);R.HL.B.h =I; M_WRMEM(J.W,I); break;	// Undefined op code
case RES2_L:   I=M_RDMEM(J.W);M_RES(2,I);R.HL.B.l =I; M_WRMEM(J.W,I); break;	// Undefined op code
case RES2_A:   I=M_RDMEM(J.W);M_RES(2,I);R.AF.B.h =I; M_WRMEM(J.W,I); break;	// Undefined op code

case RES3_B:   I=M_RDMEM(J.W);M_RES(3,I);R.BC.B.h =I; M_WRMEM(J.W,I); break;	// Undefined op code
case RES3_C:   I=M_RDMEM(J.W);M_RES(3,I);R.BC.B.l =I; M_WRMEM(J.W,I); break;	// Undefined op code
case RES3_D:   I=M_RDMEM(J.W);M_RES(3,I);R.DE.B.h =I; M_WRMEM(J.W,I); break;	// Undefined op code
case RES3_E:   I=M_RDMEM(J.W);M_RES(3,I);R.DE.B.l =I; M_WRMEM(J.W,I); break;	// Undefined op code
case RES3_H:   I=M_RDMEM(J.W);M_RES(3,I);R.HL.B.h =I; M_WRMEM(J.W,I); break;	// Undefined op code
case RES3_L:   I=M_RDMEM(J.W);M_RES(3,I);R.HL.B.l =I; M_WRMEM(J.W,I); break;	// Undefined op code
case RES3_A:   I=M_RDMEM(J.W);M_RES(3,I);R.AF.B.h =I; M_WRMEM(J.W,I); break;	// Undefined op code

case RES4_B:   I=M_RDMEM(J.W);M_RES(4,I);R.BC.B.h =I; M_WRMEM(J.W,I); break;	// Undefined op code
case RES4_C:   I=M_RDMEM(J.W);M_RES(4,I);R.BC.B.l =I; M_WRMEM(J.W,I); break;	// Undefined op code
case RES4_D:   I=M_RDMEM(J.W);M_RES(4,I);R.DE.B.h =I; M_WRMEM(J.W,I); break;	// Undefined op code
case RES4_E:   I=M_RDMEM(J.W);M_RES(4,I);R.DE.B.l =I; M_WRMEM(J.W,I); break;	// Undefined op code
case RES4_H:   I=M_RDMEM(J.W);M_RES(4,I);R.HL.B.h =I; M_WRMEM(J.W,I); break;	// Undefined op code
case RES4_L:   I=M_RDMEM(J.W);M_RES(4,I);R.HL.B.l =I; M_WRMEM(J.W,I); break;	// Undefined op code
case RES4_A:   I=M_RDMEM(J.W);M_RES(4,I);R.AF.B.h =I; M_WRMEM(J.W,I); break;	// Undefined op code

case RES5_B:   I=M_RDMEM(J.W);M_RES(5,I);R.BC.B.h =I; M_WRMEM(J.W,I); break;	// Undefined op code
case RES5_C:   I=M_RDMEM(J.W);M_RES(5,I);R.BC.B.l =I; M_WRMEM(J.W,I); break;	// Undefined op code
case RES5_D:   I=M_RDMEM(J.W);M_RES(5,I);R.DE.B.h =I; M_WRMEM(J.W,I); break;	// Undefined op code
case RES5_E:   I=M_RDMEM(J.W);M_RES(5,I);R.DE.B.l =I; M_WRMEM(J.W,I); break;	// Undefined op code
case RES5_H:   I=M_RDMEM(J.W);M_RES(5,I);R.HL.B.h =I; M_WRMEM(J.W,I); break;	// Undefined op code
case RES5_L:   I=M_RDMEM(J.W);M_RES(5,I);R.HL.B.l =I; M_WRMEM(J.W,I); break;	// Undefined op code
case RES5_A:   I=M_RDMEM(J.W);M_RES(5,I);R.AF.B.h =I; M_WRMEM(J.W,I); break;	// Undefined op code

case RES6_B:   I=M_RDMEM(J.W);M_RES(6,I);R.BC.B.h =I; M_WRMEM(J.W,I); break;	// Undefined op code
case RES6_C:   I=M_RDMEM(J.W);M_RES(6,I);R.BC.B.l =I; M_WRMEM(J.W,I); break;	// Undefined op code
case RES6_D:   I=M_RDMEM(J.W);M_RES(6,I);R.DE.B.h =I; M_WRMEM(J.W,I); break;	// Undefined op code
case RES6_E:   I=M_RDMEM(J.W);M_RES(6,I);R.DE.B.l =I; M_WRMEM(J.W,I); break;	// Undefined op code
case RES6_H:   I=M_RDMEM(J.W);M_RES(6,I);R.HL.B.h =I; M_WRMEM(J.W,I); break;	// Undefined op code
case RES6_L:   I=M_RDMEM(J.W);M_RES(6,I);R.HL.B.l =I; M_WRMEM(J.W,I); break;	// Undefined op code
case RES6_A:   I=M_RDMEM(J.W);M_RES(6,I);R.AF.B.h =I; M_WRMEM(J.W,I); break;	// Undefined op code

case RES7_B:   I=M_RDMEM(J.W);M_RES(7,I);R.BC.B.h =I; M_WRMEM(J.W,I); break;	// Undefined op code
case RES7_C:   I=M_RDMEM(J.W);M_RES(7,I);R.BC.B.l =I; M_WRMEM(J.W,I); break;	// Undefined op code
case RES7_D:   I=M_RDMEM(J.W);M_RES(7,I);R.DE.B.h =I; M_WRMEM(J.W,I); break;	// Undefined op code
case RES7_E:   I=M_RDMEM(J.W);M_RES(7,I);R.DE.B.l =I; M_WRMEM(J.W,I); break;	// Undefined op code
case RES7_H:   I=M_RDMEM(J.W);M_RES(7,I);R.HL.B.h =I; M_WRMEM(J.W,I); break;	// Undefined op code
case RES7_L:   I=M_RDMEM(J.W);M_RES(7,I);R.HL.B.l =I; M_WRMEM(J.W,I); break;	// Undefined op code
case RES7_A:   I=M_RDMEM(J.W);M_RES(7,I);R.AF.B.h =I; M_WRMEM(J.W,I); break;	// Undefined op code


case SET0_B:   I=M_RDMEM(J.W);M_SET(0,I);R.BC.B.h =I; M_WRMEM(J.W,I); break;	// Undefined op code
case SET0_C:   I=M_RDMEM(J.W);M_SET(0,I);R.BC.B.l =I; M_WRMEM(J.W,I); break;	// Undefined op code
case SET0_D:   I=M_RDMEM(J.W);M_SET(0,I);R.DE.B.h =I; M_WRMEM(J.W,I); break;	// Undefined op code
case SET0_E:   I=M_RDMEM(J.W);M_SET(0,I);R.DE.B.l =I; M_WRMEM(J.W,I); break;	// Undefined op code
case SET0_H:   I=M_RDMEM(J.W);M_SET(0,I);R.HL.B.h =I; M_WRMEM(J.W,I); break;	// Undefined op code
case SET0_L:   I=M_RDMEM(J.W);M_SET(0,I);R.HL.B.l =I; M_WRMEM(J.W,I); break;	// Undefined op code
case SET0_A:   I=M_RDMEM(J.W);M_SET(0,I);R.AF.B.h =I; M_WRMEM(J.W,I); break;	// Undefined op code

case SET1_B:   I=M_RDMEM(J.W);M_SET(1,I);R.BC.B.h =I; M_WRMEM(J.W,I); break;	// Undefined op code
case SET1_C:   I=M_RDMEM(J.W);M_SET(1,I);R.BC.B.l =I; M_WRMEM(J.W,I); break;	// Undefined op code
case SET1_D:   I=M_RDMEM(J.W);M_SET(1,I);R.DE.B.h =I; M_WRMEM(J.W,I); break;	// Undefined op code
case SET1_E:   I=M_RDMEM(J.W);M_SET(1,I);R.DE.B.l =I; M_WRMEM(J.W,I); break;	// Undefined op code
case SET1_H:   I=M_RDMEM(J.W);M_SET(1,I);R.HL.B.h =I; M_WRMEM(J.W,I); break;	// Undefined op code
case SET1_L:   I=M_RDMEM(J.W);M_SET(1,I);R.HL.B.l =I; M_WRMEM(J.W,I); break;	// Undefined op code
case SET1_A:   I=M_RDMEM(J.W);M_SET(1,I);R.AF.B.h =I; M_WRMEM(J.W,I); break;	// Undefined op code

case SET2_B:   I=M_RDMEM(J.W);M_SET(2,I);R.BC.B.h =I; M_WRMEM(J.W,I); break;	// Undefined op code
case SET2_C:   I=M_RDMEM(J.W);M_SET(2,I);R.BC.B.l =I; M_WRMEM(J.W,I); break;	// Undefined op code
case SET2_D:   I=M_RDMEM(J.W);M_SET(2,I);R.DE.B.h =I; M_WRMEM(J.W,I); break;	// Undefined op code
case SET2_E:   I=M_RDMEM(J.W);M_SET(2,I);R.DE.B.l =I; M_WRMEM(J.W,I); break;	// Undefined op code
case SET2_H:   I=M_RDMEM(J.W);M_SET(2,I);R.HL.B.h =I; M_WRMEM(J.W,I); break;	// Undefined op code
case SET2_L:   I=M_RDMEM(J.W);M_SET(2,I);R.HL.B.l =I; M_WRMEM(J.W,I); break;	// Undefined op code
case SET2_A:   I=M_RDMEM(J.W);M_SET(2,I);R.AF.B.h =I; M_WRMEM(J.W,I); break;	// Undefined op code

case SET3_B:   I=M_RDMEM(J.W);M_SET(3,I);R.BC.B.h =I; M_WRMEM(J.W,I); break;	// Undefined op code
case SET3_C:   I=M_RDMEM(J.W);M_SET(3,I);R.BC.B.l =I; M_WRMEM(J.W,I); break;	// Undefined op code
case SET3_D:   I=M_RDMEM(J.W);M_SET(3,I);R.DE.B.h =I; M_WRMEM(J.W,I); break;	// Undefined op code
case SET3_E:   I=M_RDMEM(J.W);M_SET(3,I);R.DE.B.l =I; M_WRMEM(J.W,I); break;	// Undefined op code
case SET3_H:   I=M_RDMEM(J.W);M_SET(3,I);R.HL.B.h =I; M_WRMEM(J.W,I); break;	// Undefined op code
case SET3_L:   I=M_RDMEM(J.W);M_SET(3,I);R.HL.B.l =I; M_WRMEM(J.W,I); break;	// Undefined op code
case SET3_A:   I=M_RDMEM(J.W);M_SET(3,I);R.AF.B.h =I; M_WRMEM(J.W,I); break;	// Undefined op code

case SET4_B:   I=M_RDMEM(J.W);M_SET(4,I);R.BC.B.h =I; M_WRMEM(J.W,I); break;	// Undefined op code
case SET4_C:   I=M_RDMEM(J.W);M_SET(4,I);R.BC.B.l =I; M_WRMEM(J.W,I); break;	// Undefined op code
case SET4_D:   I=M_RDMEM(J.W);M_SET(4,I);R.DE.B.h =I; M_WRMEM(J.W,I); break;	// Undefined op code
case SET4_E:   I=M_RDMEM(J.W);M_SET(4,I);R.DE.B.l =I; M_WRMEM(J.W,I); break;	// Undefined op code
case SET4_H:   I=M_RDMEM(J.W);M_SET(4,I);R.HL.B.h =I; M_WRMEM(J.W,I); break;	// Undefined op code
case SET4_L:   I=M_RDMEM(J.W);M_SET(4,I);R.HL.B.l =I; M_WRMEM(J.W,I); break;	// Undefined op code
case SET4_A:   I=M_RDMEM(J.W);M_SET(4,I);R.AF.B.h =I; M_WRMEM(J.W,I); break;	// Undefined op code

case SET5_B:   I=M_RDMEM(J.W);M_SET(5,I);R.BC.B.h =I; M_WRMEM(J.W,I); break;	// Undefined op code
case SET5_C:   I=M_RDMEM(J.W);M_SET(5,I);R.BC.B.l =I; M_WRMEM(J.W,I); break;	// Undefined op code
case SET5_D:   I=M_RDMEM(J.W);M_SET(5,I);R.DE.B.h =I; M_WRMEM(J.W,I); break;	// Undefined op code
case SET5_E:   I=M_RDMEM(J.W);M_SET(5,I);R.DE.B.l =I; M_WRMEM(J.W,I); break;	// Undefined op code
case SET5_H:   I=M_RDMEM(J.W);M_SET(5,I);R.HL.B.h =I; M_WRMEM(J.W,I); break;	// Undefined op code
case SET5_L:   I=M_RDMEM(J.W);M_SET(5,I);R.HL.B.l =I; M_WRMEM(J.W,I); break;	// Undefined op code
case SET5_A:   I=M_RDMEM(J.W);M_SET(5,I);R.AF.B.h =I; M_WRMEM(J.W,I); break;	// Undefined op code

case SET6_B:   I=M_RDMEM(J.W);M_SET(6,I);R.BC.B.h =I; M_WRMEM(J.W,I); break;	// Undefined op code
case SET6_C:   I=M_RDMEM(J.W);M_SET(6,I);R.BC.B.l =I; M_WRMEM(J.W,I); break;	// Undefined op code
case SET6_D:   I=M_RDMEM(J.W);M_SET(6,I);R.DE.B.h =I; M_WRMEM(J.W,I); break;	// Undefined op code
case SET6_E:   I=M_RDMEM(J.W);M_SET(6,I);R.DE.B.l =I; M_WRMEM(J.W,I); break;	// Undefined op code
case SET6_H:   I=M_RDMEM(J.W);M_SET(6,I);R.HL.B.h =I; M_WRMEM(J.W,I); break;	// Undefined op code
case SET6_L:   I=M_RDMEM(J.W);M_SET(6,I);R.HL.B.l =I; M_WRMEM(J.W,I); break;	// Undefined op code
case SET6_A:   I=M_RDMEM(J.W);M_SET(6,I);R.AF.B.h =I; M_WRMEM(J.W,I); break;	// Undefined op code

case SET7_B:   I=M_RDMEM(J.W);M_SET(7,I);R.BC.B.h =I; M_WRMEM(J.W,I); break;	// Undefined op code
case SET7_C:   I=M_RDMEM(J.W);M_SET(7,I);R.BC.B.l =I; M_WRMEM(J.W,I); break;	// Undefined op code
case SET7_D:   I=M_RDMEM(J.W);M_SET(7,I);R.DE.B.h =I; M_WRMEM(J.W,I); break;	// Undefined op code
case SET7_E:   I=M_RDMEM(J.W);M_SET(7,I);R.DE.B.l =I; M_WRMEM(J.W,I); break;	// Undefined op code
case SET7_H:   I=M_RDMEM(J.W);M_SET(7,I);R.HL.B.h =I; M_WRMEM(J.W,I); break;	// Undefined op code
case SET7_L:   I=M_RDMEM(J.W);M_SET(7,I);R.HL.B.l =I; M_WRMEM(J.W,I); break;	// Undefined op code
case SET7_A:   I=M_RDMEM(J.W);M_SET(7,I);R.AF.B.h =I; M_WRMEM(J.W,I); break;	// Undefined op code



case RLC_xHL: I=M_RDMEM(J.W);M_RLC(I);M_WRMEM(J.W,I);break;
case RRC_xHL: I=M_RDMEM(J.W);M_RRC(I);M_WRMEM(J.W,I);break;
case RL_xHL:  I=M_RDMEM(J.W);M_RL(I);M_WRMEM(J.W,I);break;
case RR_xHL:  I=M_RDMEM(J.W);M_RR(I);M_WRMEM(J.W,I);break;
case SLA_xHL: I=M_RDMEM(J.W);M_SLA(I);M_WRMEM(J.W,I);break;
case SRA_xHL: I=M_RDMEM(J.W);M_SRA(I);M_WRMEM(J.W,I);break;
case SLL_xHL: I=M_RDMEM(J.W);M_SLL(I);M_WRMEM(J.W,I);break;
case SRL_xHL: I=M_RDMEM(J.W);M_SRL(I);M_WRMEM(J.W,I);break;

case BIT0_B: case BIT0_C: case BIT0_D: case BIT0_E:
case BIT0_H: case BIT0_L: case BIT0_A:
case BIT0_xHL: I=M_RDMEM(J.W);M_BIT(0,I);break;
case BIT1_B: case BIT1_C: case BIT1_D: case BIT1_E:
case BIT1_H: case BIT1_L: case BIT1_A:
case BIT1_xHL: I=M_RDMEM(J.W);M_BIT(1,I);break;
case BIT2_B: case BIT2_C: case BIT2_D: case BIT2_E:
case BIT2_H: case BIT2_L: case BIT2_A:
case BIT2_xHL: I=M_RDMEM(J.W);M_BIT(2,I);break;
case BIT3_B: case BIT3_C: case BIT3_D: case BIT3_E:
case BIT3_H: case BIT3_L: case BIT3_A:
case BIT3_xHL: I=M_RDMEM(J.W);M_BIT(3,I);break;
case BIT4_B: case BIT4_C: case BIT4_D: case BIT4_E:
case BIT4_H: case BIT4_L: case BIT4_A:
case BIT4_xHL: I=M_RDMEM(J.W);M_BIT(4,I);break;
case BIT5_B: case BIT5_C: case BIT5_D: case BIT5_E:
case BIT5_H: case BIT5_L: case BIT5_A:
case BIT5_xHL: I=M_RDMEM(J.W);M_BIT(5,I);break;
case BIT6_B: case BIT6_C: case BIT6_D: case BIT6_E:
case BIT6_H: case BIT6_L: case BIT6_A:
case BIT6_xHL: I=M_RDMEM(J.W);M_BIT(6,I);break;
case BIT7_B: case BIT7_C: case BIT7_D: case BIT7_E:
case BIT7_H: case BIT7_L: case BIT7_A:
case BIT7_xHL: I=M_RDMEM(J.W);M_BIT(7,I);break;




case RES0_xHL: I=M_RDMEM(J.W);M_RES(0,I);M_WRMEM(J.W,I);break;
case RES1_xHL: I=M_RDMEM(J.W);M_RES(1,I);M_WRMEM(J.W,I);break;   
case RES2_xHL: I=M_RDMEM(J.W);M_RES(2,I);M_WRMEM(J.W,I);break;   
case RES3_xHL: I=M_RDMEM(J.W);M_RES(3,I);M_WRMEM(J.W,I);break;   
case RES4_xHL: I=M_RDMEM(J.W);M_RES(4,I);M_WRMEM(J.W,I);break;   
case RES5_xHL: I=M_RDMEM(J.W);M_RES(5,I);M_WRMEM(J.W,I);break;   
case RES6_xHL: I=M_RDMEM(J.W);M_RES(6,I);M_WRMEM(J.W,I);break;   
case RES7_xHL: I=M_RDMEM(J.W);M_RES(7,I);M_WRMEM(J.W,I);break;   

case SET0_xHL: I=M_RDMEM(J.W);M_SET(0,I);M_WRMEM(J.W,I);break;   
case SET1_xHL: I=M_RDMEM(J.W);M_SET(1,I);M_WRMEM(J.W,I);break; 
case SET2_xHL: I=M_RDMEM(J.W);M_SET(2,I);M_WRMEM(J.W,I);break; 
case SET3_xHL: I=M_RDMEM(J.W);M_SET(3,I);M_WRMEM(J.W,I);break; 
case SET4_xHL: I=M_RDMEM(J.W);M_SET(4,I);M_WRMEM(J.W,I);break; 
case SET5_xHL: I=M_RDMEM(J.W);M_SET(5,I);M_WRMEM(J.W,I);break; 
case SET6_xHL: I=M_RDMEM(J.W);M_SET(6,I);M_WRMEM(J.W,I);break; 
case SET7_xHL: I=M_RDMEM(J.W);M_SET(7,I);M_WRMEM(J.W,I);break; 
