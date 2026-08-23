#include <jni.h>
#include <dlfcn.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include <android/log.h>
typedef struct { int32_t code; } FfiStatus;
typedef struct { uint8_t *ptr; uintptr_t len; uintptr_t cap; uintptr_t align; } FfiBuf_u8;
typedef struct { uint8_t *ptr; uintptr_t len; uintptr_t cap; } FfiString;
#define TAG "bolt_shim"
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, TAG, __VA_ARGS__)
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, TAG, __VA_ARGS__)
static void *SYM[512];
static const char *SYM_NAME[512];
static int SYM_N = 0;
static void *sym(const char *n) {
    for (int i = 0; i < SYM_N; i++) if (!strcmp(SYM_NAME[i], n)) return SYM[i];
    static void *h;
    if (!h) { h = dlopen("libxybrid-bolt.so", RTLD_NOW | RTLD_GLOBAL);
              if (!h) LOGE("dlopen failed: %s", dlerror()); }
    void *p = h ? dlsym(h, n) : NULL;
    if (p && SYM_N < 512) { SYM[SYM_N] = p; SYM_NAME[SYM_N] = n; SYM_N++; }
    if (!p) LOGE("dlsym missing: %s", n);
    return p;
}
static void throw_ffi(JNIEnv *env, FfiBuf_u8 err) {
    char msg[512] = {0};
    if (err.ptr && err.len) { uintptr_t n = err.len < 511 ? err.len : 511; memcpy(msg, err.ptr, n); }
    else snprintf(msg, sizeof msg, "FFI error");
    void (*fr)(FfiBuf_u8*) = (void (*)(FfiBuf_u8*))sym("boltffi_free_buf");
    if (fr && err.ptr) fr(&err);
    if (!(*env)->ExceptionCheck(env)) {
        jclass ex = (*env)->FindClass(env, "java/lang/RuntimeException");
        if (ex) (*env)->ThrowNew(env, ex, msg);
    }
}
static jbyteArray buf_to_array(JNIEnv *env, FfiBuf_u8 b) {
    if (!b.ptr || b.len == 0) {
        void (*fr)(FfiBuf_u8*) = (void (*)(FfiBuf_u8*))sym("boltffi_free_buf");
        if (fr && b.ptr) fr(&b);
        return NULL;
    }
    jbyteArray arr = (*env)->NewByteArray(env, (jsize)b.len);
    if (arr) (*env)->SetByteArrayRegion(env, arr, 0, (jsize)b.len, (const jbyte*)b.ptr);
    void (*fr)(FfiBuf_u8*) = (void (*)(FfiBuf_u8*))sym("boltffi_free_buf");
    if (fr) fr(&b);
    return arr;
}
static jstring str_to_jstring(JNIEnv *env, FfiString s) {
    jstring r = NULL;
    if (s.ptr) {
        char tmp[2048]; uintptr_t n = s.len < 2047 ? s.len : 2047;
        memcpy(tmp, s.ptr, n); tmp[n] = 0;
        r = (*env)->NewStringUTF(env, tmp);
    }
    void (*fr)(FfiString*) = (void (*)(FfiString*))sym("boltffi_free_string");
    if (fr) fr(&s);
    return r;
}


JNIEXPORT void JNICALL Java_ai_xybrid_Native_boltffi_1release_1class_1xybrid_1bolt_1xybrid_1model(JNIEnv *env, jclass cls, jlong j_handle) {
    LOGI("ENTER boltffi_release_class_xybrid_bolt_xybrid_model");
    uint64_t c_handle = (uint64_t)j_handle;
    static void *fp; if (!fp) fp = sym("boltffi_release_class_xybrid_bolt_xybrid_model");
    if (!fp) { jclass ex = (*env)->FindClass(env, "java/lang/RuntimeException"); if (ex) (*env)->ThrowNew(env, ex, "shim: missing symbol"); return; }
    ((void (*)(uint64_t))fp)(c_handle);
}

JNIEXPORT jlong JNICALL Java_ai_xybrid_Native_boltffi_1init_1class_1xybrid_1bolt_1xybrid_1model_1from_1registry(JNIEnv *env, jclass cls, jobject j_id_ptr, jint j_id_len, jlong j_return_out) {
    LOGI("ENTER boltffi_init_class_xybrid_bolt_xybrid_model_from_registry");
    const uint8_t* c_id_ptr = j_id_ptr ? (const uint8_t*)(*env)->GetDirectBufferAddress(env, j_id_ptr) : NULL;
    uintptr_t c_id_len = (uintptr_t)j_id_len;
    LOGI("  boltffi_init_class_xybrid_bolt_xybrid_model_from_registry: buf[id_ptr] len=%zu head=%02x %02x %02x %02x %02x %02x %02x %02x", (size_t)c_id_len, c_id_ptr[0], c_id_ptr[1], c_id_ptr[2], c_id_ptr[3], c_id_ptr[4], c_id_ptr[5], c_id_ptr[6], c_id_ptr[7]);
    uint64_t c_return_out = 0;
    static void *fp; if (!fp) fp = sym("boltffi_init_class_xybrid_bolt_xybrid_model_from_registry");
    if (!fp) { jclass ex = (*env)->FindClass(env, "java/lang/RuntimeException"); if (ex) (*env)->ThrowNew(env, ex, "shim: missing symbol"); return 0; }
    FfiBuf_u8 __eb = ((FfiBuf_u8 (*)(const uint8_t*, uintptr_t, uint64_t*))fp)(c_id_ptr, c_id_len, &c_return_out);
    if (__eb.ptr && __eb.len) { throw_ffi(env, __eb); return 0; } else { void (*fr)(FfiBuf_u8*)=(void(*)(FfiBuf_u8*))sym("boltffi_free_buf"); if(fr&&__eb.ptr) fr(&__eb); }
    return (jlong)c_return_out;
}

JNIEXPORT jlong JNICALL Java_ai_xybrid_Native_boltffi_1init_1class_1xybrid_1bolt_1xybrid_1model_1from_1registry_1speculative(JNIEnv *env, jclass cls, jobject j_id_ptr, jint j_id_len, jlong j_return_out) {
    LOGI("ENTER boltffi_init_class_xybrid_bolt_xybrid_model_from_registry_speculative");
    const uint8_t* c_id_ptr = j_id_ptr ? (const uint8_t*)(*env)->GetDirectBufferAddress(env, j_id_ptr) : NULL;
    uintptr_t c_id_len = (uintptr_t)j_id_len;
    LOGI("  boltffi_init_class_xybrid_bolt_xybrid_model_from_registry_speculative: buf[id_ptr] len=%zu head=%02x %02x %02x %02x %02x %02x %02x %02x", (size_t)c_id_len, c_id_ptr[0], c_id_ptr[1], c_id_ptr[2], c_id_ptr[3], c_id_ptr[4], c_id_ptr[5], c_id_ptr[6], c_id_ptr[7]);
    uint64_t c_return_out = 0;
    static void *fp; if (!fp) fp = sym("boltffi_init_class_xybrid_bolt_xybrid_model_from_registry_speculative");
    if (!fp) { jclass ex = (*env)->FindClass(env, "java/lang/RuntimeException"); if (ex) (*env)->ThrowNew(env, ex, "shim: missing symbol"); return 0; }
    FfiBuf_u8 __eb = ((FfiBuf_u8 (*)(const uint8_t*, uintptr_t, uint64_t*))fp)(c_id_ptr, c_id_len, &c_return_out);
    if (__eb.ptr && __eb.len) { throw_ffi(env, __eb); return 0; } else { void (*fr)(FfiBuf_u8*)=(void(*)(FfiBuf_u8*))sym("boltffi_free_buf"); if(fr&&__eb.ptr) fr(&__eb); }
    return (jlong)c_return_out;
}

JNIEXPORT jlong JNICALL Java_ai_xybrid_Native_boltffi_1init_1class_1xybrid_1bolt_1xybrid_1model_1from_1directory(JNIEnv *env, jclass cls, jobject j_path_ptr, jint j_path_len, jlong j_return_out) {
    LOGI("ENTER boltffi_init_class_xybrid_bolt_xybrid_model_from_directory");
    const uint8_t* c_path_ptr = j_path_ptr ? (const uint8_t*)(*env)->GetDirectBufferAddress(env, j_path_ptr) : NULL;
    uintptr_t c_path_len = (uintptr_t)j_path_len;
    LOGI("  boltffi_init_class_xybrid_bolt_xybrid_model_from_directory: buf[path_ptr] len=%zu head=%02x %02x %02x %02x %02x %02x %02x %02x", (size_t)c_path_len, c_path_ptr[0], c_path_ptr[1], c_path_ptr[2], c_path_ptr[3], c_path_ptr[4], c_path_ptr[5], c_path_ptr[6], c_path_ptr[7]);
    uint64_t c_return_out = 0;
    static void *fp; if (!fp) fp = sym("boltffi_init_class_xybrid_bolt_xybrid_model_from_directory");
    if (!fp) { jclass ex = (*env)->FindClass(env, "java/lang/RuntimeException"); if (ex) (*env)->ThrowNew(env, ex, "shim: missing symbol"); return 0; }
    FfiBuf_u8 __eb = ((FfiBuf_u8 (*)(const uint8_t*, uintptr_t, uint64_t*))fp)(c_path_ptr, c_path_len, &c_return_out);
    if (__eb.ptr && __eb.len) { throw_ffi(env, __eb); return 0; } else { void (*fr)(FfiBuf_u8*)=(void(*)(FfiBuf_u8*))sym("boltffi_free_buf"); if(fr&&__eb.ptr) fr(&__eb); }
    return (jlong)c_return_out;
}

JNIEXPORT jlong JNICALL Java_ai_xybrid_Native_boltffi_1init_1class_1xybrid_1bolt_1xybrid_1model_1from_1bundle(JNIEnv *env, jclass cls, jobject j_path_ptr, jint j_path_len, jlong j_return_out) {
    LOGI("ENTER boltffi_init_class_xybrid_bolt_xybrid_model_from_bundle");
    const uint8_t* c_path_ptr = j_path_ptr ? (const uint8_t*)(*env)->GetDirectBufferAddress(env, j_path_ptr) : NULL;
    uintptr_t c_path_len = (uintptr_t)j_path_len;
    LOGI("  boltffi_init_class_xybrid_bolt_xybrid_model_from_bundle: buf[path_ptr] len=%zu head=%02x %02x %02x %02x %02x %02x %02x %02x", (size_t)c_path_len, c_path_ptr[0], c_path_ptr[1], c_path_ptr[2], c_path_ptr[3], c_path_ptr[4], c_path_ptr[5], c_path_ptr[6], c_path_ptr[7]);
    uint64_t c_return_out = 0;
    static void *fp; if (!fp) fp = sym("boltffi_init_class_xybrid_bolt_xybrid_model_from_bundle");
    if (!fp) { jclass ex = (*env)->FindClass(env, "java/lang/RuntimeException"); if (ex) (*env)->ThrowNew(env, ex, "shim: missing symbol"); return 0; }
    FfiBuf_u8 __eb = ((FfiBuf_u8 (*)(const uint8_t*, uintptr_t, uint64_t*))fp)(c_path_ptr, c_path_len, &c_return_out);
    if (__eb.ptr && __eb.len) { throw_ffi(env, __eb); return 0; } else { void (*fr)(FfiBuf_u8*)=(void(*)(FfiBuf_u8*))sym("boltffi_free_buf"); if(fr&&__eb.ptr) fr(&__eb); }
    return (jlong)c_return_out;
}

JNIEXPORT jlong JNICALL Java_ai_xybrid_Native_boltffi_1init_1class_1xybrid_1bolt_1xybrid_1model_1from_1huggingface(JNIEnv *env, jclass cls, jobject j_repo_ptr, jint j_repo_len, jlong j_return_out) {
    LOGI("ENTER boltffi_init_class_xybrid_bolt_xybrid_model_from_huggingface");
    const uint8_t* c_repo_ptr = j_repo_ptr ? (const uint8_t*)(*env)->GetDirectBufferAddress(env, j_repo_ptr) : NULL;
    uintptr_t c_repo_len = (uintptr_t)j_repo_len;
    LOGI("  boltffi_init_class_xybrid_bolt_xybrid_model_from_huggingface: buf[repo_ptr] len=%zu head=%02x %02x %02x %02x %02x %02x %02x %02x", (size_t)c_repo_len, c_repo_ptr[0], c_repo_ptr[1], c_repo_ptr[2], c_repo_ptr[3], c_repo_ptr[4], c_repo_ptr[5], c_repo_ptr[6], c_repo_ptr[7]);
    uint64_t c_return_out = 0;
    static void *fp; if (!fp) fp = sym("boltffi_init_class_xybrid_bolt_xybrid_model_from_huggingface");
    if (!fp) { jclass ex = (*env)->FindClass(env, "java/lang/RuntimeException"); if (ex) (*env)->ThrowNew(env, ex, "shim: missing symbol"); return 0; }
    FfiBuf_u8 __eb = ((FfiBuf_u8 (*)(const uint8_t*, uintptr_t, uint64_t*))fp)(c_repo_ptr, c_repo_len, &c_return_out);
    if (__eb.ptr && __eb.len) { throw_ffi(env, __eb); return 0; } else { void (*fr)(FfiBuf_u8*)=(void(*)(FfiBuf_u8*))sym("boltffi_free_buf"); if(fr&&__eb.ptr) fr(&__eb); }
    return (jlong)c_return_out;
}

