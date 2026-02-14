#ifndef BERRY_CONF_H
#define BERRY_CONF_H

/* ESP32-optimized Berry configuration for minimal footprint */

/* Use 32-bit long integers (saves memory vs. long long) */
#define BE_INTGER_TYPE                  1

/* Use single-precision float (ESP32 has single-precision FPU) */
#define BE_USE_SINGLE_FLOAT             1

#define BE_BYTES_MAX_SIZE               (16*1024)

/* Use precompiled objects to save RAM */
#define BE_USE_PRECOMPILED_OBJECT       1

/* Disable debug info to save flash/RAM */
#define BE_DEBUG_SOURCE_FILE            0
#define BE_DEBUG_RUNTIME_INFO           2
#define BE_DEBUG_VAR_INFO               0
#define BE_USE_PERF_COUNTERS            0
#define BE_VM_OBSERVABILITY_SAMPLING    20

/* Reduced stack sizes for embedded use */
#define BE_STACK_TOTAL_MAX              4000
#define BE_STACK_FREE_MIN               10
#define BE_STACK_START                  40

#define BE_CONST_SEARCH_SIZE            50
#define BE_USE_STR_HASH_CACHE           0

/* Disable file system (we load scripts via LittleFS ourselves) */
#define BE_USE_FILE_SYSTEM              0

/* Keep the compiler for eval/run */
#define BE_USE_SCRIPT_COMPILER          1

/* Disable features not needed on embedded */
#define BE_USE_BYTECODE_SAVER           0
#define BE_USE_BYTECODE_LOADER          0
#define BE_USE_SHARED_LIB               0
#define BE_USE_OVERLOAD_HASH            0
#define BE_USE_DEBUG_HOOK               0
#define BE_USE_DEBUG_GC                 0
#define BE_USE_DEBUG_STACK              0
#define BE_USE_MEM_ALIGNED              0

/* Modules: keep only essential ones */
#define BE_USE_STRING_MODULE            1
#define BE_USE_JSON_MODULE              1
#define BE_USE_MATH_MODULE              1
#define BE_USE_TIME_MODULE              0
#define BE_USE_OS_MODULE                0
#define BE_USE_GLOBAL_MODULE            1
#define BE_USE_SYS_MODULE               0
#define BE_USE_DEBUG_MODULE             0
#define BE_USE_GC_MODULE                1
#define BE_USE_SOLIDIFY_MODULE          0
#define BE_USE_INTROSPECT_MODULE        1
#define BE_USE_STRICT_MODULE            0

#define BE_EXPLICIT_ABORT               abort
#define BE_EXPLICIT_EXIT                exit
#define BE_EXPLICIT_MALLOC              malloc
#define BE_EXPLICIT_FREE                free
#define BE_EXPLICIT_REALLOC             realloc

#endif
