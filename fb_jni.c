#include <stdlib.h>
#include <string.h>
#include <pthread.h>

#include "fb_jni.h"
#include "fb_file.h"
#include "fb_utils.h"

#define LOCAL_LOG_TAG "fb_jni"
#include "android_log.h"

static const KFile kfiles[] = {
    {"file", fb_file_readdir},

    {NULL, NULL},
};

int g_reset;

static jclass jcls;
static jobject jobj;

static void *_readdir_thread(void *arg)
{
    LOGE("inter _readdir_thread\n");
    ReaddirParams *params = malloc (sizeof (ReaddirParams));
    params->cb_arg = malloc (sizeof (jint));
    memcpy (params, arg, sizeof (ReaddirParams));
    params->cb_arg = ((ReaddirParams *)arg)->cb_arg;

    const KFile *kfile;
    for (kfile = &kfiles[0];kfile->filetype != NULL;kfile++){
        if (strncmp(kfile->filetype,params->path,strlen(kfile->filetype)) == 0){
            kfile->readdir(params);
            break;
        }
    }
    free (params->cb_arg);
    free (params);
}

static int _readdir_callback(const char *json_info,void *arg)
{
    LOGE("_readdir_callback:%s %d",json_info,*(int*)arg);
}

/*
 * Class:     com_targetv_ott_FileBrowserService
 * Method:    _init
 * Signature: ()V
 */
JNIEXPORT void JNICALL Java_com_targetv_ott_FileBrowserService__1init
  (JNIEnv *env, jobject obj)
{
    jclass lcls = (*env)->FindClass(env,"com/targetv/ott/FileBrowserService");
    jcls = (*env)->NewGlobalRef(env, lcls);
    jobj = (*env)->NewGlobalRef(env, obj);
}

/*
 * Class:     com_targetv_ott_FileBrowserService
 * Method:    _uninit
 * Signature: ()V
 */
JNIEXPORT void JNICALL Java_com_targetv_ott_FileBrowserService__1uninit
  (JNIEnv *env, jobject obj)
{
    //TOKNOW: this env is not the same with that in init;
    (*env)->DeleteGlobalRef(env, jcls);
    (*env)->DeleteGlobalRef(env, jcls);
}


/*
 * Class:     com_targetv_ott_FileBrowserService
 * Method:    _readdir
 * Signature: (Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;I)V
 */
JNIEXPORT void JNICALL Java_com_targetv_ott_FileBrowserService__1readdir
  (JNIEnv *env, jobject obj, jstring path, jstring username, jstring passwd, jint maxcount, jint callid)
{
    ReaddirParams params;
    const char* lpath = (*env)->GetStringUTFChars(env, path, 0);
    memcpy (params.path, lpath, sizeof(params.path));
    (*env)->ReleaseStringUTFChars(env, path, lpath);

    const char* lusername = (*env)->GetStringUTFChars(env, username, 0);
    memcpy (params.username, lusername, sizeof(params.username));
    (*env)->ReleaseStringUTFChars(env, username, lusername);

    const char* lpasswd = (*env)->GetStringUTFChars(env, passwd, 0);
    memcpy (params.passwd, lpasswd, sizeof(params.passwd));
    (*env)->ReleaseStringUTFChars(env, passwd, lpasswd);

    params.maxcount = maxcount;
    params.cb = _readdir_callback;

    params.cb_arg = malloc (sizeof (jint));
    *(int*)params.cb_arg = callid;

    pthread_attr_t attr;
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
    pthread_create(NULL, NULL, _readdir_thread, &params);

    free (params.cb_arg);
}

/*
 * Class:     com_targetv_ott_FileBrowserService
 * Method:    _findService
 * Signature: (Ljava/lang/String;I)V
 */
JNIEXPORT void JNICALL Java_com_targetv_ott_FileBrowserService__1findService
  (JNIEnv *env, jobject obj, jstring type, jint callid){

}