JNIEXPORT jlong JNICALL Java_ai_xybrid_Native_boltffi_1init_1class_1xybrid_1bolt_1xybrid_1model_1from_1huggingface_1with_1revision(JNIEnv *env, jclass cls, jobject j_repo_ptr, jint j_repo_len, jobject j_revision_ptr, jint j_revision_len, jlong j_return_out) {
    LOGI("ENTER boltffi_init_class_xybrid_bolt_xybrid_model_from_huggingface_with_revision");
    const uint8_t* c_repo_ptr = j_repo_ptr ? (const uint8_t*)(*env)->GetDirectBufferAddress(env, j_repo_ptr) : NULL;
    uintptr_t c_repo_len = (uintptr_t)j_repo_len;
    LOGI("  boltffi_init_class_xybrid_bolt_xybrid_model_from_huggingface_with_revision: buf[repo_ptr] len=%zu head=%02x %02x %02x %02x %02x %02x %02x %02x", (size_t)c_repo_len, c_repo_ptr[0], c_repo_ptr[1], c_repo_ptr[2], c_repo_ptr[3], c_repo_ptr[4], c_repo_ptr[5], c_repo_ptr[6], c_repo_ptr[7]);
    const uint8_t* c_revision_ptr = j_revision_ptr ? (const uint8_t*)(*env)->GetDirectBufferAddress(env, j_revision_ptr) : NULL;
    uintptr_t c_revision_len = (uintptr_t)j_revision_len;
    LOGI("  boltffi_init_class_xybrid_bolt_xybrid_model_from_huggingface_with_revision: buf[revision_ptr] len=%zu head=%02x %02x %02x %02x %02x %02x %02x %02x", (size_t)c_revision_len, c_revision_ptr[0], c_revision_ptr[1], c_revision_ptr[2], c_revision_ptr[3], c_revision_ptr[4], c_revision_ptr[5], c_revision_ptr[6], c_revision_ptr[7]);
    uint64_t c_return_out = 0;
    static void *fp; if (!fp) fp = sym("boltffi_init_class_xybrid_bolt_xybrid_model_from_huggingface_with_revision");
    if (!fp) { jclass ex = (*env)->FindClass(env, "java/lang/RuntimeException"); if (ex) (*env)->ThrowNew(env, ex, "shim: missing symbol"); return 0; }
    FfiBuf_u8 __eb = ((FfiBuf_u8 (*)(const uint8_t*, uintptr_t, const uint8_t*, uintptr_t, uint64_t*))fp)(c_repo_ptr, c_repo_len, c_revision_ptr, c_revision_len, &c_return_out);
    if (__eb.ptr && __eb.len) { throw_ffi(env, __eb); return 0; } else { void (*fr)(FfiBuf_u8*)=(void(*)(FfiBuf_u8*))sym("boltffi_free_buf"); if(fr&&__eb.ptr) fr(&__eb); }
    return (jlong)c_return_out;
}

JNIEXPORT jlong JNICALL Java_ai_xybrid_Native_boltffi_1init_1class_1xybrid_1bolt_1xybrid_1model_1from_1model_1file(JNIEnv *env, jclass cls, jobject j_path_ptr, jint j_path_len, jlong j_return_out) {
    LOGI("ENTER boltffi_init_class_xybrid_bolt_xybrid_model_from_model_file");
    const uint8_t* c_path_ptr = j_path_ptr ? (const uint8_t*)(*env)->GetDirectBufferAddress(env, j_path_ptr) : NULL;
    uintptr_t c_path_len = (uintptr_t)j_path_len;
    LOGI("  boltffi_init_class_xybrid_bolt_xybrid_model_from_model_file: buf[path_ptr] len=%zu head=%02x %02x %02x %02x %02x %02x %02x %02x", (size_t)c_path_len, c_path_ptr[0], c_path_ptr[1], c_path_ptr[2], c_path_ptr[3], c_path_ptr[4], c_path_ptr[5], c_path_ptr[6], c_path_ptr[7]);
    uint64_t c_return_out = 0;
    static void *fp; if (!fp) fp = sym("boltffi_init_class_xybrid_bolt_xybrid_model_from_model_file");
    if (!fp) { jclass ex = (*env)->FindClass(env, "java/lang/RuntimeException"); if (ex) (*env)->ThrowNew(env, ex, "shim: missing symbol"); return 0; }
    FfiBuf_u8 __eb = ((FfiBuf_u8 (*)(const uint8_t*, uintptr_t, uint64_t*))fp)(c_path_ptr, c_path_len, &c_return_out);
    if (__eb.ptr && __eb.len) { throw_ffi(env, __eb); return 0; } else { void (*fr)(FfiBuf_u8*)=(void(*)(FfiBuf_u8*))sym("boltffi_free_buf"); if(fr&&__eb.ptr) fr(&__eb); }
    return (jlong)c_return_out;
}

JNIEXPORT jbyteArray JNICALL Java_ai_xybrid_Native_boltffi_1method_1class_1xybrid_1bolt_1xybrid_1model_1model_1id(JNIEnv *env, jclass cls, jlong j_receiver) {
    LOGI("ENTER boltffi_method_class_xybrid_bolt_xybrid_model_model_id");
    uint64_t c_receiver = (uint64_t)j_receiver;
    static void *fp; if (!fp) fp = sym("boltffi_method_class_xybrid_bolt_xybrid_model_model_id");
    if (!fp) { jclass ex = (*env)->FindClass(env, "java/lang/RuntimeException"); if (ex) (*env)->ThrowNew(env, ex, "shim: missing symbol"); return 0; }
    ((FfiBuf_u8 (*)(uint64_t))fp)(c_receiver);
}

JNIEXPORT jbyteArray JNICALL Java_ai_xybrid_Native_boltffi_1method_1class_1xybrid_1bolt_1xybrid_1model_1version(JNIEnv *env, jclass cls, jlong j_receiver) {
    LOGI("ENTER boltffi_method_class_xybrid_bolt_xybrid_model_version");
    uint64_t c_receiver = (uint64_t)j_receiver;
    static void *fp; if (!fp) fp = sym("boltffi_method_class_xybrid_bolt_xybrid_model_version");
    if (!fp) { jclass ex = (*env)->FindClass(env, "java/lang/RuntimeException"); if (ex) (*env)->ThrowNew(env, ex, "shim: missing symbol"); return 0; }
    ((FfiBuf_u8 (*)(uint64_t))fp)(c_receiver);
}

JNIEXPORT jint JNICALL Java_ai_xybrid_Native_boltffi_1method_1class_1xybrid_1bolt_1xybrid_1model_1output_1type(JNIEnv *env, jclass cls, jlong j_receiver) {
    LOGI("ENTER boltffi_method_class_xybrid_bolt_xybrid_model_output_type");
    uint64_t c_receiver = (uint64_t)j_receiver;
    static void *fp; if (!fp) fp = sym("boltffi_method_class_xybrid_bolt_xybrid_model_output_type");
    if (!fp) { jclass ex = (*env)->FindClass(env, "java/lang/RuntimeException"); if (ex) (*env)->ThrowNew(env, ex, "shim: missing symbol"); return 0; }
    return (jint)((int32_t (*)(uint64_t))fp)(c_receiver);
}

JNIEXPORT jboolean JNICALL Java_ai_xybrid_Native_boltffi_1method_1class_1xybrid_1bolt_1xybrid_1model_1is_1loaded(JNIEnv *env, jclass cls, jlong j_receiver) {
    LOGI("ENTER boltffi_method_class_xybrid_bolt_xybrid_model_is_loaded");
    uint64_t c_receiver = (uint64_t)j_receiver;
    static void *fp; if (!fp) fp = sym("boltffi_method_class_xybrid_bolt_xybrid_model_is_loaded");
    if (!fp) { jclass ex = (*env)->FindClass(env, "java/lang/RuntimeException"); if (ex) (*env)->ThrowNew(env, ex, "shim: missing symbol"); return 0; }
    return (jboolean)((bool (*)(uint64_t))fp)(c_receiver);
}

JNIEXPORT jboolean JNICALL Java_ai_xybrid_Native_boltffi_1method_1class_1xybrid_1bolt_1xybrid_1model_1is_1cloud_1serving(JNIEnv *env, jclass cls, jlong j_receiver) {
    LOGI("ENTER boltffi_method_class_xybrid_bolt_xybrid_model_is_cloud_serving");
    uint64_t c_receiver = (uint64_t)j_receiver;
    static void *fp; if (!fp) fp = sym("boltffi_method_class_xybrid_bolt_xybrid_model_is_cloud_serving");
    if (!fp) { jclass ex = (*env)->FindClass(env, "java/lang/RuntimeException"); if (ex) (*env)->ThrowNew(env, ex, "shim: missing symbol"); return 0; }
    return (jboolean)((bool (*)(uint64_t))fp)(c_receiver);
}

JNIEXPORT jbyteArray JNICALL Java_ai_xybrid_Native_boltffi_1method_1class_1xybrid_1bolt_1xybrid_1model_1download_1status(JNIEnv *env, jclass cls, jlong j_receiver) {
    LOGI("ENTER boltffi_method_class_xybrid_bolt_xybrid_model_download_status");
    uint64_t c_receiver = (uint64_t)j_receiver;
    static void *fp; if (!fp) fp = sym("boltffi_method_class_xybrid_bolt_xybrid_model_download_status");
    if (!fp) { jclass ex = (*env)->FindClass(env, "java/lang/RuntimeException"); if (ex) (*env)->ThrowNew(env, ex, "shim: missing symbol"); return 0; }
    ((FfiBuf_u8 (*)(uint64_t))fp)(c_receiver);
}

JNIEXPORT jbyteArray JNICALL Java_ai_xybrid_Native_boltffi_1method_1class_1xybrid_1bolt_1xybrid_1model_1await_1download(JNIEnv *env, jclass cls, jlong j_receiver, jlong j_timeout_ms) {
    LOGI("ENTER boltffi_method_class_xybrid_bolt_xybrid_model_await_download");
    uint64_t c_receiver = (uint64_t)j_receiver;
    uint64_t c_timeout_ms = (uint64_t)j_timeout_ms;
    static void *fp; if (!fp) fp = sym("boltffi_method_class_xybrid_bolt_xybrid_model_await_download");
    if (!fp) { jclass ex = (*env)->FindClass(env, "java/lang/RuntimeException"); if (ex) (*env)->ThrowNew(env, ex, "shim: missing symbol"); return 0; }
    ((FfiBuf_u8 (*)(uint64_t, uint64_t))fp)(c_receiver, c_timeout_ms);
}

JNIEXPORT jboolean JNICALL Java_ai_xybrid_Native_boltffi_1method_1class_1xybrid_1bolt_1xybrid_1model_1supports_1streaming(JNIEnv *env, jclass cls, jlong j_receiver) {
    LOGI("ENTER boltffi_method_class_xybrid_bolt_xybrid_model_supports_streaming");
    uint64_t c_receiver = (uint64_t)j_receiver;
    static void *fp; if (!fp) fp = sym("boltffi_method_class_xybrid_bolt_xybrid_model_supports_streaming");
    if (!fp) { jclass ex = (*env)->FindClass(env, "java/lang/RuntimeException"); if (ex) (*env)->ThrowNew(env, ex, "shim: missing symbol"); return 0; }
    return (jboolean)((bool (*)(uint64_t))fp)(c_receiver);
}

JNIEXPORT jboolean JNICALL Java_ai_xybrid_Native_boltffi_1method_1class_1xybrid_1bolt_1xybrid_1model_1supports_1token_1streaming(JNIEnv *env, jclass cls, jlong j_receiver) {
    LOGI("ENTER boltffi_method_class_xybrid_bolt_xybrid_model_supports_token_streaming");
    uint64_t c_receiver = (uint64_t)j_receiver;
    static void *fp; if (!fp) fp = sym("boltffi_method_class_xybrid_bolt_xybrid_model_supports_token_streaming");
    if (!fp) { jclass ex = (*env)->FindClass(env, "java/lang/RuntimeException"); if (ex) (*env)->ThrowNew(env, ex, "shim: missing symbol"); return 0; }
    return (jboolean)((bool (*)(uint64_t))fp)(c_receiver);
}

JNIEXPORT jbyteArray JNICALL Java_ai_xybrid_Native_boltffi_1method_1class_1xybrid_1bolt_1xybrid_1model_1default_1generation_1config(JNIEnv *env, jclass cls, jlong j_receiver) {
    LOGI("ENTER boltffi_method_class_xybrid_bolt_xybrid_model_default_generation_config");
    uint64_t c_receiver = (uint64_t)j_receiver;
    static void *fp; if (!fp) fp = sym("boltffi_method_class_xybrid_bolt_xybrid_model_default_generation_config");
    if (!fp) { jclass ex = (*env)->FindClass(env, "java/lang/RuntimeException"); if (ex) (*env)->ThrowNew(env, ex, "shim: missing symbol"); return 0; }
    ((FfiBuf_u8 (*)(uint64_t))fp)(c_receiver);
}

JNIEXPORT jboolean JNICALL Java_ai_xybrid_Native_boltffi_1method_1class_1xybrid_1bolt_1xybrid_1model_1is_1llm(JNIEnv *env, jclass cls, jlong j_receiver) {
    LOGI("ENTER boltffi_method_class_xybrid_bolt_xybrid_model_is_llm");
    uint64_t c_receiver = (uint64_t)j_receiver;
    static void *fp; if (!fp) fp = sym("boltffi_method_class_xybrid_bolt_xybrid_model_is_llm");
    if (!fp) { jclass ex = (*env)->FindClass(env, "java/lang/RuntimeException"); if (ex) (*env)->ThrowNew(env, ex, "shim: missing symbol"); return 0; }
    return (jboolean)((bool (*)(uint64_t))fp)(c_receiver);
}

JNIEXPORT jbyteArray JNICALL Java_ai_xybrid_Native_boltffi_1method_1class_1xybrid_1bolt_1xybrid_1model_1supports_1tool_1calling(JNIEnv *env, jclass cls, jlong j_receiver) {
    LOGI("ENTER boltffi_method_class_xybrid_bolt_xybrid_model_supports_tool_calling");
    uint64_t c_receiver = (uint64_t)j_receiver;
    static void *fp; if (!fp) fp = sym("boltffi_method_class_xybrid_bolt_xybrid_model_supports_tool_calling");
    if (!fp) { jclass ex = (*env)->FindClass(env, "java/lang/RuntimeException"); if (ex) (*env)->ThrowNew(env, ex, "shim: missing symbol"); return 0; }
    ((FfiBuf_u8 (*)(uint64_t))fp)(c_receiver);
}

JNIEXPORT jboolean JNICALL Java_ai_xybrid_Native_boltffi_1method_1class_1xybrid_1bolt_1xybrid_1model_1has_1voices(JNIEnv *env, jclass cls, jlong j_receiver) {
    LOGI("ENTER boltffi_method_class_xybrid_bolt_xybrid_model_has_voices");
    uint64_t c_receiver = (uint64_t)j_receiver;
    static void *fp; if (!fp) fp = sym("boltffi_method_class_xybrid_bolt_xybrid_model_has_voices");
    if (!fp) { jclass ex = (*env)->FindClass(env, "java/lang/RuntimeException"); if (ex) (*env)->ThrowNew(env, ex, "shim: missing symbol"); return 0; }
    return (jboolean)((bool (*)(uint64_t))fp)(c_receiver);
}

