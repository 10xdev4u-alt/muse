# Native JNI bridge for xybrid-kotlin 0.5.0

Upstream's published AAR ships `libxybrid-bolt.so` whose BoltFFI symbols are
plain C (`boltffi_*`) while its Kotlin glue declares classic JNI externals
(`Java_ai_xybrid_Native_*`) — so the SDK cannot load on any Android device
as published (see issue #77).

Our workaround: this directory carries a generated JNI bridge,
packaged as `libxybrid_bolt.so` (underscore — the name the Kotlin glue
dlopens). It dlsym's the plain symbols from the hyphen-named original
(still shipped by the AAR) and forwards with full type conversion.

## Files

- `gen_bridge.py` — generator; parses the authoritative C header
  (`bindings/apple/include/xybrid-bolt.h` in xybrid-ai/xybrid) plus the
  Kotlin externals, emits `bolt_shim_generated.c`. 85/85 symbols covered.
- `bolt_shim_generated.c` — committed output of the generator.
- `../app/src/main/jniLibs/{arm64-v8a,armeabi-v7a}/libxybrid_bolt.so` —
  compiled shims (NDK r26, minSdk 26).

## Regenerate

```bash
python3 gen_bridge.py            # needs xybrid checkout path edited in-script
$NDK/llvm/bin/aarch64-linux-android26-clang -shared -O2 bolt_shim_generated.c \
    -o libxybrid_bolt.so -llog
```

Remove entirely once upstream fixes the AAR packaging mismatch.
