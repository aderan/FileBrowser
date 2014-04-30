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

static ThreadHandle *thread_handle;
static pthread_mutex_t thread_mutex;
static pthread_mutex_t cb_mutex;


int g_reset;

static JavaVM* jvm;
static jclass jcls;
static jobject jobj;

static jmethodID get_string;
static jmethodID get_int;


static ThreadHandle *clean_threads()
{
    LOGE("Clean Threads\n");
    ThreadHandle *last_vaild = NULL ;
    int i;
    for (i=0;i<MAX_THREAD_NUM;i++)
        if (thread_handle[i].finished)
        {
            last_vaild = &thread_handle[i];
            pthread_join(thread_handle[i].pid ,NULL);
            LOGE("Thread Joined\n");
            thread_handle[i].joined = 1;
            thread_handle[i].finished = 0;
            thread_handle[i].pid = 0;
        }
    return last_vaild;
}

static ThreadHandle *get_thread()
{
    int i;
    for (i=0; i<MAX_THREAD_NUM; ++i){
        LOGE("Get Thread %d\n",i);
        //FIXME : For Better Choose;
        if (!thread_handle[i].pid)
            return &thread_handle[i];
    }

    return clean_threads();
}


static void *_readdir_thread(void *arg)
{
    LOGE("inter _readdir_thread\n");
    ThreadHandle *handle = arg;
    ReaddirParams *params = handle->content;

    LOGE("cb_arg 2 is %d\n",*(jint*)params->cb_arg);
    const KFile *kfile;
    for (kfile = &kfiles[0];kfile->filetype != NULL;kfile++){
        if (strncmp(kfile->filetype,params->path,strlen(kfile->filetype)) == 0){
            kfile->readdir(params);
            break;
        }
    }
    handle->finished = 1;
    free(params->cb_arg);
    free(handle->content);
}

static JNIEnv* JNI_GetEnv(int *attached)
{
	JNIEnv *env = NULL;
	if ((*jvm)->GetEnv(jvm,(void**)&env, JNI_VERSION_1_4) != JNI_OK) {
		LOGE("need attach");
		if ((*jvm)->AttachCurrentThread(jvm, &env, NULL) != JNI_OK)
			LOGE("Attach Current Thread failed!");
		else
			*attached = 1;
	}
	return env;
}

static int JNI_ReleaseEnv(int *attached)
{
	if (*attached) {
		if ((*jvm)->DetachCurrentThread(jvm) != JNI_OK) {
			LOGE("Detach Current Thread failed!");
		}
		*attached = 0;
	}
}


static int _readdir_callback(const char *json_info,void *arg)
{
    LOGE("_readdir_callback:%s %d",json_info,*(int*)arg);
    int attached;
    //Add Mutex
    JNIEnv *env = JNI_GetEnv(&attached);
    pthread_mutex_lock (&cb_mutex);
	jmethodID methodID = (*env)->GetMethodID(env, jcls, "callback", "(Ljava/lang/String;I)V");
	(*env)->CallVoidMethod(env, jobj, methodID, (*env)->NewStringUTF(env, json_info), *(int*)arg);
    pthread_mutex_unlock (&cb_mutex);
	JNI_ReleaseEnv(&attached);
}

/*----------------------------------------------------------------------
|    JNI_OnLoad
+---------------------------------------------------------------------*/
JNIEXPORT jint JNICALL
JNI_OnLoad(JavaVM* vm, void* reserved)
{
    LOGE("Jni_OnLoad Called");
    jvm = vm;
    return JNI_VERSION_1_4;
}

/*
 * Class:     com_targetv_fs_FileBrowserService
 * Method:    _init
 * Signature: ()V
 */