JNIEXPORT jbyteArray JNICALL Java_ai_xybrid_Native_boltffi_1method_1class_1xybrid_1bolt_1xybrid_1model_1voices(JNIEnv *env, jclass cls, jlong j_receiver) {
    LOGI("ENTER boltffi_method_class_xybrid_bolt_xybrid_model_voices");
    uint64_t c_receiver = (uint64_t)j_receiver;
    static void *fp; if (!fp) fp = sym("boltffi_method_class_xybrid_bolt_xybrid_model_voices");
    if (!fp) { jclass ex = (*env)->FindClass(env, "java/lang/RuntimeException"); if (ex) (*env)->ThrowNew(env, ex, "shim: missing symbol"); return 0; }
    ((FfiBuf_u8 (*)(uint64_t))fp)(c_receiver);
}

JNIEXPORT jbyteArray JNICALL Java_ai_xybrid_Native_boltffi_1method_1class_1xybrid_1bolt_1xybrid_1model_1default_1voice(JNIEnv *env, jclass cls, jlong j_receiver) {
    LOGI("ENTER boltffi_method_class_xybrid_bolt_xybrid_model_default_voice");
    uint64_t c_receiver = (uint64_t)j_receiver;
    static void *fp; if (!fp) fp = sym("boltffi_method_class_xybrid_bolt_xybrid_model_default_voice");
    if (!fp) { jclass ex = (*env)->FindClass(env, "java/lang/RuntimeException"); if (ex) (*env)->ThrowNew(env, ex, "shim: missing symbol"); return 0; }
    ((FfiBuf_u8 (*)(uint64_t))fp)(c_receiver);
}

JNIEXPORT jbyteArray JNICALL Java_ai_xybrid_Native_boltffi_1method_1class_1xybrid_1bolt_1xybrid_1model_1voice(JNIEnv *env, jclass cls, jlong j_receiver, jobject j_voice_id_ptr, jint j_voice_id_len) {
    LOGI("ENTER boltffi_method_class_xybrid_bolt_xybrid_model_voice");
    uint64_t c_receiver = (uint64_t)j_receiver;
    const uint8_t* c_voice_id_ptr = j_voice_id_ptr ? (const uint8_t*)(*env)->GetDirectBufferAddress(env, j_voice_id_ptr) : NULL;
    uintptr_t c_voice_id_len = (uintptr_t)j_voice_id_len;
    LOGI("  boltffi_method_class_xybrid_bolt_xybrid_model_voice: buf[voice_id_ptr] len=%zu head=%02x %02x %02x %02x %02x %02x %02x %02x", (size_t)c_voice_id_len, c_voice_id_ptr[0], c_voice_id_ptr[1], c_voice_id_ptr[2], c_voice_id_ptr[3], c_voice_id_ptr[4], c_voice_id_ptr[5], c_voice_id_ptr[6], c_voice_id_ptr[7]);
    static void *fp; if (!fp) fp = sym("boltffi_method_class_xybrid_bolt_xybrid_model_voice");
    if (!fp) { jclass ex = (*env)->FindClass(env, "java/lang/RuntimeException"); if (ex) (*env)->ThrowNew(env, ex, "shim: missing symbol"); return 0; }
    ((FfiBuf_u8 (*)(uint64_t, const uint8_t*, uintptr_t))fp)(c_receiver, c_voice_id_ptr, c_voice_id_len);
}

JNIEXPORT jbyteArray JNICALL Java_ai_xybrid_Native_boltffi_1method_1class_1xybrid_1bolt_1xybrid_1model_1run(JNIEnv *env, jclass cls, jlong j_receiver, jobject j_envelope_ptr, jint j_envelope_len, jobject j_options_ptr, jint j_options_len, jobject j_return_out) {
    LOGI("ENTER boltffi_method_class_xybrid_bolt_xybrid_model_run");
    uint64_t c_receiver = (uint64_t)j_receiver;
    const uint8_t* c_envelope_ptr = j_envelope_ptr ? (const uint8_t*)(*env)->GetDirectBufferAddress(env, j_envelope_ptr) : NULL;
    uintptr_t c_envelope_len = (uintptr_t)j_envelope_len;
    LOGI("  boltffi_method_class_xybrid_bolt_xybrid_model_run: buf[envelope_ptr] len=%zu head=%02x %02x %02x %02x %02x %02x %02x %02x", (size_t)c_envelope_len, c_envelope_ptr[0], c_envelope_ptr[1], c_envelope_ptr[2], c_envelope_ptr[3], c_envelope_ptr[4], c_envelope_ptr[5], c_envelope_ptr[6], c_envelope_ptr[7]);
    const uint8_t* c_options_ptr = j_options_ptr ? (const uint8_t*)(*env)->GetDirectBufferAddress(env, j_options_ptr) : NULL;
    uintptr_t c_options_len = (uintptr_t)j_options_len;
    LOGI("  boltffi_method_class_xybrid_bolt_xybrid_model_run: buf[options_ptr] len=%zu head=%02x %02x %02x %02x %02x %02x %02x %02x", (size_t)c_options_len, c_options_ptr[0], c_options_ptr[1], c_options_ptr[2], c_options_ptr[3], c_options_ptr[4], c_options_ptr[5], c_options_ptr[6], c_options_ptr[7]);
    FfiBuf_u8 c_return_out; memset(&c_return_out, 0, sizeof(FfiBuf_u8));
    static void *fp; if (!fp) fp = sym("boltffi_method_class_xybrid_bolt_xybrid_model_run");
    if (!fp) { jclass ex = (*env)->FindClass(env, "java/lang/RuntimeException"); if (ex) (*env)->ThrowNew(env, ex, "shim: missing symbol"); return 0; }
    FfiBuf_u8 __eb = ((FfiBuf_u8 (*)(uint64_t, const uint8_t*, uintptr_t, const uint8_t*, uintptr_t, FfiBuf_u8*))fp)(c_receiver, c_envelope_ptr, c_envelope_len, c_options_ptr, c_options_len, &c_return_out);
    if (__eb.ptr && __eb.len) { throw_ffi(env, __eb); return NULL; }
    else { void (*fr)(FfiBuf_u8*)=(void(*)(FfiBuf_u8*))sym("boltffi_free_buf"); if(fr&&__eb.ptr) fr(&__eb); }
    return buf_to_array(env, c_return_out);
}

JNIEXPORT jlong JNICALL Java_ai_xybrid_Native_boltffi_1method_1class_1xybrid_1bolt_1xybrid_1model_1run_1stream(JNIEnv *env, jclass cls, jlong j_receiver, jobject j_envelope_ptr, jint j_envelope_len, jobject j_options_ptr, jint j_options_len, jlong j_return_out) {
    LOGI("ENTER boltffi_method_class_xybrid_bolt_xybrid_model_run_stream");
    uint64_t c_receiver = (uint64_t)j_receiver;
    const uint8_t* c_envelope_ptr = j_envelope_ptr ? (const uint8_t*)(*env)->GetDirectBufferAddress(env, j_envelope_ptr) : NULL;
    uintptr_t c_envelope_len = (uintptr_t)j_envelope_len;
    LOGI("  boltffi_method_class_xybrid_bolt_xybrid_model_run_stream: buf[envelope_ptr] len=%zu head=%02x %02x %02x %02x %02x %02x %02x %02x", (size_t)c_envelope_len, c_envelope_ptr[0], c_envelope_ptr[1], c_envelope_ptr[2], c_envelope_ptr[3], c_envelope_ptr[4], c_envelope_ptr[5], c_envelope_ptr[6], c_envelope_ptr[7]);
    const uint8_t* c_options_ptr = j_options_ptr ? (const uint8_t*)(*env)->GetDirectBufferAddress(env, j_options_ptr) : NULL;
    uintptr_t c_options_len = (uintptr_t)j_options_len;
    LOGI("  boltffi_method_class_xybrid_bolt_xybrid_model_run_stream: buf[options_ptr] len=%zu head=%02x %02x %02x %02x %02x %02x %02x %02x", (size_t)c_options_len, c_options_ptr[0], c_options_ptr[1], c_options_ptr[2], c_options_ptr[3], c_options_ptr[4], c_options_ptr[5], c_options_ptr[6], c_options_ptr[7]);
    uint64_t c_return_out = 0;
    static void *fp; if (!fp) fp = sym("boltffi_method_class_xybrid_bolt_xybrid_model_run_stream");
    if (!fp) { jclass ex = (*env)->FindClass(env, "java/lang/RuntimeException"); if (ex) (*env)->ThrowNew(env, ex, "shim: missing symbol"); return 0; }
    FfiBuf_u8 __eb = ((FfiBuf_u8 (*)(uint64_t, const uint8_t*, uintptr_t, const uint8_t*, uintptr_t, uint64_t*))fp)(c_receiver, c_envelope_ptr, c_envelope_len, c_options_ptr, c_options_len, &c_return_out);
    if (__eb.ptr && __eb.len) { throw_ffi(env, __eb); return 0; } else { void (*fr)(FfiBuf_u8*)=(void(*)(FfiBuf_u8*))sym("boltffi_free_buf"); if(fr&&__eb.ptr) fr(&__eb); }
    return (jlong)c_return_out;
}

JNIEXPORT jbyteArray JNICALL Java_ai_xybrid_Native_boltffi_1method_1class_1xybrid_1bolt_1xybrid_1model_1stream_1next(JNIEnv *env, jclass cls, jlong j_receiver, jlong j_stream_id, jobject j_return_out) {
    LOGI("ENTER boltffi_method_class_xybrid_bolt_xybrid_model_stream_next");
    uint64_t c_receiver = (uint64_t)j_receiver;
    uint64_t c_stream_id = (uint64_t)j_stream_id;
    FfiBuf_u8 c_return_out; memset(&c_return_out, 0, sizeof(FfiBuf_u8));
    static void *fp; if (!fp) fp = sym("boltffi_method_class_xybrid_bolt_xybrid_model_stream_next");
    if (!fp) { jclass ex = (*env)->FindClass(env, "java/lang/RuntimeException"); if (ex) (*env)->ThrowNew(env, ex, "shim: missing symbol"); return 0; }
    FfiBuf_u8 __eb = ((FfiBuf_u8 (*)(uint64_t, uint64_t, FfiBuf_u8*))fp)(c_receiver, c_stream_id, &c_return_out);
    if (__eb.ptr && __eb.len) { throw_ffi(env, __eb); return NULL; }
    else { void (*fr)(FfiBuf_u8*)=(void(*)(FfiBuf_u8*))sym("boltffi_free_buf"); if(fr&&__eb.ptr) fr(&__eb); }
    return buf_to_array(env, c_return_out);
}

JNIEXPORT jbyteArray JNICALL Java_ai_xybrid_Native_boltffi_1method_1class_1xybrid_1bolt_1xybrid_1model_1stream_1result(JNIEnv *env, jclass cls, jlong j_receiver, jlong j_stream_id, jobject j_return_out) {
    LOGI("ENTER boltffi_method_class_xybrid_bolt_xybrid_model_stream_result");
    uint64_t c_receiver = (uint64_t)j_receiver;
    uint64_t c_stream_id = (uint64_t)j_stream_id;
    FfiBuf_u8 c_return_out; memset(&c_return_out, 0, sizeof(FfiBuf_u8));
    static void *fp; if (!fp) fp = sym("boltffi_method_class_xybrid_bolt_xybrid_model_stream_result");
    if (!fp) { jclass ex = (*env)->FindClass(env, "java/lang/RuntimeException"); if (ex) (*env)->ThrowNew(env, ex, "shim: missing symbol"); return 0; }
    FfiBuf_u8 __eb = ((FfiBuf_u8 (*)(uint64_t, uint64_t, FfiBuf_u8*))fp)(c_receiver, c_stream_id, &c_return_out);
    if (__eb.ptr && __eb.len) { throw_ffi(env, __eb); return NULL; }
    else { void (*fr)(FfiBuf_u8*)=(void(*)(FfiBuf_u8*))sym("boltffi_free_buf"); if(fr&&__eb.ptr) fr(&__eb); }
    return buf_to_array(env, c_return_out);
}

JNIEXPORT void JNICALL Java_ai_xybrid_Native_boltffi_1method_1class_1xybrid_1bolt_1xybrid_1model_1stream_1close(JNIEnv *env, jclass cls, jlong j_receiver, jlong j_stream_id) {
    LOGI("ENTER boltffi_method_class_xybrid_bolt_xybrid_model_stream_close");
    uint64_t c_receiver = (uint64_t)j_receiver;
    uint64_t c_stream_id = (uint64_t)j_stream_id;
    static void *fp; if (!fp) fp = sym("boltffi_method_class_xybrid_bolt_xybrid_model_stream_close");
    if (!fp) { jclass ex = (*env)->FindClass(env, "java/lang/RuntimeException"); if (ex) (*env)->ThrowNew(env, ex, "shim: missing symbol"); return; }
    FfiStatus __st = ((FfiStatus (*)(uint64_t, uint64_t))fp)(c_receiver, c_stream_id);
    if (__st.code != 0) { FfiBuf_u8 e={0}; throw_ffi(env, e); }
}

JNIEXPORT jbyteArray JNICALL Java_ai_xybrid_Native_boltffi_1method_1class_1xybrid_1bolt_1xybrid_1model_1run_1with_1context(JNIEnv *env, jclass cls, jlong j_receiver, jobject j_envelope_ptr, jint j_envelope_len, jlong j_context, jobject j_options_ptr, jint j_options_len, jobject j_return_out) {
    LOGI("ENTER boltffi_method_class_xybrid_bolt_xybrid_model_run_with_context");
    uint64_t c_receiver = (uint64_t)j_receiver;
    const uint8_t* c_envelope_ptr = j_envelope_ptr ? (const uint8_t*)(*env)->GetDirectBufferAddress(env, j_envelope_ptr) : NULL;
    uintptr_t c_envelope_len = (uintptr_t)j_envelope_len;
    LOGI("  boltffi_method_class_xybrid_bolt_xybrid_model_run_with_context: buf[envelope_ptr] len=%zu head=%02x %02x %02x %02x %02x %02x %02x %02x", (size_t)c_envelope_len, c_envelope_ptr[0], c_envelope_ptr[1], c_envelope_ptr[2], c_envelope_ptr[3], c_envelope_ptr[4], c_envelope_ptr[5], c_envelope_ptr[6], c_envelope_ptr[7]);
    uint64_t c_context = (uint64_t)j_context;
    const uint8_t* c_options_ptr = j_options_ptr ? (const uint8_t*)(*env)->GetDirectBufferAddress(env, j_options_ptr) : NULL;
    uintptr_t c_options_len = (uintptr_t)j_options_len;
    LOGI("  boltffi_method_class_xybrid_bolt_xybrid_model_run_with_context: buf[options_ptr] len=%zu head=%02x %02x %02x %02x %02x %02x %02x %02x", (size_t)c_options_len, c_options_ptr[0], c_options_ptr[1], c_options_ptr[2], c_options_ptr[3], c_options_ptr[4], c_options_ptr[5], c_options_ptr[6], c_options_ptr[7]);
    FfiBuf_u8 c_return_out; memset(&c_return_out, 0, sizeof(FfiBuf_u8));
    static void *fp; if (!fp) fp = sym("boltffi_method_class_xybrid_bolt_xybrid_model_run_with_context");
    if (!fp) { jclass ex = (*env)->FindClass(env, "java/lang/RuntimeException"); if (ex) (*env)->ThrowNew(env, ex, "shim: missing symbol"); return 0; }
    FfiBuf_u8 __eb = ((FfiBuf_u8 (*)(uint64_t, const uint8_t*, uintptr_t, uint64_t, const uint8_t*, uintptr_t, FfiBuf_u8*))fp)(c_receiver, c_envelope_ptr, c_envelope_len, c_context, c_options_ptr, c_options_len, &c_return_out);
    if (__eb.ptr && __eb.len) { throw_ffi(env, __eb); return NULL; }
    else { void (*fr)(FfiBuf_u8*)=(void(*)(FfiBuf_u8*))sym("boltffi_free_buf"); if(fr&&__eb.ptr) fr(&__eb); }
    return buf_to_array(env, c_return_out);
}

