#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <assert.h>
#include <dlfcn.h>

#include "libsmbclient.h"

#include "fb_utils.h"
#include "fb_smb.h"

#define WORKGROUP "WORKGROUP"
static char g_username[MAX_LENGTH_NAME];
static char g_password[MAX_LENGTH_PASSWD];
static SMBCCTX *g_curctx;
static smbc_get_auth_data_fn auth_fn;
static char g_curserver[MAX_LENGTH_URL] = {'\0'};

extern int g_reset;

#define LOCAL_LOG_TAG "filebrowser_smb"
#include "android_log.h"

void with_auth_fn(const char *server, const char *share, char *workgroup, int wgmaxlen,
		char *username, int unmaxlen, char *password, int pwmaxlen)
{
	strncpy(workgroup, WORKGROUP, wgmaxlen - 1);
	strncpy(username, g_username, unmaxlen - 1);
	strncpy(password, g_password, pwmaxlen - 1);
}

void without_auth_fn(const char *server, const char *share, char *workgroup, int wgmaxlen,
		char *username, int unmaxlen, char *password, int pwmaxlen)
{
}

void fb_smb_readdir_init(const char *path)
{

    smbc_init(without_auth_fn, 0);

    int serlen = strlen(g_curserver);
    if ((serlen == 0) || strncmp(g_curserver, path, serlen)){
        LOGE("Change CXT, Init again\n");
        if (g_curctx)
            smbc_free_context(g_curctx, 1);
        // setup our context
        SMBCCTX *l_ctx = smbc_new_context();
#ifdef DEPRECATED_SMBC_INTERFACE
        smbc_setDebug(l_ctx, 0);
        smbc_setFunctionAuthData(l_ctx, auth_fn);
        smbc_setOptionOneSharePerServer(l_ctx, 0);
        smbc_setOptionBrowseMaxLmbCount(l_ctx, 0);
        smbc_setTimeout(l_ctx, 10 * 1000);
#else
        l_ctx->debug = 0;
        l_ctx->callbacks.auth_fn = auth_fn;
        l_ctx->options.one_share_per_server = 0;
        l_ctx->options.browse_max_lmb_count = 0;
        l_ctx->timeout = 10 * 1000;
#endif
        // initialize samba and do some hacking into the settings
        if (smbc_init_context(l_ctx))
        {
            smbc_set_context(l_ctx);
            g_curctx = l_ctx;
            strncpy (g_curserver, path, MAX_LENGTH_URL);
        }
        else
        {
            smbc_free_context(l_ctx, 1);
            //g_curctx = NULL;
        }
    }
}

int fb_smb_readdir(ReaddirParams *params)
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

    if (params->username && params->passwd){
        auth_fn = with_auth_fn;
        strncpy(g_username, params->username, MAX_LENGTH_NAME);
        strncpy(g_password, params->passwd, MAX_LENGTH_PASSWD);
    }
    else
        auth_fn = without_auth_fn;

    fb_smb_readdir_init(params->path);

	int curdir = smbc_opendir(params->path);
	if (curdir < 0){
		errno_tmp = errno;
		LOGW("opendir error\n");
		return errno_tmp == EACCES ? PERMITTION_DENIED:
					errno_tmp == ENOENT ? NOT_EXIST:
						UNKNOWN_ERRPR;
	}

	struct smbc_dirent *dir_item;
    int items_count = 0;
	errno_tmp = errno;
    //NEED CS LOCK
	while((dir_item = smbc_readdir(curdir)) != NULL && !g_reset)
	{
        if (strcmp(dir_item->name,".") == 0
            || strcmp(dir_item->name,"..") == 0 )
            continue;
		*item_move = malloc (sizeof(FileItem));
		memset (*item_move, 0, sizeof(FileItem));
        {
            int len = strlen(params->path)+ dir_item->namelen + 1;
            if (len > MAX_LENGTH_URL)
            {
                LOGW("path is too long");
                free (*item_move);
                *item_move = NULL;
                continue;
            }
            strcpy ((*item_move)->sourcetype, "smb");
		    strcpy ((*item_move)->fullpath, params->path);
		    strcat ((*item_move)->fullpath, "/");
		    strcat ((*item_move)->fullpath, dir_item->name);
	   	    strncpy ((*item_move)->name, dir_item->name, MAX_LENGTH_NAME);
		}

		struct stat buf;
		if (smbc_stat ((*item_move)->fullpath, &buf) != 0){
			LOGW("stat file error %d\n", errno_tmp);
            free (*item_move);
            *item_move = NULL;
            continue;
		}
        LOGE("time is %d , %d , %d\n",buf.st_ctime, buf.st_atime, buf.st_mtime);
		(*item_move)->ctime = buf.st_ctime;
		(*item_move)->atime = buf.st_atime;
		(*item_move)->mtime = buf.st_mtime;
		if (dir_item->smbc_type != SMBC_DIR)
			(*item_move)->size  = buf.st_size;
		else {
            int tmpdir = smbc_opendir ((*item_move)->fullpath);
            if (tmpdir < 0){
                LOGW("opendir error when get dir_size!\n");
                free (*item_move);
                *item_move = NULL;
		        continue;
            }
            struct smbc_dirent *dirent_tmp;
            while ((dirent_tmp = smbc_readdir(tmpdir)) != NULL){
                if (strcmp(dirent_tmp->name,".") == 0
                    || strcmp(dirent_tmp->name,"..") == 0 )
                    continue;
                if (dirent_tmp->smbc_type == SMBC_DIR || dirent_tmp->smbc_type == SMBC_FILE)
                    (*item_move)->size++ ;
            }
            (*item_move)->isdir = 1;
            if (smbc_closedir(tmpdir) != 0){
                LOGW("close error when get dir_size!\n");
            }
		}

        char value[20];
        if (smbc_getxattr((*item_move)->fullpath, "system.dos_attr.mode", value, sizeof(value)) > 0)
        {
            long longvalue = strtol(value, NULL, 16);
            LOGE("mode is %ld\n", longvalue);

            if (longvalue & SMBC_DOS_MODE_READONLY){
                (*item_move)->permission = READ_OK;
            }
            else
                (*item_move)->permission = 0x6;
        }

		item_move = &(*item_move)->next;
        if (params->maxcount != 0 && ++items_count>= params->maxcount)
        {
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

    if (smbc_closedir(curdir) != 0){
        LOGW("closedir error before ret!\n");
    }

    if (!g_reset){
       LOGE("Callback for the end");

       char *json_info;
       json_info = fb_convert(items, items_count, 0, 1);
       LOGE("cb_arg 3 is %d %p\n",*(int*)params->cb_arg,params->cb);
       params->cb(json_info, params->cb_arg);
       free(json_info);
       fb_free_items(items);
    }

    return NO_ERR;
}

void fb_smb_reset()
{
    printf ("Reset Context!\n");
    smbc_free_context(g_curctx, 1);
    g_curctx = NULL;
    g_curserver[0] = '\0';
    printf ("Reset Context END!\n");
}


int fb_smb_findservice(ServiceParams *params)
{

}