JNIEXPORT void JNICALL Java_com_targetv_fs_FileBrowserService__1init
  (JNIEnv *env, jobject obj)
{
    LOGE("FBService Init Called");
    jclass lcls = (*env)->FindClass(env,"com/targetv/fs/FileBrowserService");
    jcls = (*env)->NewGlobalRef(env, lcls);
    jobj = (*env)->NewGlobalRef(env, obj);

    jclass jbundle = (*env)->FindClass(env, "android/os/Bundle");
    get_string = (*env)->GetMethodID(env, jbundle, "getString",
        "(Ljava/lang/String;)Ljava/lang/String;");
    get_int = (*env)->GetMethodID(env, jbundle, "getInt",
        "(Ljava/lang/String;)I");

    thread_handle = malloc (sizeof (ThreadHandle) * MAX_THREAD_NUM);
    memset (thread_handle, 0, sizeof (ThreadHandle) * MAX_THREAD_NUM);
    pthread_mutex_init(&thread_mutex, NULL);
    pthread_mutex_init(&cb_mutex, NULL);
}


/*
 * Class:     com_targetv_fs_FileBrowserService
 * Method:    _uninit
 * Signature: ()V
 */
JNIEXPORT void JNICALL Java_com_targetv_fs_FileBrowserService__1uninit
  (JNIEnv *env, jobject obj)
{
    //TOKNOW: this env is not the same with that in init;
    (*env)->DeleteGlobalRef(env, jcls);
    (*env)->DeleteGlobalRef(env, jcls);

    free(thread_handle);
    pthread_mutex_destroy(&thread_mutex);
    pthread_mutex_destroy(&cb_mutex);
}

//bundle tihuan

/*
 * Class:     com_targetv_fs_FileBrowserService
 * Method:    _readdir
 * Signature: (Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;I)V
 */
JNIEXPORT void JNICALL Java_com_targetv_fs_FileBrowserService__1readdir
  (JNIEnv *env, jobject obj, jstring path, jstring username, jstring passwd, jint maxcount, jint callid)
{
    ReaddirParams *params;
    params = malloc (sizeof(ReaddirParams));

    const char* lpath = (*env)->GetStringUTFChars(env, path, 0);
    memcpy (params->path, lpath, sizeof(params->path));
    (*env)->ReleaseStringUTFChars(env, path, lpath);
    const char* lusername = (*env)->GetStringUTFChars(env, username, 0);
    memcpy (params->username, lusername, sizeof(params->username));
    (*env)->ReleaseStringUTFChars(env, username, lusername);
    const char* lpasswd = (*env)->GetStringUTFChars(env, passwd, 0);
    memcpy (params->passwd, lpasswd, sizeof(params->passwd));
    (*env)->ReleaseStringUTFChars(env, passwd, lpasswd);

    params->maxcount = maxcount;
    params->cb = _readdir_callback;
    params->cb_arg = malloc (sizeof (jint));
    *(jint*)params->cb_arg = callid;

    LOGE("cb_arg is %d\n",*(jint*)params->cb_arg);

    pthread_mutex_lock (&thread_mutex);
    ThreadHandle *handle = get_thread();
    if (!handle){
        LOGE("Get Thread ERROR\n");
        pthread_mutex_unlock (&thread_mutex);
        return ;
    }
    handle->content = params;
    handle->type = TYPE_READDIR;
    if (pthread_create(&handle->pid, NULL, _readdir_thread, handle) != 0){
        LOGE("Create pthread Error\n");
        handle->pid = 0;
    }
    pthread_mutex_unlock(&thread_mutex);

}