JNIEXPORT jlong JNICALL Java_ai_xybrid_Native_boltffi_1method_1class_1xybrid_1bolt_1xybrid_1model_1run_1stream_1with_1context(JNIEnv *env, jclass cls, jlong j_receiver, jobject j_envelope_ptr, jint j_envelope_len, jlong j_context, jobject j_options_ptr, jint j_options_len, jlong j_return_out) {
    LOGI("ENTER boltffi_method_class_xybrid_bolt_xybrid_model_run_stream_with_context");
    uint64_t c_receiver = (uint64_t)j_receiver;
    const uint8_t* c_envelope_ptr = j_envelope_ptr ? (const uint8_t*)(*env)->GetDirectBufferAddress(env, j_envelope_ptr) : NULL;
    uintptr_t c_envelope_len = (uintptr_t)j_envelope_len;
    LOGI("  boltffi_method_class_xybrid_bolt_xybrid_model_run_stream_with_context: buf[envelope_ptr] len=%zu head=%02x %02x %02x %02x %02x %02x %02x %02x", (size_t)c_envelope_len, c_envelope_ptr[0], c_envelope_ptr[1], c_envelope_ptr[2], c_envelope_ptr[3], c_envelope_ptr[4], c_envelope_ptr[5], c_envelope_ptr[6], c_envelope_ptr[7]);
    uint64_t c_context = (uint64_t)j_context;
    const uint8_t* c_options_ptr = j_options_ptr ? (const uint8_t*)(*env)->GetDirectBufferAddress(env, j_options_ptr) : NULL;
    uintptr_t c_options_len = (uintptr_t)j_options_len;
    LOGI("  boltffi_method_class_xybrid_bolt_xybrid_model_run_stream_with_context: buf[options_ptr] len=%zu head=%02x %02x %02x %02x %02x %02x %02x %02x", (size_t)c_options_len, c_options_ptr[0], c_options_ptr[1], c_options_ptr[2], c_options_ptr[3], c_options_ptr[4], c_options_ptr[5], c_options_ptr[6], c_options_ptr[7]);
    uint64_t c_return_out = 0;
    static void *fp; if (!fp) fp = sym("boltffi_method_class_xybrid_bolt_xybrid_model_run_stream_with_context");
    if (!fp) { jclass ex = (*env)->FindClass(env, "java/lang/RuntimeException"); if (ex) (*env)->ThrowNew(env, ex, "shim: missing symbol"); return 0; }
    FfiBuf_u8 __eb = ((FfiBuf_u8 (*)(uint64_t, const uint8_t*, uintptr_t, uint64_t, const uint8_t*, uintptr_t, uint64_t*))fp)(c_receiver, c_envelope_ptr, c_envelope_len, c_context, c_options_ptr, c_options_len, &c_return_out);
    if (__eb.ptr && __eb.len) { throw_ffi(env, __eb); return 0; } else { void (*fr)(FfiBuf_u8*)=(void(*)(FfiBuf_u8*))sym("boltffi_free_buf"); if(fr&&__eb.ptr) fr(&__eb); }
    return (jlong)c_return_out;
}

JNIEXPORT void JNICALL Java_ai_xybrid_Native_boltffi_1method_1class_1xybrid_1bolt_1xybrid_1model_1warmup(JNIEnv *env, jclass cls, jlong j_receiver) {
    LOGI("ENTER boltffi_method_class_xybrid_bolt_xybrid_model_warmup");
    uint64_t c_receiver = (uint64_t)j_receiver;
    static void *fp; if (!fp) fp = sym("boltffi_method_class_xybrid_bolt_xybrid_model_warmup");
    if (!fp) { jclass ex = (*env)->FindClass(env, "java/lang/RuntimeException"); if (ex) (*env)->ThrowNew(env, ex, "shim: missing symbol"); return; }
    FfiBuf_u8 __eb = ((FfiBuf_u8 (*)(uint64_t))fp)(c_receiver);
    if (__eb.ptr && __eb.len) { throw_ffi(env, __eb); } else { void (*fr)(FfiBuf_u8*)=(void(*)(FfiBuf_u8*))sym("boltffi_free_buf"); if(fr&&__eb.ptr) fr(&__eb); }
}

JNIEXPORT void JNICALL Java_ai_xybrid_Native_boltffi_1method_1class_1xybrid_1bolt_1xybrid_1model_1unload(JNIEnv *env, jclass cls, jlong j_receiver) {
    LOGI("ENTER boltffi_method_class_xybrid_bolt_xybrid_model_unload");
    uint64_t c_receiver = (uint64_t)j_receiver;
    static void *fp; if (!fp) fp = sym("boltffi_method_class_xybrid_bolt_xybrid_model_unload");
    if (!fp) { jclass ex = (*env)->FindClass(env, "java/lang/RuntimeException"); if (ex) (*env)->ThrowNew(env, ex, "shim: missing symbol"); return; }
    FfiBuf_u8 __eb = ((FfiBuf_u8 (*)(uint64_t))fp)(c_receiver);
    if (__eb.ptr && __eb.len) { throw_ffi(env, __eb); } else { void (*fr)(FfiBuf_u8*)=(void(*)(FfiBuf_u8*))sym("boltffi_free_buf"); if(fr&&__eb.ptr) fr(&__eb); }
}

JNIEXPORT void JNICALL Java_ai_xybrid_Native_boltffi_1release_1class_1xybrid_1bolt_1xybrid_1conversation_1context(JNIEnv *env, jclass cls, jlong j_handle) {
    LOGI("ENTER boltffi_release_class_xybrid_bolt_xybrid_conversation_context");
    uint64_t c_handle = (uint64_t)j_handle;
    static void *fp; if (!fp) fp = sym("boltffi_release_class_xybrid_bolt_xybrid_conversation_context");
    if (!fp) { jclass ex = (*env)->FindClass(env, "java/lang/RuntimeException"); if (ex) (*env)->ThrowNew(env, ex, "shim: missing symbol"); return; }
    ((void (*)(uint64_t))fp)(c_handle);
}

JNIEXPORT jlong JNICALL Java_ai_xybrid_Native_boltffi_1init_1class_1xybrid_1bolt_1xybrid_1conversation_1context_1new(JNIEnv *env, jclass cls) {
    LOGI("ENTER boltffi_init_class_xybrid_bolt_xybrid_conversation_context_new");
    static void *fp; if (!fp) fp = sym("boltffi_init_class_xybrid_bolt_xybrid_conversation_context_new");
    if (!fp) { jclass ex = (*env)->FindClass(env, "java/lang/RuntimeException"); if (ex) (*env)->ThrowNew(env, ex, "shim: missing symbol"); return 0; }
    return (jlong)((uint64_t (*)())fp)();
}

JNIEXPORT jlong JNICALL Java_ai_xybrid_Native_boltffi_1init_1class_1xybrid_1bolt_1xybrid_1conversation_1context_1with_1id(JNIEnv *env, jclass cls, jobject j_id_ptr, jint j_id_len) {
    LOGI("ENTER boltffi_init_class_xybrid_bolt_xybrid_conversation_context_with_id");
    const uint8_t* c_id_ptr = j_id_ptr ? (const uint8_t*)(*env)->GetDirectBufferAddress(env, j_id_ptr) : NULL;
    uintptr_t c_id_len = (uintptr_t)j_id_len;
    LOGI("  boltffi_init_class_xybrid_bolt_xybrid_conversation_context_with_id: buf[id_ptr] len=%zu head=%02x %02x %02x %02x %02x %02x %02x %02x", (size_t)c_id_len, c_id_ptr[0], c_id_ptr[1], c_id_ptr[2], c_id_ptr[3], c_id_ptr[4], c_id_ptr[5], c_id_ptr[6], c_id_ptr[7]);
    static void *fp; if (!fp) fp = sym("boltffi_init_class_xybrid_bolt_xybrid_conversation_context_with_id");
    if (!fp) { jclass ex = (*env)->FindClass(env, "java/lang/RuntimeException"); if (ex) (*env)->ThrowNew(env, ex, "shim: missing symbol"); return 0; }
    return (jlong)((uint64_t (*)(const uint8_t*, uintptr_t))fp)(c_id_ptr, c_id_len);
}

JNIEXPORT void JNICALL Java_ai_xybrid_Native_boltffi_1method_1class_1xybrid_1bolt_1xybrid_1conversation_1context_1push(JNIEnv *env, jclass cls, jlong j_receiver, jobject j_envelope_ptr, jint j_envelope_len) {
    LOGI("ENTER boltffi_method_class_xybrid_bolt_xybrid_conversation_context_push");
    uint64_t c_receiver = (uint64_t)j_receiver;
    const uint8_t* c_envelope_ptr = j_envelope_ptr ? (const uint8_t*)(*env)->GetDirectBufferAddress(env, j_envelope_ptr) : NULL;
    uintptr_t c_envelope_len = (uintptr_t)j_envelope_len;
    LOGI("  boltffi_method_class_xybrid_bolt_xybrid_conversation_context_push: buf[envelope_ptr] len=%zu head=%02x %02x %02x %02x %02x %02x %02x %02x", (size_t)c_envelope_len, c_envelope_ptr[0], c_envelope_ptr[1], c_envelope_ptr[2], c_envelope_ptr[3], c_envelope_ptr[4], c_envelope_ptr[5], c_envelope_ptr[6], c_envelope_ptr[7]);
    static void *fp; if (!fp) fp = sym("boltffi_method_class_xybrid_bolt_xybrid_conversation_context_push");
    if (!fp) { jclass ex = (*env)->FindClass(env, "java/lang/RuntimeException"); if (ex) (*env)->ThrowNew(env, ex, "shim: missing symbol"); return; }
    FfiBuf_u8 __eb = ((FfiBuf_u8 (*)(uint64_t, const uint8_t*, uintptr_t))fp)(c_receiver, c_envelope_ptr, c_envelope_len);
    if (__eb.ptr && __eb.len) { throw_ffi(env, __eb); } else { void (*fr)(FfiBuf_u8*)=(void(*)(FfiBuf_u8*))sym("boltffi_free_buf"); if(fr&&__eb.ptr) fr(&__eb); }
}

JNIEXPORT void JNICALL Java_ai_xybrid_Native_boltffi_1method_1class_1xybrid_1bolt_1xybrid_1conversation_1context_1set_1system(JNIEnv *env, jclass cls, jlong j_receiver, jobject j_envelope_ptr, jint j_envelope_len) {
    LOGI("ENTER boltffi_method_class_xybrid_bolt_xybrid_conversation_context_set_system");
    uint64_t c_receiver = (uint64_t)j_receiver;
    const uint8_t* c_envelope_ptr = j_envelope_ptr ? (const uint8_t*)(*env)->GetDirectBufferAddress(env, j_envelope_ptr) : NULL;
    uintptr_t c_envelope_len = (uintptr_t)j_envelope_len;
    LOGI("  boltffi_method_class_xybrid_bolt_xybrid_conversation_context_set_system: buf[envelope_ptr] len=%zu head=%02x %02x %02x %02x %02x %02x %02x %02x", (size_t)c_envelope_len, c_envelope_ptr[0], c_envelope_ptr[1], c_envelope_ptr[2], c_envelope_ptr[3], c_envelope_ptr[4], c_envelope_ptr[5], c_envelope_ptr[6], c_envelope_ptr[7]);
    static void *fp; if (!fp) fp = sym("boltffi_method_class_xybrid_bolt_xybrid_conversation_context_set_system");
    if (!fp) { jclass ex = (*env)->FindClass(env, "java/lang/RuntimeException"); if (ex) (*env)->ThrowNew(env, ex, "shim: missing symbol"); return; }
    FfiBuf_u8 __eb = ((FfiBuf_u8 (*)(uint64_t, const uint8_t*, uintptr_t))fp)(c_receiver, c_envelope_ptr, c_envelope_len);
    if (__eb.ptr && __eb.len) { throw_ffi(env, __eb); } else { void (*fr)(FfiBuf_u8*)=(void(*)(FfiBuf_u8*))sym("boltffi_free_buf"); if(fr&&__eb.ptr) fr(&__eb); }
}

JNIEXPORT void JNICALL Java_ai_xybrid_Native_boltffi_1method_1class_1xybrid_1bolt_1xybrid_1conversation_1context_1clear(JNIEnv *env, jclass cls, jlong j_receiver) {
    LOGI("ENTER boltffi_method_class_xybrid_bolt_xybrid_conversation_context_clear");
    uint64_t c_receiver = (uint64_t)j_receiver;
    static void *fp; if (!fp) fp = sym("boltffi_method_class_xybrid_bolt_xybrid_conversation_context_clear");
    if (!fp) { jclass ex = (*env)->FindClass(env, "java/lang/RuntimeException"); if (ex) (*env)->ThrowNew(env, ex, "shim: missing symbol"); return; }
    FfiStatus __st = ((FfiStatus (*)(uint64_t))fp)(c_receiver);
    if (__st.code != 0) { FfiBuf_u8 e={0}; throw_ffi(env, e); }
}

