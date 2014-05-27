#ifndef _FB_SAMBA_H_
#define _FB_SAMBA_H_

#include "fb_utils.h"

#ifdef __cplusplus
extern "C" {
#endif

int fb_smb_readdir(ReaddirParams *params);
int fb_smb_findservice(ServiceParams *params);

#ifdef __cplusplus
}
#endif
#endif

