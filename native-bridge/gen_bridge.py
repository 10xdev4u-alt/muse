#!/usr/bin/env python3
"""Generate JNI bridge for ai.xybrid.Native (Muse shim v3, clean rewrite).

Reads authoritative header bindings/apple/include/xybrid-bolt.h, intersects
with Kotlin externals in XybridBolt.kt, emits Java_ai_xybrid_Native_* hooks.

ABI conventions (header-verified):
  ByteBuffer + Int(len) pair      -> const uint8_t* ptr, uintptr_t len
  Long receiver/handle            -> uint64_t
  Int                             -> int32_t ; Byte -> int8_t ; Boolean -> bool
  uint64_t* return_out            -> handle out; Kotlin sees jlong
  FfiBuf_u8 return                -> ByteArray? data (freed post-copy)
  FfiBuf_u8* out + FfiBuf_u8 ret  -> ret = error payload, out = data
  FfiStatus ret                   -> void for Kotlin (throw on !=OK)
  FfiString ret                   -> String ; ___EnumX -> jint
"""
import re

HDR = '/home/princetheprogrammerbtw/xybrid/bindings/apple/include/xybrid-bolt.h'
KT = '/home/princetheprogrammerbtw/xybrid/bindings/kotlin/src/main/kotlin/ai/xybrid/XybridBolt.kt'
OUT = '/tmp/opencode/bolt_shim.c'

hdr = re.sub(r'//.*', '', open(HDR).read())
kt_src = open(KT).read()
kt_fns = {}
for _m in re.finditer(r'external fun (boltffi_\w+)\(([^)]*)\)\s*:\s*([^\n]+)', kt_src):
    kt_fns[_m.group(1)] = (_m.group(2), _m.group(3).strip())

decl_re = re.compile(r'^\s*[\w\s\*]*?\b(boltffi_\w+)\s*\(([^)]*)\)\s*;', re.M)
PRELUDE = '''#include <jni.h>
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
'''

KIND_CTYPE = {'BUF_PTR':'const uint8_t*', 'BUF_PAIR':'uintptr_t', 'HANDLE':'uint64_t',
              'JLONG':'int64_t', 'INT':'int32_t', 'UINT32':'uint32_t', 'BYTE':'int32_t',
              'BOOL':'bool', 'OUT_U64':'uint64_t*', 'OUT_BUF':'FfiBuf_u8*', 'ENUM':'int32_t'}

def classify(header_args):
    """Return list of (kind, cname) or raise on unknown."""
    ps = []
    for a in header_args:
        a = a.strip()
        if a == 'void':
            continue
        idx = a.rfind(a.split()[-1].lstrip('*&'))
        cname = a.split()[-1].lstrip('*&')
        before = a[:a.rfind(cname)]
        is_ptr = '*' in before or '*' in cname
        t = before.strip().rstrip('*').strip()
        if is_ptr and 'uint8_t' in t and 'return_out' not in a:
            ps.append(('BUF_PTR', cname))
        elif is_ptr and 'FfiBuf_u8' in t:
            ps.append(('OUT_BUF', cname))
        elif is_ptr and 'uint64_t' in t:
            ps.append(('OUT_U64', cname))
        elif 'uintptr_t' in t:
            ps.append(('BUF_PAIR', cname))
        elif t in ('uint64_t','int64_t'):
            ps.append(('HANDLE' if t=='uint64_t' else 'JLONG', cname))
        elif t in ('int32_t','uint32_t') or t.startswith('___'):
            ps.append(('ENUM' if t.startswith('___') else ('UINT32' if t=='uint32_t' else 'INT'), cname))
        elif t == 'uint8_t':
            ps.append(('BYTE', cname))
        elif t == 'bool':
            ps.append(('BOOL', cname))
        else:
            raise ValueError(f'unhandled type in: {a}')
    return ps