JNIEXPORT jbyteArray JNICALL Java_ai_xybrid_Native_boltffi_1method_1class_1xybrid_1bolt_1xybrid_1conversation_1context_1id(JNIEnv *env, jclass cls, jlong j_receiver) {
    LOGI("ENTER boltffi_method_class_xybrid_bolt_xybrid_conversation_context_id");
    uint64_t c_receiver = (uint64_t)j_receiver;
    static void *fp; if (!fp) fp = sym("boltffi_method_class_xybrid_bolt_xybrid_conversation_context_id");
    if (!fp) { jclass ex = (*env)->FindClass(env, "java/lang/RuntimeException"); if (ex) (*env)->ThrowNew(env, ex, "shim: missing symbol"); return 0; }
    ((FfiBuf_u8 (*)(uint64_t))fp)(c_receiver);
}

JNIEXPORT jint JNICALL Java_ai_xybrid_Native_boltffi_1method_1class_1xybrid_1bolt_1xybrid_1conversation_1context_1history_1len(JNIEnv *env, jclass cls, jlong j_receiver) {
    LOGI("ENTER boltffi_method_class_xybrid_bolt_xybrid_conversation_context_history_len");
    uint64_t c_receiver = (uint64_t)j_receiver;
    static void *fp; if (!fp) fp = sym("boltffi_method_class_xybrid_bolt_xybrid_conversation_context_history_len");
    if (!fp) { jclass ex = (*env)->FindClass(env, "java/lang/RuntimeException"); if (ex) (*env)->ThrowNew(env, ex, "shim: missing symbol"); return 0; }
    return (jint)((uint32_t (*)(uint64_t))fp)(c_receiver);
}

JNIEXPORT jbyteArray JNICALL Java_ai_xybrid_Native_boltffi_1method_1class_1xybrid_1bolt_1xybrid_1conversation_1context_1history(JNIEnv *env, jclass cls, jlong j_receiver) {
    LOGI("ENTER boltffi_method_class_xybrid_bolt_xybrid_conversation_context_history");
    uint64_t c_receiver = (uint64_t)j_receiver;
    static void *fp; if (!fp) fp = sym("boltffi_method_class_xybrid_bolt_xybrid_conversation_context_history");
    if (!fp) { jclass ex = (*env)->FindClass(env, "java/lang/RuntimeException"); if (ex) (*env)->ThrowNew(env, ex, "shim: missing symbol"); return 0; }
    ((FfiBuf_u8 (*)(uint64_t))fp)(c_receiver);
}

JNIEXPORT jboolean JNICALL Java_ai_xybrid_Native_boltffi_1method_1class_1xybrid_1bolt_1xybrid_1conversation_1context_1has_1system(JNIEnv *env, jclass cls, jlong j_receiver) {
    LOGI("ENTER boltffi_method_class_xybrid_bolt_xybrid_conversation_context_has_system");
    uint64_t c_receiver = (uint64_t)j_receiver;
    static void *fp; if (!fp) fp = sym("boltffi_method_class_xybrid_bolt_xybrid_conversation_context_has_system");
    if (!fp) { jclass ex = (*env)->FindClass(env, "java/lang/RuntimeException"); if (ex) (*env)->ThrowNew(env, ex, "shim: missing symbol"); return 0; }
    return (jboolean)((bool (*)(uint64_t))fp)(c_receiver);
}

JNIEXPORT void JNICALL Java_ai_xybrid_Native_boltffi_1method_1class_1xybrid_1bolt_1xybrid_1conversation_1context_1set_1max_1history_1len(JNIEnv *env, jclass cls, jlong j_receiver, jint j_len) {
    LOGI("ENTER boltffi_method_class_xybrid_bolt_xybrid_conversation_context_set_max_history_len");
    uint64_t c_receiver = (uint64_t)j_receiver;
    uint32_t c_len = (uint32_t)j_len;
    static void *fp; if (!fp) fp = sym("boltffi_method_class_xybrid_bolt_xybrid_conversation_context_set_max_history_len");
    if (!fp) { jclass ex = (*env)->FindClass(env, "java/lang/RuntimeException"); if (ex) (*env)->ThrowNew(env, ex, "shim: missing symbol"); return; }
    FfiStatus __st = ((FfiStatus (*)(uint64_t, uint32_t))fp)(c_receiver, c_len);
    if (__st.code != 0) { FfiBuf_u8 e={0}; throw_ffi(env, e); }
}

JNIEXPORT void JNICALL Java_ai_xybrid_Native_boltffi_1release_1class_1xybrid_1bolt_1xybrid_1telemetry_1config(JNIEnv *env, jclass cls, jlong j_handle) {
    LOGI("ENTER boltffi_release_class_xybrid_bolt_xybrid_telemetry_config");
    uint64_t c_handle = (uint64_t)j_handle;
    static void *fp; if (!fp) fp = sym("boltffi_release_class_xybrid_bolt_xybrid_telemetry_config");
    if (!fp) { jclass ex = (*env)->FindClass(env, "java/lang/RuntimeException"); if (ex) (*env)->ThrowNew(env, ex, "shim: missing symbol"); return; }
    ((void (*)(uint64_t))fp)(c_handle);
}

JNIEXPORT jlong JNICALL Java_ai_xybrid_Native_boltffi_1init_1class_1xybrid_1bolt_1xybrid_1telemetry_1config_1new(JNIEnv *env, jclass cls, jobject j_api_key_ptr, jint j_api_key_len) {
    LOGI("ENTER boltffi_init_class_xybrid_bolt_xybrid_telemetry_config_new");
    const uint8_t* c_api_key_ptr = j_api_key_ptr ? (const uint8_t*)(*env)->GetDirectBufferAddress(env, j_api_key_ptr) : NULL;
    uintptr_t c_api_key_len = (uintptr_t)j_api_key_len;
    LOGI("  boltffi_init_class_xybrid_bolt_xybrid_telemetry_config_new: buf[api_key_ptr] len=%zu head=%02x %02x %02x %02x %02x %02x %02x %02x", (size_t)c_api_key_len, c_api_key_ptr[0], c_api_key_ptr[1], c_api_key_ptr[2], c_api_key_ptr[3], c_api_key_ptr[4], c_api_key_ptr[5], c_api_key_ptr[6], c_api_key_ptr[7]);
    static void *fp; if (!fp) fp = sym("boltffi_init_class_xybrid_bolt_xybrid_telemetry_config_new");
    if (!fp) { jclass ex = (*env)->FindClass(env, "java/lang/RuntimeException"); if (ex) (*env)->ThrowNew(env, ex, "shim: missing symbol"); return 0; }
    return (jlong)((uint64_t (*)(const uint8_t*, uintptr_t))fp)(c_api_key_ptr, c_api_key_len);
}

JNIEXPORT void JNICALL Java_ai_xybrid_Native_boltffi_1method_1class_1xybrid_1bolt_1xybrid_1telemetry_1config_1set_1endpoint(JNIEnv *env, jclass cls, jlong j_receiver, jobject j_endpoint_ptr, jint j_endpoint_len) {
    LOGI("ENTER boltffi_method_class_xybrid_bolt_xybrid_telemetry_config_set_endpoint");
    uint64_t c_receiver = (uint64_t)j_receiver;
    const uint8_t* c_endpoint_ptr = j_endpoint_ptr ? (const uint8_t*)(*env)->GetDirectBufferAddress(env, j_endpoint_ptr) : NULL;
    uintptr_t c_endpoint_len = (uintptr_t)j_endpoint_len;
    LOGI("  boltffi_method_class_xybrid_bolt_xybrid_telemetry_config_set_endpoint: buf[endpoint_ptr] len=%zu head=%02x %02x %02x %02x %02x %02x %02x %02x", (size_t)c_endpoint_len, c_endpoint_ptr[0], c_endpoint_ptr[1], c_endpoint_ptr[2], c_endpoint_ptr[3], c_endpoint_ptr[4], c_endpoint_ptr[5], c_endpoint_ptr[6], c_endpoint_ptr[7]);
    static void *fp; if (!fp) fp = sym("boltffi_method_class_xybrid_bolt_xybrid_telemetry_config_set_endpoint");
    if (!fp) { jclass ex = (*env)->FindClass(env, "java/lang/RuntimeException"); if (ex) (*env)->ThrowNew(env, ex, "shim: missing symbol"); return; }
    FfiStatus __st = ((FfiStatus (*)(uint64_t, const uint8_t*, uintptr_t))fp)(c_receiver, c_endpoint_ptr, c_endpoint_len);
    if (__st.code != 0) { FfiBuf_u8 e={0}; throw_ffi(env, e); }
}

JNIEXPORT void JNICALL Java_ai_xybrid_Native_boltffi_1method_1class_1xybrid_1bolt_1xybrid_1telemetry_1config_1set_1app_1version(JNIEnv *env, jclass cls, jlong j_receiver, jobject j_version_ptr, jint j_version_len) {
    LOGI("ENTER boltffi_method_class_xybrid_bolt_xybrid_telemetry_config_set_app_version");
    uint64_t c_receiver = (uint64_t)j_receiver;
    const uint8_t* c_version_ptr = j_version_ptr ? (const uint8_t*)(*env)->GetDirectBufferAddress(env, j_version_ptr) : NULL;
    uintptr_t c_version_len = (uintptr_t)j_version_len;
    LOGI("  boltffi_method_class_xybrid_bolt_xybrid_telemetry_config_set_app_version: buf[version_ptr] len=%zu head=%02x %02x %02x %02x %02x %02x %02x %02x", (size_t)c_version_len, c_version_ptr[0], c_version_ptr[1], c_version_ptr[2], c_version_ptr[3], c_version_ptr[4], c_version_ptr[5], c_version_ptr[6], c_version_ptr[7]);
    static void *fp; if (!fp) fp = sym("boltffi_method_class_xybrid_bolt_xybrid_telemetry_config_set_app_version");
    if (!fp) { jclass ex = (*env)->FindClass(env, "java/lang/RuntimeException"); if (ex) (*env)->ThrowNew(env, ex, "shim: missing symbol"); return; }
    FfiStatus __st = ((FfiStatus (*)(uint64_t, const uint8_t*, uintptr_t))fp)(c_receiver, c_version_ptr, c_version_len);
    if (__st.code != 0) { FfiBuf_u8 e={0}; throw_ffi(env, e); }
}

JNIEXPORT void JNICALL Java_ai_xybrid_Native_boltffi_1method_1class_1xybrid_1bolt_1xybrid_1telemetry_1config_1set_1device_1label(JNIEnv *env, jclass cls, jlong j_receiver, jobject j_label_ptr, jint j_label_len) {
    LOGI("ENTER boltffi_method_class_xybrid_bolt_xybrid_telemetry_config_set_device_label");
    uint64_t c_receiver = (uint64_t)j_receiver;
    const uint8_t* c_label_ptr = j_label_ptr ? (const uint8_t*)(*env)->GetDirectBufferAddress(env, j_label_ptr) : NULL;
    uintptr_t c_label_len = (uintptr_t)j_label_len;
    LOGI("  boltffi_method_class_xybrid_bolt_xybrid_telemetry_config_set_device_label: buf[label_ptr] len=%zu head=%02x %02x %02x %02x %02x %02x %02x %02x", (size_t)c_label_len, c_label_ptr[0], c_label_ptr[1], c_label_ptr[2], c_label_ptr[3], c_label_ptr[4], c_label_ptr[5], c_label_ptr[6], c_label_ptr[7]);
    static void *fp; if (!fp) fp = sym("boltffi_method_class_xybrid_bolt_xybrid_telemetry_config_set_device_label");
    if (!fp) { jclass ex = (*env)->FindClass(env, "java/lang/RuntimeException"); if (ex) (*env)->ThrowNew(env, ex, "shim: missing symbol"); return; }
    FfiStatus __st = ((FfiStatus (*)(uint64_t, const uint8_t*, uintptr_t))fp)(c_receiver, c_label_ptr, c_label_len);
    if (__st.code != 0) { FfiBuf_u8 e={0}; throw_ffi(env, e); }
}

JNIEXPORT void JNICALL Java_ai_xybrid_Native_boltffi_1method_1class_1xybrid_1bolt_1xybrid_1telemetry_1config_1set_1device_1attribute(JNIEnv *env, jclass cls, jlong j_receiver, jobject j_key_ptr, jint j_key_len, jobject j_value_ptr, jint j_value_len) {
    LOGI("ENTER boltffi_method_class_xybrid_bolt_xybrid_telemetry_config_set_device_attribute");
    uint64_t c_receiver = (uint64_t)j_receiver;
    const uint8_t* c_key_ptr = j_key_ptr ? (const uint8_t*)(*env)->GetDirectBufferAddress(env, j_key_ptr) : NULL;
    uintptr_t c_key_len = (uintptr_t)j_key_len;
    LOGI("  boltffi_method_class_xybrid_bolt_xybrid_telemetry_config_set_device_attribute: buf[key_ptr] len=%zu head=%02x %02x %02x %02x %02x %02x %02x %02x", (size_t)c_key_len, c_key_ptr[0], c_key_ptr[1], c_key_ptr[2], c_key_ptr[3], c_key_ptr[4], c_key_ptr[5], c_key_ptr[6], c_key_ptr[7]);
    const uint8_t* c_value_ptr = j_value_ptr ? (const uint8_t*)(*env)->GetDirectBufferAddress(env, j_value_ptr) : NULL;
    uintptr_t c_value_len = (uintptr_t)j_value_len;
    LOGI("  boltffi_method_class_xybrid_bolt_xybrid_telemetry_config_set_device_attribute: buf[value_ptr] len=%zu head=%02x %02x %02x %02x %02x %02x %02x %02x", (size_t)c_value_len, c_value_ptr[0], c_value_ptr[1], c_value_ptr[2], c_value_ptr[3], c_value_ptr[4], c_value_ptr[5], c_value_ptr[6], c_value_ptr[7]);
    static void *fp; if (!fp) fp = sym("boltffi_method_class_xybrid_bolt_xybrid_telemetry_config_set_device_attribute");
    if (!fp) { jclass ex = (*env)->FindClass(env, "java/lang/RuntimeException"); if (ex) (*env)->ThrowNew(env, ex, "shim: missing symbol"); return; }
    FfiStatus __st = ((FfiStatus (*)(uint64_t, const uint8_t*, uintptr_t, const uint8_t*, uintptr_t))fp)(c_receiver, c_key_ptr, c_key_len, c_value_ptr, c_value_len);
    if (__st.code != 0) { FfiBuf_u8 e={0}; throw_ffi(env, e); }
}

