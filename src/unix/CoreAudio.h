#ifndef __COREAUDIO_
#define __COREAUDIO_

#include "../sysdep/sysdep_dsp.h"
#include "../sysdep/sysdep_dsp_priv.h"
#include "../sysdep/plugin_manager.h"

void coreaudio_dsp_destroy(struct sysdep_dsp_struct *dsp);
int coreaudio_dsp_write(struct sysdep_dsp_struct *dsp, unsigned char *data, int count);
void* coreaudio_dsp_create(struct sysdep_dsp_create_params *params);

extern const struct plugin_struct sysdep_dsp_coreaudio;


#endif