static void jni_readdir(JNIEnv *env, jobject bundle_params)
{
    ReaddirParams *params;
    params = malloc (sizeof(ReaddirParams));

    jstring path = (*env)->CallObjectMethod(env, bundle_params, get_string,
        (*env)->NewStringUTF(env, "path"));
    const char* lpath = (*env)->GetStringUTFChars(env, path, 0);
    memcpy (params->path, lpath, sizeof(params->path));
    (*env)->ReleaseStringUTFChars(env, path, lpath);

    LOGE("step 1");
    jstring username = (*env)->CallObjectMethod(env, bundle_params, get_string,
        (*env)->NewStringUTF(env, "username"));
    const char* lusername = (*env)->GetStringUTFChars(env, username, 0);
    memcpy (params->username, lusername, sizeof(params->username));
    (*env)->ReleaseStringUTFChars(env, username, lusername);
    LOGE("step 2");
    jstring passwd = (*env)->CallObjectMethod(env, bundle_params, get_string,
        (*env)->NewStringUTF(env, "password"));
    const char* lpasswd = (*env)->GetStringUTFChars(env, passwd, 0);
    memcpy (params->passwd, lpasswd, sizeof(params->passwd));
    (*env)->ReleaseStringUTFChars(env, passwd, lpasswd);
    LOGE("step 3");
    jint maxcount = (*env)->CallIntMethod(env, bundle_params, get_int,
        (*env)->NewStringUTF(env, "maxcount"));
    jint callid = (*env)->CallIntMethod(env, bundle_params, get_int,
        (*env)->NewStringUTF(env, "callid"));

    LOGE("callid is %ld, maxcount is %ld\n",callid, maxcount);

    params->maxcount = maxcount;
    params->cb = _readdir_callback;
    params->cb_arg = malloc (sizeof (jint));
    *(jint*)params->cb_arg = callid;

    LOGE("cb_arg is %ld, maxcount is %ld\n",*(jint*)params->cb_arg,params->maxcount);

    pthread_mutex_lock (&thread_mutex);
    ThreadHandle *handle = get_thread();
    if (!handle){
        LOGE("Get Thread ERROR\n");
        pthread_mutex_unlock (&thread_mutex);
        return ;
    }
    handle->content = params;
    handle->type = TYPE_READDIR;
    if (pthread_create(&handle->pid, NULL, _readdir_thread, handle) != 0){
        LOGE("Create pthread Error\n");
        handle->pid = 0;
    }
    pthread_mutex_unlock(&thread_mutex);
}

static void *_reset_thread(void *arg)
{
    ThreadHandle *handle = (ThreadHandle *)arg;
    int i;
    for (i=0;i<MAX_THREAD_NUM;i++)
        if (handle->pid != thread_handle[i].pid){
            pthread_join(thread_handle[i].pid ,NULL);
            LOGE("Thread Joined\n");
            thread_handle[i].joined = 1;
            thread_handle[i].finished = 0;
            thread_handle[i].pid = 0;
        }

    handle->finished = 0;
}


static void jni_reset(JNIEnv *env, jobject bundle_params)
{
    LOGE("jni_reset Called\n");

    g_reset = 1;

    pthread_mutex_lock (&thread_mutex);
    ThreadHandle *handle = get_thread();
    if (!handle){
        LOGE("Get Thread Error\n");
        pthread_mutex_unlock (&thread_mutex);
        return ;
    }
    handle->type = TYPE_RESET;
    if (pthread_create(&handle->pid, NULL, _reset_thread, handle){
        LOGE("Create pthread error jni_reset\n");
        handle->pid = 0;
    }
    pthread_mutex_unlock (&thread_mutex);
}

static void jni_abort(JNIEnv *env, jobject bundle_params)
{

}


typedef void (*FUNC_METHOD)(JNIEnv *env, jobject bundle_params);

typedef struct {
    const char *name;
    FUNC_METHOD func;
}FB_JNI_CALL;

static const FB_JNI_CALL jni_call[] = {
    {"readdir",jni_readdir},
    {"reset",jni_reset},
    {"abort",jni_abort},
    {NULL, NULL},
};

/*
 * Class:     com_targetv_fs_FileBrowserService
 * Method:    _asyncNativeCall
 * Signature: (Landroid/os/Bundle;)V
 */
JNIEXPORT void JNICALL Java_com_targetv_fs_FileBrowserService__1asyncNativeCall
  (JNIEnv *env, jobject obj, jobject bundle_params)
{
    jstring method = (*env)->CallObjectMethod(env, bundle_params, get_string,
        (*env)->NewStringUTF(env, "method"));

    const char* lmethod = (*env)->GetStringUTFChars(env, method, 0);
    int i;
    for (i=0; jni_call[i].name!=NULL; ++i)
        if (strcmp(lmethod, jni_call[i].name) == 0){
            jni_call[i].func(env, bundle_params);
            break;
        }
    (*env)->ReleaseStringUTFChars(env, method, lmethod);
}