JNIEXPORT void JNICALL Java_ai_xybrid_Native_boltffi_1method_1class_1xybrid_1bolt_1xybrid_1telemetry_1config_1set_1batch_1size(JNIEnv *env, jclass cls, jlong j_receiver, jint j_batch_size) {
    LOGI("ENTER boltffi_method_class_xybrid_bolt_xybrid_telemetry_config_set_batch_size");
    uint64_t c_receiver = (uint64_t)j_receiver;
    uint32_t c_batch_size = (uint32_t)j_batch_size;
    static void *fp; if (!fp) fp = sym("boltffi_method_class_xybrid_bolt_xybrid_telemetry_config_set_batch_size");
    if (!fp) { jclass ex = (*env)->FindClass(env, "java/lang/RuntimeException"); if (ex) (*env)->ThrowNew(env, ex, "shim: missing symbol"); return; }
    FfiStatus __st = ((FfiStatus (*)(uint64_t, uint32_t))fp)(c_receiver, c_batch_size);
    if (__st.code != 0) { FfiBuf_u8 e={0}; throw_ffi(env, e); }
}

JNIEXPORT void JNICALL Java_ai_xybrid_Native_boltffi_1method_1class_1xybrid_1bolt_1xybrid_1telemetry_1config_1set_1flush_1interval_1secs(JNIEnv *env, jclass cls, jlong j_receiver, jint j_secs) {
    LOGI("ENTER boltffi_method_class_xybrid_bolt_xybrid_telemetry_config_set_flush_interval_secs");
    uint64_t c_receiver = (uint64_t)j_receiver;
    uint32_t c_secs = (uint32_t)j_secs;
    static void *fp; if (!fp) fp = sym("boltffi_method_class_xybrid_bolt_xybrid_telemetry_config_set_flush_interval_secs");
    if (!fp) { jclass ex = (*env)->FindClass(env, "java/lang/RuntimeException"); if (ex) (*env)->ThrowNew(env, ex, "shim: missing symbol"); return; }
    FfiStatus __st = ((FfiStatus (*)(uint64_t, uint32_t))fp)(c_receiver, c_secs);
    if (__st.code != 0) { FfiBuf_u8 e={0}; throw_ffi(env, e); }
}

JNIEXPORT void JNICALL Java_ai_xybrid_Native_boltffi_1method_1class_1xybrid_1bolt_1xybrid_1telemetry_1config_1init(JNIEnv *env, jclass cls, jlong j_receiver) {
    LOGI("ENTER boltffi_method_class_xybrid_bolt_xybrid_telemetry_config_init");
    uint64_t c_receiver = (uint64_t)j_receiver;
    static void *fp; if (!fp) fp = sym("boltffi_method_class_xybrid_bolt_xybrid_telemetry_config_init");
    if (!fp) { jclass ex = (*env)->FindClass(env, "java/lang/RuntimeException"); if (ex) (*env)->ThrowNew(env, ex, "shim: missing symbol"); return; }
    FfiBuf_u8 __eb = ((FfiBuf_u8 (*)(uint64_t))fp)(c_receiver);
    if (__eb.ptr && __eb.len) { throw_ffi(env, __eb); } else { void (*fr)(FfiBuf_u8*)=(void(*)(FfiBuf_u8*))sym("boltffi_free_buf"); if(fr&&__eb.ptr) fr(&__eb); }
}

JNIEXPORT void JNICALL Java_ai_xybrid_Native_boltffi_1release_1class_1xybrid_1bolt_1xybrid_1bundle(JNIEnv *env, jclass cls, jlong j_handle) {
    LOGI("ENTER boltffi_release_class_xybrid_bolt_xybrid_bundle");
    uint64_t c_handle = (uint64_t)j_handle;
    static void *fp; if (!fp) fp = sym("boltffi_release_class_xybrid_bolt_xybrid_bundle");
    if (!fp) { jclass ex = (*env)->FindClass(env, "java/lang/RuntimeException"); if (ex) (*env)->ThrowNew(env, ex, "shim: missing symbol"); return; }
    ((void (*)(uint64_t))fp)(c_handle);
}

JNIEXPORT jlong JNICALL Java_ai_xybrid_Native_boltffi_1init_1class_1xybrid_1bolt_1xybrid_1bundle_1open(JNIEnv *env, jclass cls, jobject j_path_ptr, jint j_path_len, jlong j_return_out) {
    LOGI("ENTER boltffi_init_class_xybrid_bolt_xybrid_bundle_open");
    const uint8_t* c_path_ptr = j_path_ptr ? (const uint8_t*)(*env)->GetDirectBufferAddress(env, j_path_ptr) : NULL;
    uintptr_t c_path_len = (uintptr_t)j_path_len;
    LOGI("  boltffi_init_class_xybrid_bolt_xybrid_bundle_open: buf[path_ptr] len=%zu head=%02x %02x %02x %02x %02x %02x %02x %02x", (size_t)c_path_len, c_path_ptr[0], c_path_ptr[1], c_path_ptr[2], c_path_ptr[3], c_path_ptr[4], c_path_ptr[5], c_path_ptr[6], c_path_ptr[7]);
    uint64_t c_return_out = 0;
    static void *fp; if (!fp) fp = sym("boltffi_init_class_xybrid_bolt_xybrid_bundle_open");
    if (!fp) { jclass ex = (*env)->FindClass(env, "java/lang/RuntimeException"); if (ex) (*env)->ThrowNew(env, ex, "shim: missing symbol"); return 0; }
    FfiBuf_u8 __eb = ((FfiBuf_u8 (*)(const uint8_t*, uintptr_t, uint64_t*))fp)(c_path_ptr, c_path_len, &c_return_out);
    if (__eb.ptr && __eb.len) { throw_ffi(env, __eb); return 0; } else { void (*fr)(FfiBuf_u8*)=(void(*)(FfiBuf_u8*))sym("boltffi_free_buf"); if(fr&&__eb.ptr) fr(&__eb); }
    return (jlong)c_return_out;
}

JNIEXPORT jbyteArray JNICALL Java_ai_xybrid_Native_boltffi_1method_1class_1xybrid_1bolt_1xybrid_1bundle_1model_1id(JNIEnv *env, jclass cls, jlong j_receiver) {
    LOGI("ENTER boltffi_method_class_xybrid_bolt_xybrid_bundle_model_id");
    uint64_t c_receiver = (uint64_t)j_receiver;
    static void *fp; if (!fp) fp = sym("boltffi_method_class_xybrid_bolt_xybrid_bundle_model_id");
    if (!fp) { jclass ex = (*env)->FindClass(env, "java/lang/RuntimeException"); if (ex) (*env)->ThrowNew(env, ex, "shim: missing symbol"); return 0; }
    ((FfiBuf_u8 (*)(uint64_t))fp)(c_receiver);
}

JNIEXPORT jbyteArray JNICALL Java_ai_xybrid_Native_boltffi_1method_1class_1xybrid_1bolt_1xybrid_1bundle_1version(JNIEnv *env, jclass cls, jlong j_receiver) {
    LOGI("ENTER boltffi_method_class_xybrid_bolt_xybrid_bundle_version");
    uint64_t c_receiver = (uint64_t)j_receiver;
    static void *fp; if (!fp) fp = sym("boltffi_method_class_xybrid_bolt_xybrid_bundle_version");
    if (!fp) { jclass ex = (*env)->FindClass(env, "java/lang/RuntimeException"); if (ex) (*env)->ThrowNew(env, ex, "shim: missing symbol"); return 0; }
    ((FfiBuf_u8 (*)(uint64_t))fp)(c_receiver);
}

JNIEXPORT jbyteArray JNICALL Java_ai_xybrid_Native_boltffi_1method_1class_1xybrid_1bolt_1xybrid_1bundle_1target(JNIEnv *env, jclass cls, jlong j_receiver) {
    LOGI("ENTER boltffi_method_class_xybrid_bolt_xybrid_bundle_target");
    uint64_t c_receiver = (uint64_t)j_receiver;
    static void *fp; if (!fp) fp = sym("boltffi_method_class_xybrid_bolt_xybrid_bundle_target");
    if (!fp) { jclass ex = (*env)->FindClass(env, "java/lang/RuntimeException"); if (ex) (*env)->ThrowNew(env, ex, "shim: missing symbol"); return 0; }
    ((FfiBuf_u8 (*)(uint64_t))fp)(c_receiver);
}

JNIEXPORT jbyteArray JNICALL Java_ai_xybrid_Native_boltffi_1method_1class_1xybrid_1bolt_1xybrid_1bundle_1hash(JNIEnv *env, jclass cls, jlong j_receiver) {
    LOGI("ENTER boltffi_method_class_xybrid_bolt_xybrid_bundle_hash");
    uint64_t c_receiver = (uint64_t)j_receiver;
    static void *fp; if (!fp) fp = sym("boltffi_method_class_xybrid_bolt_xybrid_bundle_hash");
    if (!fp) { jclass ex = (*env)->FindClass(env, "java/lang/RuntimeException"); if (ex) (*env)->ThrowNew(env, ex, "shim: missing symbol"); return 0; }
    ((FfiBuf_u8 (*)(uint64_t))fp)(c_receiver);
}

JNIEXPORT jboolean JNICALL Java_ai_xybrid_Native_boltffi_1method_1class_1xybrid_1bolt_1xybrid_1bundle_1has_1metadata(JNIEnv *env, jclass cls, jlong j_receiver) {
    LOGI("ENTER boltffi_method_class_xybrid_bolt_xybrid_bundle_has_metadata");
    uint64_t c_receiver = (uint64_t)j_receiver;
    static void *fp; if (!fp) fp = sym("boltffi_method_class_xybrid_bolt_xybrid_bundle_has_metadata");
    if (!fp) { jclass ex = (*env)->FindClass(env, "java/lang/RuntimeException"); if (ex) (*env)->ThrowNew(env, ex, "shim: missing symbol"); return 0; }
    return (jboolean)((bool (*)(uint64_t))fp)(c_receiver);
}

JNIEXPORT jint JNICALL Java_ai_xybrid_Native_boltffi_1method_1class_1xybrid_1bolt_1xybrid_1bundle_1file_1count(JNIEnv *env, jclass cls, jlong j_receiver) {
    LOGI("ENTER boltffi_method_class_xybrid_bolt_xybrid_bundle_file_count");
    uint64_t c_receiver = (uint64_t)j_receiver;
    static void *fp; if (!fp) fp = sym("boltffi_method_class_xybrid_bolt_xybrid_bundle_file_count");
    if (!fp) { jclass ex = (*env)->FindClass(env, "java/lang/RuntimeException"); if (ex) (*env)->ThrowNew(env, ex, "shim: missing symbol"); return 0; }
    return (jint)((uint32_t (*)(uint64_t))fp)(c_receiver);
}

JNIEXPORT jbyteArray JNICALL Java_ai_xybrid_Native_boltffi_1method_1class_1xybrid_1bolt_1xybrid_1bundle_1file_1name(JNIEnv *env, jclass cls, jlong j_receiver, jint j_index) {
    LOGI("ENTER boltffi_method_class_xybrid_bolt_xybrid_bundle_file_name");
    uint64_t c_receiver = (uint64_t)j_receiver;
    uint32_t c_index = (uint32_t)j_index;
    static void *fp; if (!fp) fp = sym("boltffi_method_class_xybrid_bolt_xybrid_bundle_file_name");
    if (!fp) { jclass ex = (*env)->FindClass(env, "java/lang/RuntimeException"); if (ex) (*env)->ThrowNew(env, ex, "shim: missing symbol"); return 0; }
    ((FfiBuf_u8 (*)(uint64_t, uint32_t))fp)(c_receiver, c_index);
}

JNIEXPORT jbyteArray JNICALL Java_ai_xybrid_Native_boltffi_1method_1class_1xybrid_1bolt_1xybrid_1bundle_1manifest_1json(JNIEnv *env, jclass cls, jlong j_receiver, jobject j_return_out) {
    LOGI("ENTER boltffi_method_class_xybrid_bolt_xybrid_bundle_manifest_json");
    uint64_t c_receiver = (uint64_t)j_receiver;
    FfiBuf_u8 c_return_out; memset(&c_return_out, 0, sizeof(FfiBuf_u8));
    static void *fp; if (!fp) fp = sym("boltffi_method_class_xybrid_bolt_xybrid_bundle_manifest_json");
    if (!fp) { jclass ex = (*env)->FindClass(env, "java/lang/RuntimeException"); if (ex) (*env)->ThrowNew(env, ex, "shim: missing symbol"); return 0; }
    FfiBuf_u8 __eb = ((FfiBuf_u8 (*)(uint64_t, FfiBuf_u8*))fp)(c_receiver, &c_return_out);
    if (__eb.ptr && __eb.len) { throw_ffi(env, __eb); return NULL; }
    else { void (*fr)(FfiBuf_u8*)=(void(*)(FfiBuf_u8*))sym("boltffi_free_buf"); if(fr&&__eb.ptr) fr(&__eb); }
    return buf_to_array(env, c_return_out);
}

JNIEXPORT jbyteArray JNICALL Java_ai_xybrid_Native_boltffi_1method_1class_1xybrid_1bolt_1xybrid_1bundle_1metadata_1json(JNIEnv *env, jclass cls, jlong j_receiver, jobject j_return_out) {
    LOGI("ENTER boltffi_method_class_xybrid_bolt_xybrid_bundle_metadata_json");
    uint64_t c_receiver = (uint64_t)j_receiver;
    FfiBuf_u8 c_return_out; memset(&c_return_out, 0, sizeof(FfiBuf_u8));
    static void *fp; if (!fp) fp = sym("boltffi_method_class_xybrid_bolt_xybrid_bundle_metadata_json");
    if (!fp) { jclass ex = (*env)->FindClass(env, "java/lang/RuntimeException"); if (ex) (*env)->ThrowNew(env, ex, "shim: missing symbol"); return 0; }
    FfiBuf_u8 __eb = ((FfiBuf_u8 (*)(uint64_t, FfiBuf_u8*))fp)(c_receiver, &c_return_out);
    if (__eb.ptr && __eb.len) { throw_ffi(env, __eb); return NULL; }
    else { void (*fr)(FfiBuf_u8*)=(void(*)(FfiBuf_u8*))sym("boltffi_free_buf"); if(fr&&__eb.ptr) fr(&__eb); }
    return buf_to_array(env, c_return_out);
}

