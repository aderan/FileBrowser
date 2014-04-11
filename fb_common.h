#ifndef _FB_COMMON_H_
#define _FB_COMMON_H_

#include <stdint.h>

	enum ReadErr{
	    NO_ERR,
		UNKNOWN_ERRPR = 1,
		PERMITTION_DENIED,
		NOT_EXIST,
		TIME_OUT,
	};

    #define READ_OK 0x4;
    #define WRITE_OK 0x2;

    #define MAX_LENGTH_URL 4096
    #define MAX_LENGTH_NAME 256
    #define MAX_NUM_THUMBNAILS 4
    #define MAX_NUM_URIS 4

	struct Thumbnail {
		int width;
		int height;
		char uri[4096];
	};

	struct _FileItem {
		struct _FileItem *next;
		char sourcetype[64];
		unsigned char permission;
        unsigned char isdir;
		uint64_t size;
		unsigned long ctime;
		unsigned long mtime;
		unsigned long atime;
		char name[256];
		char fullpath[4096];
        int uris_num;
        char *uris[4];
        int thumbnails_num;
		struct Thumbnail *thumbnails[4];
	};
	typedef struct _FileItem FileItem;

	char *fb_convert(FileItem **items);
    void fb_free_items(FileItem *items);

#endif
