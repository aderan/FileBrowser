#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>
#include <errno.h>
#include <assert.h>

#include <stdlib.h>
#include <string.h>
#include <stdio.h>


#include "fb_file.h"
#include "fb_utils.h"

#define LOCAL_LOG_TAG "filebrowser_file"
#include "android_log.h"

extern int g_reset;

int fb_file_readdir(ReaddirParams *params)
{
	assert (params != NULL);

	int errno_tmp;
    enum ReadErr rderr = NO_ERR;
    FileItem *items = NULL;
    FileItem **item_move = &items;

	if (params->path == NULL){
		LOGW("fullpath should not be null\n");
		return NOT_EXIST;
	}

	DIR *curdir = opendir(params->path);
	if (curdir == NULL){
		errno_tmp = errno;
		LOGW("opendir error\n");
		return errno_tmp == EACCES ? PERMITTION_DENIED:
					errno_tmp == ENOENT ? NOT_EXIST:
						UNKNOWN_ERRPR;
	}

	struct dirent *dir_item;
    int items_count = 0;
	errno_tmp = errno;
	while((dir_item = readdir(curdir)) != NULL)
	{
        if (strcmp(dir_item->d_name,".") == 0
            || strcmp(dir_item->d_name,"..") == 0 )
            continue;
		*item_move = malloc (sizeof(FileItem));
		memset (*item_move, 0, sizeof(FileItem));
        {
            int len = strlen(params->path)+strlen(dir_item->d_name)+1;
            if (len > MAX_LENGTH_URL)
            {
                LOGW("path is too long");
                free (*item_move);
                *item_move = NULL;
                continue;
            }
            strcpy ((*item_move)->sourcetype, "file");
		    strcpy ((*item_move)->fullpath, params->path);
		    strcat ((*item_move)->fullpath, "/");
		    strcat ((*item_move)->fullpath, dir_item->d_name);
	   	    strncpy ((*item_move)->name, dir_item->d_name, MAX_LENGTH_NAME);
		}

		struct stat buf;
		if (stat ((*item_move)->fullpath, &buf) != 0){
			LOGW("stat file error %d\n", errno_tmp);
            free (*item_move);
            *item_move = NULL;
            continue;
		}
		(*item_move)->ctime = buf.st_ctime;
		(*item_move)->atime = buf.st_atime;
		(*item_move)->mtime = buf.st_mtime;
		if (dir_item->d_type != DT_DIR)
			(*item_move)->size  = buf.st_size;
		else {
            DIR *tmpdir = opendir ((*item_move)->fullpath);
            if (tmpdir == NULL){
                LOGW("opendir error when get dir_size!\n");
                free (*item_move);
                *item_move = NULL;
		        continue;
            }
            struct dirent *dirent_tmp;
            while ((dirent_tmp = readdir(tmpdir)) != NULL){
                if (strcmp(dirent_tmp->d_name,".") == 0
                    || strcmp(dirent_tmp->d_name,"..") == 0 )
                    continue;
                if (dirent_tmp->d_type == DT_DIR || dirent_tmp->d_type == DT_REG)
                    (*item_move)->size++ ;
            }
            (*item_move)->isdir = 1;
            if (closedir(tmpdir) != 0){
                LOGW("close error when get dir_size!\n");
            }
		}

        if (access ((*item_move)->fullpath, R_OK) == 0)
            (*item_move)->permission |= READ_OK;
        if (access ((*item_move)->fullpath, W_OK) == 0)
            (*item_move)->permission |= WRITE_OK;

		item_move = &(*item_move)->next;
        if (params->maxcount != 0 && ++items_count>= params->maxcount){
            LOGE("Callback for count to the max");
            char *json_info;
            json_info = fb_convert(items, items_count, 0, 0);
            params->cb(json_info, params->cb_arg);
            free(json_info);
            fb_free_items(items);
            items = NULL;
            item_move = &items;
            items_count = 0;
        }
        errno_tmp = errno;
	}
    if (errno_tmp != errno){
		LOGW("readdir error!\n");
	}

    if (closedir(curdir) != 0){
        LOGW("closedir error before ret!\n");
    }

    {
       LOGE("Callback for the end");
       char *json_info;
       json_info = fb_convert(items, items_count, 0, 1);
       params->cb(json_info, params->cb_arg);
       free(json_info);
       fb_free_items(items);
    }

    return NO_ERR;
}