JNIEXPORT void JNICALL Java_ai_xybrid_Native_boltffi_1method_1class_1xybrid_1bolt_1xybrid_1bundle_1extract(JNIEnv *env, jclass cls, jlong j_receiver, jobject j_output_dir_ptr, jint j_output_dir_len) {
    LOGI("ENTER boltffi_method_class_xybrid_bolt_xybrid_bundle_extract");
    uint64_t c_receiver = (uint64_t)j_receiver;
    const uint8_t* c_output_dir_ptr = j_output_dir_ptr ? (const uint8_t*)(*env)->GetDirectBufferAddress(env, j_output_dir_ptr) : NULL;
    uintptr_t c_output_dir_len = (uintptr_t)j_output_dir_len;
    LOGI("  boltffi_method_class_xybrid_bolt_xybrid_bundle_extract: buf[output_dir_ptr] len=%zu head=%02x %02x %02x %02x %02x %02x %02x %02x", (size_t)c_output_dir_len, c_output_dir_ptr[0], c_output_dir_ptr[1], c_output_dir_ptr[2], c_output_dir_ptr[3], c_output_dir_ptr[4], c_output_dir_ptr[5], c_output_dir_ptr[6], c_output_dir_ptr[7]);
    static void *fp; if (!fp) fp = sym("boltffi_method_class_xybrid_bolt_xybrid_bundle_extract");
    if (!fp) { jclass ex = (*env)->FindClass(env, "java/lang/RuntimeException"); if (ex) (*env)->ThrowNew(env, ex, "shim: missing symbol"); return; }
    FfiBuf_u8 __eb = ((FfiBuf_u8 (*)(uint64_t, const uint8_t*, uintptr_t))fp)(c_receiver, c_output_dir_ptr, c_output_dir_len);
    if (__eb.ptr && __eb.len) { throw_ffi(env, __eb); } else { void (*fr)(FfiBuf_u8*)=(void(*)(FfiBuf_u8*))sym("boltffi_free_buf"); if(fr&&__eb.ptr) fr(&__eb); }
}

JNIEXPORT jbyteArray JNICALL Java_ai_xybrid_Native_boltffi_1function_1xybrid_1bolt_1tool_1results_1envelope(JNIEnv *env, jclass cls, jobject j_user_text_ptr, jint j_user_text_len, jobject j_prior_assistant_text_ptr, jint j_prior_assistant_text_len, jobject j_results_ptr, jint j_results_len, jobject j_return_out) {
    LOGI("ENTER boltffi_function_xybrid_bolt_tool_results_envelope");
    const uint8_t* c_user_text_ptr = j_user_text_ptr ? (const uint8_t*)(*env)->GetDirectBufferAddress(env, j_user_text_ptr) : NULL;
    uintptr_t c_user_text_len = (uintptr_t)j_user_text_len;
    LOGI("  boltffi_function_xybrid_bolt_tool_results_envelope: buf[user_text_ptr] len=%zu head=%02x %02x %02x %02x %02x %02x %02x %02x", (size_t)c_user_text_len, c_user_text_ptr[0], c_user_text_ptr[1], c_user_text_ptr[2], c_user_text_ptr[3], c_user_text_ptr[4], c_user_text_ptr[5], c_user_text_ptr[6], c_user_text_ptr[7]);
    const uint8_t* c_prior_assistant_text_ptr = j_prior_assistant_text_ptr ? (const uint8_t*)(*env)->GetDirectBufferAddress(env, j_prior_assistant_text_ptr) : NULL;
    uintptr_t c_prior_assistant_text_len = (uintptr_t)j_prior_assistant_text_len;
    LOGI("  boltffi_function_xybrid_bolt_tool_results_envelope: buf[prior_assistant_text_ptr] len=%zu head=%02x %02x %02x %02x %02x %02x %02x %02x", (size_t)c_prior_assistant_text_len, c_prior_assistant_text_ptr[0], c_prior_assistant_text_ptr[1], c_prior_assistant_text_ptr[2], c_prior_assistant_text_ptr[3], c_prior_assistant_text_ptr[4], c_prior_assistant_text_ptr[5], c_prior_assistant_text_ptr[6], c_prior_assistant_text_ptr[7]);
    const uint8_t* c_results_ptr = j_results_ptr ? (const uint8_t*)(*env)->GetDirectBufferAddress(env, j_results_ptr) : NULL;
    uintptr_t c_results_len = (uintptr_t)j_results_len;
    LOGI("  boltffi_function_xybrid_bolt_tool_results_envelope: buf[results_ptr] len=%zu head=%02x %02x %02x %02x %02x %02x %02x %02x", (size_t)c_results_len, c_results_ptr[0], c_results_ptr[1], c_results_ptr[2], c_results_ptr[3], c_results_ptr[4], c_results_ptr[5], c_results_ptr[6], c_results_ptr[7]);
    FfiBuf_u8 c_return_out; memset(&c_return_out, 0, sizeof(FfiBuf_u8));
    static void *fp; if (!fp) fp = sym("boltffi_function_xybrid_bolt_tool_results_envelope");
    if (!fp) { jclass ex = (*env)->FindClass(env, "java/lang/RuntimeException"); if (ex) (*env)->ThrowNew(env, ex, "shim: missing symbol"); return 0; }
    FfiBuf_u8 __eb = ((FfiBuf_u8 (*)(const uint8_t*, uintptr_t, const uint8_t*, uintptr_t, const uint8_t*, uintptr_t, FfiBuf_u8*))fp)(c_user_text_ptr, c_user_text_len, c_prior_assistant_text_ptr, c_prior_assistant_text_len, c_results_ptr, c_results_len, &c_return_out);
    if (__eb.ptr && __eb.len) { throw_ffi(env, __eb); return NULL; }
    else { void (*fr)(FfiBuf_u8*)=(void(*)(FfiBuf_u8*))sym("boltffi_free_buf"); if(fr&&__eb.ptr) fr(&__eb); }
    return buf_to_array(env, c_return_out);
}

JNIEXPORT jbyteArray JNICALL Java_ai_xybrid_Native_boltffi_1function_1xybrid_1bolt_1json_1schema_1to_1gbnf(JNIEnv *env, jclass cls, jobject j_schema_json_ptr, jint j_schema_json_len, jobject j_return_out) {
    LOGI("ENTER boltffi_function_xybrid_bolt_json_schema_to_gbnf");
    const uint8_t* c_schema_json_ptr = j_schema_json_ptr ? (const uint8_t*)(*env)->GetDirectBufferAddress(env, j_schema_json_ptr) : NULL;
    uintptr_t c_schema_json_len = (uintptr_t)j_schema_json_len;
    LOGI("  boltffi_function_xybrid_bolt_json_schema_to_gbnf: buf[schema_json_ptr] len=%zu head=%02x %02x %02x %02x %02x %02x %02x %02x", (size_t)c_schema_json_len, c_schema_json_ptr[0], c_schema_json_ptr[1], c_schema_json_ptr[2], c_schema_json_ptr[3], c_schema_json_ptr[4], c_schema_json_ptr[5], c_schema_json_ptr[6], c_schema_json_ptr[7]);
    FfiBuf_u8 c_return_out; memset(&c_return_out, 0, sizeof(FfiBuf_u8));
    static void *fp; if (!fp) fp = sym("boltffi_function_xybrid_bolt_json_schema_to_gbnf");
    if (!fp) { jclass ex = (*env)->FindClass(env, "java/lang/RuntimeException"); if (ex) (*env)->ThrowNew(env, ex, "shim: missing symbol"); return 0; }
    FfiBuf_u8 __eb = ((FfiBuf_u8 (*)(const uint8_t*, uintptr_t, FfiBuf_u8*))fp)(c_schema_json_ptr, c_schema_json_len, &c_return_out);
    if (__eb.ptr && __eb.len) { throw_ffi(env, __eb); return NULL; }
    else { void (*fr)(FfiBuf_u8*)=(void(*)(FfiBuf_u8*))sym("boltffi_free_buf"); if(fr&&__eb.ptr) fr(&__eb); }
    return buf_to_array(env, c_return_out);
}

JNIEXPORT void JNICALL Java_ai_xybrid_Native_boltffi_1function_1xybrid_1bolt_1set_1thermal_1state(JNIEnv *env, jclass cls, jint j_state) {
    LOGI("ENTER boltffi_function_xybrid_bolt_set_thermal_state");
    int32_t c_state = (int32_t)j_state;
    static void *fp; if (!fp) fp = sym("boltffi_function_xybrid_bolt_set_thermal_state");
    if (!fp) { jclass ex = (*env)->FindClass(env, "java/lang/RuntimeException"); if (ex) (*env)->ThrowNew(env, ex, "shim: missing symbol"); return; }
    FfiStatus __st = ((FfiStatus (*)(int32_t))fp)(c_state);
    if (__st.code != 0) { FfiBuf_u8 e={0}; throw_ffi(env, e); }
}

JNIEXPORT void JNICALL Java_ai_xybrid_Native_boltffi_1function_1xybrid_1bolt_1clear_1thermal_1state(JNIEnv *env, jclass cls) {
    LOGI("ENTER boltffi_function_xybrid_bolt_clear_thermal_state");
    static void *fp; if (!fp) fp = sym("boltffi_function_xybrid_bolt_clear_thermal_state");
    if (!fp) { jclass ex = (*env)->FindClass(env, "java/lang/RuntimeException"); if (ex) (*env)->ThrowNew(env, ex, "shim: missing symbol"); return; }
    ((void (*)())fp)();
}

JNIEXPORT void JNICALL Java_ai_xybrid_Native_boltffi_1function_1xybrid_1bolt_1set_1battery_1level(JNIEnv *env, jclass cls, jbyte j_percent) {
    LOGI("ENTER boltffi_function_xybrid_bolt_set_battery_level");
    int32_t c_percent = (int32_t)j_percent;
    static void *fp; if (!fp) fp = sym("boltffi_function_xybrid_bolt_set_battery_level");
    if (!fp) { jclass ex = (*env)->FindClass(env, "java/lang/RuntimeException"); if (ex) (*env)->ThrowNew(env, ex, "shim: missing symbol"); return; }
    FfiStatus __st = ((FfiStatus (*)(int32_t))fp)(c_percent);
    if (__st.code != 0) { FfiBuf_u8 e={0}; throw_ffi(env, e); }
}

JNIEXPORT void JNICALL Java_ai_xybrid_Native_boltffi_1function_1xybrid_1bolt_1clear_1battery_1level(JNIEnv *env, jclass cls) {
    LOGI("ENTER boltffi_function_xybrid_bolt_clear_battery_level");
    static void *fp; if (!fp) fp = sym("boltffi_function_xybrid_bolt_clear_battery_level");
    if (!fp) { jclass ex = (*env)->FindClass(env, "java/lang/RuntimeException"); if (ex) (*env)->ThrowNew(env, ex, "shim: missing symbol"); return; }
    ((void (*)())fp)();
}

JNIEXPORT void JNICALL Java_ai_xybrid_Native_boltffi_1function_1xybrid_1bolt_1configure_1runtime(JNIEnv *env, jclass cls, jobject j_api_key_ptr, jint j_api_key_len, jobject j_gateway_url_ptr, jint j_gateway_url_len, jobject j_ingest_url_ptr, jint j_ingest_url_len) {
    LOGI("ENTER boltffi_function_xybrid_bolt_configure_runtime");
    const uint8_t* c_api_key_ptr = j_api_key_ptr ? (const uint8_t*)(*env)->GetDirectBufferAddress(env, j_api_key_ptr) : NULL;
    uintptr_t c_api_key_len = (uintptr_t)j_api_key_len;
    LOGI("  boltffi_function_xybrid_bolt_configure_runtime: buf[api_key_ptr] len=%zu head=%02x %02x %02x %02x %02x %02x %02x %02x", (size_t)c_api_key_len, c_api_key_ptr[0], c_api_key_ptr[1], c_api_key_ptr[2], c_api_key_ptr[3], c_api_key_ptr[4], c_api_key_ptr[5], c_api_key_ptr[6], c_api_key_ptr[7]);
    const uint8_t* c_gateway_url_ptr = j_gateway_url_ptr ? (const uint8_t*)(*env)->GetDirectBufferAddress(env, j_gateway_url_ptr) : NULL;
    uintptr_t c_gateway_url_len = (uintptr_t)j_gateway_url_len;
    LOGI("  boltffi_function_xybrid_bolt_configure_runtime: buf[gateway_url_ptr] len=%zu head=%02x %02x %02x %02x %02x %02x %02x %02x", (size_t)c_gateway_url_len, c_gateway_url_ptr[0], c_gateway_url_ptr[1], c_gateway_url_ptr[2], c_gateway_url_ptr[3], c_gateway_url_ptr[4], c_gateway_url_ptr[5], c_gateway_url_ptr[6], c_gateway_url_ptr[7]);
    const uint8_t* c_ingest_url_ptr = j_ingest_url_ptr ? (const uint8_t*)(*env)->GetDirectBufferAddress(env, j_ingest_url_ptr) : NULL;
    uintptr_t c_ingest_url_len = (uintptr_t)j_ingest_url_len;
    LOGI("  boltffi_function_xybrid_bolt_configure_runtime: buf[ingest_url_ptr] len=%zu head=%02x %02x %02x %02x %02x %02x %02x %02x", (size_t)c_ingest_url_len, c_ingest_url_ptr[0], c_ingest_url_ptr[1], c_ingest_url_ptr[2], c_ingest_url_ptr[3], c_ingest_url_ptr[4], c_ingest_url_ptr[5], c_ingest_url_ptr[6], c_ingest_url_ptr[7]);
    static void *fp; if (!fp) fp = sym("boltffi_function_xybrid_bolt_configure_runtime");
    if (!fp) { jclass ex = (*env)->FindClass(env, "java/lang/RuntimeException"); if (ex) (*env)->ThrowNew(env, ex, "shim: missing symbol"); return; }
    FfiStatus __st = ((FfiStatus (*)(const uint8_t*, uintptr_t, const uint8_t*, uintptr_t, const uint8_t*, uintptr_t))fp)(c_api_key_ptr, c_api_key_len, c_gateway_url_ptr, c_gateway_url_len, c_ingest_url_ptr, c_ingest_url_len);
    if (__st.code != 0) { FfiBuf_u8 e={0}; throw_ffi(env, e); }
}

