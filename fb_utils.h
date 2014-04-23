#ifndef _FB_UTILS_H_
#define _FB_UTILS_H_

#ifdef __cplusplus
    extern "C" {
#endif


#include <stdint.h>
#include <sys/types.h>
#include <pthread.h>

	enum ReadErr{
	    NO_ERR,
		UNKNOWN_ERRPR = 1,
		PERMITTION_DENIED,
		NOT_EXIST,
		TIME_OUT,
	};

    #define READ_OK 0x4;
    #define WRITE_OK 0x2;

    #define MAX_LENGTH_TYPE 64
    #define MAX_LENGTH_PASSWD 64
    #define MAX_LENGTH_URL 4096
    #define MAX_LENGTH_NAME 256
    #define MAX_NUM_THUMBNAILS 4
    #define MAX_NUM_URIS 4

    #define MAX_THREAD_NUM 10

	struct Thumbnail {
		int width;
		int height;
		char uri[MAX_LENGTH_URL];
	};

	typedef struct _FileItem {
		struct _FileItem *next;
		char sourcetype[MAX_LENGTH_TYPE];
		unsigned char permission;
        unsigned char isdir;
		uint64_t size;
		size_t ctime;
		size_t mtime;
		size_t atime;
		char name[MAX_LENGTH_NAME];
		char fullpath[MAX_LENGTH_URL];
        int uris_num;
        char *uris[MAX_NUM_URIS];
        int thumbnails_num;
		struct Thumbnail *thumbnails[MAX_NUM_THUMBNAILS];
	} FileItem;

    /*
       **Json_info: {items:[] count: err: over:}
       **arg: uni-id for callback,
       */
    typedef int (*FuncUpdir)(const char *json_info, void *arg);
    typedef struct _ReaddirParams{
        char path[MAX_LENGTH_URL];
        int maxcount;
        char username[MAX_LENGTH_NAME];
        char passwd[MAX_LENGTH_PASSWD];
        FuncUpdir cb;
        void *cb_arg;
    }ReaddirParams;

    enum ThreadType{
        TYPE_READDIR,
        TYPE_FINDSERVER,
    };

    typedef struct _ThreadHandle{
        enum ThreadType type;
        void * content;
        pthread_t pid;
        int finished;
        int joined;
    }ThreadHandle;

    typedef int (*FuncReaddir)(ReaddirParams *params);
    typedef struct _KFile{
        const char *filetype;
        FuncReaddir readdir;
    }KFile;

	char *fb_convert(FileItem *items, int num_items, int err, int over);
    void fb_free_items(FileItem *items);

#ifdef __cplusplus
}
#endif

#endif

