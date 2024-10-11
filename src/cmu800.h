/*
name is cmu800.h

*/

#include "types.h"

void CMU800_init(void);
void CMU800_DoOut(byte Port, byte Value);
void CMU800_DoIn(byte Port,byte *Value);
void CMU800_setTempo(int _tempo);
int cnv(word Value, int* octave, int* pitchNo);