JNIEXPORT void JNICALL Java_ai_xybrid_Native_boltffi_1function_1xybrid_1bolt_1init_1sdk_1cache_1dir(JNIEnv *env, jclass cls, jobject j_cache_dir_ptr, jint j_cache_dir_len) {
    LOGI("ENTER boltffi_function_xybrid_bolt_init_sdk_cache_dir");
    const uint8_t* c_cache_dir_ptr = j_cache_dir_ptr ? (const uint8_t*)(*env)->GetDirectBufferAddress(env, j_cache_dir_ptr) : NULL;
    uintptr_t c_cache_dir_len = (uintptr_t)j_cache_dir_len;
    LOGI("  boltffi_function_xybrid_bolt_init_sdk_cache_dir: buf[cache_dir_ptr] len=%zu head=%02x %02x %02x %02x %02x %02x %02x %02x", (size_t)c_cache_dir_len, c_cache_dir_ptr[0], c_cache_dir_ptr[1], c_cache_dir_ptr[2], c_cache_dir_ptr[3], c_cache_dir_ptr[4], c_cache_dir_ptr[5], c_cache_dir_ptr[6], c_cache_dir_ptr[7]);
    static void *fp; if (!fp) fp = sym("boltffi_function_xybrid_bolt_init_sdk_cache_dir");
    if (!fp) { jclass ex = (*env)->FindClass(env, "java/lang/RuntimeException"); if (ex) (*env)->ThrowNew(env, ex, "shim: missing symbol"); return; }
    FfiStatus __st = ((FfiStatus (*)(const uint8_t*, uintptr_t))fp)(c_cache_dir_ptr, c_cache_dir_len);
    if (__st.code != 0) { FfiBuf_u8 e={0}; throw_ffi(env, e); }
}

JNIEXPORT void JNICALL Java_ai_xybrid_Native_boltffi_1function_1xybrid_1bolt_1set_1binding(JNIEnv *env, jclass cls, jobject j_binding_ptr, jint j_binding_len) {
    LOGI("ENTER boltffi_function_xybrid_bolt_set_binding");
    const uint8_t* c_binding_ptr = j_binding_ptr ? (const uint8_t*)(*env)->GetDirectBufferAddress(env, j_binding_ptr) : NULL;
    uintptr_t c_binding_len = (uintptr_t)j_binding_len;
    LOGI("  boltffi_function_xybrid_bolt_set_binding: buf[binding_ptr] len=%zu head=%02x %02x %02x %02x %02x %02x %02x %02x", (size_t)c_binding_len, c_binding_ptr[0], c_binding_ptr[1], c_binding_ptr[2], c_binding_ptr[3], c_binding_ptr[4], c_binding_ptr[5], c_binding_ptr[6], c_binding_ptr[7]);
    static void *fp; if (!fp) fp = sym("boltffi_function_xybrid_bolt_set_binding");
    if (!fp) { jclass ex = (*env)->FindClass(env, "java/lang/RuntimeException"); if (ex) (*env)->ThrowNew(env, ex, "shim: missing symbol"); return; }
    FfiStatus __st = ((FfiStatus (*)(const uint8_t*, uintptr_t))fp)(c_binding_ptr, c_binding_len);
    if (__st.code != 0) { FfiBuf_u8 e={0}; throw_ffi(env, e); }
}

JNIEXPORT void JNICALL Java_ai_xybrid_Native_boltffi_1function_1xybrid_1bolt_1set_1api_1key(JNIEnv *env, jclass cls, jobject j_api_key_ptr, jint j_api_key_len) {
    LOGI("ENTER boltffi_function_xybrid_bolt_set_api_key");
    const uint8_t* c_api_key_ptr = j_api_key_ptr ? (const uint8_t*)(*env)->GetDirectBufferAddress(env, j_api_key_ptr) : NULL;
    uintptr_t c_api_key_len = (uintptr_t)j_api_key_len;
    LOGI("  boltffi_function_xybrid_bolt_set_api_key: buf[api_key_ptr] len=%zu head=%02x %02x %02x %02x %02x %02x %02x %02x", (size_t)c_api_key_len, c_api_key_ptr[0], c_api_key_ptr[1], c_api_key_ptr[2], c_api_key_ptr[3], c_api_key_ptr[4], c_api_key_ptr[5], c_api_key_ptr[6], c_api_key_ptr[7]);
    static void *fp; if (!fp) fp = sym("boltffi_function_xybrid_bolt_set_api_key");
    if (!fp) { jclass ex = (*env)->FindClass(env, "java/lang/RuntimeException"); if (ex) (*env)->ThrowNew(env, ex, "shim: missing symbol"); return; }
    FfiStatus __st = ((FfiStatus (*)(const uint8_t*, uintptr_t))fp)(c_api_key_ptr, c_api_key_len);
    if (__st.code != 0) { FfiBuf_u8 e={0}; throw_ffi(env, e); }
}

JNIEXPORT void JNICALL Java_ai_xybrid_Native_boltffi_1function_1xybrid_1bolt_1set_1provider_1api_1key(JNIEnv *env, jclass cls, jobject j_provider_ptr, jint j_provider_len, jobject j_api_key_ptr, jint j_api_key_len) {
    LOGI("ENTER boltffi_function_xybrid_bolt_set_provider_api_key");
    const uint8_t* c_provider_ptr = j_provider_ptr ? (const uint8_t*)(*env)->GetDirectBufferAddress(env, j_provider_ptr) : NULL;
    uintptr_t c_provider_len = (uintptr_t)j_provider_len;
    LOGI("  boltffi_function_xybrid_bolt_set_provider_api_key: buf[provider_ptr] len=%zu head=%02x %02x %02x %02x %02x %02x %02x %02x", (size_t)c_provider_len, c_provider_ptr[0], c_provider_ptr[1], c_provider_ptr[2], c_provider_ptr[3], c_provider_ptr[4], c_provider_ptr[5], c_provider_ptr[6], c_provider_ptr[7]);
    const uint8_t* c_api_key_ptr = j_api_key_ptr ? (const uint8_t*)(*env)->GetDirectBufferAddress(env, j_api_key_ptr) : NULL;
    uintptr_t c_api_key_len = (uintptr_t)j_api_key_len;
    LOGI("  boltffi_function_xybrid_bolt_set_provider_api_key: buf[api_key_ptr] len=%zu head=%02x %02x %02x %02x %02x %02x %02x %02x", (size_t)c_api_key_len, c_api_key_ptr[0], c_api_key_ptr[1], c_api_key_ptr[2], c_api_key_ptr[3], c_api_key_ptr[4], c_api_key_ptr[5], c_api_key_ptr[6], c_api_key_ptr[7]);
    static void *fp; if (!fp) fp = sym("boltffi_function_xybrid_bolt_set_provider_api_key");
    if (!fp) { jclass ex = (*env)->FindClass(env, "java/lang/RuntimeException"); if (ex) (*env)->ThrowNew(env, ex, "shim: missing symbol"); return; }
    FfiStatus __st = ((FfiStatus (*)(const uint8_t*, uintptr_t, const uint8_t*, uintptr_t))fp)(c_provider_ptr, c_provider_len, c_api_key_ptr, c_api_key_len);
    if (__st.code != 0) { FfiBuf_u8 e={0}; throw_ffi(env, e); }
}

JNIEXPORT void JNICALL Java_ai_xybrid_Native_boltffi_1function_1xybrid_1bolt_1set_1platform_1url(JNIEnv *env, jclass cls, jobject j_url_ptr, jint j_url_len) {
    LOGI("ENTER boltffi_function_xybrid_bolt_set_platform_url");
    const uint8_t* c_url_ptr = j_url_ptr ? (const uint8_t*)(*env)->GetDirectBufferAddress(env, j_url_ptr) : NULL;
    uintptr_t c_url_len = (uintptr_t)j_url_len;
    LOGI("  boltffi_function_xybrid_bolt_set_platform_url: buf[url_ptr] len=%zu head=%02x %02x %02x %02x %02x %02x %02x %02x", (size_t)c_url_len, c_url_ptr[0], c_url_ptr[1], c_url_ptr[2], c_url_ptr[3], c_url_ptr[4], c_url_ptr[5], c_url_ptr[6], c_url_ptr[7]);
    static void *fp; if (!fp) fp = sym("boltffi_function_xybrid_bolt_set_platform_url");
    if (!fp) { jclass ex = (*env)->FindClass(env, "java/lang/RuntimeException"); if (ex) (*env)->ThrowNew(env, ex, "shim: missing symbol"); return; }
    FfiStatus __st = ((FfiStatus (*)(const uint8_t*, uintptr_t))fp)(c_url_ptr, c_url_len);
    if (__st.code != 0) { FfiBuf_u8 e={0}; throw_ffi(env, e); }
}

JNIEXPORT void JNICALL Java_ai_xybrid_Native_boltffi_1function_1xybrid_1bolt_1set_1speculative_1cloud(JNIEnv *env, jclass cls, jboolean j_enabled) {
    LOGI("ENTER boltffi_function_xybrid_bolt_set_speculative_cloud");
    bool c_enabled = j_enabled ? true : false;
    static void *fp; if (!fp) fp = sym("boltffi_function_xybrid_bolt_set_speculative_cloud");
    if (!fp) { jclass ex = (*env)->FindClass(env, "java/lang/RuntimeException"); if (ex) (*env)->ThrowNew(env, ex, "shim: missing symbol"); return; }
    FfiStatus __st = ((FfiStatus (*)(bool))fp)(c_enabled);
    if (__st.code != 0) { FfiBuf_u8 e={0}; throw_ffi(env, e); }
}

JNIEXPORT jboolean JNICALL Java_ai_xybrid_Native_boltffi_1function_1xybrid_1bolt_1has_1api_1key(JNIEnv *env, jclass cls) {
    LOGI("ENTER boltffi_function_xybrid_bolt_has_api_key");
    static void *fp; if (!fp) fp = sym("boltffi_function_xybrid_bolt_has_api_key");
    if (!fp) { jclass ex = (*env)->FindClass(env, "java/lang/RuntimeException"); if (ex) (*env)->ThrowNew(env, ex, "shim: missing symbol"); return 0; }
    return (jboolean)((bool (*)())fp)();
}

JNIEXPORT jboolean JNICALL Java_ai_xybrid_Native_boltffi_1function_1xybrid_1bolt_1is_1speculative_1cloud_1enabled(JNIEnv *env, jclass cls) {
    LOGI("ENTER boltffi_function_xybrid_bolt_is_speculative_cloud_enabled");
    static void *fp; if (!fp) fp = sym("boltffi_function_xybrid_bolt_is_speculative_cloud_enabled");
    if (!fp) { jclass ex = (*env)->FindClass(env, "java/lang/RuntimeException"); if (ex) (*env)->ThrowNew(env, ex, "shim: missing symbol"); return 0; }
    return (jboolean)((bool (*)())fp)();
}

JNIEXPORT jboolean JNICALL Java_ai_xybrid_Native_boltffi_1function_1xybrid_1bolt_1will_1speculate_1for_1model(JNIEnv *env, jclass cls, jobject j_model_id_ptr, jint j_model_id_len) {
    LOGI("ENTER boltffi_function_xybrid_bolt_will_speculate_for_model");
    const uint8_t* c_model_id_ptr = j_model_id_ptr ? (const uint8_t*)(*env)->GetDirectBufferAddress(env, j_model_id_ptr) : NULL;
    uintptr_t c_model_id_len = (uintptr_t)j_model_id_len;
    LOGI("  boltffi_function_xybrid_bolt_will_speculate_for_model: buf[model_id_ptr] len=%zu head=%02x %02x %02x %02x %02x %02x %02x %02x", (size_t)c_model_id_len, c_model_id_ptr[0], c_model_id_ptr[1], c_model_id_ptr[2], c_model_id_ptr[3], c_model_id_ptr[4], c_model_id_ptr[5], c_model_id_ptr[6], c_model_id_ptr[7]);
    static void *fp; if (!fp) fp = sym("boltffi_function_xybrid_bolt_will_speculate_for_model");
    if (!fp) { jclass ex = (*env)->FindClass(env, "java/lang/RuntimeException"); if (ex) (*env)->ThrowNew(env, ex, "shim: missing symbol"); return 0; }
    return (jboolean)((bool (*)(const uint8_t*, uintptr_t))fp)(c_model_id_ptr, c_model_id_len);
}

JNIEXPORT jbyteArray JNICALL Java_ai_xybrid_Native_boltffi_1function_1xybrid_1bolt_1version(JNIEnv *env, jclass cls) {
    LOGI("ENTER boltffi_function_xybrid_bolt_version");
    static void *fp; if (!fp) fp = sym("boltffi_function_xybrid_bolt_version");
    if (!fp) { jclass ex = (*env)->FindClass(env, "java/lang/RuntimeException"); if (ex) (*env)->ThrowNew(env, ex, "shim: missing symbol"); return 0; }
    ((FfiBuf_u8 (*)())fp)();
}

JNIEXPORT jbyteArray JNICALL Java_ai_xybrid_Native_boltffi_1function_1xybrid_1bolt_1telemetry_1default_1endpoint(JNIEnv *env, jclass cls) {
    LOGI("ENTER boltffi_function_xybrid_bolt_telemetry_default_endpoint");
    static void *fp; if (!fp) fp = sym("boltffi_function_xybrid_bolt_telemetry_default_endpoint");
    if (!fp) { jclass ex = (*env)->FindClass(env, "java/lang/RuntimeException"); if (ex) (*env)->ThrowNew(env, ex, "shim: missing symbol"); return 0; }
    ((FfiBuf_u8 (*)())fp)();
}

JNIEXPORT void JNICALL Java_ai_xybrid_Native_boltffi_1function_1xybrid_1bolt_1telemetry_1flush(JNIEnv *env, jclass cls) {
    LOGI("ENTER boltffi_function_xybrid_bolt_telemetry_flush");
    static void *fp; if (!fp) fp = sym("boltffi_function_xybrid_bolt_telemetry_flush");
    if (!fp) { jclass ex = (*env)->FindClass(env, "java/lang/RuntimeException"); if (ex) (*env)->ThrowNew(env, ex, "shim: missing symbol"); return; }
    ((void (*)())fp)();
}

JNIEXPORT void JNICALL Java_ai_xybrid_Native_boltffi_1function_1xybrid_1bolt_1telemetry_1shutdown(JNIEnv *env, jclass cls) {
    LOGI("ENTER boltffi_function_xybrid_bolt_telemetry_shutdown");
    static void *fp; if (!fp) fp = sym("boltffi_function_xybrid_bolt_telemetry_shutdown");
    if (!fp) { jclass ex = (*env)->FindClass(env, "java/lang/RuntimeException"); if (ex) (*env)->ThrowNew(env, ex, "shim: missing symbol"); return; }
    ((void (*)())fp)();
}

JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM *vm, void *r) { LOGI("JNI_OnLoad: Muse bridge v3 live"); return JNI_VERSION_1_6; }