def kotlin_jni_ret(name, header_ret, has_outbuf):
    kret = (kt_fns.get(name, ('',''))[1] or '').strip()
    if has_outbuf:
        if 'Long' in kret.split(',')[-1]:
            return 'jlong'
    if header_ret == 'FfiBuf_u8' and kret.strip().endswith('Unit'):
        return 'void'
    if header_ret == 'FfiBuf_u8' and kret.strip().endswith('Long'):
        # error-payload-by-value + handle via return_out (init_* family)
        return 'jlong'
        return 'jbyteArray' if 'ByteArray' in kret else 'jstring' if 'String' in kret else None
    if header_ret == 'FfiBuf_u8':
        return 'jbyteArray' if 'ByteArray' in kret else None
    if header_ret == 'FfiString':
        return 'jstring' if 'String' in kret else None
    if header_ret == 'FfiStatus':
        return 'void'
    if header_ret == 'bool':
        return 'jboolean'
    if header_ret in ('int32_t','uint32_t') or header_ret.startswith('___'):
        return 'jint'
    if header_ret in ('int64_t','uint64_t'):
        return 'jlong'
    if header_ret == 'void':
        return 'void'
    return None

out = [PRELUDE]
generated, skipped = 0, []
for m in decl_re.finditer(hdr):
    name, args_s = m.group(1), m.group(2)
    if name not in kt_fns:
        continue
    decl = re.search(r'^\s*([\w\s\*]+?)\b' + name + r'\s*\(', hdr, re.M)
    header_ret = decl.group(1).strip()
    try:
        ps = classify([a for a in args_s.split(',') if a.strip()])
    except ValueError as e:
        skipped.append(f'{name}: {e}'); continue

    has_outbuf = any(k=='OUT_BUF' for k,_, in [(p[0],0) for p in ps])
    jret = kotlin_jni_ret(name, header_ret, has_outbuf)
    if jret is None:
        skipped.append(f'{name}: no JNI ret mapping (hdr={header_ret}, kt={kt_fns.get(name)}'); continue

    jparams = ', '.join(f'{ {"BUF_PTR":"jobject","BUF_PAIR":"jint","HANDLE":"jlong","JLONG":"jlong","INT":"jint","UINT32":"jint","BYTE":"jbyte","BOOL":"jboolean","OUT_U64":"jlong","OUT_BUF":"jobject","ENUM":"jint"}[k] } j_{cn}' for k,cn in ps)
    out.append(f'\nJNIEXPORT {jret} JNICALL Java_ai_xybrid_Native_{name.replace("_","_1")}(JNIEnv *env, jclass cls{", " if ps else ""}{jparams}) {{')
    out.append(f'    LOGI("ENTER {name}");')
    call_args = []
    for i,(k,cn) in enumerate(ps):
        C = KIND_CTYPE[k]
        if k == 'BUF_PTR':
            out.append(f'    {C} c_{cn} = j_{cn} ? (const uint8_t*)(*env)->GetDirectBufferAddress(env, j_{cn}) : NULL;')
            call_args.append(f'c_{cn}')
        elif k == 'BUF_PAIR':
            # partner ptr var = previous BUF_PTR's cname
            partner = ps[i-1][1]
            out.append(f'    uintptr_t c_{cn} = (uintptr_t)j_{cn};')
            dbg = ('    LOGI("  ' + name + ': buf[' + partner + '] len=%zu head=' +
                   '%02x %02x %02x %02x %02x %02x %02x %02x", (size_t)c_' + cn +
                   ', c_' + partner + '[0], c_' + partner + '[1], c_' + partner + '[2], c_' + partner + '[3], c_' + partner + '[4], c_' + partner + '[5], c_' + partner + '[6], c_' + partner + '[7]);')
            out.append(dbg)
            call_args.append(f'c_{cn}')
        elif k == 'HANDLE':
            out.append(f'    uint64_t c_{cn} = (uint64_t)j_{cn};')
            call_args.append(f'c_{cn}')
        elif k == 'JLONG':
            out.append(f'    int64_t c_{cn} = (int64_t)j_{cn};')
            call_args.append(f'c_{cn}')
        elif k in ('INT','ENUM'):
            out.append(f'    int32_t c_{cn} = (int32_t)j_{cn};')
            call_args.append(f'c_{cn}')
        elif k == 'UINT32':
            out.append(f'    uint32_t c_{cn} = (uint32_t)j_{cn};')
            call_args.append(f'c_{cn}')
        elif k == 'BYTE':
            out.append(f'    int32_t c_{cn} = (int32_t)j_{cn};')
            call_args.append(f'c_{cn}')
        elif k == 'BOOL':
            out.append(f'    bool c_{cn} = j_{cn} ? true : false;')
            call_args.append(f'c_{cn}')
        elif k == 'OUT_U64':
            out.append(f'    uint64_t c_{cn} = 0;')
            call_args.append(f'&c_{cn}')
        elif k == 'OUT_BUF':
            out.append(f'    FfiBuf_u8 c_{cn}; memset(&c_{cn}, 0, sizeof(FfiBuf_u8));')
            call_args.append(f'&c_{cn}')
    cret = ('FfiStatus' if header_ret=='FfiStatus' else
            'bool' if header_ret=='bool' else
            'int32_t' if header_ret in ('int32_t',) or header_ret.startswith('___') else
            'uint32_t' if header_ret=='uint32_t' else
            'int64_t' if header_ret=='int64_t' else
            'uint64_t' if header_ret=='uint64_t' else
            'FfiBuf_u8' if header_ret.startswith('FfiBuf_u8') else
            'FfiString' if header_ret.startswith('FfiString') else 'void')
    body_cast = f'({cret} (*)({", ".join(KIND_CTYPE[k] for k,_ in ps)}))'
    out.append(f'    static void *fp; if (!fp) fp = sym("{name}");')
    out.append('    if (!fp) { jclass ex = (*env)->FindClass(env, "java/lang/RuntimeException"); if (ex) (*env)->ThrowNew(env, ex, "shim: missing symbol"); ' + ('return 0;' if jret!='void' else 'return;') + ' }')
    call = f'({body_cast}fp)({", ".join(call_args)})'

    ob = next((c for k,c in ps if k=='OUT_BUF'), None)
    if ob:
        if cret == 'FfiBuf_u8':
            out.append(f'    FfiBuf_u8 __eb = {call};')
            out.append('    if (__eb.ptr && __eb.len) { throw_ffi(env, __eb); return NULL; }')
            out.append('    else { void (*fr)(FfiBuf_u8*)=(void(*)(FfiBuf_u8*))sym("boltffi_free_buf"); if(fr&&__eb.ptr) fr(&__eb); }')
        else:
            out.append(f'    FfiStatus __st = {call};')
            out.append('    if (__st.code != 0) { FfiBuf_u8 e={0}; throw_ffi(env, e); return NULL; }')
        if jret == 'jlong':
            out.append(f'    return (jlong)c_{ob};')
        else:
            out.append(f'    return buf_to_array(env, c_{ob});')
    elif header_ret.startswith('FfiBuf_u8') and jret == 'jlong':
        # init_* family: returned buf = error payload; handle rides OUT_U64
        out.append(f'    FfiBuf_u8 __eb = {call};')
        out.append('    if (__eb.ptr && __eb.len) { throw_ffi(env, __eb); return 0; } else { void (*fr)(FfiBuf_u8*)=(void(*)(FfiBuf_u8*))sym("boltffi_free_buf"); if(fr&&__eb.ptr) fr(&__eb); }')
        ou64 = next((c for k,c in ps if k=='OUT_U64'), None)
        out.append(f'    return (jlong)c_{ou64};')
    elif header_ret == 'FfiString':
        out.append(f'    FfiString __s = {call};')
        out.append('    return str_to_jstring(env, __s);')
    elif header_ret == 'FfiStatus':
        out.append(f'    FfiStatus __st = {call};')
        out.append('    if (__st.code != 0) { FfiBuf_u8 e={0}; throw_ffi(env, e); }')
    elif jret == 'void' and header_ret.startswith('FfiBuf_u8'):
        out.append(f'    FfiBuf_u8 __eb = {call};')
        out.append('    if (__eb.ptr && __eb.len) { throw_ffi(env, __eb); } else { void (*fr)(FfiBuf_u8*)=(void(*)(FfiBuf_u8*))sym("boltffi_free_buf"); if(fr&&__eb.ptr) fr(&__eb); }')
    elif header_ret == 'bool':
        out.append(f'    return (jboolean){call};')
    elif header_ret in ('int32_t',) or header_ret.startswith('___'):
        out.append(f'    return (jint){call};')
    elif header_ret == 'uint32_t':
        out.append(f'    return (jint){call};')
    elif header_ret in ('int64_t','uint64_t'):
        out.append(f'    return (jlong){call};')
    else:
        out.append(f'    {call};')
    out.append('}')
    generated += 1

out.append('\nJNIEXPORT jint JNICALL JNI_OnLoad(JavaVM *vm, void *r) { LOGI("JNI_OnLoad: Muse bridge v3 live"); return JNI_VERSION_1_6; }')
open(OUT,'w').write('\n'.join(out)+'\n')
print(f'GENERATED {generated}; SKIPPED {len(skipped)}')
for s in skipped: print('  SKIP:', s)
