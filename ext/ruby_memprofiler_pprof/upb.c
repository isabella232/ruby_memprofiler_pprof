/* Amalgamated source file */
#include "upb.h"
/*
 * Copyright (c) 2009-2021, Google LLC
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of Google LLC nor the
 *       names of its contributors may be used to endorse or promote products
 *       derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL Google LLC BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/*
 * This is where we define macros used across upb.
 *
 * All of these macros are undef'd in port_undef.inc to avoid leaking them to
 * users.
 *
 * The correct usage is:
 *
 *   #include "upb/foobar.h"
 *   #include "upb/baz.h"
 *
 *   // MUST be last included header.
 *   #include "upb/port_def.inc"
 *
 *   // Code for this file.
 *   // <...>
 *
 *   // Can be omitted for .c files, required for .h.
 *   #include "upb/port_undef.inc"
 *
 * This file is private and must not be included by users!
 */

#if !((defined(__STDC_VERSION__) && __STDC_VERSION__ >= 199901L) || \
      (defined(__cplusplus) && __cplusplus >= 201103L) ||           \
      (defined(_MSC_VER) && _MSC_VER >= 1900))
#error upb requires C99 or C++11 or MSVC >= 2015.
#endif

#include <stdint.h>
#include <stddef.h>

#if UINTPTR_MAX == 0xffffffff
#define UPB_SIZE(size32, size64) size32
#else
#define UPB_SIZE(size32, size64) size64
#endif

/* If we always read/write as a consistent type to each address, this shouldn't
 * violate aliasing.
 */
#define UPB_PTR_AT(msg, ofs, type) ((type*)((char*)(msg) + (ofs)))

#define UPB_READ_ONEOF(msg, fieldtype, offset, case_offset, case_val, default) \
  *UPB_PTR_AT(msg, case_offset, int) == case_val                              \
      ? *UPB_PTR_AT(msg, offset, fieldtype)                                   \
      : default

#define UPB_WRITE_ONEOF(msg, fieldtype, offset, value, case_offset, case_val) \
  *UPB_PTR_AT(msg, case_offset, int) = case_val;                             \
  *UPB_PTR_AT(msg, offset, fieldtype) = value;

#define UPB_MAPTYPE_STRING 0

/* UPB_INLINE: inline if possible, emit standalone code if required. */
#ifdef __cplusplus
#define UPB_INLINE inline
#elif defined (__GNUC__) || defined(__clang__)
#define UPB_INLINE static __inline__
#else
#define UPB_INLINE static
#endif

#define UPB_ALIGN_UP(size, align) (((size) + (align) - 1) / (align) * (align))
#define UPB_ALIGN_DOWN(size, align) ((size) / (align) * (align))
#define UPB_ALIGN_MALLOC(size) UPB_ALIGN_UP(size, 16)
#define UPB_ALIGN_OF(type) offsetof (struct { char c; type member; }, member)

/* Hints to the compiler about likely/unlikely branches. */
#if defined (__GNUC__) || defined(__clang__)
#define UPB_LIKELY(x) __builtin_expect((x),1)
#define UPB_UNLIKELY(x) __builtin_expect((x),0)
#else
#define UPB_LIKELY(x) (x)
#define UPB_UNLIKELY(x) (x)
#endif

/* Macros for function attributes on compilers that support them. */
#ifdef __GNUC__
#define UPB_FORCEINLINE __inline__ __attribute__((always_inline))
#define UPB_NOINLINE __attribute__((noinline))
#define UPB_NORETURN __attribute__((__noreturn__))
#define UPB_PRINTF(str, first_vararg) __attribute__((format (printf, str, first_vararg)))
#elif defined(_MSC_VER)
#define UPB_NOINLINE
#define UPB_FORCEINLINE
#define UPB_NORETURN __declspec(noreturn)
#define UPB_PRINTF(str, first_vararg)
#else  /* !defined(__GNUC__) */
#define UPB_FORCEINLINE
#define UPB_NOINLINE
#define UPB_NORETURN
#define UPB_PRINTF(str, first_vararg)
#endif

#define UPB_MAX(x, y) ((x) > (y) ? (x) : (y))
#define UPB_MIN(x, y) ((x) < (y) ? (x) : (y))

#define UPB_UNUSED(var) (void)var

/* UPB_ASSUME(): in release mode, we tell the compiler to assume this is true.
 */
#ifdef NDEBUG
#ifdef __GNUC__
#define UPB_ASSUME(expr) if (!(expr)) __builtin_unreachable()
#elif defined _MSC_VER
#define UPB_ASSUME(expr) if (!(expr)) __assume(0)
#else
#define UPB_ASSUME(expr) do {} while (false && (expr))
#endif
#else
#define UPB_ASSUME(expr) assert(expr)
#endif

/* UPB_ASSERT(): in release mode, we use the expression without letting it be
 * evaluated.  This prevents "unused variable" warnings. */
#ifdef NDEBUG
#define UPB_ASSERT(expr) do {} while (false && (expr))
#else
#define UPB_ASSERT(expr) assert(expr)
#endif

#if defined(__GNUC__) || defined(__clang__)
#define UPB_UNREACHABLE() do { assert(0); __builtin_unreachable(); } while(0)
#else
#define UPB_UNREACHABLE() do { assert(0); } while(0)
#endif

/* UPB_SETJMP() / UPB_LONGJMP(): avoid setting/restoring signal mask. */
#ifdef __APPLE__
#define UPB_SETJMP(buf) _setjmp(buf)
#define UPB_LONGJMP(buf, val) _longjmp(buf, val)
#else
#define UPB_SETJMP(buf) setjmp(buf)
#define UPB_LONGJMP(buf, val) longjmp(buf, val)
#endif

/* UPB_PTRADD(ptr, ofs): add pointer while avoiding "NULL + 0" UB */
#define UPB_PTRADD(ptr, ofs) ((ofs) ? (ptr) + (ofs) : (ptr))

/* Configure whether fasttable is switched on or not. *************************/

#ifdef __has_attribute
#define UPB_HAS_ATTRIBUTE(x) __has_attribute(x)
#else
#define UPB_HAS_ATTRIBUTE(x) 0
#endif

#if UPB_HAS_ATTRIBUTE(musttail)
#define UPB_MUSTTAIL __attribute__((musttail))
#else
#define UPB_MUSTTAIL
#endif

#undef UPB_HAS_ATTRIBUTE

/* This check is not fully robust: it does not require that we have "musttail"
 * support available. We need tail calls to avoid consuming arbitrary amounts
 * of stack space.
 *
 * GCC/Clang can mostly be trusted to generate tail calls as long as
 * optimization is enabled, but, debug builds will not generate tail calls
 * unless "musttail" is available.
 *
 * We should probably either:
 *   1. require that the compiler supports musttail.
 *   2. add some fallback code for when musttail isn't available (ie. return
 *      instead of tail calling). This is safe and portable, but this comes at
 *      a CPU cost.
 */
#if (defined(__x86_64__) || defined(__aarch64__)) && defined(__GNUC__)
#define UPB_FASTTABLE_SUPPORTED 1
#else
#define UPB_FASTTABLE_SUPPORTED 0
#endif

/* define UPB_ENABLE_FASTTABLE to force fast table support.
 * This is useful when we want to ensure we are really getting fasttable,
 * for example for testing or benchmarking. */
#if defined(UPB_ENABLE_FASTTABLE)
#if !UPB_FASTTABLE_SUPPORTED
#error fasttable is x86-64/ARM64 only and requires GCC or Clang.
#endif
#define UPB_FASTTABLE 1
/* Define UPB_TRY_ENABLE_FASTTABLE to use fasttable if possible.
 * This is useful for releasing code that might be used on multiple platforms,
 * for example the PHP or Ruby C extensions. */
#elif defined(UPB_TRY_ENABLE_FASTTABLE)
#define UPB_FASTTABLE UPB_FASTTABLE_SUPPORTED
#else
#define UPB_FASTTABLE 0
#endif

/* UPB_FASTTABLE_INIT() allows protos compiled for fasttable to gracefully
 * degrade to non-fasttable if we are using UPB_TRY_ENABLE_FASTTABLE. */
#if !UPB_FASTTABLE && defined(UPB_TRY_ENABLE_FASTTABLE)
#define UPB_FASTTABLE_INIT(...)
#else
#define UPB_FASTTABLE_INIT(...) __VA_ARGS__
#endif

#undef UPB_FASTTABLE_SUPPORTED

/* ASAN poisoning (for arena) *************************************************/

#if defined(__SANITIZE_ADDRESS__)
#define UPB_ASAN 1
#ifdef __cplusplus
extern "C" {
#endif
void __asan_poison_memory_region(void const volatile *addr, size_t size);
void __asan_unpoison_memory_region(void const volatile *addr, size_t size);
#ifdef __cplusplus
}  /* extern "C" */
#endif
#define UPB_POISON_MEMORY_REGION(addr, size) \
  __asan_poison_memory_region((addr), (size))
#define UPB_UNPOISON_MEMORY_REGION(addr, size) \
  __asan_unpoison_memory_region((addr), (size))
#else
#define UPB_ASAN 0
#define UPB_POISON_MEMORY_REGION(addr, size) \
  ((void)(addr), (void)(size))
#define UPB_UNPOISON_MEMORY_REGION(addr, size) \
  ((void)(addr), (void)(size))
#endif

/** upb/decode.c ************************************************************/

#include <setjmp.h>
#include <string.h>


/* Must be last. */

/* Maps descriptor type -> elem_size_lg2.  */
static const uint8_t desctype_to_elem_size_lg2[] = {
    -1,               /* invalid descriptor type */
    3,  /* DOUBLE */
    2,   /* FLOAT */
    3,   /* INT64 */
    3,  /* UINT64 */
    2,   /* INT32 */
    3,  /* FIXED64 */
    2,  /* FIXED32 */
    0,    /* BOOL */
    UPB_SIZE(3, 4),  /* STRING */
    UPB_SIZE(2, 3),  /* GROUP */
    UPB_SIZE(2, 3),  /* MESSAGE */
    UPB_SIZE(3, 4),  /* BYTES */
    2,  /* UINT32 */
    2,    /* ENUM */
    2,   /* SFIXED32 */
    3,   /* SFIXED64 */
    2,   /* SINT32 */
    3,   /* SINT64 */
};

/* Maps descriptor type -> upb map size.  */
static const uint8_t desctype_to_mapsize[] = {
    -1,                 /* invalid descriptor type */
    8,                  /* DOUBLE */
    4,                  /* FLOAT */
    8,                  /* INT64 */
    8,                  /* UINT64 */
    4,                  /* INT32 */
    8,                  /* FIXED64 */
    4,                  /* FIXED32 */
    1,                  /* BOOL */
    UPB_MAPTYPE_STRING, /* STRING */
    sizeof(void *),     /* GROUP */
    sizeof(void *),     /* MESSAGE */
    UPB_MAPTYPE_STRING, /* BYTES */
    4,                  /* UINT32 */
    4,                  /* ENUM */
    4,                  /* SFIXED32 */
    8,                  /* SFIXED64 */
    4,                  /* SINT32 */
    8,                  /* SINT64 */
};

static const unsigned FIXED32_OK_MASK = (1 << UPB_DTYPE_FLOAT) |
                                        (1 << UPB_DTYPE_FIXED32) |
                                        (1 << UPB_DTYPE_SFIXED32);

static const unsigned FIXED64_OK_MASK = (1 << UPB_DTYPE_DOUBLE) |
                                        (1 << UPB_DTYPE_FIXED64) |
                                        (1 << UPB_DTYPE_SFIXED64);

/* Three fake field types for MessageSet. */
#define TYPE_MSGSET_ITEM 19
#define TYPE_MSGSET_TYPE_ID 20
#define TYPE_COUNT 20

/* Op: an action to be performed for a wire-type/field-type combination. */
#define OP_UNKNOWN -1             /* Unknown field. */
#define OP_MSGSET_ITEM -2
#define OP_MSGSET_TYPEID -3
#define OP_SCALAR_LG2(n) (n)      /* n in [0, 2, 3] => op in [0, 2, 3] */
#define OP_ENUM 1
#define OP_STRING 4
#define OP_BYTES 5
#define OP_SUBMSG 6
/* Scalar fields use only ops above. Repeated fields can use any op.  */
#define OP_FIXPCK_LG2(n) (n + 5)  /* n in [2, 3] => op in [7, 8] */
#define OP_VARPCK_LG2(n) (n + 9)  /* n in [0, 2, 3] => op in [9, 11, 12] */
#define OP_PACKED_ENUM 13

static const int8_t varint_ops[] = {
    OP_UNKNOWN,       /* field not found */
    OP_UNKNOWN,       /* DOUBLE */
    OP_UNKNOWN,       /* FLOAT */
    OP_SCALAR_LG2(3), /* INT64 */
    OP_SCALAR_LG2(3), /* UINT64 */
    OP_SCALAR_LG2(2), /* INT32 */
    OP_UNKNOWN,       /* FIXED64 */
    OP_UNKNOWN,       /* FIXED32 */
    OP_SCALAR_LG2(0), /* BOOL */
    OP_UNKNOWN,       /* STRING */
    OP_UNKNOWN,       /* GROUP */
    OP_UNKNOWN,       /* MESSAGE */
    OP_UNKNOWN,       /* BYTES */
    OP_SCALAR_LG2(2), /* UINT32 */
    OP_ENUM,          /* ENUM */
    OP_UNKNOWN,       /* SFIXED32 */
    OP_UNKNOWN,       /* SFIXED64 */
    OP_SCALAR_LG2(2), /* SINT32 */
    OP_SCALAR_LG2(3), /* SINT64 */
    OP_UNKNOWN,       /* MSGSET_ITEM */
    OP_MSGSET_TYPEID, /* MSGSET TYPEID */
};

static const int8_t delim_ops[] = {
    /* For non-repeated field type. */
    OP_UNKNOWN,       /* field not found */
    OP_UNKNOWN,       /* DOUBLE */
    OP_UNKNOWN,       /* FLOAT */
    OP_UNKNOWN,       /* INT64 */
    OP_UNKNOWN,       /* UINT64 */
    OP_UNKNOWN,       /* INT32 */
    OP_UNKNOWN,       /* FIXED64 */
    OP_UNKNOWN,       /* FIXED32 */
    OP_UNKNOWN,       /* BOOL */
    OP_STRING,        /* STRING */
    OP_UNKNOWN,       /* GROUP */
    OP_SUBMSG,        /* MESSAGE */
    OP_BYTES,         /* BYTES */
    OP_UNKNOWN,       /* UINT32 */
    OP_UNKNOWN,       /* ENUM */
    OP_UNKNOWN,       /* SFIXED32 */
    OP_UNKNOWN,       /* SFIXED64 */
    OP_UNKNOWN,       /* SINT32 */
    OP_UNKNOWN,       /* SINT64 */
    OP_UNKNOWN,       /* MSGSET_ITEM */
    OP_UNKNOWN,       /* MSGSET TYPEID */
    /* For repeated field type. */
    OP_FIXPCK_LG2(3), /* REPEATED DOUBLE */
    OP_FIXPCK_LG2(2), /* REPEATED FLOAT */
    OP_VARPCK_LG2(3), /* REPEATED INT64 */
    OP_VARPCK_LG2(3), /* REPEATED UINT64 */
    OP_VARPCK_LG2(2), /* REPEATED INT32 */
    OP_FIXPCK_LG2(3), /* REPEATED FIXED64 */
    OP_FIXPCK_LG2(2), /* REPEATED FIXED32 */
    OP_VARPCK_LG2(0), /* REPEATED BOOL */
    OP_STRING,        /* REPEATED STRING */
    OP_SUBMSG,        /* REPEATED GROUP */
    OP_SUBMSG,        /* REPEATED MESSAGE */
    OP_BYTES,         /* REPEATED BYTES */
    OP_VARPCK_LG2(2), /* REPEATED UINT32 */
    OP_PACKED_ENUM,   /* REPEATED ENUM */
    OP_FIXPCK_LG2(2), /* REPEATED SFIXED32 */
    OP_FIXPCK_LG2(3), /* REPEATED SFIXED64 */
    OP_VARPCK_LG2(2), /* REPEATED SINT32 */
    OP_VARPCK_LG2(3), /* REPEATED SINT64 */
    /* Omitting MSGSET_*, because we never emit a repeated msgset type */
};

typedef union {
  bool bool_val;
  uint32_t uint32_val;
  uint64_t uint64_val;
  uint32_t size;
} wireval;

static const char *decode_msg(upb_decstate *d, const char *ptr, upb_msg *msg,
                              const upb_msglayout *layout);

UPB_NORETURN static void *decode_err(upb_decstate *d, upb_DecodeStatus status) {
  assert(status != kUpb_DecodeStatus_Ok);
  UPB_LONGJMP(d->err, status);
}

const char *fastdecode_err(upb_decstate *d, int status) {
  assert(status != kUpb_DecodeStatus_Ok);
  UPB_LONGJMP(d->err, status);
  return NULL;
}
static void decode_verifyutf8(upb_decstate *d, const char *buf, int len) {
  if (!decode_verifyutf8_inl(buf, len)) decode_err(d, kUpb_DecodeStatus_BadUtf8);
}

static bool decode_reserve(upb_decstate *d, upb_array *arr, size_t elem) {
  bool need_realloc = arr->size - arr->len < elem;
  if (need_realloc && !_upb_array_realloc(arr, arr->len + elem, &d->arena)) {
    decode_err(d, kUpb_DecodeStatus_OutOfMemory);
  }
  return need_realloc;
}

typedef struct {
  const char *ptr;
  uint64_t val;
} decode_vret;

UPB_NOINLINE
static decode_vret decode_longvarint64(const char *ptr, uint64_t val) {
  decode_vret ret = {NULL, 0};
  uint64_t byte;
  int i;
  for (i = 1; i < 10; i++) {
    byte = (uint8_t)ptr[i];
    val += (byte - 1) << (i * 7);
    if (!(byte & 0x80)) {
      ret.ptr = ptr + i + 1;
      ret.val = val;
      return ret;
    }
  }
  return ret;
}

UPB_FORCEINLINE
static const char *decode_varint64(upb_decstate *d, const char *ptr,
                                   uint64_t *val) {
  uint64_t byte = (uint8_t)*ptr;
  if (UPB_LIKELY((byte & 0x80) == 0)) {
    *val = byte;
    return ptr + 1;
  } else {
    decode_vret res = decode_longvarint64(ptr, byte);
    if (!res.ptr) return decode_err(d, kUpb_DecodeStatus_Malformed);
    *val = res.val;
    return res.ptr;
  }
}

UPB_FORCEINLINE
static const char *decode_tag(upb_decstate *d, const char *ptr, uint32_t *val) {
  uint64_t byte = (uint8_t)*ptr;
  if (UPB_LIKELY((byte & 0x80) == 0)) {
    *val = byte;
    return ptr + 1;
  } else {
    const char *start = ptr;
    decode_vret res = decode_longvarint64(ptr, byte);
    if (!res.ptr || res.ptr - start > 5 || res.val > UINT32_MAX) {
      return decode_err(d, kUpb_DecodeStatus_Malformed);
    }
    *val = res.val;
    return res.ptr;
  }
}

static void decode_munge_int32(wireval *val) {
  if (!_upb_isle()) {
    /* The next stage will memcpy(dst, &val, 4) */
    val->uint32_val = val->uint64_val;
  }
}

static void decode_munge(int type, wireval *val) {
  switch (type) {
    case UPB_DESCRIPTOR_TYPE_BOOL:
      val->bool_val = val->uint64_val != 0;
      break;
    case UPB_DESCRIPTOR_TYPE_SINT32: {
      uint32_t n = val->uint64_val;
      val->uint32_val = (n >> 1) ^ -(int32_t)(n & 1);
      break;
    }
    case UPB_DESCRIPTOR_TYPE_SINT64: {
      uint64_t n = val->uint64_val;
      val->uint64_val = (n >> 1) ^ -(int64_t)(n & 1);
      break;
    }
    case UPB_DESCRIPTOR_TYPE_INT32:
    case UPB_DESCRIPTOR_TYPE_UINT32:
    case UPB_DESCRIPTOR_TYPE_ENUM:
      decode_munge_int32(val);
      break;
  }
}

static upb_msg *decode_newsubmsg(upb_decstate *d, const upb_msglayout_sub *subs,
                                 const upb_msglayout_field *field) {
  const upb_msglayout *subl = subs[field->submsg_index].submsg;
  return _upb_msg_new_inl(subl, &d->arena);
}

UPB_NOINLINE
const char *decode_isdonefallback(upb_decstate *d, const char *ptr,
                                  int overrun) {
  int status; 
  ptr = decode_isdonefallback_inl(d, ptr, overrun, &status);
  if (ptr == NULL) {
    return decode_err(d, status);
  }
  return ptr;
}

static const char *decode_readstr(upb_decstate *d, const char *ptr, int size,
                                  upb_strview *str) {
  if (d->options & kUpb_DecodeOption_AliasString) {
    str->data = ptr;
  } else {
    char *data =  upb_arena_malloc(&d->arena, size);
    if (!data) return decode_err(d, kUpb_DecodeStatus_OutOfMemory);
    memcpy(data, ptr, size);
    str->data = data;
  }
  str->size = size;
  return ptr + size;
}

UPB_FORCEINLINE
static const char *decode_tosubmsg2(upb_decstate *d, const char *ptr,
                                    upb_msg *submsg, const upb_msglayout *subl,
                                    int size) {
  int saved_delta = decode_pushlimit(d, ptr, size);
  if (--d->depth < 0) return decode_err(d, kUpb_DecodeStatus_MaxDepthExceeded);
  ptr = decode_msg(d, ptr, submsg, subl);
  if (d->end_group != DECODE_NOGROUP) return decode_err(d, kUpb_DecodeStatus_Malformed);
  decode_poplimit(d, ptr, saved_delta);
  d->depth++;
  return ptr;
}

UPB_FORCEINLINE
static const char *decode_tosubmsg(upb_decstate *d, const char *ptr,
                                   upb_msg *submsg,
                                   const upb_msglayout_sub *subs,
                                   const upb_msglayout_field *field, int size) {
  return decode_tosubmsg2(d, ptr, submsg, subs[field->submsg_index].submsg,
                          size);
}

UPB_FORCEINLINE
static const char *decode_group(upb_decstate *d, const char *ptr,
                                upb_msg *submsg, const upb_msglayout *subl,
                                uint32_t number) {
  if (--d->depth < 0) return decode_err(d, kUpb_DecodeStatus_MaxDepthExceeded);
  if (decode_isdone(d, &ptr)) {
    return decode_err(d, kUpb_DecodeStatus_Malformed);
  }
  ptr = decode_msg(d, ptr, submsg, subl);
  if (d->end_group != number) return decode_err(d, kUpb_DecodeStatus_Malformed);
  d->end_group = DECODE_NOGROUP;
  d->depth++;
  return ptr;
}

UPB_FORCEINLINE
static const char *decode_togroup(upb_decstate *d, const char *ptr,
                                  upb_msg *submsg,
                                  const upb_msglayout_sub *subs,
                                  const upb_msglayout_field *field) {
  const upb_msglayout *subl = subs[field->submsg_index].submsg;
  return decode_group(d, ptr, submsg, subl, field->number);
}

static char *encode_varint32(uint32_t val, char *ptr) {
  do {
    uint8_t byte = val & 0x7fU;
    val >>= 7;
    if (val) byte |= 0x80U;
    *(ptr++) = byte;
  } while (val);
  return ptr;
}

UPB_NOINLINE
static bool decode_checkenum_slow(upb_decstate *d, const char *ptr, upb_msg *msg,
                             const upb_enumlayout *e,
                             const upb_msglayout_field *field, uint32_t v) {
  // OPT: binary search long lists?
  int n = e->value_count;
  for (int i = 0; i < n; i++) {
    if ((uint32_t)e->values[i] == v) return true;
  }

  // Unrecognized enum goes into unknown fields.
  // For packed fields the tag could be arbitrarily far in the past, so we
  // just re-encode the tag here.
  char buf[20];
  char *end = buf;
  uint32_t tag = ((uint32_t)field->number << 3) | UPB_WIRE_TYPE_VARINT;
  end = encode_varint32(tag, end);
  end = encode_varint32(v, end);

  if (!_upb_msg_addunknown(msg, buf, end - buf, &d->arena)) {
    decode_err(d, kUpb_DecodeStatus_OutOfMemory);
  }

  return false;
}

UPB_FORCEINLINE
static bool decode_checkenum(upb_decstate *d, const char *ptr, upb_msg *msg,
                             const upb_enumlayout *e,
                             const upb_msglayout_field *field, wireval *val) {
  uint32_t v = val->uint32_val;

  if (UPB_LIKELY(v < 64) && UPB_LIKELY(((1ULL << v) & e->mask))) return true;

  return decode_checkenum_slow(d, ptr, msg, e, field, v);
}

UPB_NOINLINE
static const char *decode_enum_toarray(upb_decstate *d, const char *ptr,
                                       upb_msg *msg, upb_array *arr,
                                       const upb_msglayout_sub *subs,
                                       const upb_msglayout_field *field,
                                       wireval *val) {
  const upb_enumlayout *e = subs[field->submsg_index].subenum;
  if (!decode_checkenum(d, ptr, msg, e, field, val)) return ptr;
  void *mem = UPB_PTR_AT(_upb_array_ptr(arr), arr->len * 4, void);
  arr->len++;
  memcpy(mem, val, 4);
  return ptr;
}

UPB_FORCEINLINE
static const char *decode_fixed_packed(upb_decstate *d, const char *ptr,
                                       upb_array *arr, wireval *val,
                                       const upb_msglayout_field *field,
                                       int lg2) {
  int mask = (1 << lg2) - 1;
  size_t count = val->size >> lg2;
  if ((val->size & mask) != 0) {
    // Length isn't a round multiple of elem size.
    return decode_err(d, kUpb_DecodeStatus_Malformed);
  }
  decode_reserve(d, arr, count);
  void *mem = UPB_PTR_AT(_upb_array_ptr(arr), arr->len << lg2, void);
  arr->len += count;
  // Note: if/when the decoder supports multi-buffer input, we will need to
  // handle buffer seams here.
  if (_upb_isle()) {
    memcpy(mem, ptr, val->size);
    ptr += val->size;
  } else {
    const char *end = ptr + val->size;
    char *dst = mem;
    while (ptr < end) {
      if (lg2 == 2) {
        uint32_t val;
        memcpy(&val, ptr, sizeof(val));
        val = _upb_be_swap32(val);
        memcpy(dst, &val, sizeof(val));
      } else {
        UPB_ASSERT(lg2 == 3);
        uint64_t val;
        memcpy(&val, ptr, sizeof(val));
        val = _upb_be_swap64(val);
        memcpy(dst, &val, sizeof(val));
      }
      ptr += 1 << lg2;
      dst += 1 << lg2;
    }
  }

  return ptr;
}

UPB_FORCEINLINE
static const char *decode_varint_packed(upb_decstate *d, const char *ptr,
                                        upb_array *arr, wireval *val,
                                        const upb_msglayout_field *field,
                                        int lg2) {
  int scale = 1 << lg2;
  int saved_limit = decode_pushlimit(d, ptr, val->size);
  char *out = UPB_PTR_AT(_upb_array_ptr(arr), arr->len << lg2, void);
  while (!decode_isdone(d, &ptr)) {
    wireval elem;
    ptr = decode_varint64(d, ptr, &elem.uint64_val);
    decode_munge(field->descriptortype, &elem);
    if (decode_reserve(d, arr, 1)) {
      out = UPB_PTR_AT(_upb_array_ptr(arr), arr->len << lg2, void);
    }
    arr->len++;
    memcpy(out, &elem, scale);
    out += scale;
  }
  decode_poplimit(d, ptr, saved_limit);
  return ptr;
}

UPB_NOINLINE
static const char *decode_enum_packed(upb_decstate *d, const char *ptr,
                                      upb_msg *msg, upb_array *arr,
                                      const upb_msglayout_sub *subs,
                                      const upb_msglayout_field *field,
                                      wireval *val) {
  const upb_enumlayout *e = subs[field->submsg_index].subenum;
  int saved_limit = decode_pushlimit(d, ptr, val->size);
  char *out = UPB_PTR_AT(_upb_array_ptr(arr), arr->len * 4, void);
  while (!decode_isdone(d, &ptr)) {
    wireval elem;
    ptr = decode_varint64(d, ptr, &elem.uint64_val);
    decode_munge_int32(&elem);
    if (!decode_checkenum(d, ptr, msg, e, field, &elem)) {
      continue;
    }
    if (decode_reserve(d, arr, 1)) {
      out = UPB_PTR_AT(_upb_array_ptr(arr), arr->len * 4, void);
    }
    arr->len++;
    memcpy(out, &elem, 4);
    out += 4;
  }
  decode_poplimit(d, ptr, saved_limit);
  return ptr;
}

static const char *decode_toarray(upb_decstate *d, const char *ptr,
                                  upb_msg *msg,
                                  const upb_msglayout_sub *subs,
                                  const upb_msglayout_field *field, wireval *val,
                                  int op) {
  upb_array **arrp = UPB_PTR_AT(msg, field->offset, void);
  upb_array *arr = *arrp;
  void *mem;

  if (arr) {
    decode_reserve(d, arr, 1);
  } else {
    size_t lg2 = desctype_to_elem_size_lg2[field->descriptortype];
    arr = _upb_array_new(&d->arena, 4, lg2);
    if (!arr) return decode_err(d, kUpb_DecodeStatus_OutOfMemory);
    *arrp = arr;
  }

  switch (op) {
    case OP_SCALAR_LG2(0):
    case OP_SCALAR_LG2(2):
    case OP_SCALAR_LG2(3):
      /* Append scalar value. */
      mem = UPB_PTR_AT(_upb_array_ptr(arr), arr->len << op, void);
      arr->len++;
      memcpy(mem, val, 1 << op);
      return ptr;
    case OP_STRING:
      decode_verifyutf8(d, ptr, val->size);
      /* Fallthrough. */
    case OP_BYTES: {
      /* Append bytes. */
      upb_strview *str = (upb_strview*)_upb_array_ptr(arr) + arr->len;
      arr->len++;
      return decode_readstr(d, ptr, val->size, str);
    }
    case OP_SUBMSG: {
      /* Append submessage / group. */
      upb_msg *submsg = decode_newsubmsg(d, subs, field);
      *UPB_PTR_AT(_upb_array_ptr(arr), arr->len * sizeof(void *), upb_msg *) =
          submsg;
      arr->len++;
      if (UPB_UNLIKELY(field->descriptortype == UPB_DTYPE_GROUP)) {
        return decode_togroup(d, ptr, submsg, subs, field);
      } else {
        return decode_tosubmsg(d, ptr, submsg, subs, field, val->size);
      }
    }
    case OP_FIXPCK_LG2(2):
    case OP_FIXPCK_LG2(3):
      return decode_fixed_packed(d, ptr, arr, val, field,
                                 op - OP_FIXPCK_LG2(0));
    case OP_VARPCK_LG2(0):
    case OP_VARPCK_LG2(2):
    case OP_VARPCK_LG2(3):
      return decode_varint_packed(d, ptr, arr, val, field,
                                  op - OP_VARPCK_LG2(0));
    case OP_ENUM:
      return decode_enum_toarray(d, ptr, msg, arr, subs, field, val);
    case OP_PACKED_ENUM:
      return decode_enum_packed(d, ptr, msg, arr, subs, field, val);
    default:
      UPB_UNREACHABLE();
  }
}

static const char *decode_tomap(upb_decstate *d, const char *ptr, upb_msg *msg,
                                const upb_msglayout_sub *subs,
                                const upb_msglayout_field *field,
                                wireval *val) {
  upb_map **map_p = UPB_PTR_AT(msg, field->offset, upb_map *);
  upb_map *map = *map_p;
  upb_map_entry ent;
  const upb_msglayout *entry = subs[field->submsg_index].submsg;

  if (!map) {
    /* Lazily create map. */
    const upb_msglayout_field *key_field = &entry->fields[0];
    const upb_msglayout_field *val_field = &entry->fields[1];
    char key_size = desctype_to_mapsize[key_field->descriptortype];
    char val_size = desctype_to_mapsize[val_field->descriptortype];
    UPB_ASSERT(key_field->offset == 0);
    UPB_ASSERT(val_field->offset == sizeof(upb_strview));
    map = _upb_map_new(&d->arena, key_size, val_size);
    *map_p = map;
  }

  /* Parse map entry. */
  memset(&ent, 0, sizeof(ent));

  if (entry->fields[1].descriptortype == UPB_DESCRIPTOR_TYPE_MESSAGE ||
      entry->fields[1].descriptortype == UPB_DESCRIPTOR_TYPE_GROUP) {
    /* Create proactively to handle the case where it doesn't appear. */
    ent.v.val = upb_value_ptr(_upb_msg_new(entry->subs[0].submsg, &d->arena));
  }

  ptr = decode_tosubmsg(d, ptr, &ent.k, subs, field, val->size);
  _upb_map_set(map, &ent.k, map->key_size, &ent.v, map->val_size, &d->arena);
  return ptr;
}

static const char *decode_tomsg(upb_decstate *d, const char *ptr, upb_msg *msg,
                                const upb_msglayout_sub *subs,
                                const upb_msglayout_field *field, wireval *val,
                                int op) {
  void *mem = UPB_PTR_AT(msg, field->offset, void);
  int type = field->descriptortype;

  if (UPB_UNLIKELY(op == OP_ENUM) &&
      !decode_checkenum(d, ptr, msg, subs[field->submsg_index].subenum, field,
                        val)) {
    return ptr;
  }

  /* Set presence if necessary. */
  if (field->presence > 0) {
    _upb_sethas_field(msg, field);
  } else if (field->presence < 0) {
    /* Oneof case */
    uint32_t *oneof_case = _upb_oneofcase_field(msg, field);
    if (op == OP_SUBMSG && *oneof_case != field->number) {
      memset(mem, 0, sizeof(void*));
    }
    *oneof_case = field->number;
  }

  /* Store into message. */
  switch (op) {
    case OP_SUBMSG: {
      upb_msg **submsgp = mem;
      upb_msg *submsg = *submsgp;
      if (!submsg) {
        submsg = decode_newsubmsg(d, subs, field);
        *submsgp = submsg;
      }
      if (UPB_UNLIKELY(type == UPB_DTYPE_GROUP)) {
        ptr = decode_togroup(d, ptr, submsg, subs, field);
      } else {
        ptr = decode_tosubmsg(d, ptr, submsg, subs, field, val->size);
      }
      break;
    }
    case OP_STRING:
      decode_verifyutf8(d, ptr, val->size);
      /* Fallthrough. */
    case OP_BYTES:
      return decode_readstr(d, ptr, val->size, mem);
    case OP_SCALAR_LG2(3):
      memcpy(mem, val, 8);
      break;
    case OP_ENUM:
    case OP_SCALAR_LG2(2):
      memcpy(mem, val, 4);
      break;
    case OP_SCALAR_LG2(0):
      memcpy(mem, val, 1);
      break;
    default:
      UPB_UNREACHABLE();
  }

  return ptr;
}

UPB_NOINLINE
const char *decode_checkrequired(upb_decstate *d, const char *ptr,
                                 const upb_msg *msg, const upb_msglayout *l) {
  assert(l->required_count);
  if (UPB_LIKELY((d->options & kUpb_DecodeOption_CheckRequired) == 0)) {
    return ptr;
  }
  uint64_t msg_head;
  memcpy(&msg_head, msg, 8);
  msg_head = _upb_be_swap64(msg_head);
  if (upb_msglayout_requiredmask(l) & ~msg_head) {
    d->missing_required = true;
  }
  return ptr;
}

UPB_FORCEINLINE
static bool decode_tryfastdispatch(upb_decstate *d, const char **ptr,
                                   upb_msg *msg, const upb_msglayout *layout) {
#if UPB_FASTTABLE
  if (layout && layout->table_mask != (unsigned char)-1) {
    uint16_t tag = fastdecode_loadtag(*ptr);
    intptr_t table = decode_totable(layout);
    *ptr = fastdecode_tagdispatch(d, *ptr, msg, table, 0, tag);
    return true;
  }
#endif
  return false;
}

static const char *decode_msgset(upb_decstate *d, const char *ptr, upb_msg *msg,
                                 const upb_msglayout *layout) {
  // We create a temporary upb_msglayout here and abuse its fields as temporary
  // storage, to avoid creating lots of MessageSet-specific parsing code-paths:
  //   1. We store 'layout' in item_layout.subs.  We will need this later as
  //      a key to look up extensions for this MessageSet.
  //   2. We use item_layout.fields as temporary storage to store the extension we
  //      found when parsing the type id.
  upb_msglayout item_layout = {
      .subs = (const upb_msglayout_sub[]){{.submsg = layout}},
      .fields = NULL,
      .size = 0,
      .field_count = 0,
      .ext = _UPB_MSGEXT_MSGSET_ITEM,
      .dense_below = 0,
      .table_mask = -1};
  return decode_group(d, ptr, msg, &item_layout, 1);
}

static const upb_msglayout_field *decode_findfield(upb_decstate *d,
                                                   const upb_msglayout *l,
                                                   uint32_t field_number,
                                                   int *last_field_index) {
  static upb_msglayout_field none = {0, 0, 0, 0, 0, 0};
  if (l == NULL) return &none;

  size_t idx = ((size_t)field_number) - 1;  // 0 wraps to SIZE_MAX
  if (idx < l->dense_below) {
    /* Fastest case: index into dense fields. */
    goto found;
  }

  if (l->dense_below < l->field_count) {
    /* Linear search non-dense fields. Resume scanning from last_field_index
     * since fields are usually in order. */
    int last = *last_field_index;
    for (idx = last; idx < l->field_count; idx++) {
      if (l->fields[idx].number == field_number) {
        goto found;
      }
    }

    for (idx = l->dense_below; idx < last; idx++) {
      if (l->fields[idx].number == field_number) {
        goto found;
      }
    }
  }

  if (d->extreg) {
    switch (l->ext) {
      case _UPB_MSGEXT_EXTENDABLE: {
        const upb_msglayout_ext *ext =
            _upb_extreg_get(d->extreg, l, field_number);
        if (ext) return &ext->field;
        break;
      }
      case _UPB_MSGEXT_MSGSET:
        if (field_number == _UPB_MSGSET_ITEM) {
          static upb_msglayout_field item = {0, 0, 0, 0, TYPE_MSGSET_ITEM, 0};
          return &item;
        }
        break;
      case _UPB_MSGEXT_MSGSET_ITEM:
        switch (field_number) {
          case _UPB_MSGSET_TYPEID: {
            static upb_msglayout_field type_id = {
                0, 0, 0, 0, TYPE_MSGSET_TYPE_ID, 0};
            return &type_id;
          }
          case _UPB_MSGSET_MESSAGE:
            if (l->fields) {
              // We saw type_id previously and succeeded in looking up msg.
              return l->fields;
            } else {
              // TODO: out of order MessageSet.
              // This is a very rare case: all serializers will emit in-order
              // MessageSets.  To hit this case there has to be some kind of
              // re-ordering proxy.  We should eventually handle this case, but
              // not today.
            }
            break;
        }
    }
  }

  return &none; /* Unknown field. */

 found:
  UPB_ASSERT(l->fields[idx].number == field_number);
  *last_field_index = idx;
  return &l->fields[idx];
 }

UPB_FORCEINLINE
static const char *decode_wireval(upb_decstate *d, const char *ptr,
                                  const upb_msglayout_field *field,
                                  int wire_type, wireval *val, int *op) {
  switch (wire_type) {
    case UPB_WIRE_TYPE_VARINT:
      ptr = decode_varint64(d, ptr, &val->uint64_val);
      *op = varint_ops[field->descriptortype];
      decode_munge(field->descriptortype, val);
      return ptr;
    case UPB_WIRE_TYPE_32BIT:
      memcpy(&val->uint32_val, ptr, 4);
      val->uint32_val = _upb_be_swap32(val->uint32_val);
      *op = OP_SCALAR_LG2(2);
      if (((1 << field->descriptortype) & FIXED32_OK_MASK) == 0) {
        *op = OP_UNKNOWN;
      }
      return ptr + 4;
    case UPB_WIRE_TYPE_64BIT:
      memcpy(&val->uint64_val, ptr, 8);
      val->uint64_val = _upb_be_swap64(val->uint64_val);
      *op = OP_SCALAR_LG2(3);
      if (((1 << field->descriptortype) & FIXED64_OK_MASK) == 0) {
        *op = OP_UNKNOWN;
      }
      return ptr + 8;
    case UPB_WIRE_TYPE_DELIMITED: {
      int ndx = field->descriptortype;
      uint64_t size;
      if (_upb_getmode(field) == _UPB_MODE_ARRAY) ndx += TYPE_COUNT;
      ptr = decode_varint64(d, ptr, &size);
      if (size >= INT32_MAX || ptr - d->end + (int32_t)size > d->limit) {
        break; /* Length overflow. */
      }
      *op = delim_ops[ndx];
      val->size = size;
      return ptr;
    }
    case UPB_WIRE_TYPE_START_GROUP:
      val->uint32_val = field->number;
      if (field->descriptortype == UPB_DTYPE_GROUP) {
        *op = OP_SUBMSG;
      } else if (field->descriptortype == TYPE_MSGSET_ITEM) {
        *op = OP_MSGSET_ITEM;
      } else {
        *op = OP_UNKNOWN;
      }
      return ptr;
    default:
      break;
  }
  return decode_err(d, kUpb_DecodeStatus_Malformed);
}

UPB_FORCEINLINE
static const char *decode_known(upb_decstate *d, const char *ptr, upb_msg *msg,
                                const upb_msglayout *layout,
                                const upb_msglayout_field *field, int op,
                                wireval *val) {
  const upb_msglayout_sub *subs = layout->subs;
  uint8_t mode = field->mode;

  if (UPB_UNLIKELY(mode & _UPB_MODE_IS_EXTENSION)) {
    const upb_msglayout_ext *ext_layout = (const upb_msglayout_ext*)field;
    upb_msg_ext *ext = _upb_msg_getorcreateext(msg, ext_layout, &d->arena);
        if (UPB_UNLIKELY(!ext)) return decode_err(d, kUpb_DecodeStatus_OutOfMemory);
    msg = &ext->data;
    subs = &ext->ext->sub;
  }

  switch (mode & _UPB_MODE_MASK) {
    case _UPB_MODE_ARRAY:
      return decode_toarray(d, ptr, msg, subs, field, val, op);
    case _UPB_MODE_MAP:
      return decode_tomap(d, ptr, msg, subs, field, val);
    case _UPB_MODE_SCALAR:
      return decode_tomsg(d, ptr, msg, subs, field, val, op);
    default:
      UPB_UNREACHABLE();
  }
}

static const char *decode_reverse_skip_varint(const char *ptr, uint32_t val) {
  uint32_t seen = 0;
  do {
    ptr--;
    seen <<= 7;
    seen |= *ptr & 0x7f;
  } while (seen != val);
  return ptr;
}

static const char *decode_unknown(upb_decstate *d, const char *ptr,
                                  upb_msg *msg, int field_number, int wire_type,
                                  wireval val) {
  if (field_number == 0) return decode_err(d, kUpb_DecodeStatus_Malformed);

  // Since unknown fields are the uncommon case, we do a little extra work here
  // to walk backwards through the buffer to find the field start.  This frees
  // up a register in the fast paths (when the field is known), which leads to
  // significant speedups in benchmarks.
  const char *start = ptr;

  if (wire_type == UPB_WIRE_TYPE_DELIMITED) ptr += val.size;
  if (msg) {
    switch (wire_type) {
      case UPB_WIRE_TYPE_VARINT:
      case UPB_WIRE_TYPE_DELIMITED:
        start--;
        while (start[-1] & 0x80) start--;
        break;
      case UPB_WIRE_TYPE_32BIT:
        start -= 4;
        break;
      case UPB_WIRE_TYPE_64BIT:
        start -= 8;
        break;
      default:
        break;
    }

    assert(start == d->debug_valstart);
    uint32_t tag = ((uint32_t)field_number << 3) | wire_type;
    start = decode_reverse_skip_varint(start, tag);
    assert(start == d->debug_tagstart);

    if (wire_type == UPB_WIRE_TYPE_START_GROUP) {
      d->unknown = start;
      d->unknown_msg = msg;
      ptr = decode_group(d, ptr, NULL, NULL, field_number);
      start = d->unknown;
      d->unknown_msg = NULL;
      d->unknown = NULL;
    }
    if (!_upb_msg_addunknown(msg, start, ptr - start, &d->arena)) {
      return decode_err(d, kUpb_DecodeStatus_OutOfMemory);
    }
  } else if (wire_type == UPB_WIRE_TYPE_START_GROUP) {
    ptr = decode_group(d, ptr, NULL, NULL, field_number);
  }
  return ptr;
}

UPB_NOINLINE
static const char *decode_msg(upb_decstate *d, const char *ptr, upb_msg *msg,
                              const upb_msglayout *layout) {
  int last_field_index = 0;

#if UPB_FASTTABLE
  // The first time we want to skip fast dispatch, because we may have just been
  // invoked by the fast parser to handle a case that it bailed on.
  if (!decode_isdone(d, &ptr)) goto nofast;
#endif

  while (!decode_isdone(d, &ptr)) {
    uint32_t tag;
    const upb_msglayout_field *field;
    int field_number;
    int wire_type;
    wireval val;
    int op;

    if (decode_tryfastdispatch(d, &ptr, msg, layout)) break;

#if UPB_FASTTABLE
  nofast:
#endif

#ifndef NDEBUG
    d->debug_tagstart = ptr;
#endif

    UPB_ASSERT(ptr < d->limit_ptr);
    ptr = decode_tag(d, ptr, &tag);
    field_number = tag >> 3;
    wire_type = tag & 7;

#ifndef NDEBUG
    d->debug_valstart = ptr;
#endif

    if (wire_type == UPB_WIRE_TYPE_END_GROUP) {
      d->end_group = field_number;
      return ptr;
    }

    field = decode_findfield(d, layout, field_number, &last_field_index);
    ptr = decode_wireval(d, ptr, field, wire_type, &val, &op);

    if (op >= 0) {
      ptr = decode_known(d, ptr, msg, layout, field, op, &val);
    } else {
      switch (op) {
        case OP_UNKNOWN:
          ptr = decode_unknown(d, ptr, msg, field_number, wire_type, val);
          break;
        case OP_MSGSET_ITEM:
          ptr = decode_msgset(d, ptr, msg, layout);
          break;
        case OP_MSGSET_TYPEID: {
          const upb_msglayout_ext *ext = _upb_extreg_get(
              d->extreg, layout->subs[0].submsg, val.uint64_val);
          if (ext) ((upb_msglayout *)layout)->fields = &ext->field;
          break;
        }
      }
    }
  }

  return UPB_UNLIKELY(layout && layout->required_count)
             ? decode_checkrequired(d, ptr, msg, layout)
             : ptr;
}

const char *fastdecode_generic(struct upb_decstate *d, const char *ptr,
                               upb_msg *msg, intptr_t table, uint64_t hasbits,
                               uint64_t data) {
  (void)data;
  *(uint32_t*)msg |= hasbits;
  return decode_msg(d, ptr, msg, decode_totablep(table));
}

static upb_DecodeStatus decode_top(struct upb_decstate *d, const char *buf,
                                   void *msg, const upb_msglayout *l) {
  if (!decode_tryfastdispatch(d, &buf, msg, l)) {
    decode_msg(d, buf, msg, l);
  }
  if (d->end_group != DECODE_NOGROUP) return kUpb_DecodeStatus_Malformed;
  if (d->missing_required) return kUpb_DecodeStatus_MissingRequired;
  return kUpb_DecodeStatus_Ok;
}

upb_DecodeStatus _upb_decode(const char *buf, size_t size, void *msg,
                             const upb_msglayout *l, const upb_extreg *extreg,
                             int options, upb_arena *arena) {
  upb_decstate state;
  unsigned depth = (unsigned)options >> 16;

  if (size <= 16) {
    memset(&state.patch, 0, 32);
    memcpy(&state.patch, buf, size);
    buf = state.patch;
    state.end = buf + size;
    state.limit = 0;
    options &= ~kUpb_DecodeOption_AliasString;  // Can't alias patch buf.
  } else {
    state.end = buf + size - 16;
    state.limit = 16;
  }

  state.extreg = extreg;
  state.limit_ptr = state.end;
  state.unknown_msg = NULL;
  state.depth = depth ? depth : 64;
  state.end_group = DECODE_NOGROUP;
  state.options = (uint16_t)options;
  state.missing_required = false;
  state.arena.head = arena->head;
  state.arena.last_size = arena->last_size;
  state.arena.cleanup_metadata = arena->cleanup_metadata;
  state.arena.parent = arena;

  upb_DecodeStatus status = UPB_SETJMP(state.err);
  if (UPB_LIKELY(status == kUpb_DecodeStatus_Ok)) {
    status = decode_top(&state, buf, msg, l);
  }

  arena->head.ptr = state.arena.head.ptr;
  arena->head.end = state.arena.head.end;
  arena->cleanup_metadata = state.arena.cleanup_metadata;
  return status;
}

#undef OP_UNKNOWN
#undef OP_SKIP
#undef OP_SCALAR_LG2
#undef OP_FIXPCK_LG2
#undef OP_VARPCK_LG2
#undef OP_STRING
#undef OP_BYTES
#undef OP_SUBMSG

/** upb/encode.c ************************************************************/
/* We encode backwards, to avoid pre-computing lengths (one-pass encode). */


#include <setjmp.h>
#include <string.h>


/* Must be last. */

#define UPB_PB_VARINT_MAX_LEN 10

UPB_NOINLINE
static size_t encode_varint64(uint64_t val, char *buf) {
  size_t i = 0;
  do {
    uint8_t byte = val & 0x7fU;
    val >>= 7;
    if (val) byte |= 0x80U;
    buf[i++] = byte;
  } while (val);
  return i;
}

static uint32_t encode_zz32(int32_t n) { return ((uint32_t)n << 1) ^ (n >> 31); }
static uint64_t encode_zz64(int64_t n) { return ((uint64_t)n << 1) ^ (n >> 63); }

typedef struct {
  jmp_buf err;
  upb_alloc *alloc;
  char *buf, *ptr, *limit;
  int options;
  int depth;
  _upb_mapsorter sorter;
} upb_encstate;

static size_t upb_roundup_pow2(size_t bytes) {
  size_t ret = 128;
  while (ret < bytes) {
    ret *= 2;
  }
  return ret;
}

UPB_NORETURN static void encode_err(upb_encstate *e) {
  UPB_LONGJMP(e->err, 1);
}

UPB_NOINLINE
static void encode_growbuffer(upb_encstate *e, size_t bytes) {
  size_t old_size = e->limit - e->buf;
  size_t new_size = upb_roundup_pow2(bytes + (e->limit - e->ptr));
  char *new_buf = upb_realloc(e->alloc, e->buf, old_size, new_size);

  if (!new_buf) encode_err(e);

  /* We want previous data at the end, realloc() put it at the beginning. */
  if (old_size > 0) {
    memmove(new_buf + new_size - old_size, e->buf, old_size);
  }

  e->ptr = new_buf + new_size - (e->limit - e->ptr);
  e->limit = new_buf + new_size;
  e->buf = new_buf;

  e->ptr -= bytes;
}

/* Call to ensure that at least "bytes" bytes are available for writing at
 * e->ptr.  Returns false if the bytes could not be allocated. */
UPB_FORCEINLINE
static void encode_reserve(upb_encstate *e, size_t bytes) {
  if ((size_t)(e->ptr - e->buf) < bytes) {
    encode_growbuffer(e, bytes);
    return;
  }

  e->ptr -= bytes;
}

/* Writes the given bytes to the buffer, handling reserve/advance. */
static void encode_bytes(upb_encstate *e, const void *data, size_t len) {
  if (len == 0) return;  /* memcpy() with zero size is UB */
  encode_reserve(e, len);
  memcpy(e->ptr, data, len);
}

static void encode_fixed64(upb_encstate *e, uint64_t val) {
  val = _upb_be_swap64(val);
  encode_bytes(e, &val, sizeof(uint64_t));
}

static void encode_fixed32(upb_encstate *e, uint32_t val) {
  val = _upb_be_swap32(val);
  encode_bytes(e, &val, sizeof(uint32_t));
}

UPB_NOINLINE
static void encode_longvarint(upb_encstate *e, uint64_t val) {
  size_t len;
  char *start;

  encode_reserve(e, UPB_PB_VARINT_MAX_LEN);
  len = encode_varint64(val, e->ptr);
  start = e->ptr + UPB_PB_VARINT_MAX_LEN - len;
  memmove(start, e->ptr, len);
  e->ptr = start;
}

UPB_FORCEINLINE
static void encode_varint(upb_encstate *e, uint64_t val) {
  if (val < 128 && e->ptr != e->buf) {
    --e->ptr;
    *e->ptr = val;
  } else {
    encode_longvarint(e, val);
  }
}

static void encode_double(upb_encstate *e, double d) {
  uint64_t u64;
  UPB_ASSERT(sizeof(double) == sizeof(uint64_t));
  memcpy(&u64, &d, sizeof(uint64_t));
  encode_fixed64(e, u64);
}

static void encode_float(upb_encstate *e, float d) {
  uint32_t u32;
  UPB_ASSERT(sizeof(float) == sizeof(uint32_t));
  memcpy(&u32, &d, sizeof(uint32_t));
  encode_fixed32(e, u32);
}

static void encode_tag(upb_encstate *e, uint32_t field_number,
                       uint8_t wire_type) {
  encode_varint(e, (field_number << 3) | wire_type);
}

static void encode_fixedarray(upb_encstate *e, const upb_array *arr,
                               size_t elem_size, uint32_t tag) {
  size_t bytes = arr->len * elem_size;
  const char* data = _upb_array_constptr(arr);
  const char* ptr = data + bytes - elem_size;

  if (tag || !_upb_isle()) {
    while (true) {
      if (elem_size == 4) {
        uint32_t val;
        memcpy(&val, ptr, sizeof(val));
        val = _upb_be_swap32(val);
        encode_bytes(e, &val, elem_size);
      } else {
        UPB_ASSERT(elem_size == 8);
        uint64_t val;
        memcpy(&val, ptr, sizeof(val));
        val = _upb_be_swap64(val);
        encode_bytes(e, &val, elem_size);
      }

      if (tag) encode_varint(e, tag);
      if (ptr == data) break;
      ptr -= elem_size;
    }
  } else {
    encode_bytes(e, data, bytes);
  }
}

static void encode_message(upb_encstate *e, const upb_msg *msg,
                           const upb_msglayout *m, size_t *size);

static void encode_scalar(upb_encstate *e, const void *_field_mem,
                          const upb_msglayout_sub *subs,
                          const upb_msglayout_field *f) {
  const char *field_mem = _field_mem;
  int wire_type;

#define CASE(ctype, type, wtype, encodeval) \
  {                                         \
    ctype val = *(ctype *)field_mem;        \
    encode_##type(e, encodeval);            \
    wire_type = wtype;                      \
    break;                                  \
  }

  switch (f->descriptortype) {
    case UPB_DESCRIPTOR_TYPE_DOUBLE:
      CASE(double, double, UPB_WIRE_TYPE_64BIT, val);
    case UPB_DESCRIPTOR_TYPE_FLOAT:
      CASE(float, float, UPB_WIRE_TYPE_32BIT, val);
    case UPB_DESCRIPTOR_TYPE_INT64:
    case UPB_DESCRIPTOR_TYPE_UINT64:
      CASE(uint64_t, varint, UPB_WIRE_TYPE_VARINT, val);
    case UPB_DESCRIPTOR_TYPE_UINT32:
      CASE(uint32_t, varint, UPB_WIRE_TYPE_VARINT, val);
    case UPB_DESCRIPTOR_TYPE_INT32:
    case UPB_DESCRIPTOR_TYPE_ENUM:
      CASE(int32_t, varint, UPB_WIRE_TYPE_VARINT, (int64_t)val);
    case UPB_DESCRIPTOR_TYPE_SFIXED64:
    case UPB_DESCRIPTOR_TYPE_FIXED64:
      CASE(uint64_t, fixed64, UPB_WIRE_TYPE_64BIT, val);
    case UPB_DESCRIPTOR_TYPE_FIXED32:
    case UPB_DESCRIPTOR_TYPE_SFIXED32:
      CASE(uint32_t, fixed32, UPB_WIRE_TYPE_32BIT, val);
    case UPB_DESCRIPTOR_TYPE_BOOL:
      CASE(bool, varint, UPB_WIRE_TYPE_VARINT, val);
    case UPB_DESCRIPTOR_TYPE_SINT32:
      CASE(int32_t, varint, UPB_WIRE_TYPE_VARINT, encode_zz32(val));
    case UPB_DESCRIPTOR_TYPE_SINT64:
      CASE(int64_t, varint, UPB_WIRE_TYPE_VARINT, encode_zz64(val));
    case UPB_DESCRIPTOR_TYPE_STRING:
    case UPB_DESCRIPTOR_TYPE_BYTES: {
      upb_strview view = *(upb_strview*)field_mem;
      encode_bytes(e, view.data, view.size);
      encode_varint(e, view.size);
      wire_type = UPB_WIRE_TYPE_DELIMITED;
      break;
    }
    case UPB_DESCRIPTOR_TYPE_GROUP: {
      size_t size;
      void *submsg = *(void **)field_mem;
      const upb_msglayout *subm = subs[f->submsg_index].submsg;
      if (submsg == NULL) {
        return;
      }
      if (--e->depth == 0) encode_err(e);
      encode_tag(e, f->number, UPB_WIRE_TYPE_END_GROUP);
      encode_message(e, submsg, subm, &size);
      wire_type = UPB_WIRE_TYPE_START_GROUP;
      e->depth++;
      break;
    }
    case UPB_DESCRIPTOR_TYPE_MESSAGE: {
      size_t size;
      void *submsg = *(void **)field_mem;
      const upb_msglayout *subm = subs[f->submsg_index].submsg;
      if (submsg == NULL) {
        return;
      }
      if (--e->depth == 0) encode_err(e);
      encode_message(e, submsg, subm, &size);
      encode_varint(e, size);
      wire_type = UPB_WIRE_TYPE_DELIMITED;
      e->depth++;
      break;
    }
    default:
      UPB_UNREACHABLE();
  }
#undef CASE

  encode_tag(e, f->number, wire_type);
}

static void encode_array(upb_encstate *e, const upb_msg *msg,
                         const upb_msglayout_sub *subs,
                         const upb_msglayout_field *f) {
  const upb_array *arr = *UPB_PTR_AT(msg, f->offset, upb_array*);
  bool packed = f->mode & _UPB_MODE_IS_PACKED;
  size_t pre_len = e->limit - e->ptr;

  if (arr == NULL || arr->len == 0) {
    return;
  }

#define VARINT_CASE(ctype, encode)                                       \
  {                                                                      \
    const ctype *start = _upb_array_constptr(arr);                       \
    const ctype *ptr = start + arr->len;                                 \
    uint32_t tag = packed ? 0 : (f->number << 3) | UPB_WIRE_TYPE_VARINT; \
    do {                                                                 \
      ptr--;                                                             \
      encode_varint(e, encode);                                          \
      if (tag) encode_varint(e, tag);                                    \
    } while (ptr != start);                                              \
  }                                                                      \
  break;

#define TAG(wire_type) (packed ? 0 : (f->number << 3 | wire_type))

  switch (f->descriptortype) {
    case UPB_DESCRIPTOR_TYPE_DOUBLE:
      encode_fixedarray(e, arr, sizeof(double), TAG(UPB_WIRE_TYPE_64BIT));
      break;
    case UPB_DESCRIPTOR_TYPE_FLOAT:
      encode_fixedarray(e, arr, sizeof(float), TAG(UPB_WIRE_TYPE_32BIT));
      break;
    case UPB_DESCRIPTOR_TYPE_SFIXED64:
    case UPB_DESCRIPTOR_TYPE_FIXED64:
      encode_fixedarray(e, arr, sizeof(uint64_t), TAG(UPB_WIRE_TYPE_64BIT));
      break;
    case UPB_DESCRIPTOR_TYPE_FIXED32:
    case UPB_DESCRIPTOR_TYPE_SFIXED32:
      encode_fixedarray(e, arr, sizeof(uint32_t), TAG(UPB_WIRE_TYPE_32BIT));
      break;
    case UPB_DESCRIPTOR_TYPE_INT64:
    case UPB_DESCRIPTOR_TYPE_UINT64:
      VARINT_CASE(uint64_t, *ptr);
    case UPB_DESCRIPTOR_TYPE_UINT32:
      VARINT_CASE(uint32_t, *ptr);
    case UPB_DESCRIPTOR_TYPE_INT32:
    case UPB_DESCRIPTOR_TYPE_ENUM:
      VARINT_CASE(int32_t, (int64_t)*ptr);
    case UPB_DESCRIPTOR_TYPE_BOOL:
      VARINT_CASE(bool, *ptr);
    case UPB_DESCRIPTOR_TYPE_SINT32:
      VARINT_CASE(int32_t, encode_zz32(*ptr));
    case UPB_DESCRIPTOR_TYPE_SINT64:
      VARINT_CASE(int64_t, encode_zz64(*ptr));
    case UPB_DESCRIPTOR_TYPE_STRING:
    case UPB_DESCRIPTOR_TYPE_BYTES: {
      const upb_strview *start = _upb_array_constptr(arr);
      const upb_strview *ptr = start + arr->len;
      do {
        ptr--;
        encode_bytes(e, ptr->data, ptr->size);
        encode_varint(e, ptr->size);
        encode_tag(e, f->number, UPB_WIRE_TYPE_DELIMITED);
      } while (ptr != start);
      return;
    }
    case UPB_DESCRIPTOR_TYPE_GROUP: {
      const void *const*start = _upb_array_constptr(arr);
      const void *const*ptr = start + arr->len;
      const upb_msglayout *subm = subs[f->submsg_index].submsg;
      if (--e->depth == 0) encode_err(e);
      do {
        size_t size;
        ptr--;
        encode_tag(e, f->number, UPB_WIRE_TYPE_END_GROUP);
        encode_message(e, *ptr, subm, &size);
        encode_tag(e, f->number, UPB_WIRE_TYPE_START_GROUP);
      } while (ptr != start);
      e->depth++;
      return;
    }
    case UPB_DESCRIPTOR_TYPE_MESSAGE: {
      const void *const*start = _upb_array_constptr(arr);
      const void *const*ptr = start + arr->len;
      const upb_msglayout *subm = subs[f->submsg_index].submsg;
      if (--e->depth == 0) encode_err(e);
      do {
        size_t size;
        ptr--;
        encode_message(e, *ptr, subm, &size);
        encode_varint(e, size);
        encode_tag(e, f->number, UPB_WIRE_TYPE_DELIMITED);
      } while (ptr != start);
      e->depth++;
      return;
    }
  }
#undef VARINT_CASE

  if (packed) {
    encode_varint(e, e->limit - e->ptr - pre_len);
    encode_tag(e, f->number, UPB_WIRE_TYPE_DELIMITED);
  }
}

static void encode_mapentry(upb_encstate *e, uint32_t number,
                            const upb_msglayout *layout,
                            const upb_map_entry *ent) {
  const upb_msglayout_field *key_field = &layout->fields[0];
  const upb_msglayout_field *val_field = &layout->fields[1];
  size_t pre_len = e->limit - e->ptr;
  size_t size;
  encode_scalar(e, &ent->v, layout->subs, val_field);
  encode_scalar(e, &ent->k, layout->subs, key_field);
  size = (e->limit - e->ptr) - pre_len;
  encode_varint(e, size);
  encode_tag(e, number, UPB_WIRE_TYPE_DELIMITED);
}

static void encode_map(upb_encstate *e, const upb_msg *msg,
                       const upb_msglayout_sub *subs,
                       const upb_msglayout_field *f) {
  const upb_map *map = *UPB_PTR_AT(msg, f->offset, const upb_map*);
  const upb_msglayout *layout = subs[f->submsg_index].submsg;
  UPB_ASSERT(layout->field_count == 2);

  if (map == NULL) return;

  if (e->options & UPB_ENCODE_DETERMINISTIC) {
    _upb_sortedmap sorted;
    _upb_mapsorter_pushmap(&e->sorter, layout->fields[0].descriptortype, map,
                           &sorted);
    upb_map_entry ent;
    while (_upb_sortedmap_next(&e->sorter, map, &sorted, &ent)) {
      encode_mapentry(e, f->number, layout, &ent);
    }
    _upb_mapsorter_popmap(&e->sorter, &sorted);
  } else {
    upb_strtable_iter i;
    upb_strtable_begin(&i, &map->table);
    for(; !upb_strtable_done(&i); upb_strtable_next(&i)) {
      upb_strview key = upb_strtable_iter_key(&i);
      const upb_value val = upb_strtable_iter_value(&i);
      upb_map_entry ent;
      _upb_map_fromkey(key, &ent.k, map->key_size);
      _upb_map_fromvalue(val, &ent.v, map->val_size);
      encode_mapentry(e, f->number, layout, &ent);
    }
  }
}

static bool encode_shouldencode(upb_encstate *e, const upb_msg *msg,
                                const upb_msglayout_sub *subs,
                                const upb_msglayout_field *f) {
  if (f->presence == 0) {
    /* Proto3 presence or map/array. */
    const void *mem = UPB_PTR_AT(msg, f->offset, void);
    switch (f->mode >> _UPB_REP_SHIFT) {
      case _UPB_REP_1BYTE: {
        char ch;
        memcpy(&ch, mem, 1);
        return ch != 0;
      }
      case _UPB_REP_4BYTE: {
        uint32_t u32;
        memcpy(&u32, mem, 4);
        return u32 != 0;
      }
      case _UPB_REP_8BYTE: {
        uint64_t u64;
        memcpy(&u64, mem, 8);
        return u64 != 0;
      }
      case _UPB_REP_STRVIEW: {
        const upb_strview *str = (const upb_strview*)mem;
        return str->size != 0;
      }
      default:
        UPB_UNREACHABLE();
    }
  } else if (f->presence > 0) {
    /* Proto2 presence: hasbit. */
    return _upb_hasbit_field(msg, f);
  } else {
    /* Field is in a oneof. */
    return _upb_getoneofcase_field(msg, f) == f->number;
  }
}

static void encode_field(upb_encstate *e, const upb_msg *msg,
                         const upb_msglayout_sub *subs,
                         const upb_msglayout_field *field) {
  switch (_upb_getmode(field)) {
    case _UPB_MODE_ARRAY:
      encode_array(e, msg, subs, field);
      break;
    case _UPB_MODE_MAP:
      encode_map(e, msg, subs, field);
      break;
    case _UPB_MODE_SCALAR:
      encode_scalar(e, UPB_PTR_AT(msg, field->offset, void), subs, field);
      break;
    default:
      UPB_UNREACHABLE();
  }
}

/* message MessageSet {
 *   repeated group Item = 1 {
 *     required int32 type_id = 2;
 *     required string message = 3;
 *   }
 * } */
static void encode_msgset_item(upb_encstate *e, const upb_msg_ext *ext) {
  size_t size;
  encode_tag(e, 1, UPB_WIRE_TYPE_END_GROUP);
  encode_message(e, ext->data.ptr, ext->ext->sub.submsg, &size);
  encode_varint(e, size);
  encode_tag(e, 3, UPB_WIRE_TYPE_DELIMITED);
  encode_varint(e, ext->ext->field.number);
  encode_tag(e, 2, UPB_WIRE_TYPE_VARINT);
  encode_tag(e, 1, UPB_WIRE_TYPE_START_GROUP);
}

static void encode_message(upb_encstate *e, const upb_msg *msg,
                           const upb_msglayout *m, size_t *size) {
  size_t pre_len = e->limit - e->ptr;

  if ((e->options & UPB_ENCODE_CHECKREQUIRED) && m->required_count) {
    uint64_t msg_head;
    memcpy(&msg_head, msg, 8);
    msg_head = _upb_be_swap64(msg_head);
    if (upb_msglayout_requiredmask(m) & ~msg_head) {
      encode_err(e);
    }
  }

  if ((e->options & UPB_ENCODE_SKIPUNKNOWN) == 0) {
    size_t unknown_size;
    const char *unknown = upb_msg_getunknown(msg, &unknown_size);

    if (unknown) {
      encode_bytes(e, unknown, unknown_size);
    }
  }

  if (m->ext != _UPB_MSGEXT_NONE) {
    /* Encode all extensions together. Unlike C++, we do not attempt to keep
     * these in field number order relative to normal fields or even to each
     * other. */
    size_t ext_count;
    const upb_msg_ext *ext = _upb_msg_getexts(msg, &ext_count);
    const upb_msg_ext *end = ext + ext_count;
    if (ext_count) {
      for (; ext != end; ext++) {
        if (UPB_UNLIKELY(m->ext == _UPB_MSGEXT_MSGSET)) {
          encode_msgset_item(e, ext);
        } else {
          encode_field(e, &ext->data, &ext->ext->sub, &ext->ext->field);
        }
      }
    }
  }

  const upb_msglayout_field *f = &m->fields[m->field_count];
  const upb_msglayout_field *first = &m->fields[0];
  while (f != first) {
    f--;
    if (encode_shouldencode(e, msg, m->subs, f)) {
      encode_field(e, msg, m->subs, f);
    }
  }

  *size = (e->limit - e->ptr) - pre_len;
}

char *upb_encode_ex(const void *msg, const upb_msglayout *l, int options,
                    upb_arena *arena, size_t *size) {
  upb_encstate e;
  unsigned depth = (unsigned)options >> 16;

  e.alloc = upb_arena_alloc(arena);
  e.buf = NULL;
  e.limit = NULL;
  e.ptr = NULL;
  e.depth = depth ? depth : 64;
  e.options = options;
  _upb_mapsorter_init(&e.sorter);
  char *ret = NULL;

  if (UPB_SETJMP(e.err)) {
    *size = 0;
    ret = NULL;
  } else {
    encode_message(&e, msg, l, size);
    *size = e.limit - e.ptr;
    if (*size == 0) {
      static char ch;
      ret = &ch;
    } else {
      UPB_ASSERT(e.ptr);
      ret = e.ptr;
    }
  }

  _upb_mapsorter_destroy(&e.sorter);
  return ret;
}

/** upb/msg.c ************************************************************/


/** upb_msg *******************************************************************/

static const size_t overhead = sizeof(upb_msg_internaldata);

static const upb_msg_internal *upb_msg_getinternal_const(const upb_msg *msg) {
  ptrdiff_t size = sizeof(upb_msg_internal);
  return (upb_msg_internal*)((char*)msg - size);
}

upb_msg *_upb_msg_new(const upb_msglayout *l, upb_arena *a) {
  return _upb_msg_new_inl(l, a);
}

void _upb_msg_clear(upb_msg *msg, const upb_msglayout *l) {
  void *mem = UPB_PTR_AT(msg, -sizeof(upb_msg_internal), char);
  memset(mem, 0, upb_msg_sizeof(l));
}

static bool realloc_internal(upb_msg *msg, size_t need, upb_arena *arena) {
  upb_msg_internal *in = upb_msg_getinternal(msg);
  if (!in->internal) {
    /* No internal data, allocate from scratch. */
    size_t size = UPB_MAX(128, _upb_lg2ceilsize(need + overhead));
    upb_msg_internaldata *internal = upb_arena_malloc(arena, size);
    if (!internal) return false;
    internal->size = size;
    internal->unknown_end = overhead;
    internal->ext_begin = size;
    in->internal = internal;
  } else if (in->internal->ext_begin - in->internal->unknown_end < need) {
    /* Internal data is too small, reallocate. */
    size_t new_size = _upb_lg2ceilsize(in->internal->size + need);
    size_t ext_bytes = in->internal->size - in->internal->ext_begin;
    size_t new_ext_begin = new_size - ext_bytes;
    upb_msg_internaldata *internal =
        upb_arena_realloc(arena, in->internal, in->internal->size, new_size);
    if (!internal) return false;
    if (ext_bytes) {
      /* Need to move extension data to the end. */
      char *ptr = (char*)internal;
      memmove(ptr + new_ext_begin, ptr + internal->ext_begin, ext_bytes);
    }
    internal->ext_begin = new_ext_begin;
    internal->size = new_size;
    in->internal = internal;
  }
  UPB_ASSERT(in->internal->ext_begin - in->internal->unknown_end >= need);
  return true;
}

bool _upb_msg_addunknown(upb_msg *msg, const char *data, size_t len,
                         upb_arena *arena) {
  if (!realloc_internal(msg, len, arena)) return false;
  upb_msg_internal *in = upb_msg_getinternal(msg);
  memcpy(UPB_PTR_AT(in->internal, in->internal->unknown_end, char), data, len);
  in->internal->unknown_end += len;
  return true;
}

void _upb_msg_discardunknown_shallow(upb_msg *msg) {
  upb_msg_internal *in = upb_msg_getinternal(msg);
  if (in->internal) {
    in->internal->unknown_end = overhead;
  }
}

const char *upb_msg_getunknown(const upb_msg *msg, size_t *len) {
  const upb_msg_internal *in = upb_msg_getinternal_const(msg);
  if (in->internal) {
    *len = in->internal->unknown_end - overhead;
    return (char*)(in->internal + 1);
  } else {
    *len = 0;
    return NULL;
  }
}

const upb_msg_ext *_upb_msg_getexts(const upb_msg *msg, size_t *count) {
  const upb_msg_internal *in = upb_msg_getinternal_const(msg);
  if (in->internal) {
    *count =
        (in->internal->size - in->internal->ext_begin) / sizeof(upb_msg_ext);
    return UPB_PTR_AT(in->internal, in->internal->ext_begin, void);
  } else {
    *count = 0;
    return NULL;
  }
}

const upb_msg_ext *_upb_msg_getext(const upb_msg *msg,
                                   const upb_msglayout_ext *e) {
  size_t n;
  const upb_msg_ext *ext = _upb_msg_getexts(msg, &n);

  /* For now we use linear search exclusively to find extensions. If this
   * becomes an issue due to messages with lots of extensions, we can introduce
   * a table of some sort. */
  for (size_t i = 0; i < n; i++) {
    if (ext[i].ext == e) {
      return &ext[i];
    }
  }

  return NULL;
}

void _upb_msg_clearext(upb_msg *msg, const upb_msglayout_ext *ext_l) {
  upb_msg_internal *in = upb_msg_getinternal(msg);
  if (!in->internal) return;
  const upb_msg_ext *base =
      UPB_PTR_AT(in->internal, in->internal->ext_begin, void);
  upb_msg_ext *ext = (upb_msg_ext*)_upb_msg_getext(msg, ext_l);
  if (ext) {
    *ext = *base;
    in->internal->ext_begin += sizeof(upb_msg_ext);
  }
}

upb_msg_ext *_upb_msg_getorcreateext(upb_msg *msg, const upb_msglayout_ext *e,
                                     upb_arena *arena) {
  upb_msg_ext *ext = (upb_msg_ext*)_upb_msg_getext(msg, e);
  if (ext) return ext;
  if (!realloc_internal(msg, sizeof(upb_msg_ext), arena)) return NULL;
  upb_msg_internal *in = upb_msg_getinternal(msg);
  in->internal->ext_begin -= sizeof(upb_msg_ext);
  ext = UPB_PTR_AT(in->internal, in->internal->ext_begin, void);
  memset(ext, 0, sizeof(upb_msg_ext));
  ext->ext = e;
  return ext;
}

size_t upb_msg_extcount(const upb_msg *msg) {
  size_t count;
  _upb_msg_getexts(msg, &count);
  return count;
}

/** upb_array *****************************************************************/

bool _upb_array_realloc(upb_array *arr, size_t min_size, upb_arena *arena) {
  size_t new_size = UPB_MAX(arr->size, 4);
  int elem_size_lg2 = arr->data & 7;
  size_t old_bytes = arr->size << elem_size_lg2;
  size_t new_bytes;
  void* ptr = _upb_array_ptr(arr);

  /* Log2 ceiling of size. */
  while (new_size < min_size) new_size *= 2;

  new_bytes = new_size << elem_size_lg2;
  ptr = upb_arena_realloc(arena, ptr, old_bytes, new_bytes);

  if (!ptr) {
    return false;
  }

  arr->data = _upb_tag_arrptr(ptr, elem_size_lg2);
  arr->size = new_size;
  return true;
}

static upb_array *getorcreate_array(upb_array **arr_ptr, int elem_size_lg2,
                                    upb_arena *arena) {
  upb_array *arr = *arr_ptr;
  if (!arr) {
    arr = _upb_array_new(arena, 4, elem_size_lg2);
    if (!arr) return NULL;
    *arr_ptr = arr;
  }
  return arr;
}

void *_upb_array_resize_fallback(upb_array **arr_ptr, size_t size,
                                 int elem_size_lg2, upb_arena *arena) {
  upb_array *arr = getorcreate_array(arr_ptr, elem_size_lg2, arena);
  return arr && _upb_array_resize(arr, size, arena) ? _upb_array_ptr(arr)
                                                    : NULL;
}

bool _upb_array_append_fallback(upb_array **arr_ptr, const void *value,
                                int elem_size_lg2, upb_arena *arena) {
  upb_array *arr = getorcreate_array(arr_ptr, elem_size_lg2, arena);
  if (!arr) return false;

  size_t elems = arr->len;

  if (!_upb_array_resize(arr, elems + 1, arena)) {
    return false;
  }

  char *data = _upb_array_ptr(arr);
  memcpy(data + (elems << elem_size_lg2), value, 1 << elem_size_lg2);
  return true;
}

/** upb_map *******************************************************************/

upb_map *_upb_map_new(upb_arena *a, size_t key_size, size_t value_size) {
  upb_map *map = upb_arena_malloc(a, sizeof(upb_map));

  if (!map) {
    return NULL;
  }

  upb_strtable_init(&map->table, 4, a);
  map->key_size = key_size;
  map->val_size = value_size;

  return map;
}

static void _upb_mapsorter_getkeys(const void *_a, const void *_b, void *a_key,
                                   void *b_key, size_t size) {
  const upb_tabent *const*a = _a;
  const upb_tabent *const*b = _b;
  upb_strview a_tabkey = upb_tabstrview((*a)->key);
  upb_strview b_tabkey = upb_tabstrview((*b)->key);
  _upb_map_fromkey(a_tabkey, a_key, size);
  _upb_map_fromkey(b_tabkey, b_key, size);
}

#define UPB_COMPARE_INTEGERS(a, b) ((a) < (b) ? -1 : ((a) == (b) ? 0 : 1))

static int _upb_mapsorter_cmpi64(const void *_a, const void *_b) {
  int64_t a, b;
  _upb_mapsorter_getkeys(_a, _b, &a, &b, 8);
  return UPB_COMPARE_INTEGERS(a, b);
}

static int _upb_mapsorter_cmpu64(const void *_a, const void *_b) {
  uint64_t a, b;
  _upb_mapsorter_getkeys(_a, _b, &a, &b, 8);
  return UPB_COMPARE_INTEGERS(a, b);
}

static int _upb_mapsorter_cmpi32(const void *_a, const void *_b) {
  int32_t a, b;
  _upb_mapsorter_getkeys(_a, _b, &a, &b, 4);
  return UPB_COMPARE_INTEGERS(a, b);
}

static int _upb_mapsorter_cmpu32(const void *_a, const void *_b) {
  uint32_t a, b;
  _upb_mapsorter_getkeys(_a, _b, &a, &b, 4);
  return UPB_COMPARE_INTEGERS(a, b);
}

static int _upb_mapsorter_cmpbool(const void *_a, const void *_b) {
  bool a, b;
  _upb_mapsorter_getkeys(_a, _b, &a, &b, 1);
  return UPB_COMPARE_INTEGERS(a, b);
}

static int _upb_mapsorter_cmpstr(const void *_a, const void *_b) {
  upb_strview a, b;
  _upb_mapsorter_getkeys(_a, _b, &a, &b, UPB_MAPTYPE_STRING);
  size_t common_size = UPB_MIN(a.size, b.size);
  int cmp = memcmp(a.data, b.data, common_size);
  if (cmp) return -cmp;
  return UPB_COMPARE_INTEGERS(a.size, b.size);
}

#undef UPB_COMPARE_INTEGERS

bool _upb_mapsorter_pushmap(_upb_mapsorter *s, upb_descriptortype_t key_type,
                            const upb_map *map, _upb_sortedmap *sorted) {
  int map_size = _upb_map_size(map);
  sorted->start = s->size;
  sorted->pos = sorted->start;
  sorted->end = sorted->start + map_size;

  /* Grow s->entries if necessary. */
  if (sorted->end > s->cap) {
    s->cap = _upb_lg2ceilsize(sorted->end);
    s->entries = realloc(s->entries, s->cap * sizeof(*s->entries));
    if (!s->entries) return false;
  }

  s->size = sorted->end;

  /* Copy non-empty entries from the table to s->entries. */
  upb_tabent const**dst = &s->entries[sorted->start];
  const upb_tabent *src = map->table.t.entries;
  const upb_tabent *end = src + upb_table_size(&map->table.t);
  for (; src < end; src++) {
    if (!upb_tabent_isempty(src)) {
      *dst = src;
      dst++;
    }
  }
  UPB_ASSERT(dst == &s->entries[sorted->end]);

  /* Sort entries according to the key type. */

  int (*compar)(const void *, const void *);

  switch (key_type) {
    case UPB_DESCRIPTOR_TYPE_INT64:
    case UPB_DESCRIPTOR_TYPE_SFIXED64:
    case UPB_DESCRIPTOR_TYPE_SINT64:
      compar = _upb_mapsorter_cmpi64;
      break;
    case UPB_DESCRIPTOR_TYPE_UINT64:
    case UPB_DESCRIPTOR_TYPE_FIXED64:
      compar = _upb_mapsorter_cmpu64;
      break;
    case UPB_DESCRIPTOR_TYPE_INT32:
    case UPB_DESCRIPTOR_TYPE_SINT32:
    case UPB_DESCRIPTOR_TYPE_SFIXED32:
    case UPB_DESCRIPTOR_TYPE_ENUM:
      compar = _upb_mapsorter_cmpi32;
      break;
    case UPB_DESCRIPTOR_TYPE_UINT32:
    case UPB_DESCRIPTOR_TYPE_FIXED32:
      compar = _upb_mapsorter_cmpu32;
      break;
    case UPB_DESCRIPTOR_TYPE_BOOL:
      compar = _upb_mapsorter_cmpbool;
      break;
    case UPB_DESCRIPTOR_TYPE_STRING:
    case UPB_DESCRIPTOR_TYPE_BYTES:
      compar = _upb_mapsorter_cmpstr;
      break;
    default:
      UPB_UNREACHABLE();
  }

  qsort(&s->entries[sorted->start], map_size, sizeof(*s->entries), compar);
  return true;
}

/** upb_extreg ****************************************************************/

struct upb_extreg {
  upb_arena *arena;
  upb_strtable exts;  /* Key is upb_msglayout* concatenated with fieldnum. */
};

#define EXTREG_KEY_SIZE (sizeof(upb_msglayout*) + sizeof(uint32_t))

static void extreg_key(char *buf, const upb_msglayout *l, uint32_t fieldnum) {
  memcpy(buf, &l, sizeof(l));
  memcpy(buf + sizeof(l), &fieldnum, sizeof(fieldnum));
}

upb_extreg *upb_extreg_new(upb_arena *arena) {
  upb_extreg *r = upb_arena_malloc(arena, sizeof(*r));
  if (!r) return NULL;
  r->arena = arena;
  if (!upb_strtable_init(&r->exts, 8, arena)) return NULL;
  return r;
}

bool _upb_extreg_add(upb_extreg *r, const upb_msglayout_ext **e, size_t count) {
  char buf[EXTREG_KEY_SIZE];
  const upb_msglayout_ext **start = e;
  const upb_msglayout_ext **end = UPB_PTRADD(e, count);
  for (; e < end; e++) {
    const upb_msglayout_ext *ext = *e;
    extreg_key(buf, ext->extendee, ext->field.number);
    if (!upb_strtable_insert(&r->exts, buf, EXTREG_KEY_SIZE,
                             upb_value_constptr(ext), r->arena)) {
      goto failure;
    }
  }
  return true;

failure:
  /* Back out the entries previously added. */
  for (end = e, e = start; e < end; e++) {
    const upb_msglayout_ext *ext = *e;
    extreg_key(buf, ext->extendee, ext->field.number);
    upb_strtable_remove2(&r->exts, buf, EXTREG_KEY_SIZE, NULL);
  }
  return false;
}

const upb_msglayout_ext *_upb_extreg_get(const upb_extreg *r,
                                         const upb_msglayout *l, uint32_t num) {
  char buf[EXTREG_KEY_SIZE];
  upb_value v;
  extreg_key(buf, l, num);
  if (upb_strtable_lookup2(&r->exts, buf, EXTREG_KEY_SIZE, &v)) {
    return upb_value_getconstptr(v);
  } else {
    return NULL;
  }
}

/** upb/table.c ************************************************************/
/*
 * upb_table Implementation
 *
 * Implementation is heavily inspired by Lua's ltable.c.
 */

#include <string.h>


/* Must be last. */

#define UPB_MAXARRSIZE 16  /* 64k. */

/* From Chromium. */
#define ARRAY_SIZE(x) \
    ((sizeof(x)/sizeof(0[x])) / ((size_t)(!(sizeof(x) % sizeof(0[x])))))

static const double MAX_LOAD = 0.85;

/* The minimum utilization of the array part of a mixed hash/array table.  This
 * is a speed/memory-usage tradeoff (though it's not straightforward because of
 * cache effects).  The lower this is, the more memory we'll use. */
static const double MIN_DENSITY = 0.1;

static bool is_pow2(uint64_t v) { return v == 0 || (v & (v - 1)) == 0; }

static upb_value _upb_value_val(uint64_t val) {
  upb_value ret;
  _upb_value_setval(&ret, val);
  return ret;
}

static int log2ceil(uint64_t v) {
  int ret = 0;
  bool pow2 = is_pow2(v);
  while (v >>= 1) ret++;
  ret = pow2 ? ret : ret + 1;  /* Ceiling. */
  return UPB_MIN(UPB_MAXARRSIZE, ret);
}

char *upb_strdup2(const char *s, size_t len, upb_arena *a) {
  size_t n;
  char *p;

  /* Prevent overflow errors. */
  if (len == SIZE_MAX) return NULL;
  /* Always null-terminate, even if binary data; but don't rely on the input to
   * have a null-terminating byte since it may be a raw binary buffer. */
  n = len + 1;
  p = upb_arena_malloc(a, n);
  if (p) {
    memcpy(p, s, len);
    p[len] = 0;
  }
  return p;
}

/* A type to represent the lookup key of either a strtable or an inttable. */
typedef union {
  uintptr_t num;
  struct {
    const char *str;
    size_t len;
  } str;
} lookupkey_t;

static lookupkey_t strkey2(const char *str, size_t len) {
  lookupkey_t k;
  k.str.str = str;
  k.str.len = len;
  return k;
}

static lookupkey_t intkey(uintptr_t key) {
  lookupkey_t k;
  k.num = key;
  return k;
}

typedef uint32_t hashfunc_t(upb_tabkey key);
typedef bool eqlfunc_t(upb_tabkey k1, lookupkey_t k2);

/* Base table (shared code) ***************************************************/

static uint32_t upb_inthash(uintptr_t key) {
  return (uint32_t)key;
}

static const upb_tabent *upb_getentry(const upb_table *t, uint32_t hash) {
  return t->entries + (hash & t->mask);
}

static bool upb_arrhas(upb_tabval key) {
  return key.val != (uint64_t)-1;
}


static bool isfull(upb_table *t) {
  return t->count == t->max_count;
}

static bool init(upb_table *t, uint8_t size_lg2, upb_arena *a) {
  size_t bytes;

  t->count = 0;
  t->size_lg2 = size_lg2;
  t->mask = upb_table_size(t) ? upb_table_size(t) - 1 : 0;
  t->max_count = upb_table_size(t) * MAX_LOAD;
  bytes = upb_table_size(t) * sizeof(upb_tabent);
  if (bytes > 0) {
    t->entries = upb_arena_malloc(a, bytes);
    if (!t->entries) return false;
    memset(t->entries, 0, bytes);
  } else {
    t->entries = NULL;
  }
  return true;
}

static upb_tabent *emptyent(upb_table *t, upb_tabent *e) {
  upb_tabent *begin = t->entries;
  upb_tabent *end = begin + upb_table_size(t);
  for (e = e + 1; e < end; e++) {
    if (upb_tabent_isempty(e)) return e;
  }
  for (e = begin; e < end; e++) {
    if (upb_tabent_isempty(e)) return e;
  }
  UPB_ASSERT(false);
  return NULL;
}

static upb_tabent *getentry_mutable(upb_table *t, uint32_t hash) {
  return (upb_tabent*)upb_getentry(t, hash);
}

static const upb_tabent *findentry(const upb_table *t, lookupkey_t key,
                                   uint32_t hash, eqlfunc_t *eql) {
  const upb_tabent *e;

  if (t->size_lg2 == 0) return NULL;
  e = upb_getentry(t, hash);
  if (upb_tabent_isempty(e)) return NULL;
  while (1) {
    if (eql(e->key, key)) return e;
    if ((e = e->next) == NULL) return NULL;
  }
}

static upb_tabent *findentry_mutable(upb_table *t, lookupkey_t key,
                                     uint32_t hash, eqlfunc_t *eql) {
  return (upb_tabent*)findentry(t, key, hash, eql);
}

static bool lookup(const upb_table *t, lookupkey_t key, upb_value *v,
                   uint32_t hash, eqlfunc_t *eql) {
  const upb_tabent *e = findentry(t, key, hash, eql);
  if (e) {
    if (v) {
      _upb_value_setval(v, e->val.val);
    }
    return true;
  } else {
    return false;
  }
}

/* The given key must not already exist in the table. */
static void insert(upb_table *t, lookupkey_t key, upb_tabkey tabkey,
                   upb_value val, uint32_t hash,
                   hashfunc_t *hashfunc, eqlfunc_t *eql) {
  upb_tabent *mainpos_e;
  upb_tabent *our_e;

  UPB_ASSERT(findentry(t, key, hash, eql) == NULL);

  t->count++;
  mainpos_e = getentry_mutable(t, hash);
  our_e = mainpos_e;

  if (upb_tabent_isempty(mainpos_e)) {
    /* Our main position is empty; use it. */
    our_e->next = NULL;
  } else {
    /* Collision. */
    upb_tabent *new_e = emptyent(t, mainpos_e);
    /* Head of collider's chain. */
    upb_tabent *chain = getentry_mutable(t, hashfunc(mainpos_e->key));
    if (chain == mainpos_e) {
      /* Existing ent is in its main position (it has the same hash as us, and
       * is the head of our chain).  Insert to new ent and append to this chain. */
      new_e->next = mainpos_e->next;
      mainpos_e->next = new_e;
      our_e = new_e;
    } else {
      /* Existing ent is not in its main position (it is a node in some other
       * chain).  This implies that no existing ent in the table has our hash.
       * Evict it (updating its chain) and use its ent for head of our chain. */
      *new_e = *mainpos_e;  /* copies next. */
      while (chain->next != mainpos_e) {
        chain = (upb_tabent*)chain->next;
        UPB_ASSERT(chain);
      }
      chain->next = new_e;
      our_e = mainpos_e;
      our_e->next = NULL;
    }
  }
  our_e->key = tabkey;
  our_e->val.val = val.val;
  UPB_ASSERT(findentry(t, key, hash, eql) == our_e);
}

static bool rm(upb_table *t, lookupkey_t key, upb_value *val,
               upb_tabkey *removed, uint32_t hash, eqlfunc_t *eql) {
  upb_tabent *chain = getentry_mutable(t, hash);
  if (upb_tabent_isempty(chain)) return false;
  if (eql(chain->key, key)) {
    /* Element to remove is at the head of its chain. */
    t->count--;
    if (val) _upb_value_setval(val, chain->val.val);
    if (removed) *removed = chain->key;
    if (chain->next) {
      upb_tabent *move = (upb_tabent*)chain->next;
      *chain = *move;
      move->key = 0;  /* Make the slot empty. */
    } else {
      chain->key = 0;  /* Make the slot empty. */
    }
    return true;
  } else {
    /* Element to remove is either in a non-head position or not in the
     * table. */
    while (chain->next && !eql(chain->next->key, key)) {
      chain = (upb_tabent*)chain->next;
    }
    if (chain->next) {
      /* Found element to remove. */
      upb_tabent *rm = (upb_tabent*)chain->next;
      t->count--;
      if (val) _upb_value_setval(val, chain->next->val.val);
      if (removed) *removed = rm->key;
      rm->key = 0;  /* Make the slot empty. */
      chain->next = rm->next;
      return true;
    } else {
      /* Element to remove is not in the table. */
      return false;
    }
  }
}

static size_t next(const upb_table *t, size_t i) {
  do {
    if (++i >= upb_table_size(t))
      return SIZE_MAX - 1;  /* Distinct from -1. */
  } while(upb_tabent_isempty(&t->entries[i]));

  return i;
}

static size_t begin(const upb_table *t) {
  return next(t, -1);
}


/* upb_strtable ***************************************************************/

/* A simple "subclass" of upb_table that only adds a hash function for strings. */

static upb_tabkey strcopy(lookupkey_t k2, upb_arena *a) {
  uint32_t len = (uint32_t) k2.str.len;
  char *str = upb_arena_malloc(a, k2.str.len + sizeof(uint32_t) + 1);
  if (str == NULL) return 0;
  memcpy(str, &len, sizeof(uint32_t));
  if (k2.str.len) memcpy(str + sizeof(uint32_t), k2.str.str, k2.str.len);
  str[sizeof(uint32_t) + k2.str.len] = '\0';
  return (uintptr_t)str;
}

/* Adapted from ABSL's wyhash. */

static uint64_t UnalignedLoad64(const void *p) {
  uint64_t val;
  memcpy(&val, p, 8);
  return val;
}

static uint32_t UnalignedLoad32(const void *p) {
  uint32_t val;
  memcpy(&val, p, 4);
  return val;
}

#if defined(_MSC_VER) && defined(_M_X64)
#include <intrin.h>
#endif

/* Computes a * b, returning the low 64 bits of the result and storing the high
 * 64 bits in |*high|. */
static uint64_t upb_umul128(uint64_t v0, uint64_t v1, uint64_t* out_high) {
#ifdef __SIZEOF_INT128__
  __uint128_t p = v0;
  p *= v1;
  *out_high = (uint64_t)(p >> 64);
  return (uint64_t)p;
#elif defined(_MSC_VER) && defined(_M_X64)
  return _umul128(v0, v1, out_high);
#else
  uint64_t a32 = v0 >> 32;
  uint64_t a00 = v0 & 0xffffffff;
  uint64_t b32 = v1 >> 32;
  uint64_t b00 = v1 & 0xffffffff;
  uint64_t high = a32 * b32;
  uint64_t low = a00 * b00;
  uint64_t mid1 = a32 * b00;
  uint64_t mid2 = a00 * b32;
  low += (mid1 << 32) + (mid2 << 32);
  // Omit carry bit, for mixing we do not care about exact numerical precision.
  high += (mid1 >> 32) + (mid2 >> 32);
  *out_high = high;
  return low;
#endif
}

static uint64_t WyhashMix(uint64_t v0, uint64_t v1) {
  uint64_t high;
  uint64_t low = upb_umul128(v0, v1, &high);
  return low ^ high;
}

static uint64_t Wyhash(const void *data, size_t len, uint64_t seed,
                       const uint64_t salt[]) {
  const uint8_t* ptr = (const uint8_t*)data;
  uint64_t starting_length = (uint64_t)len;
  uint64_t current_state = seed ^ salt[0];

  if (len > 64) {
    // If we have more than 64 bytes, we're going to handle chunks of 64
    // bytes at a time. We're going to build up two separate hash states
    // which we will then hash together.
    uint64_t duplicated_state = current_state;

    do {
      uint64_t a = UnalignedLoad64(ptr);
      uint64_t b = UnalignedLoad64(ptr + 8);
      uint64_t c = UnalignedLoad64(ptr + 16);
      uint64_t d = UnalignedLoad64(ptr + 24);
      uint64_t e = UnalignedLoad64(ptr + 32);
      uint64_t f = UnalignedLoad64(ptr + 40);
      uint64_t g = UnalignedLoad64(ptr + 48);
      uint64_t h = UnalignedLoad64(ptr + 56);

      uint64_t cs0 = WyhashMix(a ^ salt[1], b ^ current_state);
      uint64_t cs1 = WyhashMix(c ^ salt[2], d ^ current_state);
      current_state = (cs0 ^ cs1);

      uint64_t ds0 = WyhashMix(e ^ salt[3], f ^ duplicated_state);
      uint64_t ds1 = WyhashMix(g ^ salt[4], h ^ duplicated_state);
      duplicated_state = (ds0 ^ ds1);

      ptr += 64;
      len -= 64;
    } while (len > 64);

    current_state = current_state ^ duplicated_state;
  }

  // We now have a data `ptr` with at most 64 bytes and the current state
  // of the hashing state machine stored in current_state.
  while (len > 16) {
    uint64_t a = UnalignedLoad64(ptr);
    uint64_t b = UnalignedLoad64(ptr + 8);

    current_state = WyhashMix(a ^ salt[1], b ^ current_state);

    ptr += 16;
    len -= 16;
  }

  // We now have a data `ptr` with at most 16 bytes.
  uint64_t a = 0;
  uint64_t b = 0;
  if (len > 8) {
    // When we have at least 9 and at most 16 bytes, set A to the first 64
    // bits of the input and B to the last 64 bits of the input. Yes, they will
    // overlap in the middle if we are working with less than the full 16
    // bytes.
    a = UnalignedLoad64(ptr);
    b = UnalignedLoad64(ptr + len - 8);
  } else if (len > 3) {
    // If we have at least 4 and at most 8 bytes, set A to the first 32
    // bits and B to the last 32 bits.
    a = UnalignedLoad32(ptr);
    b = UnalignedLoad32(ptr + len - 4);
  } else if (len > 0) {
    // If we have at least 1 and at most 3 bytes, read all of the provided
    // bits into A, with some adjustments.
    a = ((ptr[0] << 16) | (ptr[len >> 1] << 8) | ptr[len - 1]);
    b = 0;
  } else {
    a = 0;
    b = 0;
  }

  uint64_t w = WyhashMix(a ^ salt[1], b ^ current_state);
  uint64_t z = salt[1] ^ starting_length;
  return WyhashMix(w, z);
}

const uint64_t kWyhashSalt[5] = {
    0x243F6A8885A308D3ULL, 0x13198A2E03707344ULL, 0xA4093822299F31D0ULL,
    0x082EFA98EC4E6C89ULL, 0x452821E638D01377ULL,
};

static uint32_t table_hash(const char *p, size_t n) {
  return Wyhash(p, n, 0, kWyhashSalt);
}

static uint32_t strhash(upb_tabkey key) {
  uint32_t len;
  char *str = upb_tabstr(key, &len);
  return table_hash(str, len);
}

static bool streql(upb_tabkey k1, lookupkey_t k2) {
  uint32_t len;
  char *str = upb_tabstr(k1, &len);
  return len == k2.str.len && (len == 0 || memcmp(str, k2.str.str, len) == 0);
}

bool upb_strtable_init(upb_strtable *t, size_t expected_size, upb_arena *a) {
  // Multiply by approximate reciprocal of MAX_LOAD (0.85), with pow2 denominator.
  size_t need_entries = (expected_size + 1) * 1204 / 1024;
  UPB_ASSERT(need_entries >= expected_size * 0.85);
  int size_lg2 = _upb_lg2ceil(need_entries);
  return init(&t->t, size_lg2, a);
}

void upb_strtable_clear(upb_strtable *t) {
  size_t bytes = upb_table_size(&t->t) * sizeof(upb_tabent);
  t->t.count = 0;
  memset((char*)t->t.entries, 0, bytes);
}

bool upb_strtable_resize(upb_strtable *t, size_t size_lg2, upb_arena *a) {
  upb_strtable new_table;
  upb_strtable_iter i;

  if (!init(&new_table.t, size_lg2, a))
    return false;
  upb_strtable_begin(&i, t);
  for ( ; !upb_strtable_done(&i); upb_strtable_next(&i)) {
    upb_strview key = upb_strtable_iter_key(&i);
    upb_strtable_insert(&new_table, key.data, key.size,
                        upb_strtable_iter_value(&i), a);
  }
  *t = new_table;
  return true;
}

bool upb_strtable_insert(upb_strtable *t, const char *k, size_t len,
                         upb_value v, upb_arena *a) {
  lookupkey_t key;
  upb_tabkey tabkey;
  uint32_t hash;

  if (isfull(&t->t)) {
    /* Need to resize.  New table of double the size, add old elements to it. */
    if (!upb_strtable_resize(t, t->t.size_lg2 + 1, a)) {
      return false;
    }
  }

  key = strkey2(k, len);
  tabkey = strcopy(key, a);
  if (tabkey == 0) return false;

  hash = table_hash(key.str.str, key.str.len);
  insert(&t->t, key, tabkey, v, hash, &strhash, &streql);
  return true;
}

bool upb_strtable_lookup2(const upb_strtable *t, const char *key, size_t len,
                          upb_value *v) {
  uint32_t hash = table_hash(key, len);
  return lookup(&t->t, strkey2(key, len), v, hash, &streql);
}

bool upb_strtable_remove2(upb_strtable *t, const char *key, size_t len,
                          upb_value *val) {
  uint32_t hash = table_hash(key, len);
  upb_tabkey tabkey;
  return rm(&t->t, strkey2(key, len), val, &tabkey, hash, &streql);
}

/* Iteration */

void upb_strtable_begin(upb_strtable_iter *i, const upb_strtable *t) {
  i->t = t;
  i->index = begin(&t->t);
}

void upb_strtable_next(upb_strtable_iter *i) {
  i->index = next(&i->t->t, i->index);
}

bool upb_strtable_done(const upb_strtable_iter *i) {
  if (!i->t) return true;
  return i->index >= upb_table_size(&i->t->t) ||
         upb_tabent_isempty(str_tabent(i));
}

upb_strview upb_strtable_iter_key(const upb_strtable_iter *i) {
  upb_strview key;
  uint32_t len;
  UPB_ASSERT(!upb_strtable_done(i));
  key.data = upb_tabstr(str_tabent(i)->key, &len);
  key.size = len;
  return key;
}

upb_value upb_strtable_iter_value(const upb_strtable_iter *i) {
  UPB_ASSERT(!upb_strtable_done(i));
  return _upb_value_val(str_tabent(i)->val.val);
}

void upb_strtable_iter_setdone(upb_strtable_iter *i) {
  i->t = NULL;
  i->index = SIZE_MAX;
}

bool upb_strtable_iter_isequal(const upb_strtable_iter *i1,
                               const upb_strtable_iter *i2) {
  if (upb_strtable_done(i1) && upb_strtable_done(i2))
    return true;
  return i1->t == i2->t && i1->index == i2->index;
}


/* upb_inttable ***************************************************************/

/* For inttables we use a hybrid structure where small keys are kept in an
 * array and large keys are put in the hash table. */

static uint32_t inthash(upb_tabkey key) { return upb_inthash(key); }

static bool inteql(upb_tabkey k1, lookupkey_t k2) {
  return k1 == k2.num;
}

static upb_tabval *mutable_array(upb_inttable *t) {
  return (upb_tabval*)t->array;
}

static upb_tabval *inttable_val(upb_inttable *t, uintptr_t key) {
  if (key < t->array_size) {
    return upb_arrhas(t->array[key]) ? &(mutable_array(t)[key]) : NULL;
  } else {
    upb_tabent *e =
        findentry_mutable(&t->t, intkey(key), upb_inthash(key), &inteql);
    return e ? &e->val : NULL;
  }
}

static const upb_tabval *inttable_val_const(const upb_inttable *t,
                                            uintptr_t key) {
  return inttable_val((upb_inttable*)t, key);
}

size_t upb_inttable_count(const upb_inttable *t) {
  return t->t.count + t->array_count;
}

static void check(upb_inttable *t) {
  UPB_UNUSED(t);
#if defined(UPB_DEBUG_TABLE) && !defined(NDEBUG)
  {
    /* This check is very expensive (makes inserts/deletes O(N)). */
    size_t count = 0;
    upb_inttable_iter i;
    upb_inttable_begin(&i, t);
    for(; !upb_inttable_done(&i); upb_inttable_next(&i), count++) {
      UPB_ASSERT(upb_inttable_lookup(t, upb_inttable_iter_key(&i), NULL));
    }
    UPB_ASSERT(count == upb_inttable_count(t));
  }
#endif
}

bool upb_inttable_sizedinit(upb_inttable *t, size_t asize, int hsize_lg2,
                            upb_arena *a) {
  size_t array_bytes;

  if (!init(&t->t, hsize_lg2, a)) return false;
  /* Always make the array part at least 1 long, so that we know key 0
   * won't be in the hash part, which simplifies things. */
  t->array_size = UPB_MAX(1, asize);
  t->array_count = 0;
  array_bytes = t->array_size * sizeof(upb_value);
  t->array = upb_arena_malloc(a, array_bytes);
  if (!t->array) {
    return false;
  }
  memset(mutable_array(t), 0xff, array_bytes);
  check(t);
  return true;
}

bool upb_inttable_init(upb_inttable *t, upb_arena *a) {
  return upb_inttable_sizedinit(t, 0, 4, a);
}

bool upb_inttable_insert(upb_inttable *t, uintptr_t key, upb_value val,
                         upb_arena *a) {
  upb_tabval tabval;
  tabval.val = val.val;
  UPB_ASSERT(upb_arrhas(tabval));  /* This will reject (uint64_t)-1.  Fix this. */

  if (key < t->array_size) {
    UPB_ASSERT(!upb_arrhas(t->array[key]));
    t->array_count++;
    mutable_array(t)[key].val = val.val;
  } else {
    if (isfull(&t->t)) {
      /* Need to resize the hash part, but we re-use the array part. */
      size_t i;
      upb_table new_table;

      if (!init(&new_table, t->t.size_lg2 + 1, a)) {
        return false;
      }

      for (i = begin(&t->t); i < upb_table_size(&t->t); i = next(&t->t, i)) {
        const upb_tabent *e = &t->t.entries[i];
        uint32_t hash;
        upb_value v;

        _upb_value_setval(&v, e->val.val);
        hash = upb_inthash(e->key);
        insert(&new_table, intkey(e->key), e->key, v, hash, &inthash, &inteql);
      }

      UPB_ASSERT(t->t.count == new_table.count);

      t->t = new_table;
    }
    insert(&t->t, intkey(key), key, val, upb_inthash(key), &inthash, &inteql);
  }
  check(t);
  return true;
}

bool upb_inttable_lookup(const upb_inttable *t, uintptr_t key, upb_value *v) {
  const upb_tabval *table_v = inttable_val_const(t, key);
  if (!table_v) return false;
  if (v) _upb_value_setval(v, table_v->val);
  return true;
}

bool upb_inttable_replace(upb_inttable *t, uintptr_t key, upb_value val) {
  upb_tabval *table_v = inttable_val(t, key);
  if (!table_v) return false;
  table_v->val = val.val;
  return true;
}

bool upb_inttable_remove(upb_inttable *t, uintptr_t key, upb_value *val) {
  bool success;
  if (key < t->array_size) {
    if (upb_arrhas(t->array[key])) {
      upb_tabval empty = UPB_TABVALUE_EMPTY_INIT;
      t->array_count--;
      if (val) {
        _upb_value_setval(val, t->array[key].val);
      }
      mutable_array(t)[key] = empty;
      success = true;
    } else {
      success = false;
    }
  } else {
    success = rm(&t->t, intkey(key), val, NULL, upb_inthash(key), &inteql);
  }
  check(t);
  return success;
}

void upb_inttable_compact(upb_inttable *t, upb_arena *a) {
  /* A power-of-two histogram of the table keys. */
  size_t counts[UPB_MAXARRSIZE + 1] = {0};

  /* The max key in each bucket. */
  uintptr_t max[UPB_MAXARRSIZE + 1] = {0};

  upb_inttable_iter i;
  size_t arr_count;
  int size_lg2;
  upb_inttable new_t;

  upb_inttable_begin(&i, t);
  for (; !upb_inttable_done(&i); upb_inttable_next(&i)) {
    uintptr_t key = upb_inttable_iter_key(&i);
    int bucket = log2ceil(key);
    max[bucket] = UPB_MAX(max[bucket], key);
    counts[bucket]++;
  }

  /* Find the largest power of two that satisfies the MIN_DENSITY
   * definition (while actually having some keys). */
  arr_count = upb_inttable_count(t);

  for (size_lg2 = ARRAY_SIZE(counts) - 1; size_lg2 > 0; size_lg2--) {
    if (counts[size_lg2] == 0) {
      /* We can halve again without losing any entries. */
      continue;
    } else if (arr_count >= (1 << size_lg2) * MIN_DENSITY) {
      break;
    }

    arr_count -= counts[size_lg2];
  }

  UPB_ASSERT(arr_count <= upb_inttable_count(t));

  {
    /* Insert all elements into new, perfectly-sized table. */
    size_t arr_size = max[size_lg2] + 1;  /* +1 so arr[max] will fit. */
    size_t hash_count = upb_inttable_count(t) - arr_count;
    size_t hash_size = hash_count ? (hash_count / MAX_LOAD) + 1 : 0;
    int hashsize_lg2 = log2ceil(hash_size);

    upb_inttable_sizedinit(&new_t, arr_size, hashsize_lg2, a);
    upb_inttable_begin(&i, t);
    for (; !upb_inttable_done(&i); upb_inttable_next(&i)) {
      uintptr_t k = upb_inttable_iter_key(&i);
      upb_inttable_insert(&new_t, k, upb_inttable_iter_value(&i), a);
    }
    UPB_ASSERT(new_t.array_size == arr_size);
    UPB_ASSERT(new_t.t.size_lg2 == hashsize_lg2);
  }
  *t = new_t;
}

/* Iteration. */

static const upb_tabent *int_tabent(const upb_inttable_iter *i) {
  UPB_ASSERT(!i->array_part);
  return &i->t->t.entries[i->index];
}

static upb_tabval int_arrent(const upb_inttable_iter *i) {
  UPB_ASSERT(i->array_part);
  return i->t->array[i->index];
}

void upb_inttable_begin(upb_inttable_iter *i, const upb_inttable *t) {
  i->t = t;
  i->index = -1;
  i->array_part = true;
  upb_inttable_next(i);
}

void upb_inttable_next(upb_inttable_iter *iter) {
  const upb_inttable *t = iter->t;
  if (iter->array_part) {
    while (++iter->index < t->array_size) {
      if (upb_arrhas(int_arrent(iter))) {
        return;
      }
    }
    iter->array_part = false;
    iter->index = begin(&t->t);
  } else {
    iter->index = next(&t->t, iter->index);
  }
}

bool upb_inttable_next2(const upb_inttable *t, uintptr_t *key, upb_value *val,
                        intptr_t *iter) {
  intptr_t i = *iter;
  if (i < t->array_size) {
    while (++i < t->array_size) {
      upb_tabval ent = t->array[i];
      if (upb_arrhas(ent)) {
        *key = i;
        *val = _upb_value_val(ent.val);
        *iter = i;
        return true;
      }
    }
  }

  size_t tab_idx = next(&t->t, i == -1 ? -1 : i - t->array_size);
  if (tab_idx < upb_table_size(&t->t)) {
    upb_tabent *ent = &t->t.entries[tab_idx];
    *key = ent->key;
    *val = _upb_value_val(ent->val.val);
    *iter = tab_idx + t->array_size;
    return true;
  }

  return false;
}

void upb_inttable_removeiter(upb_inttable *t, intptr_t *iter) {
  intptr_t i = *iter;
  if (i < t->array_size) {
    t->array_count--;
    mutable_array(t)[i].val = -1;
  } else {
    upb_tabent *ent = &t->t.entries[i - t->array_size];
    upb_tabent *prev = NULL;

    // Linear search, not great.
    upb_tabent *end = &t->t.entries[upb_table_size(&t->t)];
    for (upb_tabent *e = t->t.entries; e != end; e++) {
      if (e->next == ent) {
        prev = e;
        break;
      }
    }

    if (prev) {
      prev->next = ent->next;
    }

    t->t.count--;
    ent->key = 0;
    ent->next = NULL;
  }
}

bool upb_strtable_next2(const upb_strtable *t, upb_strview *key, upb_value *val,
                        intptr_t *iter) {
  size_t tab_idx = next(&t->t, *iter);
  if (tab_idx < upb_table_size(&t->t)) {
    upb_tabent *ent = &t->t.entries[tab_idx];
    uint32_t len;
    key->data = upb_tabstr(ent->key, &len);
    key->size = len;
    *val = _upb_value_val(ent->val.val);
    *iter = tab_idx;
    return true;
  }

  return false;
}

void upb_strtable_removeiter(upb_strtable *t, intptr_t *iter) {
  intptr_t i = *iter;
  upb_tabent *ent = &t->t.entries[i];
  upb_tabent *prev = NULL;

  // Linear search, not great.
  upb_tabent *end = &t->t.entries[upb_table_size(&t->t)];
  for (upb_tabent *e = t->t.entries; e != end; e++) {
    if (e->next == ent) {
      prev = e;
      break;
    }
  }

  if (prev) {
    prev->next = ent->next;
  }

  t->t.count--;
  ent->key = 0;
  ent->next = NULL;
}

bool upb_inttable_done(const upb_inttable_iter *i) {
  if (!i->t) return true;
  if (i->array_part) {
    return i->index >= i->t->array_size ||
           !upb_arrhas(int_arrent(i));
  } else {
    return i->index >= upb_table_size(&i->t->t) ||
           upb_tabent_isempty(int_tabent(i));
  }
}

uintptr_t upb_inttable_iter_key(const upb_inttable_iter *i) {
  UPB_ASSERT(!upb_inttable_done(i));
  return i->array_part ? i->index : int_tabent(i)->key;
}

upb_value upb_inttable_iter_value(const upb_inttable_iter *i) {
  UPB_ASSERT(!upb_inttable_done(i));
  return _upb_value_val(
      i->array_part ? i->t->array[i->index].val : int_tabent(i)->val.val);
}

void upb_inttable_iter_setdone(upb_inttable_iter *i) {
  i->t = NULL;
  i->index = SIZE_MAX;
  i->array_part = false;
}

bool upb_inttable_iter_isequal(const upb_inttable_iter *i1,
                                          const upb_inttable_iter *i2) {
  if (upb_inttable_done(i1) && upb_inttable_done(i2))
    return true;
  return i1->t == i2->t && i1->index == i2->index &&
         i1->array_part == i2->array_part;
}

/** upb/upb.c ************************************************************/

#include <errno.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


/* upb_status *****************************************************************/

void upb_status_clear(upb_status *status) {
  if (!status) return;
  status->ok = true;
  status->msg[0] = '\0';
}

bool upb_ok(const upb_status *status) { return status->ok; }

const char *upb_status_errmsg(const upb_status *status) { return status->msg; }

void upb_status_seterrmsg(upb_status *status, const char *msg) {
  if (!status) return;
  status->ok = false;
  strncpy(status->msg, msg, UPB_STATUS_MAX_MESSAGE - 1);
  status->msg[UPB_STATUS_MAX_MESSAGE - 1] = '\0';
}

void upb_status_seterrf(upb_status *status, const char *fmt, ...) {
  va_list args;
  va_start(args, fmt);
  upb_status_vseterrf(status, fmt, args);
  va_end(args);
}

void upb_status_vseterrf(upb_status *status, const char *fmt, va_list args) {
  if (!status) return;
  status->ok = false;
  vsnprintf(status->msg, sizeof(status->msg), fmt, args);
  status->msg[UPB_STATUS_MAX_MESSAGE - 1] = '\0';
}

void upb_status_vappenderrf(upb_status *status, const char *fmt, va_list args) {
  size_t len;
  if (!status) return;
  status->ok = false;
  len = strlen(status->msg);
  vsnprintf(status->msg + len, sizeof(status->msg) - len, fmt, args);
  status->msg[UPB_STATUS_MAX_MESSAGE - 1] = '\0';
}

/* upb_alloc ******************************************************************/

static void *upb_global_allocfunc(upb_alloc *alloc, void *ptr, size_t oldsize,
                                  size_t size) {
  UPB_UNUSED(alloc);
  UPB_UNUSED(oldsize);
  if (size == 0) {
    free(ptr);
    return NULL;
  } else {
    return realloc(ptr, size);
  }
}

static uint32_t *upb_cleanup_pointer(uintptr_t cleanup_metadata) {
  return (uint32_t *)(cleanup_metadata & ~0x1);
}

static bool upb_cleanup_has_initial_block(uintptr_t cleanup_metadata) {
  return cleanup_metadata & 0x1;
}

static uintptr_t upb_cleanup_metadata(uint32_t *cleanup,
                                      bool has_initial_block) {
  return (uintptr_t)cleanup | has_initial_block;
}

upb_alloc upb_alloc_global = {&upb_global_allocfunc};

/* upb_arena ******************************************************************/

/* Be conservative and choose 16 in case anyone is using SSE. */

struct mem_block {
  struct mem_block *next;
  uint32_t size;
  uint32_t cleanups;
  /* Data follows. */
};

typedef struct cleanup_ent {
  upb_cleanup_func *cleanup;
  void *ud;
} cleanup_ent;

static const size_t memblock_reserve = UPB_ALIGN_UP(sizeof(mem_block), 16);

static upb_arena *arena_findroot(upb_arena *a) {
  /* Path splitting keeps time complexity down, see:
   *   https://en.wikipedia.org/wiki/Disjoint-set_data_structure */
  while (a->parent != a) {
    upb_arena *next = a->parent;
    a->parent = next->parent;
    a = next;
  }
  return a;
}

static void upb_arena_addblock(upb_arena *a, upb_arena *root, void *ptr,
                               size_t size) {
  mem_block *block = ptr;

  /* The block is for arena |a|, but should appear in the freelist of |root|. */
  block->next = root->freelist;
  block->size = (uint32_t)size;
  block->cleanups = 0;
  root->freelist = block;
  a->last_size = block->size;
  if (!root->freelist_tail) root->freelist_tail = block;

  a->head.ptr = UPB_PTR_AT(block, memblock_reserve, char);
  a->head.end = UPB_PTR_AT(block, size, char);
  a->cleanup_metadata = upb_cleanup_metadata(
      &block->cleanups, upb_cleanup_has_initial_block(a->cleanup_metadata));

  UPB_POISON_MEMORY_REGION(a->head.ptr, a->head.end - a->head.ptr);
}

static bool upb_arena_allocblock(upb_arena *a, size_t size) {
  upb_arena *root = arena_findroot(a);
  size_t block_size = UPB_MAX(size, a->last_size * 2) + memblock_reserve;
  mem_block *block = upb_malloc(root->block_alloc, block_size);

  if (!block) return false;
  upb_arena_addblock(a, root, block, block_size);
  return true;
}

void *_upb_arena_slowmalloc(upb_arena *a, size_t size) {
  if (!upb_arena_allocblock(a, size)) return NULL;  /* Out of memory. */
  UPB_ASSERT(_upb_arenahas(a) >= size);
  return upb_arena_malloc(a, size);
}

static void *upb_arena_doalloc(upb_alloc *alloc, void *ptr, size_t oldsize,
                               size_t size) {
  upb_arena *a = (upb_arena*)alloc;  /* upb_alloc is initial member. */
  return upb_arena_realloc(a, ptr, oldsize, size);
}

/* Public Arena API ***********************************************************/

upb_arena *arena_initslow(void *mem, size_t n, upb_alloc *alloc) {
  const size_t first_block_overhead = sizeof(upb_arena) + memblock_reserve;
  upb_arena *a;

  /* We need to malloc the initial block. */
  n = first_block_overhead + 256;
  if (!alloc || !(mem = upb_malloc(alloc, n))) {
    return NULL;
  }

  a = UPB_PTR_AT(mem, n - sizeof(*a), upb_arena);
  n -= sizeof(*a);

  a->head.alloc.func = &upb_arena_doalloc;
  a->block_alloc = alloc;
  a->parent = a;
  a->refcount = 1;
  a->freelist = NULL;
  a->freelist_tail = NULL;
  a->cleanup_metadata = upb_cleanup_metadata(NULL, false);

  upb_arena_addblock(a, a, mem, n);

  return a;
}

upb_arena *upb_arena_init(void *mem, size_t n, upb_alloc *alloc) {
  upb_arena *a;

  if (n) {
    /* Align initial pointer up so that we return properly-aligned pointers. */
    void *aligned = (void*)UPB_ALIGN_UP((uintptr_t)mem, 16);
    size_t delta = (uintptr_t)aligned - (uintptr_t)mem;
    n = delta <= n ? n - delta : 0;
    mem = aligned;
  }

  /* Round block size down to alignof(*a) since we will allocate the arena
   * itself at the end. */
  n = UPB_ALIGN_DOWN(n, UPB_ALIGN_OF(upb_arena));

  if (UPB_UNLIKELY(n < sizeof(upb_arena))) {
    return arena_initslow(mem, n, alloc);
  }

  a = UPB_PTR_AT(mem, n - sizeof(*a), upb_arena);

  a->head.alloc.func = &upb_arena_doalloc;
  a->block_alloc = alloc;
  a->parent = a;
  a->refcount = 1;
  a->last_size = UPB_MAX(128, n);
  a->head.ptr = mem;
  a->head.end = UPB_PTR_AT(mem, n - sizeof(*a), char);
  a->freelist = NULL;
  a->cleanup_metadata = upb_cleanup_metadata(NULL, true);

  return a;
}

static void arena_dofree(upb_arena *a) {
  mem_block *block = a->freelist;
  UPB_ASSERT(a->parent == a);
  UPB_ASSERT(a->refcount == 0);

  while (block) {
    /* Load first since we are deleting block. */
    mem_block *next = block->next;

    if (block->cleanups > 0) {
      cleanup_ent *end = UPB_PTR_AT(block, block->size, void);
      cleanup_ent *ptr = end - block->cleanups;

      for (; ptr < end; ptr++) {
        ptr->cleanup(ptr->ud);
      }
    }

    upb_free(a->block_alloc, block);
    block = next;
  }
}

void upb_arena_free(upb_arena *a) {
  a = arena_findroot(a);
  if (--a->refcount == 0) arena_dofree(a);
}

bool upb_arena_addcleanup(upb_arena *a, void *ud, upb_cleanup_func *func) {
  cleanup_ent *ent;
  uint32_t* cleanups = upb_cleanup_pointer(a->cleanup_metadata);

  if (!cleanups || _upb_arenahas(a) < sizeof(cleanup_ent)) {
    if (!upb_arena_allocblock(a, 128)) return false;  /* Out of memory. */
    UPB_ASSERT(_upb_arenahas(a) >= sizeof(cleanup_ent));
    cleanups = upb_cleanup_pointer(a->cleanup_metadata);
  }

  a->head.end -= sizeof(cleanup_ent);
  ent = (cleanup_ent*)a->head.end;
  (*cleanups)++;
  UPB_UNPOISON_MEMORY_REGION(ent, sizeof(cleanup_ent));

  ent->cleanup = func;
  ent->ud = ud;

  return true;
}

bool upb_arena_fuse(upb_arena *a1, upb_arena *a2) {
  upb_arena *r1 = arena_findroot(a1);
  upb_arena *r2 = arena_findroot(a2);

  if (r1 == r2) return true;  /* Already fused. */

  /* Do not fuse initial blocks since we cannot lifetime extend them. */
  if (upb_cleanup_has_initial_block(r1->cleanup_metadata)) return false;
  if (upb_cleanup_has_initial_block(r2->cleanup_metadata)) return false;

  /* Only allow fuse with a common allocator */
  if (r1->block_alloc != r2->block_alloc) return false;

  /* We want to join the smaller tree to the larger tree.
   * So swap first if they are backwards. */
  if (r1->refcount < r2->refcount) {
    upb_arena *tmp = r1;
    r1 = r2;
    r2 = tmp;
  }

  /* r1 takes over r2's freelist and refcount. */
  r1->refcount += r2->refcount;
  if (r2->freelist_tail) {
    UPB_ASSERT(r2->freelist_tail->next == NULL);
    r2->freelist_tail->next = r1->freelist;
    r1->freelist = r2->freelist;
  }
  r2->parent = r1;
  return true;
}

/** upb/decode_fast.c ************************************************************/
// Fast decoder: ~3x the speed of decode.c, but requires x86-64/ARM64.
// Also the table size grows by 2x.
//
// Could potentially be ported to other 64-bit archs that pass at least six
// arguments in registers and have 8 unused high bits in pointers.
//
// The overall design is to create specialized functions for every possible
// field type (eg. oneof boolean field with a 1 byte tag) and then dispatch
// to the specialized function as quickly as possible.



/* Must be last. */

#if UPB_FASTTABLE

// The standard set of arguments passed to each parsing function.
// Thanks to x86-64 calling conventions, these will stay in registers.
#define UPB_PARSE_PARAMS                                          \
  upb_decstate *d, const char *ptr, upb_msg *msg, intptr_t table, \
      uint64_t hasbits, uint64_t data

#define UPB_PARSE_ARGS d, ptr, msg, table, hasbits, data

#define RETURN_GENERIC(m)                                                      \
  /* Uncomment either of these for debugging purposes. */                      \
  /* fprintf(stderr, m); */                                                    \
  /*__builtin_trap(); */                                                       \
  return fastdecode_generic(d, ptr, msg, table, hasbits, 0);

typedef enum {
  CARD_s = 0,  /* Singular (optional, non-repeated) */
  CARD_o = 1,  /* Oneof */
  CARD_r = 2,  /* Repeated */
  CARD_p = 3   /* Packed Repeated */
} upb_card;

UPB_NOINLINE
static const char *fastdecode_isdonefallback(UPB_PARSE_PARAMS) {
  int overrun = data;
  int status;
  ptr = decode_isdonefallback_inl(d, ptr, overrun, &status);
  if (ptr == NULL) {
    return fastdecode_err(d, status);
  }
  data = fastdecode_loadtag(ptr);
  UPB_MUSTTAIL return fastdecode_tagdispatch(UPB_PARSE_ARGS);
}

UPB_FORCEINLINE
static const char *fastdecode_dispatch(UPB_PARSE_PARAMS) {
  if (UPB_UNLIKELY(ptr >= d->limit_ptr)) {
    int overrun = ptr - d->end;
    if (UPB_LIKELY(overrun == d->limit)) {
      // Parse is finished.
      *(uint32_t*)msg |= hasbits;  // Sync hasbits.
      const upb_msglayout *l = decode_totablep(table);
      return UPB_UNLIKELY(l->required_count)
                 ? decode_checkrequired(d, ptr, msg, l)
                 : ptr;
    } else {
      data = overrun;
      UPB_MUSTTAIL return fastdecode_isdonefallback(UPB_PARSE_ARGS);
    }
  }

  // Read two bytes of tag data (for a one-byte tag, the high byte is junk).
  data = fastdecode_loadtag(ptr);
  UPB_MUSTTAIL return fastdecode_tagdispatch(UPB_PARSE_ARGS);
}

UPB_FORCEINLINE
static bool fastdecode_checktag(uint16_t data, int tagbytes) {
  if (tagbytes == 1) {
    return (data & 0xff) == 0;
  } else {
    return data == 0;
  }
}

UPB_FORCEINLINE
static const char *fastdecode_longsize(const char *ptr, int *size) {
  int i;
  UPB_ASSERT(*size & 0x80);
  *size &= 0xff;
  for (i = 0; i < 3; i++) {
    ptr++;
    size_t byte = (uint8_t)ptr[-1];
    *size += (byte - 1) << (7 + 7 * i);
    if (UPB_LIKELY((byte & 0x80) == 0)) return ptr;
  }
  ptr++;
  size_t byte = (uint8_t)ptr[-1];
  // len is limited by 2gb not 4gb, hence 8 and not 16 as normally expected
  // for a 32 bit varint.
  if (UPB_UNLIKELY(byte >= 8)) return NULL;
  *size += (byte - 1) << 28;
  return ptr;
}

UPB_FORCEINLINE
static bool fastdecode_boundscheck(const char *ptr, size_t len,
                                   const char *end) {
  uintptr_t uptr = (uintptr_t)ptr;
  uintptr_t uend = (uintptr_t)end + 16;
  uintptr_t res = uptr + len;
  return res < uptr || res > uend;
}

UPB_FORCEINLINE
static bool fastdecode_boundscheck2(const char *ptr, size_t len,
                                    const char *end) {
  // This is one extra branch compared to the more normal:
  //   return (size_t)(end - ptr) < size;
  // However it is one less computation if we are just about to use "ptr + len":
  //   https://godbolt.org/z/35YGPz
  // In microbenchmarks this shows an overall 4% improvement.
  uintptr_t uptr = (uintptr_t)ptr;
  uintptr_t uend = (uintptr_t)end;
  uintptr_t res = uptr + len;
  return res < uptr || res > uend;
}

typedef const char *fastdecode_delimfunc(upb_decstate *d, const char *ptr,
                                         void *ctx);

UPB_FORCEINLINE
static const char *fastdecode_delimited(upb_decstate *d, const char *ptr,
                                        fastdecode_delimfunc *func, void *ctx) {
  ptr++;
  int len = (int8_t)ptr[-1];
  if (fastdecode_boundscheck2(ptr, len, d->limit_ptr)) {
    // Slow case: Sub-message is >=128 bytes and/or exceeds the current buffer.
    // If it exceeds the buffer limit, limit/limit_ptr will change during
    // sub-message parsing, so we need to preserve delta, not limit.
    if (UPB_UNLIKELY(len & 0x80)) {
      // Size varint >1 byte (length >= 128).
      ptr = fastdecode_longsize(ptr, &len);
      if (!ptr) {
        // Corrupt wire format: size exceeded INT_MAX.
        return NULL;
      }
    }
    if (ptr - d->end + (int)len > d->limit) {
      // Corrupt wire format: invalid limit.
      return NULL;
    }
    int delta = decode_pushlimit(d, ptr, len);
    ptr = func(d, ptr, ctx);
    decode_poplimit(d, ptr, delta);
  } else {
    // Fast case: Sub-message is <128 bytes and fits in the current buffer.
    // This means we can preserve limit/limit_ptr verbatim.
    const char *saved_limit_ptr = d->limit_ptr;
    int saved_limit = d->limit;
    d->limit_ptr = ptr + len;
    d->limit = d->limit_ptr - d->end;
    UPB_ASSERT(d->limit_ptr == d->end + UPB_MIN(0, d->limit));
    ptr = func(d, ptr, ctx);
    d->limit_ptr = saved_limit_ptr;
    d->limit = saved_limit;
    UPB_ASSERT(d->limit_ptr == d->end + UPB_MIN(0, d->limit));
  }
  return ptr;
}

/* singular, oneof, repeated field handling ***********************************/

typedef struct {
  upb_array *arr;
  void *end;
} fastdecode_arr;

typedef enum {
  FD_NEXT_ATLIMIT,
  FD_NEXT_SAMEFIELD,
  FD_NEXT_OTHERFIELD
} fastdecode_next;

typedef struct {
  void *dst;
  fastdecode_next next;
  uint32_t tag;
} fastdecode_nextret;

UPB_FORCEINLINE
static void *fastdecode_resizearr(upb_decstate *d, void *dst,
                                  fastdecode_arr *farr, int valbytes) {
  if (UPB_UNLIKELY(dst == farr->end)) {
    size_t old_size = farr->arr->size;
    size_t old_bytes = old_size * valbytes;
    size_t new_size = old_size * 2;
    size_t new_bytes = new_size * valbytes;
    char *old_ptr = _upb_array_ptr(farr->arr);
    char *new_ptr = upb_arena_realloc(&d->arena, old_ptr, old_bytes, new_bytes);
    uint8_t elem_size_lg2 = __builtin_ctz(valbytes);
    farr->arr->size = new_size;
    farr->arr->data = _upb_array_tagptr(new_ptr, elem_size_lg2);
    dst = (void*)(new_ptr + (old_size * valbytes));
    farr->end = (void*)(new_ptr + (new_size * valbytes));
  }
  return dst;
}

UPB_FORCEINLINE
static bool fastdecode_tagmatch(uint32_t tag, uint64_t data, int tagbytes) {
  if (tagbytes == 1) {
    return (uint8_t)tag == (uint8_t)data;
  } else {
    return (uint16_t)tag == (uint16_t)data;
  }
}

UPB_FORCEINLINE
static void fastdecode_commitarr(void *dst, fastdecode_arr *farr,
                                 int valbytes) {
  farr->arr->len =
      (size_t)((char *)dst - (char *)_upb_array_ptr(farr->arr)) / valbytes;
}

UPB_FORCEINLINE
static fastdecode_nextret fastdecode_nextrepeated(upb_decstate *d, void *dst,
                                                  const char **ptr,
                                                  fastdecode_arr *farr,
                                                  uint64_t data, int tagbytes,
                                                  int valbytes) {
  fastdecode_nextret ret;
  dst = (char *)dst + valbytes;

  if (UPB_LIKELY(!decode_isdone(d, ptr))) {
    ret.tag = fastdecode_loadtag(*ptr);
    if (fastdecode_tagmatch(ret.tag, data, tagbytes)) {
      ret.next = FD_NEXT_SAMEFIELD;
    } else {
      fastdecode_commitarr(dst, farr, valbytes);
      ret.next = FD_NEXT_OTHERFIELD;
    }
  } else {
    fastdecode_commitarr(dst, farr, valbytes);
    ret.next = FD_NEXT_ATLIMIT;
  }

  ret.dst = dst;
  return ret;
}

UPB_FORCEINLINE
static void *fastdecode_fieldmem(upb_msg *msg, uint64_t data) {
  size_t ofs = data >> 48;
  return (char *)msg + ofs;
}

UPB_FORCEINLINE
static void *fastdecode_getfield(upb_decstate *d, const char *ptr, upb_msg *msg,
                                 uint64_t *data, uint64_t *hasbits,
                                 fastdecode_arr *farr, int valbytes,
                                 upb_card card) {
  switch (card) {
    case CARD_s: {
      uint8_t hasbit_index = *data >> 24;
      // Set hasbit and return pointer to scalar field.
      *hasbits |= 1ull << hasbit_index;
      return fastdecode_fieldmem(msg, *data);
    }
    case CARD_o: {
      uint16_t case_ofs = *data >> 32;
      uint32_t *oneof_case = UPB_PTR_AT(msg, case_ofs, uint32_t);
      uint8_t field_number = *data >> 24;
      *oneof_case = field_number;
      return fastdecode_fieldmem(msg, *data);
    }
    case CARD_r: {
      // Get pointer to upb_array and allocate/expand if necessary.
      uint8_t elem_size_lg2 = __builtin_ctz(valbytes);
      upb_array **arr_p = fastdecode_fieldmem(msg, *data);
      char *begin;
      *(uint32_t*)msg |= *hasbits;
      *hasbits = 0;
      if (UPB_LIKELY(!*arr_p)) {
        farr->arr = _upb_array_new(&d->arena, 8, elem_size_lg2);
        *arr_p = farr->arr;
      } else {
        farr->arr = *arr_p;
      }
      begin = _upb_array_ptr(farr->arr);
      farr->end = begin + (farr->arr->size * valbytes);
      *data = fastdecode_loadtag(ptr);
      return begin + (farr->arr->len * valbytes);
    }
    default:
      UPB_UNREACHABLE();
  }
}

UPB_FORCEINLINE
static bool fastdecode_flippacked(uint64_t *data, int tagbytes) {
  *data ^= (0x2 ^ 0x0);  // Patch data to match packed wiretype.
  return fastdecode_checktag(*data, tagbytes);
}

#define FASTDECODE_CHECKPACKED(tagbytes, card, func)                           \
  if (UPB_UNLIKELY(!fastdecode_checktag(data, tagbytes))) {                    \
    if (card == CARD_r && fastdecode_flippacked(&data, tagbytes)) {            \
      UPB_MUSTTAIL return func(UPB_PARSE_ARGS);                                \
    }                                                                          \
    RETURN_GENERIC("packed check tag mismatch\n");                             \
  }

/* varint fields **************************************************************/

UPB_FORCEINLINE
static uint64_t fastdecode_munge(uint64_t val, int valbytes, bool zigzag) {
  if (valbytes == 1) {
    return val != 0;
  } else if (zigzag) {
    if (valbytes == 4) {
      uint32_t n = val;
      return (n >> 1) ^ -(int32_t)(n & 1);
    } else if (valbytes == 8) {
      return (val >> 1) ^ -(int64_t)(val & 1);
    }
    UPB_UNREACHABLE();
  }
  return val;
}

UPB_FORCEINLINE
static const char *fastdecode_varint64(const char *ptr, uint64_t *val) {
  ptr++;
  *val = (uint8_t)ptr[-1];
  if (UPB_UNLIKELY(*val & 0x80)) {
    int i;
    for (i = 0; i < 8; i++) {
      ptr++;
      uint64_t byte = (uint8_t)ptr[-1];
      *val += (byte - 1) << (7 + 7 * i);
      if (UPB_LIKELY((byte & 0x80) == 0)) goto done;
    }
    ptr++;
    uint64_t byte = (uint8_t)ptr[-1];
    if (byte > 1) {
      return NULL;
    }
    *val += (byte - 1) << 63;
  }
done:
  UPB_ASSUME(ptr != NULL);
  return ptr;
}

#define FASTDECODE_UNPACKEDVARINT(d, ptr, msg, table, hasbits, data, tagbytes, \
                                  valbytes, card, zigzag, packed)              \
  uint64_t val;                                                                \
  void *dst;                                                                   \
  fastdecode_arr farr;                                                         \
                                                                               \
  FASTDECODE_CHECKPACKED(tagbytes, card, packed);                              \
                                                                               \
  dst = fastdecode_getfield(d, ptr, msg, &data, &hasbits, &farr, valbytes,     \
                            card);                                             \
  if (card == CARD_r) {                                                        \
    if (UPB_UNLIKELY(!dst)) {                                                  \
      RETURN_GENERIC("need array resize\n");                                   \
    }                                                                          \
  }                                                                            \
                                                                               \
  again:                                                                       \
  if (card == CARD_r) {                                                        \
    dst = fastdecode_resizearr(d, dst, &farr, valbytes);                       \
  }                                                                            \
                                                                               \
  ptr += tagbytes;                                                             \
  ptr = fastdecode_varint64(ptr, &val);                                        \
  if (ptr == NULL) return fastdecode_err(d, kUpb_DecodeStatus_Malformed);      \
  val = fastdecode_munge(val, valbytes, zigzag);                               \
  memcpy(dst, &val, valbytes);                                                 \
                                                                               \
  if (card == CARD_r) {                                                        \
    fastdecode_nextret ret = fastdecode_nextrepeated(                          \
        d, dst, &ptr, &farr, data, tagbytes, valbytes);                        \
    switch (ret.next) {                                                        \
      case FD_NEXT_SAMEFIELD:                                                  \
        dst = ret.dst;                                                         \
        goto again;                                                            \
      case FD_NEXT_OTHERFIELD:                                                 \
        data = ret.tag;                                                        \
        UPB_MUSTTAIL return fastdecode_tagdispatch(UPB_PARSE_ARGS);            \
      case FD_NEXT_ATLIMIT:                                                    \
        return ptr;                                                            \
    }                                                                          \
  }                                                                            \
                                                                               \
  UPB_MUSTTAIL return fastdecode_dispatch(UPB_PARSE_ARGS);

typedef struct {
  uint8_t valbytes;
  bool zigzag;
  void *dst;
  fastdecode_arr farr;
} fastdecode_varintdata;

UPB_FORCEINLINE
static const char *fastdecode_topackedvarint(upb_decstate *d, const char *ptr,
                                             void *ctx) {
  fastdecode_varintdata *data = ctx;
  void *dst = data->dst;
  uint64_t val;

  while (!decode_isdone(d, &ptr)) {
    dst = fastdecode_resizearr(d, dst, &data->farr, data->valbytes);
    ptr = fastdecode_varint64(ptr, &val);
    if (ptr == NULL) return NULL;
    val = fastdecode_munge(val, data->valbytes, data->zigzag);
    memcpy(dst, &val, data->valbytes);
    dst = (char *)dst + data->valbytes;
  }

  fastdecode_commitarr(dst, &data->farr, data->valbytes);
  return ptr;
}

#define FASTDECODE_PACKEDVARINT(d, ptr, msg, table, hasbits, data, tagbytes,   \
                                valbytes, zigzag, unpacked)                    \
  fastdecode_varintdata ctx = {valbytes, zigzag};                              \
                                                                               \
  FASTDECODE_CHECKPACKED(tagbytes, CARD_r, unpacked);                          \
                                                                               \
  ctx.dst = fastdecode_getfield(d, ptr, msg, &data, &hasbits, &ctx.farr,       \
                                valbytes, CARD_r);                             \
  if (UPB_UNLIKELY(!ctx.dst)) {                                                \
    RETURN_GENERIC("need array resize\n");                                     \
  }                                                                            \
                                                                               \
  ptr += tagbytes;                                                             \
  ptr = fastdecode_delimited(d, ptr, &fastdecode_topackedvarint, &ctx);        \
                                                                               \
  if (UPB_UNLIKELY(ptr == NULL)) {                                             \
    return fastdecode_err(d, kUpb_DecodeStatus_Malformed);                     \
  }                                                                            \
                                                                               \
  UPB_MUSTTAIL return fastdecode_dispatch(d, ptr, msg, table, hasbits, 0);

#define FASTDECODE_VARINT(d, ptr, msg, table, hasbits, data, tagbytes,     \
                          valbytes, card, zigzag, unpacked, packed)        \
  if (card == CARD_p) {                                                    \
    FASTDECODE_PACKEDVARINT(d, ptr, msg, table, hasbits, data, tagbytes,   \
                            valbytes, zigzag, unpacked);                   \
  } else {                                                                 \
    FASTDECODE_UNPACKEDVARINT(d, ptr, msg, table, hasbits, data, tagbytes, \
                              valbytes, card, zigzag, packed);             \
  }

#define z_ZZ true
#define b_ZZ false
#define v_ZZ false

/* Generate all combinations:
 * {s,o,r,p} x {b1,v4,z4,v8,z8} x {1bt,2bt} */

#define F(card, type, valbytes, tagbytes)                                      \
  UPB_NOINLINE                                                                 \
  const char *upb_p##card##type##valbytes##_##tagbytes##bt(UPB_PARSE_PARAMS) { \
    FASTDECODE_VARINT(d, ptr, msg, table, hasbits, data, tagbytes, valbytes,   \
                      CARD_##card, type##_ZZ,                                  \
                      upb_pr##type##valbytes##_##tagbytes##bt,                 \
                      upb_pp##type##valbytes##_##tagbytes##bt);                \
  }

#define TYPES(card, tagbytes) \
  F(card, b, 1, tagbytes)     \
  F(card, v, 4, tagbytes)     \
  F(card, v, 8, tagbytes)     \
  F(card, z, 4, tagbytes)     \
  F(card, z, 8, tagbytes)

#define TAGBYTES(card) \
  TYPES(card, 1)       \
  TYPES(card, 2)

TAGBYTES(s)
TAGBYTES(o)
TAGBYTES(r)
TAGBYTES(p)

#undef z_ZZ
#undef b_ZZ
#undef v_ZZ
#undef o_ONEOF
#undef s_ONEOF
#undef r_ONEOF
#undef F
#undef TYPES
#undef TAGBYTES
#undef FASTDECODE_UNPACKEDVARINT
#undef FASTDECODE_PACKEDVARINT
#undef FASTDECODE_VARINT


/* fixed fields ***************************************************************/

#define FASTDECODE_UNPACKEDFIXED(d, ptr, msg, table, hasbits, data, tagbytes,  \
                                 valbytes, card, packed)                       \
  void *dst;                                                                   \
  fastdecode_arr farr;                                                         \
                                                                               \
  FASTDECODE_CHECKPACKED(tagbytes, card, packed)                               \
                                                                               \
  dst = fastdecode_getfield(d, ptr, msg, &data, &hasbits, &farr, valbytes,     \
                            card);                                             \
  if (card == CARD_r) {                                                        \
    if (UPB_UNLIKELY(!dst)) {                                                  \
      RETURN_GENERIC("couldn't allocate array in arena\n");                    \
    }                                                                          \
  }                                                                            \
                                                                               \
  again:                                                                       \
  if (card == CARD_r) {                                                        \
    dst = fastdecode_resizearr(d, dst, &farr, valbytes);                       \
  }                                                                            \
                                                                               \
  ptr += tagbytes;                                                             \
  memcpy(dst, ptr, valbytes);                                                  \
  ptr += valbytes;                                                             \
                                                                               \
  if (card == CARD_r) {                                                        \
    fastdecode_nextret ret = fastdecode_nextrepeated(                          \
        d, dst, &ptr, &farr, data, tagbytes, valbytes);                        \
    switch (ret.next) {                                                        \
    case FD_NEXT_SAMEFIELD:                                                    \
      dst = ret.dst;                                                           \
      goto again;                                                              \
    case FD_NEXT_OTHERFIELD:                                                   \
      data = ret.tag;                                                          \
      UPB_MUSTTAIL return fastdecode_tagdispatch(UPB_PARSE_ARGS);              \
    case FD_NEXT_ATLIMIT:                                                      \
      return ptr;                                                              \
    }                                                                          \
  }                                                                            \
                                                                               \
  UPB_MUSTTAIL return fastdecode_dispatch(UPB_PARSE_ARGS);

#define FASTDECODE_PACKEDFIXED(d, ptr, msg, table, hasbits, data, tagbytes, \
                               valbytes, unpacked)                          \
  FASTDECODE_CHECKPACKED(tagbytes, CARD_r, unpacked)                        \
                                                                            \
  ptr += tagbytes;                                                          \
  int size = (uint8_t)ptr[0];                                               \
  ptr++;                                                                    \
  if (size & 0x80) {                                                        \
    ptr = fastdecode_longsize(ptr, &size);                                  \
  }                                                                         \
                                                                            \
  if (UPB_UNLIKELY(fastdecode_boundscheck(ptr, size, d->limit_ptr) ||       \
                   (size % valbytes) != 0)) {                               \
    return fastdecode_err(d, kUpb_DecodeStatus_Malformed);                  \
  }                                                                         \
                                                                            \
  upb_array **arr_p = fastdecode_fieldmem(msg, data);                       \
  upb_array *arr = *arr_p;                                                  \
  uint8_t elem_size_lg2 = __builtin_ctz(valbytes);                          \
  int elems = size / valbytes;                                              \
                                                                            \
  if (UPB_LIKELY(!arr)) {                                                   \
    *arr_p = arr = _upb_array_new(&d->arena, elems, elem_size_lg2);         \
    if (!arr) {                                                             \
      return fastdecode_err(d, kUpb_DecodeStatus_Malformed);                \
    }                                                                       \
  } else {                                                                  \
    _upb_array_resize(arr, elems, &d->arena);                               \
  }                                                                         \
                                                                            \
  char *dst = _upb_array_ptr(arr);                                          \
  memcpy(dst, ptr, size);                                                   \
  arr->len = elems;                                                         \
                                                                            \
  ptr += size;                                                              \
  UPB_MUSTTAIL return fastdecode_dispatch(UPB_PARSE_ARGS);

#define FASTDECODE_FIXED(d, ptr, msg, table, hasbits, data, tagbytes,     \
                         valbytes, card, unpacked, packed)                \
  if (card == CARD_p) {                                                   \
    FASTDECODE_PACKEDFIXED(d, ptr, msg, table, hasbits, data, tagbytes,   \
                           valbytes, unpacked);                           \
  } else {                                                                \
    FASTDECODE_UNPACKEDFIXED(d, ptr, msg, table, hasbits, data, tagbytes, \
                             valbytes, card, packed);                     \
  }

/* Generate all combinations:
 * {s,o,r,p} x {f4,f8} x {1bt,2bt} */

#define F(card, valbytes, tagbytes)                                         \
  UPB_NOINLINE                                                              \
  const char *upb_p##card##f##valbytes##_##tagbytes##bt(UPB_PARSE_PARAMS) { \
    FASTDECODE_FIXED(d, ptr, msg, table, hasbits, data, tagbytes, valbytes, \
                     CARD_##card, upb_ppf##valbytes##_##tagbytes##bt,       \
                     upb_prf##valbytes##_##tagbytes##bt);                   \
  }

#define TYPES(card, tagbytes) \
  F(card, 4, tagbytes)        \
  F(card, 8, tagbytes)

#define TAGBYTES(card) \
  TYPES(card, 1)       \
  TYPES(card, 2)

TAGBYTES(s)
TAGBYTES(o)
TAGBYTES(r)
TAGBYTES(p)

#undef F
#undef TYPES
#undef TAGBYTES
#undef FASTDECODE_UNPACKEDFIXED
#undef FASTDECODE_PACKEDFIXED

/* string fields **************************************************************/

typedef const char *fastdecode_copystr_func(struct upb_decstate *d,
                                            const char *ptr, upb_msg *msg,
                                            const upb_msglayout *table,
                                            uint64_t hasbits, upb_strview *dst);

UPB_NOINLINE
static const char *fastdecode_verifyutf8(upb_decstate *d, const char *ptr,
                                         upb_msg *msg, intptr_t table,
                                         uint64_t hasbits, uint64_t data) {
  upb_strview *dst = (upb_strview*)data;
  if (!decode_verifyutf8_inl(dst->data, dst->size)) {
    return fastdecode_err(d, kUpb_DecodeStatus_BadUtf8);
  }
  UPB_MUSTTAIL return fastdecode_dispatch(UPB_PARSE_ARGS);
}

#define FASTDECODE_LONGSTRING(d, ptr, msg, table, hasbits, dst, validate_utf8) \
  int size = (uint8_t)ptr[0]; /* Could plumb through hasbits. */               \
  ptr++;                                                                       \
  if (size & 0x80) {                                                           \
    ptr = fastdecode_longsize(ptr, &size);                                     \
  }                                                                            \
                                                                               \
  if (UPB_UNLIKELY(fastdecode_boundscheck(ptr, size, d->limit_ptr))) {         \
    dst->size = 0;                                                             \
    return fastdecode_err(d, kUpb_DecodeStatus_Malformed);                     \
  }                                                                            \
                                                                               \
  if (d->options & kUpb_DecodeOption_AliasString) {                            \
    dst->data = ptr;                                                           \
    dst->size = size;                                                          \
  } else {                                                                     \
    char *data = upb_arena_malloc(&d->arena, size);                            \
    if (!data) {                                                               \
      return fastdecode_err(d, kUpb_DecodeStatus_OutOfMemory);                 \
    }                                                                          \
    memcpy(data, ptr, size);                                                   \
    dst->data = data;                                                          \
    dst->size = size;                                                          \
  }                                                                            \
                                                                               \
  ptr += size;                                                                 \
  if (validate_utf8) {                                                         \
    data = (uint64_t)dst;                                                      \
    UPB_MUSTTAIL return fastdecode_verifyutf8(UPB_PARSE_ARGS);                 \
  } else {                                                                     \
    UPB_MUSTTAIL return fastdecode_dispatch(UPB_PARSE_ARGS);                   \
  }

UPB_NOINLINE
static const char *fastdecode_longstring_utf8(struct upb_decstate *d,
                                              const char *ptr, upb_msg *msg,
                                              intptr_t table, uint64_t hasbits,
                                              uint64_t data) {
  upb_strview *dst = (upb_strview*)data;
  FASTDECODE_LONGSTRING(d, ptr, msg, table, hasbits, dst, true);
}

UPB_NOINLINE
static const char *fastdecode_longstring_noutf8(struct upb_decstate *d,
                                                const char *ptr, upb_msg *msg,
                                                intptr_t table,
                                                uint64_t hasbits,
                                                uint64_t data) {
  upb_strview *dst = (upb_strview*)data;
  FASTDECODE_LONGSTRING(d, ptr, msg, table, hasbits, dst, false);
}

UPB_FORCEINLINE
static void fastdecode_docopy(upb_decstate *d, const char *ptr, uint32_t size,
                              int copy, char *data, upb_strview *dst) {
  d->arena.head.ptr += copy;
  dst->data = data;
  UPB_UNPOISON_MEMORY_REGION(data, copy);
  memcpy(data, ptr, copy);
  UPB_POISON_MEMORY_REGION(data + size, copy - size);
}

#define FASTDECODE_COPYSTRING(d, ptr, msg, table, hasbits, data, tagbytes,     \
                              card, validate_utf8)                             \
  upb_strview *dst;                                                            \
  fastdecode_arr farr;                                                         \
  int64_t size;                                                                \
  size_t arena_has;                                                            \
  size_t common_has;                                                           \
  char *buf;                                                                   \
                                                                               \
  UPB_ASSERT((d->options & kUpb_DecodeOption_AliasString) == 0);               \
  UPB_ASSERT(fastdecode_checktag(data, tagbytes));                             \
                                                                               \
  dst = fastdecode_getfield(d, ptr, msg, &data, &hasbits, &farr,               \
                            sizeof(upb_strview), card);                        \
                                                                               \
  again:                                                                       \
  if (card == CARD_r) {                                                        \
    dst = fastdecode_resizearr(d, dst, &farr, sizeof(upb_strview));            \
  }                                                                            \
                                                                               \
  size = (uint8_t)ptr[tagbytes];                                               \
  ptr += tagbytes + 1;                                                         \
  dst->size = size;                                                            \
                                                                               \
  buf = d->arena.head.ptr;                                                     \
  arena_has = _upb_arenahas(&d->arena);                                        \
  common_has = UPB_MIN(arena_has, (d->end - ptr) + 16);                        \
                                                                               \
  if (UPB_LIKELY(size <= 15 - tagbytes)) {                                     \
    if (arena_has < 16) goto longstr;                                          \
    d->arena.head.ptr += 16;                                                   \
    memcpy(buf, ptr - tagbytes - 1, 16);                                       \
    dst->data = buf + tagbytes + 1;                                            \
  } else if (UPB_LIKELY(size <= 32)) {                                         \
    if (UPB_UNLIKELY(common_has < 32)) goto longstr;                           \
    fastdecode_docopy(d, ptr, size, 32, buf, dst);                             \
  } else if (UPB_LIKELY(size <= 64)) {                                         \
    if (UPB_UNLIKELY(common_has < 64)) goto longstr;                           \
    fastdecode_docopy(d, ptr, size, 64, buf, dst);                             \
  } else if (UPB_LIKELY(size < 128)) {                                         \
    if (UPB_UNLIKELY(common_has < 128)) goto longstr;                          \
    fastdecode_docopy(d, ptr, size, 128, buf, dst);                            \
  } else {                                                                     \
    goto longstr;                                                              \
  }                                                                            \
                                                                               \
  ptr += size;                                                                 \
                                                                               \
  if (card == CARD_r) {                                                        \
    if (validate_utf8 && !decode_verifyutf8_inl(dst->data, dst->size)) {       \
      return fastdecode_err(d, kUpb_DecodeStatus_BadUtf8);                     \
    }                                                                          \
    fastdecode_nextret ret = fastdecode_nextrepeated(                          \
        d, dst, &ptr, &farr, data, tagbytes, sizeof(upb_strview));             \
    switch (ret.next) {                                                        \
      case FD_NEXT_SAMEFIELD:                                                  \
        dst = ret.dst;                                                         \
        goto again;                                                            \
      case FD_NEXT_OTHERFIELD:                                                 \
        data = ret.tag;                                                        \
        UPB_MUSTTAIL return fastdecode_tagdispatch(UPB_PARSE_ARGS);            \
      case FD_NEXT_ATLIMIT:                                                    \
        return ptr;                                                            \
    }                                                                          \
  }                                                                            \
                                                                               \
  if (card != CARD_r && validate_utf8) {                                       \
    data = (uint64_t)dst;                                                      \
    UPB_MUSTTAIL return fastdecode_verifyutf8(UPB_PARSE_ARGS);                 \
  }                                                                            \
                                                                               \
  UPB_MUSTTAIL return fastdecode_dispatch(UPB_PARSE_ARGS);                     \
                                                                               \
  longstr:                                                                     \
  if (card == CARD_r) {                                                        \
    fastdecode_commitarr(dst + 1, &farr, sizeof(upb_strview));                 \
  }                                                                            \
  ptr--;                                                                       \
  if (validate_utf8) {                                                         \
    UPB_MUSTTAIL return fastdecode_longstring_utf8(d, ptr, msg, table,         \
                                                   hasbits, (uint64_t)dst);    \
  } else {                                                                     \
    UPB_MUSTTAIL return fastdecode_longstring_noutf8(d, ptr, msg, table,       \
                                                     hasbits, (uint64_t)dst);  \
  }

#define FASTDECODE_STRING(d, ptr, msg, table, hasbits, data, tagbytes, card,   \
                          copyfunc, validate_utf8)                             \
  upb_strview *dst;                                                            \
  fastdecode_arr farr;                                                         \
  int64_t size;                                                                \
                                                                               \
  if (UPB_UNLIKELY(!fastdecode_checktag(data, tagbytes))) {                    \
    RETURN_GENERIC("string field tag mismatch\n");                             \
  }                                                                            \
                                                                               \
  if (UPB_UNLIKELY((d->options & kUpb_DecodeOption_AliasString) == 0)) {       \
    UPB_MUSTTAIL return copyfunc(UPB_PARSE_ARGS);                              \
  }                                                                            \
                                                                               \
  dst = fastdecode_getfield(d, ptr, msg, &data, &hasbits, &farr,               \
                            sizeof(upb_strview), card);                        \
                                                                               \
  again:                                                                       \
  if (card == CARD_r) {                                                        \
    dst = fastdecode_resizearr(d, dst, &farr, sizeof(upb_strview));            \
  }                                                                            \
                                                                               \
  size = (int8_t)ptr[tagbytes];                                                \
  ptr += tagbytes + 1;                                                         \
  dst->data = ptr;                                                             \
  dst->size = size;                                                            \
                                                                               \
  if (UPB_UNLIKELY(fastdecode_boundscheck(ptr, size, d->end))) {               \
    ptr--;                                                                     \
    if (validate_utf8) {                                                       \
      return fastdecode_longstring_utf8(d, ptr, msg, table, hasbits,           \
                                        (uint64_t)dst);                        \
    } else {                                                                   \
      return fastdecode_longstring_noutf8(d, ptr, msg, table, hasbits,         \
                                          (uint64_t)dst);                      \
    }                                                                          \
  }                                                                            \
                                                                               \
  ptr += size;                                                                 \
                                                                               \
  if (card == CARD_r) {                                                        \
    if (validate_utf8 && !decode_verifyutf8_inl(dst->data, dst->size)) {       \
      return fastdecode_err(d, kUpb_DecodeStatus_BadUtf8);                     \
    }                                                                          \
    fastdecode_nextret ret = fastdecode_nextrepeated(                          \
        d, dst, &ptr, &farr, data, tagbytes, sizeof(upb_strview));             \
    switch (ret.next) {                                                        \
      case FD_NEXT_SAMEFIELD:                                                  \
        dst = ret.dst;                                                         \
        if (UPB_UNLIKELY((d->options & kUpb_DecodeOption_AliasString) == 0)) { \
          /* Buffer flipped and we can't alias any more. Bounce to */          \
          /* copyfunc(), but via dispatch since we need to reload table */     \
          /* data also. */                                                     \
          fastdecode_commitarr(dst, &farr, sizeof(upb_strview));               \
          data = ret.tag;                                                      \
          UPB_MUSTTAIL return fastdecode_tagdispatch(UPB_PARSE_ARGS);          \
        }                                                                      \
        goto again;                                                            \
      case FD_NEXT_OTHERFIELD:                                                 \
        data = ret.tag;                                                        \
        UPB_MUSTTAIL return fastdecode_tagdispatch(UPB_PARSE_ARGS);            \
      case FD_NEXT_ATLIMIT:                                                    \
        return ptr;                                                            \
    }                                                                          \
  }                                                                            \
                                                                               \
  if (card != CARD_r && validate_utf8) {                                       \
    data = (uint64_t)dst;                                                      \
    UPB_MUSTTAIL return fastdecode_verifyutf8(UPB_PARSE_ARGS);                 \
  }                                                                            \
                                                                               \
  UPB_MUSTTAIL return fastdecode_dispatch(UPB_PARSE_ARGS);

/* Generate all combinations:
 * {p,c} x {s,o,r} x {s, b} x {1bt,2bt} */

#define s_VALIDATE true
#define b_VALIDATE false

#define F(card, tagbytes, type)                                        \
  UPB_NOINLINE                                                         \
  const char *upb_c##card##type##_##tagbytes##bt(UPB_PARSE_PARAMS) {   \
    FASTDECODE_COPYSTRING(d, ptr, msg, table, hasbits, data, tagbytes, \
                          CARD_##card, type##_VALIDATE);               \
  }                                                                    \
  const char *upb_p##card##type##_##tagbytes##bt(UPB_PARSE_PARAMS) {   \
    FASTDECODE_STRING(d, ptr, msg, table, hasbits, data, tagbytes,     \
                      CARD_##card, upb_c##card##type##_##tagbytes##bt, \
                      type##_VALIDATE);                                \
  }

#define UTF8(card, tagbytes) \
  F(card, tagbytes, s)       \
  F(card, tagbytes, b)

#define TAGBYTES(card) \
  UTF8(card, 1)        \
  UTF8(card, 2)

TAGBYTES(s)
TAGBYTES(o)
TAGBYTES(r)

#undef s_VALIDATE
#undef b_VALIDATE
#undef F
#undef TAGBYTES
#undef FASTDECODE_LONGSTRING
#undef FASTDECODE_COPYSTRING
#undef FASTDECODE_STRING

/* message fields *************************************************************/

UPB_INLINE
upb_msg *decode_newmsg_ceil(upb_decstate *d, const upb_msglayout *l,
                            int msg_ceil_bytes) {
  size_t size = l->size + sizeof(upb_msg_internal);
  char *msg_data;
  if (UPB_LIKELY(msg_ceil_bytes > 0 &&
                 _upb_arenahas(&d->arena) >= msg_ceil_bytes)) {
    UPB_ASSERT(size <= (size_t)msg_ceil_bytes);
    msg_data = d->arena.head.ptr;
    d->arena.head.ptr += size;
    UPB_UNPOISON_MEMORY_REGION(msg_data, msg_ceil_bytes);
    memset(msg_data, 0, msg_ceil_bytes);
    UPB_POISON_MEMORY_REGION(msg_data + size, msg_ceil_bytes - size);
  } else {
    msg_data = (char*)upb_arena_malloc(&d->arena, size);
    memset(msg_data, 0, size);
  }
  return msg_data + sizeof(upb_msg_internal);
}

typedef struct {
  intptr_t table;
  upb_msg *msg;
} fastdecode_submsgdata;

UPB_FORCEINLINE
static const char *fastdecode_tosubmsg(upb_decstate *d, const char *ptr,
                                       void *ctx) {
  fastdecode_submsgdata *submsg = ctx;
  ptr = fastdecode_dispatch(d, ptr, submsg->msg, submsg->table, 0, 0);
  UPB_ASSUME(ptr != NULL);
  return ptr;
}

#define FASTDECODE_SUBMSG(d, ptr, msg, table, hasbits, data, tagbytes,    \
                          msg_ceil_bytes, card)                           \
                                                                          \
  if (UPB_UNLIKELY(!fastdecode_checktag(data, tagbytes))) {               \
    RETURN_GENERIC("submessage field tag mismatch\n");                    \
  }                                                                       \
                                                                          \
  if (--d->depth == 0) {                                                  \
    return fastdecode_err(d, kUpb_DecodeStatus_MaxDepthExceeded);         \
  }                                                                       \
                                                                          \
  upb_msg **dst;                                                          \
  uint32_t submsg_idx = (data >> 16) & 0xff;                              \
  const upb_msglayout *tablep = decode_totablep(table);                   \
  const upb_msglayout *subtablep = tablep->subs[submsg_idx].submsg;       \
  fastdecode_submsgdata submsg = {decode_totable(subtablep)};             \
  fastdecode_arr farr;                                                    \
                                                                          \
  if (subtablep->table_mask == (uint8_t)-1) {                             \
    RETURN_GENERIC("submessage doesn't have fast tables.");               \
  }                                                                       \
                                                                          \
  dst = fastdecode_getfield(d, ptr, msg, &data, &hasbits, &farr,          \
                            sizeof(upb_msg *), card);                     \
                                                                          \
  if (card == CARD_s) {                                                   \
    *(uint32_t *)msg |= hasbits;                                          \
    hasbits = 0;                                                          \
  }                                                                       \
                                                                          \
  again:                                                                  \
  if (card == CARD_r) {                                                   \
    dst = fastdecode_resizearr(d, dst, &farr, sizeof(upb_msg *));         \
  }                                                                       \
                                                                          \
  submsg.msg = *dst;                                                      \
                                                                          \
  if (card == CARD_r || UPB_LIKELY(!submsg.msg)) {                        \
    *dst = submsg.msg = decode_newmsg_ceil(d, subtablep, msg_ceil_bytes); \
  }                                                                       \
                                                                          \
  ptr += tagbytes;                                                        \
  ptr = fastdecode_delimited(d, ptr, fastdecode_tosubmsg, &submsg);       \
                                                                          \
  if (UPB_UNLIKELY(ptr == NULL || d->end_group != DECODE_NOGROUP)) {      \
    return fastdecode_err(d, kUpb_DecodeStatus_Malformed);                \
  }                                                                       \
                                                                          \
  if (card == CARD_r) {                                                   \
    fastdecode_nextret ret = fastdecode_nextrepeated(                     \
        d, dst, &ptr, &farr, data, tagbytes, sizeof(upb_msg *));          \
    switch (ret.next) {                                                   \
      case FD_NEXT_SAMEFIELD:                                             \
        dst = ret.dst;                                                    \
        goto again;                                                       \
      case FD_NEXT_OTHERFIELD:                                            \
        d->depth++;                                                       \
        data = ret.tag;                                                   \
        UPB_MUSTTAIL return fastdecode_tagdispatch(UPB_PARSE_ARGS);       \
      case FD_NEXT_ATLIMIT:                                               \
        d->depth++;                                                       \
        return ptr;                                                       \
    }                                                                     \
  }                                                                       \
                                                                          \
  d->depth++;                                                             \
  UPB_MUSTTAIL return fastdecode_dispatch(UPB_PARSE_ARGS);

#define F(card, tagbytes, size_ceil, ceil_arg)                               \
  const char *upb_p##card##m_##tagbytes##bt_max##size_ceil##b(               \
      UPB_PARSE_PARAMS) {                                                    \
    FASTDECODE_SUBMSG(d, ptr, msg, table, hasbits, data, tagbytes, ceil_arg, \
                      CARD_##card);                                          \
  }

#define SIZES(card, tagbytes) \
  F(card, tagbytes, 64, 64) \
  F(card, tagbytes, 128, 128) \
  F(card, tagbytes, 192, 192) \
  F(card, tagbytes, 256, 256) \
  F(card, tagbytes, max, -1)

#define TAGBYTES(card) \
  SIZES(card, 1) \
  SIZES(card, 2)

TAGBYTES(s)
TAGBYTES(o)
TAGBYTES(r)

#undef TAGBYTES
#undef SIZES
#undef F
#undef FASTDECODE_SUBMSG

#endif  /* UPB_FASTTABLE */

/** bazel-out/darwin-fastbuild/bin/external/com_google_protobuf/google/protobuf/descriptor.upb.c ************************************************************//* This file was generated by upbc (the upb compiler) from the input
 * file:
 *
 *     google/protobuf/descriptor.proto
 *
 * Do not edit -- your changes will be discarded when the file is
 * regenerated. */

#include <stddef.h>


static const upb_msglayout_sub google_protobuf_FileDescriptorSet_submsgs[1] = {
  {.submsg = &google_protobuf_FileDescriptorProto_msginit},
};

static const upb_msglayout_field google_protobuf_FileDescriptorSet__fields[1] = {
  {1, UPB_SIZE(0, 0), 0, 0, 11, _UPB_MODE_ARRAY | (_UPB_REP_PTR << _UPB_REP_SHIFT)},
};

const upb_msglayout google_protobuf_FileDescriptorSet_msginit = {
  &google_protobuf_FileDescriptorSet_submsgs[0],
  &google_protobuf_FileDescriptorSet__fields[0],
  UPB_SIZE(8, 8), 1, _UPB_MSGEXT_NONE, 1, 255, 0,
};

static const upb_msglayout_sub google_protobuf_FileDescriptorProto_submsgs[6] = {
  {.submsg = &google_protobuf_DescriptorProto_msginit},
  {.submsg = &google_protobuf_EnumDescriptorProto_msginit},
  {.submsg = &google_protobuf_FieldDescriptorProto_msginit},
  {.submsg = &google_protobuf_FileOptions_msginit},
  {.submsg = &google_protobuf_ServiceDescriptorProto_msginit},
  {.submsg = &google_protobuf_SourceCodeInfo_msginit},
};

static const upb_msglayout_field google_protobuf_FileDescriptorProto__fields[12] = {
  {1, UPB_SIZE(4, 8), 1, 0, 12, _UPB_MODE_SCALAR | (_UPB_REP_STRVIEW << _UPB_REP_SHIFT)},
  {2, UPB_SIZE(12, 24), 2, 0, 12, _UPB_MODE_SCALAR | (_UPB_REP_STRVIEW << _UPB_REP_SHIFT)},
  {3, UPB_SIZE(36, 72), 0, 0, 12, _UPB_MODE_ARRAY | (_UPB_REP_PTR << _UPB_REP_SHIFT)},
  {4, UPB_SIZE(40, 80), 0, 0, 11, _UPB_MODE_ARRAY | (_UPB_REP_PTR << _UPB_REP_SHIFT)},
  {5, UPB_SIZE(44, 88), 0, 1, 11, _UPB_MODE_ARRAY | (_UPB_REP_PTR << _UPB_REP_SHIFT)},
  {6, UPB_SIZE(48, 96), 0, 4, 11, _UPB_MODE_ARRAY | (_UPB_REP_PTR << _UPB_REP_SHIFT)},
  {7, UPB_SIZE(52, 104), 0, 2, 11, _UPB_MODE_ARRAY | (_UPB_REP_PTR << _UPB_REP_SHIFT)},
  {8, UPB_SIZE(28, 56), 3, 3, 11, _UPB_MODE_SCALAR | (_UPB_REP_PTR << _UPB_REP_SHIFT)},
  {9, UPB_SIZE(32, 64), 4, 5, 11, _UPB_MODE_SCALAR | (_UPB_REP_PTR << _UPB_REP_SHIFT)},
  {10, UPB_SIZE(56, 112), 0, 0, 5, _UPB_MODE_ARRAY | (_UPB_REP_PTR << _UPB_REP_SHIFT)},
  {11, UPB_SIZE(60, 120), 0, 0, 5, _UPB_MODE_ARRAY | (_UPB_REP_PTR << _UPB_REP_SHIFT)},
  {12, UPB_SIZE(20, 40), 5, 0, 12, _UPB_MODE_SCALAR | (_UPB_REP_STRVIEW << _UPB_REP_SHIFT)},
};

const upb_msglayout google_protobuf_FileDescriptorProto_msginit = {
  &google_protobuf_FileDescriptorProto_submsgs[0],
  &google_protobuf_FileDescriptorProto__fields[0],
  UPB_SIZE(64, 128), 12, _UPB_MSGEXT_NONE, 12, 255, 0,
};

static const upb_msglayout_sub google_protobuf_DescriptorProto_submsgs[7] = {
  {.submsg = &google_protobuf_DescriptorProto_msginit},
  {.submsg = &google_protobuf_DescriptorProto_ExtensionRange_msginit},
  {.submsg = &google_protobuf_DescriptorProto_ReservedRange_msginit},
  {.submsg = &google_protobuf_EnumDescriptorProto_msginit},
  {.submsg = &google_protobuf_FieldDescriptorProto_msginit},
  {.submsg = &google_protobuf_MessageOptions_msginit},
  {.submsg = &google_protobuf_OneofDescriptorProto_msginit},
};

static const upb_msglayout_field google_protobuf_DescriptorProto__fields[10] = {
  {1, UPB_SIZE(4, 8), 1, 0, 12, _UPB_MODE_SCALAR | (_UPB_REP_STRVIEW << _UPB_REP_SHIFT)},
  {2, UPB_SIZE(16, 32), 0, 4, 11, _UPB_MODE_ARRAY | (_UPB_REP_PTR << _UPB_REP_SHIFT)},
  {3, UPB_SIZE(20, 40), 0, 0, 11, _UPB_MODE_ARRAY | (_UPB_REP_PTR << _UPB_REP_SHIFT)},
  {4, UPB_SIZE(24, 48), 0, 3, 11, _UPB_MODE_ARRAY | (_UPB_REP_PTR << _UPB_REP_SHIFT)},
  {5, UPB_SIZE(28, 56), 0, 1, 11, _UPB_MODE_ARRAY | (_UPB_REP_PTR << _UPB_REP_SHIFT)},
  {6, UPB_SIZE(32, 64), 0, 4, 11, _UPB_MODE_ARRAY | (_UPB_REP_PTR << _UPB_REP_SHIFT)},
  {7, UPB_SIZE(12, 24), 2, 5, 11, _UPB_MODE_SCALAR | (_UPB_REP_PTR << _UPB_REP_SHIFT)},
  {8, UPB_SIZE(36, 72), 0, 6, 11, _UPB_MODE_ARRAY | (_UPB_REP_PTR << _UPB_REP_SHIFT)},
  {9, UPB_SIZE(40, 80), 0, 2, 11, _UPB_MODE_ARRAY | (_UPB_REP_PTR << _UPB_REP_SHIFT)},
  {10, UPB_SIZE(44, 88), 0, 0, 12, _UPB_MODE_ARRAY | (_UPB_REP_PTR << _UPB_REP_SHIFT)},
};

const upb_msglayout google_protobuf_DescriptorProto_msginit = {
  &google_protobuf_DescriptorProto_submsgs[0],
  &google_protobuf_DescriptorProto__fields[0],
  UPB_SIZE(48, 96), 10, _UPB_MSGEXT_NONE, 10, 255, 0,
};

static const upb_msglayout_sub google_protobuf_DescriptorProto_ExtensionRange_submsgs[1] = {
  {.submsg = &google_protobuf_ExtensionRangeOptions_msginit},
};

static const upb_msglayout_field google_protobuf_DescriptorProto_ExtensionRange__fields[3] = {
  {1, UPB_SIZE(4, 4), 1, 0, 5, _UPB_MODE_SCALAR | (_UPB_REP_4BYTE << _UPB_REP_SHIFT)},
  {2, UPB_SIZE(8, 8), 2, 0, 5, _UPB_MODE_SCALAR | (_UPB_REP_4BYTE << _UPB_REP_SHIFT)},
  {3, UPB_SIZE(12, 16), 3, 0, 11, _UPB_MODE_SCALAR | (_UPB_REP_PTR << _UPB_REP_SHIFT)},
};

const upb_msglayout google_protobuf_DescriptorProto_ExtensionRange_msginit = {
  &google_protobuf_DescriptorProto_ExtensionRange_submsgs[0],
  &google_protobuf_DescriptorProto_ExtensionRange__fields[0],
  UPB_SIZE(16, 24), 3, _UPB_MSGEXT_NONE, 3, 255, 0,
};

static const upb_msglayout_field google_protobuf_DescriptorProto_ReservedRange__fields[2] = {
  {1, UPB_SIZE(4, 4), 1, 0, 5, _UPB_MODE_SCALAR | (_UPB_REP_4BYTE << _UPB_REP_SHIFT)},
  {2, UPB_SIZE(8, 8), 2, 0, 5, _UPB_MODE_SCALAR | (_UPB_REP_4BYTE << _UPB_REP_SHIFT)},
};

const upb_msglayout google_protobuf_DescriptorProto_ReservedRange_msginit = {
  NULL,
  &google_protobuf_DescriptorProto_ReservedRange__fields[0],
  UPB_SIZE(16, 16), 2, _UPB_MSGEXT_NONE, 2, 255, 0,
};

static const upb_msglayout_sub google_protobuf_ExtensionRangeOptions_submsgs[1] = {
  {.submsg = &google_protobuf_UninterpretedOption_msginit},
};

static const upb_msglayout_field google_protobuf_ExtensionRangeOptions__fields[1] = {
  {999, UPB_SIZE(0, 0), 0, 0, 11, _UPB_MODE_ARRAY | (_UPB_REP_PTR << _UPB_REP_SHIFT)},
};

const upb_msglayout google_protobuf_ExtensionRangeOptions_msginit = {
  &google_protobuf_ExtensionRangeOptions_submsgs[0],
  &google_protobuf_ExtensionRangeOptions__fields[0],
  UPB_SIZE(8, 8), 1, _UPB_MSGEXT_EXTENDABLE, 0, 255, 0,
};

static const upb_msglayout_sub google_protobuf_FieldDescriptorProto_submsgs[3] = {
  {.submsg = &google_protobuf_FieldOptions_msginit},
  {.subenum = &google_protobuf_FieldDescriptorProto_Label_enuminit},
  {.subenum = &google_protobuf_FieldDescriptorProto_Type_enuminit},
};

static const upb_msglayout_field google_protobuf_FieldDescriptorProto__fields[11] = {
  {1, UPB_SIZE(24, 24), 1, 0, 12, _UPB_MODE_SCALAR | (_UPB_REP_STRVIEW << _UPB_REP_SHIFT)},
  {2, UPB_SIZE(32, 40), 2, 0, 12, _UPB_MODE_SCALAR | (_UPB_REP_STRVIEW << _UPB_REP_SHIFT)},
  {3, UPB_SIZE(12, 12), 3, 0, 5, _UPB_MODE_SCALAR | (_UPB_REP_4BYTE << _UPB_REP_SHIFT)},
  {4, UPB_SIZE(4, 4), 4, 1, 14, _UPB_MODE_SCALAR | (_UPB_REP_4BYTE << _UPB_REP_SHIFT)},
  {5, UPB_SIZE(8, 8), 5, 2, 14, _UPB_MODE_SCALAR | (_UPB_REP_4BYTE << _UPB_REP_SHIFT)},
  {6, UPB_SIZE(40, 56), 6, 0, 12, _UPB_MODE_SCALAR | (_UPB_REP_STRVIEW << _UPB_REP_SHIFT)},
  {7, UPB_SIZE(48, 72), 7, 0, 12, _UPB_MODE_SCALAR | (_UPB_REP_STRVIEW << _UPB_REP_SHIFT)},
  {8, UPB_SIZE(64, 104), 8, 0, 11, _UPB_MODE_SCALAR | (_UPB_REP_PTR << _UPB_REP_SHIFT)},
  {9, UPB_SIZE(16, 16), 9, 0, 5, _UPB_MODE_SCALAR | (_UPB_REP_4BYTE << _UPB_REP_SHIFT)},
  {10, UPB_SIZE(56, 88), 10, 0, 12, _UPB_MODE_SCALAR | (_UPB_REP_STRVIEW << _UPB_REP_SHIFT)},
  {17, UPB_SIZE(20, 20), 11, 0, 8, _UPB_MODE_SCALAR | (_UPB_REP_1BYTE << _UPB_REP_SHIFT)},
};

const upb_msglayout google_protobuf_FieldDescriptorProto_msginit = {
  &google_protobuf_FieldDescriptorProto_submsgs[0],
  &google_protobuf_FieldDescriptorProto__fields[0],
  UPB_SIZE(72, 112), 11, _UPB_MSGEXT_NONE, 10, 255, 0,
};

static const upb_msglayout_sub google_protobuf_OneofDescriptorProto_submsgs[1] = {
  {.submsg = &google_protobuf_OneofOptions_msginit},
};

static const upb_msglayout_field google_protobuf_OneofDescriptorProto__fields[2] = {
  {1, UPB_SIZE(4, 8), 1, 0, 12, _UPB_MODE_SCALAR | (_UPB_REP_STRVIEW << _UPB_REP_SHIFT)},
  {2, UPB_SIZE(12, 24), 2, 0, 11, _UPB_MODE_SCALAR | (_UPB_REP_PTR << _UPB_REP_SHIFT)},
};

const upb_msglayout google_protobuf_OneofDescriptorProto_msginit = {
  &google_protobuf_OneofDescriptorProto_submsgs[0],
  &google_protobuf_OneofDescriptorProto__fields[0],
  UPB_SIZE(16, 32), 2, _UPB_MSGEXT_NONE, 2, 255, 0,
};

static const upb_msglayout_sub google_protobuf_EnumDescriptorProto_submsgs[3] = {
  {.submsg = &google_protobuf_EnumDescriptorProto_EnumReservedRange_msginit},
  {.submsg = &google_protobuf_EnumOptions_msginit},
  {.submsg = &google_protobuf_EnumValueDescriptorProto_msginit},
};

static const upb_msglayout_field google_protobuf_EnumDescriptorProto__fields[5] = {
  {1, UPB_SIZE(4, 8), 1, 0, 12, _UPB_MODE_SCALAR | (_UPB_REP_STRVIEW << _UPB_REP_SHIFT)},
  {2, UPB_SIZE(16, 32), 0, 2, 11, _UPB_MODE_ARRAY | (_UPB_REP_PTR << _UPB_REP_SHIFT)},
  {3, UPB_SIZE(12, 24), 2, 1, 11, _UPB_MODE_SCALAR | (_UPB_REP_PTR << _UPB_REP_SHIFT)},
  {4, UPB_SIZE(20, 40), 0, 0, 11, _UPB_MODE_ARRAY | (_UPB_REP_PTR << _UPB_REP_SHIFT)},
  {5, UPB_SIZE(24, 48), 0, 0, 12, _UPB_MODE_ARRAY | (_UPB_REP_PTR << _UPB_REP_SHIFT)},
};

const upb_msglayout google_protobuf_EnumDescriptorProto_msginit = {
  &google_protobuf_EnumDescriptorProto_submsgs[0],
  &google_protobuf_EnumDescriptorProto__fields[0],
  UPB_SIZE(32, 64), 5, _UPB_MSGEXT_NONE, 5, 255, 0,
};

static const upb_msglayout_field google_protobuf_EnumDescriptorProto_EnumReservedRange__fields[2] = {
  {1, UPB_SIZE(4, 4), 1, 0, 5, _UPB_MODE_SCALAR | (_UPB_REP_4BYTE << _UPB_REP_SHIFT)},
  {2, UPB_SIZE(8, 8), 2, 0, 5, _UPB_MODE_SCALAR | (_UPB_REP_4BYTE << _UPB_REP_SHIFT)},
};

const upb_msglayout google_protobuf_EnumDescriptorProto_EnumReservedRange_msginit = {
  NULL,
  &google_protobuf_EnumDescriptorProto_EnumReservedRange__fields[0],
  UPB_SIZE(16, 16), 2, _UPB_MSGEXT_NONE, 2, 255, 0,
};

static const upb_msglayout_sub google_protobuf_EnumValueDescriptorProto_submsgs[1] = {
  {.submsg = &google_protobuf_EnumValueOptions_msginit},
};

static const upb_msglayout_field google_protobuf_EnumValueDescriptorProto__fields[3] = {
  {1, UPB_SIZE(8, 8), 1, 0, 12, _UPB_MODE_SCALAR | (_UPB_REP_STRVIEW << _UPB_REP_SHIFT)},
  {2, UPB_SIZE(4, 4), 2, 0, 5, _UPB_MODE_SCALAR | (_UPB_REP_4BYTE << _UPB_REP_SHIFT)},
  {3, UPB_SIZE(16, 24), 3, 0, 11, _UPB_MODE_SCALAR | (_UPB_REP_PTR << _UPB_REP_SHIFT)},
};

const upb_msglayout google_protobuf_EnumValueDescriptorProto_msginit = {
  &google_protobuf_EnumValueDescriptorProto_submsgs[0],
  &google_protobuf_EnumValueDescriptorProto__fields[0],
  UPB_SIZE(24, 32), 3, _UPB_MSGEXT_NONE, 3, 255, 0,
};

static const upb_msglayout_sub google_protobuf_ServiceDescriptorProto_submsgs[2] = {
  {.submsg = &google_protobuf_MethodDescriptorProto_msginit},
  {.submsg = &google_protobuf_ServiceOptions_msginit},
};

static const upb_msglayout_field google_protobuf_ServiceDescriptorProto__fields[3] = {
  {1, UPB_SIZE(4, 8), 1, 0, 12, _UPB_MODE_SCALAR | (_UPB_REP_STRVIEW << _UPB_REP_SHIFT)},
  {2, UPB_SIZE(16, 32), 0, 0, 11, _UPB_MODE_ARRAY | (_UPB_REP_PTR << _UPB_REP_SHIFT)},
  {3, UPB_SIZE(12, 24), 2, 1, 11, _UPB_MODE_SCALAR | (_UPB_REP_PTR << _UPB_REP_SHIFT)},
};

const upb_msglayout google_protobuf_ServiceDescriptorProto_msginit = {
  &google_protobuf_ServiceDescriptorProto_submsgs[0],
  &google_protobuf_ServiceDescriptorProto__fields[0],
  UPB_SIZE(24, 48), 3, _UPB_MSGEXT_NONE, 3, 255, 0,
};

static const upb_msglayout_sub google_protobuf_MethodDescriptorProto_submsgs[1] = {
  {.submsg = &google_protobuf_MethodOptions_msginit},
};

static const upb_msglayout_field google_protobuf_MethodDescriptorProto__fields[6] = {
  {1, UPB_SIZE(4, 8), 1, 0, 12, _UPB_MODE_SCALAR | (_UPB_REP_STRVIEW << _UPB_REP_SHIFT)},
  {2, UPB_SIZE(12, 24), 2, 0, 12, _UPB_MODE_SCALAR | (_UPB_REP_STRVIEW << _UPB_REP_SHIFT)},
  {3, UPB_SIZE(20, 40), 3, 0, 12, _UPB_MODE_SCALAR | (_UPB_REP_STRVIEW << _UPB_REP_SHIFT)},
  {4, UPB_SIZE(28, 56), 4, 0, 11, _UPB_MODE_SCALAR | (_UPB_REP_PTR << _UPB_REP_SHIFT)},
  {5, UPB_SIZE(1, 1), 5, 0, 8, _UPB_MODE_SCALAR | (_UPB_REP_1BYTE << _UPB_REP_SHIFT)},
  {6, UPB_SIZE(2, 2), 6, 0, 8, _UPB_MODE_SCALAR | (_UPB_REP_1BYTE << _UPB_REP_SHIFT)},
};

const upb_msglayout google_protobuf_MethodDescriptorProto_msginit = {
  &google_protobuf_MethodDescriptorProto_submsgs[0],
  &google_protobuf_MethodDescriptorProto__fields[0],
  UPB_SIZE(32, 64), 6, _UPB_MSGEXT_NONE, 6, 255, 0,
};

static const upb_msglayout_sub google_protobuf_FileOptions_submsgs[2] = {
  {.submsg = &google_protobuf_UninterpretedOption_msginit},
  {.subenum = &google_protobuf_FileOptions_OptimizeMode_enuminit},
};

static const upb_msglayout_field google_protobuf_FileOptions__fields[21] = {
  {1, UPB_SIZE(20, 24), 1, 0, 12, _UPB_MODE_SCALAR | (_UPB_REP_STRVIEW << _UPB_REP_SHIFT)},
  {8, UPB_SIZE(28, 40), 2, 0, 12, _UPB_MODE_SCALAR | (_UPB_REP_STRVIEW << _UPB_REP_SHIFT)},
  {9, UPB_SIZE(4, 4), 3, 1, 14, _UPB_MODE_SCALAR | (_UPB_REP_4BYTE << _UPB_REP_SHIFT)},
  {10, UPB_SIZE(8, 8), 4, 0, 8, _UPB_MODE_SCALAR | (_UPB_REP_1BYTE << _UPB_REP_SHIFT)},
  {11, UPB_SIZE(36, 56), 5, 0, 12, _UPB_MODE_SCALAR | (_UPB_REP_STRVIEW << _UPB_REP_SHIFT)},
  {16, UPB_SIZE(9, 9), 6, 0, 8, _UPB_MODE_SCALAR | (_UPB_REP_1BYTE << _UPB_REP_SHIFT)},
  {17, UPB_SIZE(10, 10), 7, 0, 8, _UPB_MODE_SCALAR | (_UPB_REP_1BYTE << _UPB_REP_SHIFT)},
  {18, UPB_SIZE(11, 11), 8, 0, 8, _UPB_MODE_SCALAR | (_UPB_REP_1BYTE << _UPB_REP_SHIFT)},
  {20, UPB_SIZE(12, 12), 9, 0, 8, _UPB_MODE_SCALAR | (_UPB_REP_1BYTE << _UPB_REP_SHIFT)},
  {23, UPB_SIZE(13, 13), 10, 0, 8, _UPB_MODE_SCALAR | (_UPB_REP_1BYTE << _UPB_REP_SHIFT)},
  {27, UPB_SIZE(14, 14), 11, 0, 8, _UPB_MODE_SCALAR | (_UPB_REP_1BYTE << _UPB_REP_SHIFT)},
  {31, UPB_SIZE(15, 15), 12, 0, 8, _UPB_MODE_SCALAR | (_UPB_REP_1BYTE << _UPB_REP_SHIFT)},
  {36, UPB_SIZE(44, 72), 13, 0, 12, _UPB_MODE_SCALAR | (_UPB_REP_STRVIEW << _UPB_REP_SHIFT)},
  {37, UPB_SIZE(52, 88), 14, 0, 12, _UPB_MODE_SCALAR | (_UPB_REP_STRVIEW << _UPB_REP_SHIFT)},
  {39, UPB_SIZE(60, 104), 15, 0, 12, _UPB_MODE_SCALAR | (_UPB_REP_STRVIEW << _UPB_REP_SHIFT)},
  {40, UPB_SIZE(68, 120), 16, 0, 12, _UPB_MODE_SCALAR | (_UPB_REP_STRVIEW << _UPB_REP_SHIFT)},
  {41, UPB_SIZE(76, 136), 17, 0, 12, _UPB_MODE_SCALAR | (_UPB_REP_STRVIEW << _UPB_REP_SHIFT)},
  {42, UPB_SIZE(16, 16), 18, 0, 8, _UPB_MODE_SCALAR | (_UPB_REP_1BYTE << _UPB_REP_SHIFT)},
  {44, UPB_SIZE(84, 152), 19, 0, 12, _UPB_MODE_SCALAR | (_UPB_REP_STRVIEW << _UPB_REP_SHIFT)},
  {45, UPB_SIZE(92, 168), 20, 0, 12, _UPB_MODE_SCALAR | (_UPB_REP_STRVIEW << _UPB_REP_SHIFT)},
  {999, UPB_SIZE(100, 184), 0, 0, 11, _UPB_MODE_ARRAY | (_UPB_REP_PTR << _UPB_REP_SHIFT)},
};

const upb_msglayout google_protobuf_FileOptions_msginit = {
  &google_protobuf_FileOptions_submsgs[0],
  &google_protobuf_FileOptions__fields[0],
  UPB_SIZE(104, 192), 21, _UPB_MSGEXT_EXTENDABLE, 1, 255, 0,
};

static const upb_msglayout_sub google_protobuf_MessageOptions_submsgs[1] = {
  {.submsg = &google_protobuf_UninterpretedOption_msginit},
};

static const upb_msglayout_field google_protobuf_MessageOptions__fields[5] = {
  {1, UPB_SIZE(1, 1), 1, 0, 8, _UPB_MODE_SCALAR | (_UPB_REP_1BYTE << _UPB_REP_SHIFT)},
  {2, UPB_SIZE(2, 2), 2, 0, 8, _UPB_MODE_SCALAR | (_UPB_REP_1BYTE << _UPB_REP_SHIFT)},
  {3, UPB_SIZE(3, 3), 3, 0, 8, _UPB_MODE_SCALAR | (_UPB_REP_1BYTE << _UPB_REP_SHIFT)},
  {7, UPB_SIZE(4, 4), 4, 0, 8, _UPB_MODE_SCALAR | (_UPB_REP_1BYTE << _UPB_REP_SHIFT)},
  {999, UPB_SIZE(8, 8), 0, 0, 11, _UPB_MODE_ARRAY | (_UPB_REP_PTR << _UPB_REP_SHIFT)},
};

const upb_msglayout google_protobuf_MessageOptions_msginit = {
  &google_protobuf_MessageOptions_submsgs[0],
  &google_protobuf_MessageOptions__fields[0],
  UPB_SIZE(16, 16), 5, _UPB_MSGEXT_EXTENDABLE, 3, 255, 0,
};

static const upb_msglayout_sub google_protobuf_FieldOptions_submsgs[3] = {
  {.submsg = &google_protobuf_UninterpretedOption_msginit},
  {.subenum = &google_protobuf_FieldOptions_CType_enuminit},
  {.subenum = &google_protobuf_FieldOptions_JSType_enuminit},
};

static const upb_msglayout_field google_protobuf_FieldOptions__fields[7] = {
  {1, UPB_SIZE(4, 4), 1, 1, 14, _UPB_MODE_SCALAR | (_UPB_REP_4BYTE << _UPB_REP_SHIFT)},
  {2, UPB_SIZE(12, 12), 2, 0, 8, _UPB_MODE_SCALAR | (_UPB_REP_1BYTE << _UPB_REP_SHIFT)},
  {3, UPB_SIZE(13, 13), 3, 0, 8, _UPB_MODE_SCALAR | (_UPB_REP_1BYTE << _UPB_REP_SHIFT)},
  {5, UPB_SIZE(14, 14), 4, 0, 8, _UPB_MODE_SCALAR | (_UPB_REP_1BYTE << _UPB_REP_SHIFT)},
  {6, UPB_SIZE(8, 8), 5, 2, 14, _UPB_MODE_SCALAR | (_UPB_REP_4BYTE << _UPB_REP_SHIFT)},
  {10, UPB_SIZE(15, 15), 6, 0, 8, _UPB_MODE_SCALAR | (_UPB_REP_1BYTE << _UPB_REP_SHIFT)},
  {999, UPB_SIZE(16, 16), 0, 0, 11, _UPB_MODE_ARRAY | (_UPB_REP_PTR << _UPB_REP_SHIFT)},
};

const upb_msglayout google_protobuf_FieldOptions_msginit = {
  &google_protobuf_FieldOptions_submsgs[0],
  &google_protobuf_FieldOptions__fields[0],
  UPB_SIZE(24, 24), 7, _UPB_MSGEXT_EXTENDABLE, 3, 255, 0,
};

static const upb_msglayout_sub google_protobuf_OneofOptions_submsgs[1] = {
  {.submsg = &google_protobuf_UninterpretedOption_msginit},
};

static const upb_msglayout_field google_protobuf_OneofOptions__fields[1] = {
  {999, UPB_SIZE(0, 0), 0, 0, 11, _UPB_MODE_ARRAY | (_UPB_REP_PTR << _UPB_REP_SHIFT)},
};

const upb_msglayout google_protobuf_OneofOptions_msginit = {
  &google_protobuf_OneofOptions_submsgs[0],
  &google_protobuf_OneofOptions__fields[0],
  UPB_SIZE(8, 8), 1, _UPB_MSGEXT_EXTENDABLE, 0, 255, 0,
};

static const upb_msglayout_sub google_protobuf_EnumOptions_submsgs[1] = {
  {.submsg = &google_protobuf_UninterpretedOption_msginit},
};

static const upb_msglayout_field google_protobuf_EnumOptions__fields[3] = {
  {2, UPB_SIZE(1, 1), 1, 0, 8, _UPB_MODE_SCALAR | (_UPB_REP_1BYTE << _UPB_REP_SHIFT)},
  {3, UPB_SIZE(2, 2), 2, 0, 8, _UPB_MODE_SCALAR | (_UPB_REP_1BYTE << _UPB_REP_SHIFT)},
  {999, UPB_SIZE(4, 8), 0, 0, 11, _UPB_MODE_ARRAY | (_UPB_REP_PTR << _UPB_REP_SHIFT)},
};

const upb_msglayout google_protobuf_EnumOptions_msginit = {
  &google_protobuf_EnumOptions_submsgs[0],
  &google_protobuf_EnumOptions__fields[0],
  UPB_SIZE(8, 16), 3, _UPB_MSGEXT_EXTENDABLE, 0, 255, 0,
};

static const upb_msglayout_sub google_protobuf_EnumValueOptions_submsgs[1] = {
  {.submsg = &google_protobuf_UninterpretedOption_msginit},
};

static const upb_msglayout_field google_protobuf_EnumValueOptions__fields[2] = {
  {1, UPB_SIZE(1, 1), 1, 0, 8, _UPB_MODE_SCALAR | (_UPB_REP_1BYTE << _UPB_REP_SHIFT)},
  {999, UPB_SIZE(4, 8), 0, 0, 11, _UPB_MODE_ARRAY | (_UPB_REP_PTR << _UPB_REP_SHIFT)},
};

const upb_msglayout google_protobuf_EnumValueOptions_msginit = {
  &google_protobuf_EnumValueOptions_submsgs[0],
  &google_protobuf_EnumValueOptions__fields[0],
  UPB_SIZE(8, 16), 2, _UPB_MSGEXT_EXTENDABLE, 1, 255, 0,
};

static const upb_msglayout_sub google_protobuf_ServiceOptions_submsgs[1] = {
  {.submsg = &google_protobuf_UninterpretedOption_msginit},
};

static const upb_msglayout_field google_protobuf_ServiceOptions__fields[2] = {
  {33, UPB_SIZE(1, 1), 1, 0, 8, _UPB_MODE_SCALAR | (_UPB_REP_1BYTE << _UPB_REP_SHIFT)},
  {999, UPB_SIZE(4, 8), 0, 0, 11, _UPB_MODE_ARRAY | (_UPB_REP_PTR << _UPB_REP_SHIFT)},
};

const upb_msglayout google_protobuf_ServiceOptions_msginit = {
  &google_protobuf_ServiceOptions_submsgs[0],
  &google_protobuf_ServiceOptions__fields[0],
  UPB_SIZE(8, 16), 2, _UPB_MSGEXT_EXTENDABLE, 0, 255, 0,
};

static const upb_msglayout_sub google_protobuf_MethodOptions_submsgs[2] = {
  {.submsg = &google_protobuf_UninterpretedOption_msginit},
  {.subenum = &google_protobuf_MethodOptions_IdempotencyLevel_enuminit},
};

static const upb_msglayout_field google_protobuf_MethodOptions__fields[3] = {
  {33, UPB_SIZE(8, 8), 1, 0, 8, _UPB_MODE_SCALAR | (_UPB_REP_1BYTE << _UPB_REP_SHIFT)},
  {34, UPB_SIZE(4, 4), 2, 1, 14, _UPB_MODE_SCALAR | (_UPB_REP_4BYTE << _UPB_REP_SHIFT)},
  {999, UPB_SIZE(12, 16), 0, 0, 11, _UPB_MODE_ARRAY | (_UPB_REP_PTR << _UPB_REP_SHIFT)},
};

const upb_msglayout google_protobuf_MethodOptions_msginit = {
  &google_protobuf_MethodOptions_submsgs[0],
  &google_protobuf_MethodOptions__fields[0],
  UPB_SIZE(16, 24), 3, _UPB_MSGEXT_EXTENDABLE, 0, 255, 0,
};

static const upb_msglayout_sub google_protobuf_UninterpretedOption_submsgs[1] = {
  {.submsg = &google_protobuf_UninterpretedOption_NamePart_msginit},
};

static const upb_msglayout_field google_protobuf_UninterpretedOption__fields[7] = {
  {2, UPB_SIZE(56, 80), 0, 0, 11, _UPB_MODE_ARRAY | (_UPB_REP_PTR << _UPB_REP_SHIFT)},
  {3, UPB_SIZE(32, 32), 1, 0, 12, _UPB_MODE_SCALAR | (_UPB_REP_STRVIEW << _UPB_REP_SHIFT)},
  {4, UPB_SIZE(8, 8), 2, 0, 4, _UPB_MODE_SCALAR | (_UPB_REP_8BYTE << _UPB_REP_SHIFT)},
  {5, UPB_SIZE(16, 16), 3, 0, 3, _UPB_MODE_SCALAR | (_UPB_REP_8BYTE << _UPB_REP_SHIFT)},
  {6, UPB_SIZE(24, 24), 4, 0, 1, _UPB_MODE_SCALAR | (_UPB_REP_8BYTE << _UPB_REP_SHIFT)},
  {7, UPB_SIZE(40, 48), 5, 0, 12, _UPB_MODE_SCALAR | (_UPB_REP_STRVIEW << _UPB_REP_SHIFT)},
  {8, UPB_SIZE(48, 64), 6, 0, 12, _UPB_MODE_SCALAR | (_UPB_REP_STRVIEW << _UPB_REP_SHIFT)},
};

const upb_msglayout google_protobuf_UninterpretedOption_msginit = {
  &google_protobuf_UninterpretedOption_submsgs[0],
  &google_protobuf_UninterpretedOption__fields[0],
  UPB_SIZE(64, 96), 7, _UPB_MSGEXT_NONE, 0, 255, 0,
};

static const upb_msglayout_field google_protobuf_UninterpretedOption_NamePart__fields[2] = {
  {1, UPB_SIZE(4, 8), 1, 0, 12, _UPB_MODE_SCALAR | (_UPB_REP_STRVIEW << _UPB_REP_SHIFT)},
  {2, UPB_SIZE(1, 1), 2, 0, 8, _UPB_MODE_SCALAR | (_UPB_REP_1BYTE << _UPB_REP_SHIFT)},
};

const upb_msglayout google_protobuf_UninterpretedOption_NamePart_msginit = {
  NULL,
  &google_protobuf_UninterpretedOption_NamePart__fields[0],
  UPB_SIZE(16, 32), 2, _UPB_MSGEXT_NONE, 2, 255, 2,
};

static const upb_msglayout_sub google_protobuf_SourceCodeInfo_submsgs[1] = {
  {.submsg = &google_protobuf_SourceCodeInfo_Location_msginit},
};

static const upb_msglayout_field google_protobuf_SourceCodeInfo__fields[1] = {
  {1, UPB_SIZE(0, 0), 0, 0, 11, _UPB_MODE_ARRAY | (_UPB_REP_PTR << _UPB_REP_SHIFT)},
};

const upb_msglayout google_protobuf_SourceCodeInfo_msginit = {
  &google_protobuf_SourceCodeInfo_submsgs[0],
  &google_protobuf_SourceCodeInfo__fields[0],
  UPB_SIZE(8, 8), 1, _UPB_MSGEXT_NONE, 1, 255, 0,
};

static const upb_msglayout_field google_protobuf_SourceCodeInfo_Location__fields[5] = {
  {1, UPB_SIZE(20, 40), 0, 0, 5, _UPB_MODE_ARRAY | _UPB_MODE_IS_PACKED | (_UPB_REP_PTR << _UPB_REP_SHIFT)},
  {2, UPB_SIZE(24, 48), 0, 0, 5, _UPB_MODE_ARRAY | _UPB_MODE_IS_PACKED | (_UPB_REP_PTR << _UPB_REP_SHIFT)},
  {3, UPB_SIZE(4, 8), 1, 0, 12, _UPB_MODE_SCALAR | (_UPB_REP_STRVIEW << _UPB_REP_SHIFT)},
  {4, UPB_SIZE(12, 24), 2, 0, 12, _UPB_MODE_SCALAR | (_UPB_REP_STRVIEW << _UPB_REP_SHIFT)},
  {6, UPB_SIZE(28, 56), 0, 0, 12, _UPB_MODE_ARRAY | (_UPB_REP_PTR << _UPB_REP_SHIFT)},
};

const upb_msglayout google_protobuf_SourceCodeInfo_Location_msginit = {
  NULL,
  &google_protobuf_SourceCodeInfo_Location__fields[0],
  UPB_SIZE(32, 64), 5, _UPB_MSGEXT_NONE, 4, 255, 0,
};

static const upb_msglayout_sub google_protobuf_GeneratedCodeInfo_submsgs[1] = {
  {.submsg = &google_protobuf_GeneratedCodeInfo_Annotation_msginit},
};

static const upb_msglayout_field google_protobuf_GeneratedCodeInfo__fields[1] = {
  {1, UPB_SIZE(0, 0), 0, 0, 11, _UPB_MODE_ARRAY | (_UPB_REP_PTR << _UPB_REP_SHIFT)},
};

const upb_msglayout google_protobuf_GeneratedCodeInfo_msginit = {
  &google_protobuf_GeneratedCodeInfo_submsgs[0],
  &google_protobuf_GeneratedCodeInfo__fields[0],
  UPB_SIZE(8, 8), 1, _UPB_MSGEXT_NONE, 1, 255, 0,
};

static const upb_msglayout_field google_protobuf_GeneratedCodeInfo_Annotation__fields[4] = {
  {1, UPB_SIZE(20, 32), 0, 0, 5, _UPB_MODE_ARRAY | _UPB_MODE_IS_PACKED | (_UPB_REP_PTR << _UPB_REP_SHIFT)},
  {2, UPB_SIZE(12, 16), 1, 0, 12, _UPB_MODE_SCALAR | (_UPB_REP_STRVIEW << _UPB_REP_SHIFT)},
  {3, UPB_SIZE(4, 4), 2, 0, 5, _UPB_MODE_SCALAR | (_UPB_REP_4BYTE << _UPB_REP_SHIFT)},
  {4, UPB_SIZE(8, 8), 3, 0, 5, _UPB_MODE_SCALAR | (_UPB_REP_4BYTE << _UPB_REP_SHIFT)},
};

const upb_msglayout google_protobuf_GeneratedCodeInfo_Annotation_msginit = {
  NULL,
  &google_protobuf_GeneratedCodeInfo_Annotation__fields[0],
  UPB_SIZE(24, 48), 4, _UPB_MSGEXT_NONE, 4, 255, 0,
};

static const upb_msglayout *messages_layout[27] = {
  &google_protobuf_FileDescriptorSet_msginit,
  &google_protobuf_FileDescriptorProto_msginit,
  &google_protobuf_DescriptorProto_msginit,
  &google_protobuf_DescriptorProto_ExtensionRange_msginit,
  &google_protobuf_DescriptorProto_ReservedRange_msginit,
  &google_protobuf_ExtensionRangeOptions_msginit,
  &google_protobuf_FieldDescriptorProto_msginit,
  &google_protobuf_OneofDescriptorProto_msginit,
  &google_protobuf_EnumDescriptorProto_msginit,
  &google_protobuf_EnumDescriptorProto_EnumReservedRange_msginit,
  &google_protobuf_EnumValueDescriptorProto_msginit,
  &google_protobuf_ServiceDescriptorProto_msginit,
  &google_protobuf_MethodDescriptorProto_msginit,
  &google_protobuf_FileOptions_msginit,
  &google_protobuf_MessageOptions_msginit,
  &google_protobuf_FieldOptions_msginit,
  &google_protobuf_OneofOptions_msginit,
  &google_protobuf_EnumOptions_msginit,
  &google_protobuf_EnumValueOptions_msginit,
  &google_protobuf_ServiceOptions_msginit,
  &google_protobuf_MethodOptions_msginit,
  &google_protobuf_UninterpretedOption_msginit,
  &google_protobuf_UninterpretedOption_NamePart_msginit,
  &google_protobuf_SourceCodeInfo_msginit,
  &google_protobuf_SourceCodeInfo_Location_msginit,
  &google_protobuf_GeneratedCodeInfo_msginit,
  &google_protobuf_GeneratedCodeInfo_Annotation_msginit,
};

const upb_enumlayout google_protobuf_FieldDescriptorProto_Type_enuminit = {
  NULL,
  0x7fffeULL,
  0,
};

const upb_enumlayout google_protobuf_FieldDescriptorProto_Label_enuminit = {
  NULL,
  0xeULL,
  0,
};

const upb_enumlayout google_protobuf_FileOptions_OptimizeMode_enuminit = {
  NULL,
  0xeULL,
  0,
};

const upb_enumlayout google_protobuf_FieldOptions_CType_enuminit = {
  NULL,
  0x7ULL,
  0,
};

const upb_enumlayout google_protobuf_FieldOptions_JSType_enuminit = {
  NULL,
  0x7ULL,
  0,
};

const upb_enumlayout google_protobuf_MethodOptions_IdempotencyLevel_enuminit = {
  NULL,
  0x7ULL,
  0,
};

static const upb_enumlayout *enums_layout[6] = {
  &google_protobuf_FieldDescriptorProto_Type_enuminit,
  &google_protobuf_FieldDescriptorProto_Label_enuminit,
  &google_protobuf_FileOptions_OptimizeMode_enuminit,
  &google_protobuf_FieldOptions_CType_enuminit,
  &google_protobuf_FieldOptions_JSType_enuminit,
  &google_protobuf_MethodOptions_IdempotencyLevel_enuminit,
};

const upb_msglayout_file google_protobuf_descriptor_proto_upb_file_layout = {
  messages_layout,
  enums_layout,
  NULL,
  27,
  6,
  0,
};



/** upb/def.c ************************************************************/

#include <ctype.h>
#include <errno.h>
#include <setjmp.h>
#include <stdlib.h>
#include <string.h>


/* Must be last. */

typedef struct {
  size_t len;
  char str[1];  /* Null-terminated string data follows. */
} str_t;

/* The upb core does not generally have a concept of default instances. However
 * for descriptor options we make an exception since the max size is known and
 * modest (<200 bytes). All types can share a default instance since it is
 * initialized to zeroes.
 *
 * We have to allocate an extra pointer for upb's internal metadata. */
static const char opt_default_buf[_UPB_MAXOPT_SIZE + sizeof(void*)] = {0};
static const char *opt_default = &opt_default_buf[sizeof(void*)];

struct upb_fielddef {
  const google_protobuf_FieldOptions *opts;
  const upb_filedef *file;
  const upb_msgdef *msgdef;
  const char *full_name;
  const char *json_name;
  union {
    int64_t sint;
    uint64_t uint;
    double dbl;
    float flt;
    bool boolean;
    str_t *str;
  } defaultval;
  const upb_oneofdef *oneof;
  union {
    const upb_msgdef *msgdef;
    const upb_enumdef *enumdef;
    const google_protobuf_FieldDescriptorProto *unresolved;
  } sub;
  uint32_t number_;
  uint16_t index_;
  uint16_t layout_index;  /* Index into msgdef->layout->fields or file->exts */
  bool has_default;
  bool is_extension_;
  bool packed_;
  bool proto3_optional_;
  bool has_json_name_;
  upb_descriptortype_t type_;
  upb_label_t label_;
};

struct upb_extrange {
  const google_protobuf_ExtensionRangeOptions *opts;
  int32_t start;
  int32_t end;
};

struct upb_msgdef {
  const google_protobuf_MessageOptions *opts;
  const upb_msglayout *layout;
  const upb_filedef *file;
  const upb_msgdef *containing_type;
  const char *full_name;

  /* Tables for looking up fields by number and name. */
  upb_inttable itof;
  upb_strtable ntof;

  /* All nested defs.
   * MEM: We could save some space here by putting nested defs in a contigous
   * region and calculating counts from offets or vice-versa. */
  const upb_fielddef *fields;
  const upb_oneofdef *oneofs;
  const upb_extrange *ext_ranges;
  const upb_msgdef *nested_msgs;
  const upb_enumdef *nested_enums;
  const upb_fielddef *nested_exts;
  int field_count;
  int real_oneof_count;
  int oneof_count;
  int ext_range_count;
  int nested_msg_count;
  int nested_enum_count;
  int nested_ext_count;
  bool in_message_set;
  upb_wellknowntype_t well_known_type;
};

struct upb_enumdef {
  const google_protobuf_EnumOptions *opts;
  const upb_enumlayout *layout;  // Only for proto2.
  const upb_filedef *file;
  const upb_msgdef *containing_type;  // Could be merged with "file".
  const char *full_name;
  upb_strtable ntoi;
  upb_inttable iton;
  const upb_enumvaldef *values;
  int value_count;
  int32_t defaultval;
};

struct upb_enumvaldef {
  const google_protobuf_EnumValueOptions *opts;
  const upb_enumdef *enum_;
  const char *full_name;
  int32_t number;
};

struct upb_oneofdef {
  const google_protobuf_OneofOptions *opts;
  const upb_msgdef *parent;
  const char *full_name;
  int field_count;
  bool synthetic;
  const upb_fielddef **fields;
  upb_strtable ntof;
  upb_inttable itof;
};

struct upb_filedef {
  const google_protobuf_FileOptions *opts;
  const char *name;
  const char *package;

  const upb_filedef **deps;
  const int32_t* public_deps;
  const int32_t* weak_deps;
  const upb_msgdef *top_lvl_msgs;
  const upb_enumdef *top_lvl_enums;
  const upb_fielddef *top_lvl_exts;
  const upb_servicedef *services;
  const upb_msglayout_ext **ext_layouts;
  const upb_symtab *symtab;

  int dep_count;
  int public_dep_count;
  int weak_dep_count;
  int top_lvl_msg_count;
  int top_lvl_enum_count;
  int top_lvl_ext_count;
  int service_count;
  int ext_count;  /* All exts in the file. */
  upb_syntax_t syntax;
};

struct upb_methoddef {
  const google_protobuf_MethodOptions *opts;
  upb_servicedef *service;
  const char *full_name;
  const upb_msgdef *input_type;
  const upb_msgdef *output_type;
  bool client_streaming;
  bool server_streaming;
};

struct upb_servicedef {
  const google_protobuf_ServiceOptions *opts;
  const upb_filedef *file;
  const char *full_name;
  upb_methoddef *methods;
  int method_count;
  int index;
};

struct upb_symtab {
  upb_arena *arena;
  upb_strtable syms;  /* full_name -> packed def ptr */
  upb_strtable files;  /* file_name -> upb_filedef* */
  upb_inttable exts;   /* upb_msglayout_ext* -> upb_fielddef* */
  upb_extreg *extreg;
  size_t bytes_loaded;

  // For compatibility with proto2, we have to accept json_names that conflict
  // with field names or other json_names.  This is very ill-advised, so we only
  // allow this when it is needed (and hopefully these cases can be cleaned up
  // and eliminated.  When this is enabled, the results are not well-defined.
  bool allow_name_conflicts;
};

/* Inside a symtab we store tagged pointers to specific def types. */
typedef enum {
  UPB_DEFTYPE_MASK = 7,

  /* Only inside symtab table. */
  UPB_DEFTYPE_EXT = 0,
  UPB_DEFTYPE_MSG = 1,
  UPB_DEFTYPE_ENUM = 2,
  UPB_DEFTYPE_ENUMVAL = 3,
  UPB_DEFTYPE_SERVICE = 4,

  /* Only inside message table. */
  UPB_DEFTYPE_FIELD = 0,
  UPB_DEFTYPE_ONEOF = 1,
  UPB_DEFTYPE_FIELD_JSONNAME = 2,

  /* Only inside file table. */
  UPB_DEFTYPE_FILE = 0,
  UPB_DEFTYPE_LAYOUT = 1
} upb_deftype_t;

#define FIELD_TYPE_UNSPECIFIED 0

static upb_deftype_t deftype(upb_value v) {
  uintptr_t num = (uintptr_t)upb_value_getconstptr(v);
  return num & UPB_DEFTYPE_MASK;
}

static const void *unpack_def(upb_value v, upb_deftype_t type) {
  uintptr_t num = (uintptr_t)upb_value_getconstptr(v);
  return (num & UPB_DEFTYPE_MASK) == type
             ? (const void *)(num & ~UPB_DEFTYPE_MASK)
             : NULL;
}

static upb_value pack_def(const void *ptr, upb_deftype_t type) {
  uintptr_t num = (uintptr_t)ptr;
  UPB_ASSERT((num & UPB_DEFTYPE_MASK) == 0);
  num |= type;
  return upb_value_constptr((const void*)num);
}

/* isalpha() etc. from <ctype.h> are locale-dependent, which we don't want. */
static bool upb_isbetween(uint8_t c, uint8_t low, uint8_t high) {
  return c >= low && c <= high;
}

static bool upb_isletter(char c) {
  char lower = c | 0x20;  // Per ASCII this will lowercase a letter.
  return upb_isbetween(lower, 'a', 'z') || c == '_';
}

static bool upb_isalphanum(char c) {
  return upb_isletter(c) || upb_isbetween(c, '0', '9');
}

static const char *shortdefname(const char *fullname) {
  const char *p;

  if (fullname == NULL) {
    return NULL;
  } else if ((p = strrchr(fullname, '.')) == NULL) {
    /* No '.' in the name, return the full string. */
    return fullname;
  } else {
    /* Return one past the last '.'. */
    return p + 1;
  }
}

/* All submessage fields are lower than all other fields.
 * Secondly, fields are increasing in order. */
uint32_t field_rank(const upb_fielddef *f) {
  uint32_t ret = upb_fielddef_number(f);
  const uint32_t high_bit = 1 << 30;
  UPB_ASSERT(ret < high_bit);
  if (!upb_fielddef_issubmsg(f))
    ret |= high_bit;
  return ret;
}

int cmp_fields(const void *p1, const void *p2) {
  const upb_fielddef *f1 = *(upb_fielddef*const*)p1;
  const upb_fielddef *f2 = *(upb_fielddef*const*)p2;
  return field_rank(f1) - field_rank(f2);
}

static void upb_status_setoom(upb_status *status) {
  upb_status_seterrmsg(status, "out of memory");
}

static void assign_msg_wellknowntype(upb_msgdef *m) {
  const char *name = upb_msgdef_fullname(m);
  if (name == NULL) {
    m->well_known_type = UPB_WELLKNOWN_UNSPECIFIED;
    return;
  }
  if (!strcmp(name, "google.protobuf.Any")) {
    m->well_known_type = UPB_WELLKNOWN_ANY;
  } else if (!strcmp(name, "google.protobuf.FieldMask")) {
    m->well_known_type = UPB_WELLKNOWN_FIELDMASK;
  } else if (!strcmp(name, "google.protobuf.Duration")) {
    m->well_known_type = UPB_WELLKNOWN_DURATION;
  } else if (!strcmp(name, "google.protobuf.Timestamp")) {
    m->well_known_type = UPB_WELLKNOWN_TIMESTAMP;
  } else if (!strcmp(name, "google.protobuf.DoubleValue")) {
    m->well_known_type = UPB_WELLKNOWN_DOUBLEVALUE;
  } else if (!strcmp(name, "google.protobuf.FloatValue")) {
    m->well_known_type = UPB_WELLKNOWN_FLOATVALUE;
  } else if (!strcmp(name, "google.protobuf.Int64Value")) {
    m->well_known_type = UPB_WELLKNOWN_INT64VALUE;
  } else if (!strcmp(name, "google.protobuf.UInt64Value")) {
    m->well_known_type = UPB_WELLKNOWN_UINT64VALUE;
  } else if (!strcmp(name, "google.protobuf.Int32Value")) {
    m->well_known_type = UPB_WELLKNOWN_INT32VALUE;
  } else if (!strcmp(name, "google.protobuf.UInt32Value")) {
    m->well_known_type = UPB_WELLKNOWN_UINT32VALUE;
  } else if (!strcmp(name, "google.protobuf.BoolValue")) {
    m->well_known_type = UPB_WELLKNOWN_BOOLVALUE;
  } else if (!strcmp(name, "google.protobuf.StringValue")) {
    m->well_known_type = UPB_WELLKNOWN_STRINGVALUE;
  } else if (!strcmp(name, "google.protobuf.BytesValue")) {
    m->well_known_type = UPB_WELLKNOWN_BYTESVALUE;
  } else if (!strcmp(name, "google.protobuf.Value")) {
    m->well_known_type = UPB_WELLKNOWN_VALUE;
  } else if (!strcmp(name, "google.protobuf.ListValue")) {
    m->well_known_type = UPB_WELLKNOWN_LISTVALUE;
  } else if (!strcmp(name, "google.protobuf.Struct")) {
    m->well_known_type = UPB_WELLKNOWN_STRUCT;
  } else {
    m->well_known_type = UPB_WELLKNOWN_UNSPECIFIED;
  }
}


/* upb_enumdef ****************************************************************/

const google_protobuf_EnumOptions *upb_enumdef_options(const upb_enumdef *e) {
  return e->opts;
}

bool upb_enumdef_hasoptions(const upb_enumdef *e) {
  return e->opts != (void*)opt_default;
}

const char *upb_enumdef_fullname(const upb_enumdef *e) {
  return e->full_name;
}

const char *upb_enumdef_name(const upb_enumdef *e) {
  return shortdefname(e->full_name);
}

const upb_filedef *upb_enumdef_file(const upb_enumdef *e) {
  return e->file;
}

const upb_msgdef *upb_enumdef_containingtype(const upb_enumdef *e) {
  return e->containing_type;
}

int32_t upb_enumdef_default(const upb_enumdef *e) {
  UPB_ASSERT(upb_enumdef_lookupnum(e, e->defaultval));
  return e->defaultval;
}

int upb_enumdef_valuecount(const upb_enumdef *e) {
  return e->value_count;
}

const upb_enumvaldef *upb_enumdef_lookupname(const upb_enumdef *def,
                                             const char *name, size_t len) {
  upb_value v;
  return upb_strtable_lookup2(&def->ntoi, name, len, &v)
             ? upb_value_getconstptr(v)
             : NULL;
}

const upb_enumvaldef *upb_enumdef_lookupnum(const upb_enumdef *def, int32_t num) {
  upb_value v;
  return upb_inttable_lookup(&def->iton, num, &v) ? upb_value_getconstptr(v)
                                                  : NULL;
}

bool upb_enumdef_checknum(const upb_enumdef *e, int32_t num) {
  // We could use upb_enumdef_lookupnum(e, num) != NULL, but we expect this to
  // be faster (especially for small numbers).
  return _upb_enumlayout_checkval(e->layout, num);
}

const upb_enumvaldef *upb_enumdef_value(const upb_enumdef *e, int i) {
  UPB_ASSERT(0 <= i && i < e->value_count);
  return &e->values[i];
}

// Deprecated functions.

int upb_enumdef_numvals(const upb_enumdef *e) {
  return (int)upb_strtable_count(&e->ntoi);
}

void upb_enum_begin(upb_enum_iter *i, const upb_enumdef *e) {
  /* We iterate over the ntoi table, to account for duplicate numbers. */
  upb_strtable_begin(i, &e->ntoi);
}

void upb_enum_next(upb_enum_iter *iter) { upb_strtable_next(iter); }
bool upb_enum_done(upb_enum_iter *iter) { return upb_strtable_done(iter); }

const char *upb_enum_iter_name(upb_enum_iter *iter) {
  return upb_strtable_iter_key(iter).data;
}

int32_t upb_enum_iter_number(upb_enum_iter *iter) {
  return upb_value_getint32(upb_strtable_iter_value(iter));
}


/* upb_enumvaldef *************************************************************/

const google_protobuf_EnumValueOptions *upb_enumvaldef_options(
    const upb_enumvaldef *e) {
  return e->opts;
}

bool upb_enumvaldef_hasoptions(const upb_enumvaldef *e) {
  return e->opts != (void*)opt_default;
}

const upb_enumdef *upb_enumvaldef_enum(const upb_enumvaldef *ev) {
  return ev->enum_;
}

const char *upb_enumvaldef_fullname(const upb_enumvaldef *ev) {
  return ev->full_name;
}

const char *upb_enumvaldef_name(const upb_enumvaldef *ev) {
  return shortdefname(ev->full_name);
}

int32_t upb_enumvaldef_number(const upb_enumvaldef *ev) {
  return ev->number;
}

/* upb_extrange ***************************************************************/

const google_protobuf_ExtensionRangeOptions *upb_extrange_options(
    const upb_extrange *r) {
  return r->opts;
}

bool upb_extrange_hasoptions(const upb_extrange *r) {
  return r->opts != (void*)opt_default;
}

int32_t upb_extrange_start(const upb_extrange *e) {
  return e->start;
}

int32_t upb_extrange_end(const upb_extrange *e) {
  return e->end;
}

/* upb_fielddef ***************************************************************/

const google_protobuf_FieldOptions *upb_fielddef_options(
    const upb_fielddef *f) {
  return f->opts;
}

bool upb_fielddef_hasoptions(const upb_fielddef *f) {
  return f->opts != (void*)opt_default;
}

const char *upb_fielddef_fullname(const upb_fielddef *f) {
  return f->full_name;
}

upb_fieldtype_t upb_fielddef_type(const upb_fielddef *f) {
  switch (f->type_) {
    case UPB_DESCRIPTOR_TYPE_DOUBLE:
      return UPB_TYPE_DOUBLE;
    case UPB_DESCRIPTOR_TYPE_FLOAT:
      return UPB_TYPE_FLOAT;
    case UPB_DESCRIPTOR_TYPE_INT64:
    case UPB_DESCRIPTOR_TYPE_SINT64:
    case UPB_DESCRIPTOR_TYPE_SFIXED64:
      return UPB_TYPE_INT64;
    case UPB_DESCRIPTOR_TYPE_INT32:
    case UPB_DESCRIPTOR_TYPE_SFIXED32:
    case UPB_DESCRIPTOR_TYPE_SINT32:
      return UPB_TYPE_INT32;
    case UPB_DESCRIPTOR_TYPE_UINT64:
    case UPB_DESCRIPTOR_TYPE_FIXED64:
      return UPB_TYPE_UINT64;
    case UPB_DESCRIPTOR_TYPE_UINT32:
    case UPB_DESCRIPTOR_TYPE_FIXED32:
      return UPB_TYPE_UINT32;
    case UPB_DESCRIPTOR_TYPE_ENUM:
      return UPB_TYPE_ENUM;
    case UPB_DESCRIPTOR_TYPE_BOOL:
      return UPB_TYPE_BOOL;
    case UPB_DESCRIPTOR_TYPE_STRING:
      return UPB_TYPE_STRING;
    case UPB_DESCRIPTOR_TYPE_BYTES:
      return UPB_TYPE_BYTES;
    case UPB_DESCRIPTOR_TYPE_GROUP:
    case UPB_DESCRIPTOR_TYPE_MESSAGE:
      return UPB_TYPE_MESSAGE;
  }
  UPB_UNREACHABLE();
}

upb_descriptortype_t upb_fielddef_descriptortype(const upb_fielddef *f) {
  return f->type_;
}

uint32_t upb_fielddef_index(const upb_fielddef *f) {
  return f->index_;
}

upb_label_t upb_fielddef_label(const upb_fielddef *f) {
  return f->label_;
}

uint32_t upb_fielddef_number(const upb_fielddef *f) {
  return f->number_;
}

bool upb_fielddef_isextension(const upb_fielddef *f) {
  return f->is_extension_;
}

bool upb_fielddef_packed(const upb_fielddef *f) {
  return f->packed_;
}

const char *upb_fielddef_name(const upb_fielddef *f) {
  return shortdefname(f->full_name);
}

const char *upb_fielddef_jsonname(const upb_fielddef *f) {
  return f->json_name;
}

bool upb_fielddef_hasjsonname(const upb_fielddef *f) {
  return f->has_json_name_;
}

const upb_filedef *upb_fielddef_file(const upb_fielddef *f) {
  return f->file;
}

const upb_msgdef *upb_fielddef_containingtype(const upb_fielddef *f) {
  return f->msgdef;
}

const upb_oneofdef *upb_fielddef_containingoneof(const upb_fielddef *f) {
  return f->oneof;
}

const upb_oneofdef *upb_fielddef_realcontainingoneof(const upb_fielddef *f) {
  if (!f->oneof || upb_oneofdef_issynthetic(f->oneof)) return NULL;
  return f->oneof;
}

upb_msgval upb_fielddef_default(const upb_fielddef *f) {
  UPB_ASSERT(!upb_fielddef_issubmsg(f));
  upb_msgval ret;

  switch (upb_fielddef_type(f)) {
    case UPB_TYPE_BOOL:
      ret.bool_val = upb_fielddef_defaultbool(f);
      break;
    case UPB_TYPE_INT64:
      ret.int64_val = upb_fielddef_defaultint64(f);
      break;
    case UPB_TYPE_UINT64:
      ret.uint64_val = upb_fielddef_defaultuint64(f);
      break;
    case UPB_TYPE_ENUM:
    case UPB_TYPE_INT32:
      ret.int32_val = upb_fielddef_defaultint32(f);
      break;
    case UPB_TYPE_UINT32:
      ret.uint32_val = upb_fielddef_defaultuint32(f);
      break;
    case UPB_TYPE_FLOAT:
      ret.float_val = upb_fielddef_defaultfloat(f);
      break;
    case UPB_TYPE_DOUBLE:
      ret.double_val = upb_fielddef_defaultdouble(f);
      break;
    case UPB_TYPE_STRING:
    case UPB_TYPE_BYTES: {
      ret.str_val.data = upb_fielddef_defaultstr(f, &ret.str_val.size);
      break;
    }
    default:
      UPB_UNREACHABLE();
  }

  return ret;
}

static void chkdefaulttype(const upb_fielddef *f, int ctype) {
  UPB_UNUSED(f);
  UPB_UNUSED(ctype);
}

int64_t upb_fielddef_defaultint64(const upb_fielddef *f) {
  chkdefaulttype(f, UPB_TYPE_INT64);
  return f->defaultval.sint;
}

int32_t upb_fielddef_defaultint32(const upb_fielddef *f) {
  chkdefaulttype(f, UPB_TYPE_INT32);
  return (int32_t)f->defaultval.sint;
}

uint64_t upb_fielddef_defaultuint64(const upb_fielddef *f) {
  chkdefaulttype(f, UPB_TYPE_UINT64);
  return f->defaultval.uint;
}

uint32_t upb_fielddef_defaultuint32(const upb_fielddef *f) {
  chkdefaulttype(f, UPB_TYPE_UINT32);
  return (uint32_t)f->defaultval.uint;
}

bool upb_fielddef_defaultbool(const upb_fielddef *f) {
  chkdefaulttype(f, UPB_TYPE_BOOL);
  return f->defaultval.boolean;
}

float upb_fielddef_defaultfloat(const upb_fielddef *f) {
  chkdefaulttype(f, UPB_TYPE_FLOAT);
  return f->defaultval.flt;
}

double upb_fielddef_defaultdouble(const upb_fielddef *f) {
  chkdefaulttype(f, UPB_TYPE_DOUBLE);
  return f->defaultval.dbl;
}

const char *upb_fielddef_defaultstr(const upb_fielddef *f, size_t *len) {
  str_t *str = f->defaultval.str;
  UPB_ASSERT(upb_fielddef_type(f) == UPB_TYPE_STRING ||
         upb_fielddef_type(f) == UPB_TYPE_BYTES ||
         upb_fielddef_type(f) == UPB_TYPE_ENUM);
  if (str) {
    if (len) *len = str->len;
    return str->str;
  } else {
    if (len) *len = 0;
    return NULL;
  }
}

const upb_msgdef *upb_fielddef_msgsubdef(const upb_fielddef *f) {
  return upb_fielddef_type(f) == UPB_TYPE_MESSAGE ? f->sub.msgdef : NULL;
}

const upb_enumdef *upb_fielddef_enumsubdef(const upb_fielddef *f) {
  return upb_fielddef_type(f) == UPB_TYPE_ENUM ? f->sub.enumdef : NULL;
}

const upb_msglayout_field *upb_fielddef_layout(const upb_fielddef *f) {
  UPB_ASSERT(!upb_fielddef_isextension(f));
  return &f->msgdef->layout->fields[f->layout_index];
}

const upb_msglayout_ext *_upb_fielddef_extlayout(const upb_fielddef *f) {
  UPB_ASSERT(upb_fielddef_isextension(f));
  return f->file->ext_layouts[f->layout_index];
}

bool _upb_fielddef_proto3optional(const upb_fielddef *f) {
  return f->proto3_optional_;
}

bool upb_fielddef_issubmsg(const upb_fielddef *f) {
  return upb_fielddef_type(f) == UPB_TYPE_MESSAGE;
}

bool upb_fielddef_isstring(const upb_fielddef *f) {
  return upb_fielddef_type(f) == UPB_TYPE_STRING ||
         upb_fielddef_type(f) == UPB_TYPE_BYTES;
}

bool upb_fielddef_isseq(const upb_fielddef *f) {
  return upb_fielddef_label(f) == UPB_LABEL_REPEATED;
}

bool upb_fielddef_isprimitive(const upb_fielddef *f) {
  return !upb_fielddef_isstring(f) && !upb_fielddef_issubmsg(f);
}

bool upb_fielddef_ismap(const upb_fielddef *f) {
  return upb_fielddef_isseq(f) && upb_fielddef_issubmsg(f) &&
         upb_msgdef_mapentry(upb_fielddef_msgsubdef(f));
}

bool upb_fielddef_hasdefault(const upb_fielddef *f) {
  return f->has_default;
}

bool upb_fielddef_hassubdef(const upb_fielddef *f) {
  return upb_fielddef_issubmsg(f) || upb_fielddef_type(f) == UPB_TYPE_ENUM;
}

bool upb_fielddef_haspresence(const upb_fielddef *f) {
  if (upb_fielddef_isseq(f)) return false;
  return upb_fielddef_issubmsg(f) || upb_fielddef_containingoneof(f) ||
         f->file->syntax == UPB_SYNTAX_PROTO2;
}

static bool between(int32_t x, int32_t low, int32_t high) {
  return x >= low && x <= high;
}

bool upb_fielddef_checklabel(int32_t label) { return between(label, 1, 3); }
bool upb_fielddef_checktype(int32_t type) { return between(type, 1, 11); }
bool upb_fielddef_checkintfmt(int32_t fmt) { return between(fmt, 1, 3); }

bool upb_fielddef_checkdescriptortype(int32_t type) {
  return between(type, 1, 18);
}

/* upb_msgdef *****************************************************************/

const google_protobuf_MessageOptions *upb_msgdef_options(const upb_msgdef *m) {
  return m->opts;
}

bool upb_msgdef_hasoptions(const upb_msgdef *m) {
  return m->opts != (void*)opt_default;
}

const char *upb_msgdef_fullname(const upb_msgdef *m) {
  return m->full_name;
}

const upb_filedef *upb_msgdef_file(const upb_msgdef *m) {
  return m->file;
}

const upb_msgdef *upb_msgdef_containingtype(const upb_msgdef *m) {
  return m->containing_type;
}

const char *upb_msgdef_name(const upb_msgdef *m) {
  return shortdefname(m->full_name);
}

upb_syntax_t upb_msgdef_syntax(const upb_msgdef *m) {
  return m->file->syntax;
}

const upb_fielddef *upb_msgdef_itof(const upb_msgdef *m, uint32_t i) {
  upb_value val;
  return upb_inttable_lookup(&m->itof, i, &val) ? upb_value_getconstptr(val)
                                                : NULL;
}

const upb_fielddef *upb_msgdef_ntof(const upb_msgdef *m, const char *name,
                                    size_t len) {
  upb_value val;

  if (!upb_strtable_lookup2(&m->ntof, name, len, &val)) {
    return NULL;
  }

  return unpack_def(val, UPB_DEFTYPE_FIELD);
}

const upb_oneofdef *upb_msgdef_ntoo(const upb_msgdef *m, const char *name,
                                    size_t len) {
  upb_value val;

  if (!upb_strtable_lookup2(&m->ntof, name, len, &val)) {
    return NULL;
  }

  return unpack_def(val, UPB_DEFTYPE_ONEOF);
}

bool upb_msgdef_lookupname(const upb_msgdef *m, const char *name, size_t len,
                           const upb_fielddef **out_f,
                           const upb_oneofdef **out_o) {
  upb_value val;

  if (!upb_strtable_lookup2(&m->ntof, name, len, &val)) {
    return false;
  }

  const upb_fielddef *f = unpack_def(val, UPB_DEFTYPE_FIELD);
  const upb_oneofdef *o = unpack_def(val, UPB_DEFTYPE_ONEOF);
  if (out_f) *out_f = f;
  if (out_o) *out_o = o;
  return f || o;  /* False if this was a JSON name. */
}

const upb_fielddef *upb_msgdef_lookupjsonname(const upb_msgdef *m,
                                              const char *name, size_t len) {
  upb_value val;
  const upb_fielddef* f;

  if (!upb_strtable_lookup2(&m->ntof, name, len, &val)) {
    return NULL;
  }

  f = unpack_def(val, UPB_DEFTYPE_FIELD);
  if (!f) f = unpack_def(val, UPB_DEFTYPE_FIELD_JSONNAME);

  return f;
}

int upb_msgdef_numfields(const upb_msgdef *m) {
  return m->field_count;
}

int upb_msgdef_numoneofs(const upb_msgdef *m) {
  return m->oneof_count;
}

int upb_msgdef_numrealoneofs(const upb_msgdef *m) {
  return m->real_oneof_count;
}

int upb_msgdef_extrangecount(const upb_msgdef *m) {
  return m->ext_range_count;
}

int upb_msgdef_fieldcount(const upb_msgdef *m) {
  return m->field_count;
}

int upb_msgdef_oneofcount(const upb_msgdef *m) {
  return m->oneof_count;
}

int upb_msgdef_nestedmsgcount(const upb_msgdef *m) {
  return m->nested_msg_count;
}

int upb_msgdef_nestedenumcount(const upb_msgdef *m) {
  return m->nested_enum_count;
}

int upb_msgdef_nestedextcount(const upb_msgdef *m) {
  return m->nested_ext_count;
}

int upb_msgdef_realoneofcount(const upb_msgdef *m) {
  return m->real_oneof_count;
}

const upb_msglayout *upb_msgdef_layout(const upb_msgdef *m) {
  return m->layout;
}

const upb_extrange *upb_msgdef_extrange(const upb_msgdef *m, int i) {
  UPB_ASSERT(0 <= i && i < m->ext_range_count);
  return &m->ext_ranges[i];
}

const upb_fielddef *upb_msgdef_field(const upb_msgdef *m, int i) {
  UPB_ASSERT(0 <= i && i < m->field_count);
  return &m->fields[i];
}

const upb_oneofdef *upb_msgdef_oneof(const upb_msgdef *m, int i) {
  UPB_ASSERT(0 <= i && i < m->oneof_count);
  return &m->oneofs[i];
}

const upb_msgdef *upb_msgdef_nestedmsg(const upb_msgdef *m, int i) {
  UPB_ASSERT(0 <= i && i < m->nested_msg_count);
  return &m->nested_msgs[i];
}

const upb_enumdef *upb_msgdef_nestedenum(const upb_msgdef *m, int i) {
  UPB_ASSERT(0 <= i && i < m->nested_enum_count);
  return &m->nested_enums[i];
}

const upb_fielddef *upb_msgdef_nestedext(const upb_msgdef *m, int i) {
  UPB_ASSERT(0 <= i && i < m->nested_ext_count);
  return &m->nested_exts[i];
}

upb_wellknowntype_t upb_msgdef_wellknowntype(const upb_msgdef *m) {
  return m->well_known_type;
}

bool upb_msgdef_isnumberwrapper(const upb_msgdef *m) {
  upb_wellknowntype_t type = upb_msgdef_wellknowntype(m);
  return type >= UPB_WELLKNOWN_DOUBLEVALUE &&
         type <= UPB_WELLKNOWN_UINT32VALUE;
}

bool upb_msgdef_iswrapper(const upb_msgdef *m) {
  upb_wellknowntype_t type = upb_msgdef_wellknowntype(m);
  return type >= UPB_WELLKNOWN_DOUBLEVALUE &&
         type <= UPB_WELLKNOWN_BOOLVALUE;
}

void upb_msg_field_begin(upb_msg_field_iter *iter, const upb_msgdef *m) {
  upb_inttable_begin(iter, &m->itof);
}

void upb_msg_field_next(upb_msg_field_iter *iter) { upb_inttable_next(iter); }

bool upb_msg_field_done(const upb_msg_field_iter *iter) {
  return upb_inttable_done(iter);
}

upb_fielddef *upb_msg_iter_field(const upb_msg_field_iter *iter) {
  return (upb_fielddef *)upb_value_getconstptr(upb_inttable_iter_value(iter));
}

void upb_msg_field_iter_setdone(upb_msg_field_iter *iter) {
  upb_inttable_iter_setdone(iter);
}

bool upb_msg_field_iter_isequal(const upb_msg_field_iter * iter1,
                                const upb_msg_field_iter * iter2) {
  return upb_inttable_iter_isequal(iter1, iter2);
}

void upb_msg_oneof_begin(upb_msg_oneof_iter *iter, const upb_msgdef *m) {
  upb_strtable_begin(iter, &m->ntof);
  /* We need to skip past any initial fields. */
  while (!upb_strtable_done(iter) &&
         !unpack_def(upb_strtable_iter_value(iter), UPB_DEFTYPE_ONEOF)) {
    upb_strtable_next(iter);
  }
}

void upb_msg_oneof_next(upb_msg_oneof_iter *iter) {
  /* We need to skip past fields to return only oneofs. */
  do {
    upb_strtable_next(iter);
  } while (!upb_strtable_done(iter) &&
           !unpack_def(upb_strtable_iter_value(iter), UPB_DEFTYPE_ONEOF));
}

bool upb_msg_oneof_done(const upb_msg_oneof_iter *iter) {
  return upb_strtable_done(iter);
}

const upb_oneofdef *upb_msg_iter_oneof(const upb_msg_oneof_iter *iter) {
  return unpack_def(upb_strtable_iter_value(iter), UPB_DEFTYPE_ONEOF);
}

void upb_msg_oneof_iter_setdone(upb_msg_oneof_iter *iter) {
  upb_strtable_iter_setdone(iter);
}

bool upb_msg_oneof_iter_isequal(const upb_msg_oneof_iter *iter1,
                                const upb_msg_oneof_iter *iter2) {
  return upb_strtable_iter_isequal(iter1, iter2);
}

/* upb_oneofdef ***************************************************************/

const google_protobuf_OneofOptions *upb_oneofdef_options(const upb_oneofdef *o) {
  return o->opts;
}

bool upb_oneofdef_hasoptions(const upb_oneofdef *o) {
  return o->opts != (void*)opt_default;
}

const char *upb_oneofdef_name(const upb_oneofdef *o) {
  return shortdefname(o->full_name);
}

const upb_msgdef *upb_oneofdef_containingtype(const upb_oneofdef *o) {
  return o->parent;
}

int upb_oneofdef_fieldcount(const upb_oneofdef *o) {
  return o->field_count;
}

const upb_fielddef *upb_oneofdef_field(const upb_oneofdef *o, int i) {
  UPB_ASSERT(i < o->field_count);
  return o->fields[i];
}

int upb_oneofdef_numfields(const upb_oneofdef *o) {
  return o->field_count;
}

uint32_t upb_oneofdef_index(const upb_oneofdef *o) {
  return o - o->parent->oneofs;
}

bool upb_oneofdef_issynthetic(const upb_oneofdef *o) {
  return o->synthetic;
}

const upb_fielddef *upb_oneofdef_ntof(const upb_oneofdef *o,
                                      const char *name, size_t length) {
  upb_value val;
  return upb_strtable_lookup2(&o->ntof, name, length, &val) ?
      upb_value_getptr(val) : NULL;
}

const upb_fielddef *upb_oneofdef_itof(const upb_oneofdef *o, uint32_t num) {
  upb_value val;
  return upb_inttable_lookup(&o->itof, num, &val) ? upb_value_getptr(val)
                                                  : NULL;
}

void upb_oneof_begin(upb_oneof_iter *iter, const upb_oneofdef *o) {
  upb_inttable_begin(iter, &o->itof);
}

void upb_oneof_next(upb_oneof_iter *iter) {
  upb_inttable_next(iter);
}

bool upb_oneof_done(upb_oneof_iter *iter) {
  return upb_inttable_done(iter);
}

upb_fielddef *upb_oneof_iter_field(const upb_oneof_iter *iter) {
  return (upb_fielddef *)upb_value_getconstptr(upb_inttable_iter_value(iter));
}

void upb_oneof_iter_setdone(upb_oneof_iter *iter) {
  upb_inttable_iter_setdone(iter);
}

/* upb_filedef ****************************************************************/

const google_protobuf_FileOptions *upb_filedef_options(const upb_filedef *f) {
  return f->opts;
}

bool upb_filedef_hasoptions(const upb_filedef *f) {
  return f->opts != (void*)opt_default;
}

const char *upb_filedef_name(const upb_filedef *f) {
  return f->name;
}

const char *upb_filedef_package(const upb_filedef *f) {
  return f->package;
}

upb_syntax_t upb_filedef_syntax(const upb_filedef *f) {
  return f->syntax;
}

int upb_filedef_toplvlmsgcount(const upb_filedef *f) {
  return f->top_lvl_msg_count;
}

int upb_filedef_depcount(const upb_filedef *f) {
  return f->dep_count;
}

int upb_filedef_publicdepcount(const upb_filedef *f) {
  return f->public_dep_count;
}

int upb_filedef_weakdepcount(const upb_filedef *f) {
  return f->weak_dep_count;
}

const int32_t *_upb_filedef_publicdepnums(const upb_filedef *f) {
  return f->public_deps;
}

const int32_t *_upb_filedef_weakdepnums(const upb_filedef *f) {
  return f->weak_deps;
}

int upb_filedef_toplvlenumcount(const upb_filedef *f) {
  return f->top_lvl_enum_count;
}

int upb_filedef_toplvlextcount(const upb_filedef *f) {
  return f->top_lvl_ext_count;
}

int upb_filedef_servicecount(const upb_filedef *f) {
  return f->service_count;
}

const upb_filedef *upb_filedef_dep(const upb_filedef *f, int i) {
  UPB_ASSERT(0 <= i && i < f->dep_count);
  return f->deps[i];
}

const upb_filedef *upb_filedef_publicdep(const upb_filedef *f, int i) {
  UPB_ASSERT(0 <= i && i < f->public_dep_count);
  return f->deps[f->public_deps[i]];
}

const upb_filedef *upb_filedef_weakdep(const upb_filedef *f, int i) {
  UPB_ASSERT(0 <= i && i < f->public_dep_count);
  return f->deps[f->weak_deps[i]];
}

const upb_msgdef *upb_filedef_toplvlmsg(const upb_filedef *f, int i) {
  UPB_ASSERT(0 <= i && i < f->top_lvl_msg_count);
  return &f->top_lvl_msgs[i];
}

const upb_enumdef *upb_filedef_toplvlenum(const upb_filedef *f, int i) {
  UPB_ASSERT(0 <= i && i < f->top_lvl_enum_count);
  return &f->top_lvl_enums[i];
}

const upb_fielddef *upb_filedef_toplvlext(const upb_filedef *f, int i) {
  UPB_ASSERT(0 <= i && i < f->top_lvl_ext_count);
  return &f->top_lvl_exts[i];
}

const upb_servicedef *upb_filedef_service(const upb_filedef *f, int i) {
  UPB_ASSERT(0 <= i && i < f->service_count);
  return &f->services[i];
}

const upb_symtab *upb_filedef_symtab(const upb_filedef *f) {
  return f->symtab;
}

/* upb_methoddef **************************************************************/

const google_protobuf_MethodOptions *upb_methoddef_options(
    const upb_methoddef *m) {
  return m->opts;
}

bool upb_methoddef_hasoptions(const upb_methoddef *m) {
  return m->opts != (void*)opt_default;
}

const char *upb_methoddef_fullname(const upb_methoddef *m) {
  return m->full_name;
}

const char *upb_methoddef_name(const upb_methoddef *m) {
  return shortdefname(m->full_name);
}

const upb_servicedef *upb_methoddef_service(const upb_methoddef *m) {
  return m->service;
}

const upb_msgdef *upb_methoddef_inputtype(const upb_methoddef *m) {
  return m->input_type;
}

const upb_msgdef *upb_methoddef_outputtype(const upb_methoddef *m) {
  return m->output_type;
}

bool upb_methoddef_clientstreaming(const upb_methoddef *m) {
  return m->client_streaming;
}

bool upb_methoddef_serverstreaming(const upb_methoddef *m) {
  return m->server_streaming;
}

/* upb_servicedef *************************************************************/

const google_protobuf_ServiceOptions *upb_servicedef_options(
    const upb_servicedef *s) {
  return s->opts;
}

bool upb_servicedef_hasoptions(const upb_servicedef *s) {
  return s->opts != (void*)opt_default;
}

const char *upb_servicedef_fullname(const upb_servicedef *s) {
  return s->full_name;
}

const char *upb_servicedef_name(const upb_servicedef *s) {
  return shortdefname(s->full_name);
}

int upb_servicedef_index(const upb_servicedef *s) {
  return s->index;
}

const upb_filedef *upb_servicedef_file(const upb_servicedef *s) {
  return s->file;
}

int upb_servicedef_methodcount(const upb_servicedef *s) {
  return s->method_count;
}

const upb_methoddef *upb_servicedef_method(const upb_servicedef *s, int i) {
  return i < 0 || i >= s->method_count ? NULL : &s->methods[i];
}

const upb_methoddef *upb_servicedef_lookupmethod(const upb_servicedef *s,
                                                 const char *name) {
  for (int i = 0; i < s->method_count; i++) {
    if (strcmp(name, upb_methoddef_name(&s->methods[i])) == 0) {
      return &s->methods[i];
    }
  }
  return NULL;
}

/* upb_symtab *****************************************************************/

void upb_symtab_free(upb_symtab *s) {
  upb_arena_free(s->arena);
  upb_gfree(s);
}

upb_symtab *upb_symtab_new(void) {
  upb_symtab *s = upb_gmalloc(sizeof(*s));

  if (!s) {
    return NULL;
  }

  s->arena = upb_arena_new();
  s->bytes_loaded = 0;
  s->allow_name_conflicts = false;

  if (!upb_strtable_init(&s->syms, 32, s->arena) ||
      !upb_strtable_init(&s->files, 4, s->arena) ||
      !upb_inttable_init(&s->exts, s->arena)) {
    goto err;
  }

  s->extreg = upb_extreg_new(s->arena);
  if (!s->extreg) goto err;
  return s;

err:
  upb_arena_free(s->arena);
  upb_gfree(s);
  return NULL;
}

void _upb_symtab_allownameconflicts(upb_symtab *s) {
  s->allow_name_conflicts = true;
}

static const void *symtab_lookup(const upb_symtab *s, const char *sym,
                                 upb_deftype_t type) {
  upb_value v;
  return upb_strtable_lookup(&s->syms, sym, &v) ? unpack_def(v, type) : NULL;
}

static const void *symtab_lookup2(const upb_symtab *s, const char *sym,
                                  size_t size, upb_deftype_t type) {
  upb_value v;
  return upb_strtable_lookup2(&s->syms, sym, size, &v) ? unpack_def(v, type)
                                                       : NULL;
}

const upb_msgdef *upb_symtab_lookupmsg(const upb_symtab *s, const char *sym) {
  return symtab_lookup(s, sym, UPB_DEFTYPE_MSG);
}

const upb_msgdef *upb_symtab_lookupmsg2(const upb_symtab *s, const char *sym,
                                        size_t len) {
  return symtab_lookup2(s, sym, len, UPB_DEFTYPE_MSG);
}

const upb_enumdef *upb_symtab_lookupenum(const upb_symtab *s, const char *sym) {
  return symtab_lookup(s, sym, UPB_DEFTYPE_ENUM);
}

const upb_enumvaldef *upb_symtab_lookupenumval(const upb_symtab *s,
                                               const char *sym) {
  return symtab_lookup(s, sym, UPB_DEFTYPE_ENUMVAL);
}

const upb_filedef *upb_symtab_lookupfile(const upb_symtab *s,
                                         const char *name) {
  upb_value v;
  return upb_strtable_lookup(&s->files, name, &v)
             ? unpack_def(v, UPB_DEFTYPE_FILE)
             : NULL;
}

const upb_filedef *upb_symtab_lookupfile2(const upb_symtab *s, const char *name,
                                          size_t len) {
  upb_value v;
  return upb_strtable_lookup2(&s->files, name, len, &v)
             ? unpack_def(v, UPB_DEFTYPE_FILE)
             : NULL;
}

const upb_fielddef *upb_symtab_lookupext2(const upb_symtab *s, const char *name,
                                          size_t size) {
  upb_value v;
  if (!upb_strtable_lookup2(&s->syms, name, size, &v)) return NULL;

  switch (deftype(v)) {
    case UPB_DEFTYPE_FIELD:
      return unpack_def(v, UPB_DEFTYPE_FIELD);
    case UPB_DEFTYPE_MSG: {
      const upb_msgdef *m = unpack_def(v, UPB_DEFTYPE_MSG);
      return m->in_message_set ? &m->nested_exts[0] : NULL;
    }
    default:
      break;
  }

  return NULL;
}

const upb_fielddef *upb_symtab_lookupext(const upb_symtab *s, const char *sym) {
  return upb_symtab_lookupext2(s, sym, strlen(sym));
}

const upb_servicedef *upb_symtab_lookupservice(const upb_symtab *s,
                                               const char *name) {
  return symtab_lookup(s, name, UPB_DEFTYPE_SERVICE);
}

const upb_filedef *upb_symtab_lookupfileforsym(const upb_symtab *s,
                                               const char *name) {
  upb_value v;
  // TODO(haberman): non-extension fields and oneofs.
  if (upb_strtable_lookup(&s->syms, name, &v)) {
    switch (deftype(v)) {
      case UPB_DEFTYPE_EXT: {
        const upb_fielddef *f = unpack_def(v, UPB_DEFTYPE_EXT);
        return upb_fielddef_file(f);
      }
      case UPB_DEFTYPE_MSG: {
        const upb_msgdef *m = unpack_def(v, UPB_DEFTYPE_MSG);
        return upb_msgdef_file(m);
      }
      case UPB_DEFTYPE_ENUM: {
        const upb_enumdef *e = unpack_def(v, UPB_DEFTYPE_ENUM);
        return upb_enumdef_file(e);
      }
      case UPB_DEFTYPE_ENUMVAL: {
        const upb_enumvaldef *ev = unpack_def(v, UPB_DEFTYPE_ENUMVAL);
        return upb_enumdef_file(upb_enumvaldef_enum(ev));
      }
      case UPB_DEFTYPE_SERVICE: {
        const upb_servicedef *service = unpack_def(v, UPB_DEFTYPE_SERVICE);
        return upb_servicedef_file(service);
      }
      default:
        UPB_UNREACHABLE();
    }
  }

  const char *last_dot = strrchr(name, '.');
  if (last_dot) {
    const upb_msgdef *parent = upb_symtab_lookupmsg2(s, name, last_dot - name);
    if (parent) {
      const char *shortname = last_dot + 1;
      if (upb_msgdef_lookupname(parent, shortname, strlen(shortname), NULL,
                                NULL)) {
        return upb_msgdef_file(parent);
      }
    }
  }

  return NULL;
}

/* Code to build defs from descriptor protos. *********************************/

/* There is a question of how much validation to do here.  It will be difficult
 * to perfectly match the amount of validation performed by proto2.  But since
 * this code is used to directly build defs from Ruby (for example) we do need
 * to validate important constraints like uniqueness of names and numbers. */

#define CHK_OOM(x) if (!(x)) { symtab_oomerr(ctx); }

typedef struct {
  upb_symtab *symtab;
  upb_filedef *file;              /* File we are building. */
  upb_arena *arena;               /* Allocate defs here. */
  upb_arena *tmp_arena;                 /* For temporary allocations. */
  const upb_msglayout_file *layout;  /* NULL if we should build layouts. */
  int enum_count;                 /* Count of enums built so far. */
  int msg_count;                  /* Count of messages built so far. */
  int ext_count;                  /* Count of extensions built so far. */
  upb_status *status;             /* Record errors here. */
  jmp_buf err;                    /* longjmp() on error. */
} symtab_addctx;

UPB_NORETURN UPB_NOINLINE UPB_PRINTF(2, 3)
static void symtab_errf(symtab_addctx *ctx, const char *fmt, ...) {
  va_list argp;
  va_start(argp, fmt);
  upb_status_vseterrf(ctx->status, fmt, argp);
  va_end(argp);
  UPB_LONGJMP(ctx->err, 1);
}

UPB_NORETURN UPB_NOINLINE
static void symtab_oomerr(symtab_addctx *ctx) {
  upb_status_setoom(ctx->status);
  UPB_LONGJMP(ctx->err, 1);
}

void *symtab_alloc(symtab_addctx *ctx, size_t bytes) {
  if (bytes == 0) return NULL;
  void *ret = upb_arena_malloc(ctx->arena, bytes);
  if (!ret) symtab_oomerr(ctx);
  return ret;
}

// We want to copy the options verbatim into the destination options proto.
// We use serialize+parse as our deep copy.
#define SET_OPTIONS(target, desc_type, options_type, proto)                   \
  if (google_protobuf_##desc_type##_has_options(proto)) {                     \
    size_t size;                                                              \
    char *pb = google_protobuf_##options_type##_serialize(                    \
        google_protobuf_##desc_type##_options(proto), ctx->tmp_arena, &size); \
    CHK_OOM(pb);                                                              \
    target = google_protobuf_##options_type##_parse(pb, size, ctx->arena);    \
    CHK_OOM(target);                                                          \
  } else {                                                                    \
    target = (const google_protobuf_##options_type *)opt_default;             \
  }

static void check_ident(symtab_addctx *ctx, upb_strview name, bool full) {
  const char *str = name.data;
  size_t len = name.size;
  bool start = true;
  size_t i;
  for (i = 0; i < len; i++) {
    char c = str[i];
    if (c == '.') {
      if (start || !full) {
        symtab_errf(ctx, "invalid name: unexpected '.' (%.*s)", (int)len, str);
      }
      start = true;
    } else if (start) {
      if (!upb_isletter(c)) {
        symtab_errf(
            ctx,
            "invalid name: path components must start with a letter (%.*s)",
            (int)len, str);
      }
      start = false;
    } else {
      if (!upb_isalphanum(c)) {
        symtab_errf(ctx, "invalid name: non-alphanumeric character (%.*s)",
                    (int)len, str);
      }
    }
  }
  if (start) {
    symtab_errf(ctx, "invalid name: empty part (%.*s)", (int)len, str);
  }
}

static size_t div_round_up(size_t n, size_t d) {
  return (n + d - 1) / d;
}

static size_t upb_msgval_sizeof(upb_fieldtype_t type) {
  switch (type) {
    case UPB_TYPE_DOUBLE:
    case UPB_TYPE_INT64:
    case UPB_TYPE_UINT64:
      return 8;
    case UPB_TYPE_ENUM:
    case UPB_TYPE_INT32:
    case UPB_TYPE_UINT32:
    case UPB_TYPE_FLOAT:
      return 4;
    case UPB_TYPE_BOOL:
      return 1;
    case UPB_TYPE_MESSAGE:
      return sizeof(void*);
    case UPB_TYPE_BYTES:
    case UPB_TYPE_STRING:
      return sizeof(upb_strview);
  }
  UPB_UNREACHABLE();
}

static uint8_t upb_msg_fielddefsize(const upb_fielddef *f) {
  if (upb_msgdef_mapentry(upb_fielddef_containingtype(f))) {
    upb_map_entry ent;
    UPB_ASSERT(sizeof(ent.k) == sizeof(ent.v));
    return sizeof(ent.k);
  } else if (upb_fielddef_isseq(f)) {
    return sizeof(void*);
  } else {
    return upb_msgval_sizeof(upb_fielddef_type(f));
  }
}

static uint32_t upb_msglayout_place(symtab_addctx *ctx, upb_msglayout *l,
                                    size_t size, const upb_msgdef *m) {
  size_t ofs = UPB_ALIGN_UP(l->size, size);
  size_t next = ofs + size;

  if (next > UINT16_MAX) {
    symtab_errf(ctx, "size of message %s exceeded max size of %zu bytes",
                upb_msgdef_fullname(m), (size_t)UINT16_MAX);
  }

  l->size = next;
  return ofs;
}

static int field_number_cmp(const void *p1, const void *p2) {
  const upb_msglayout_field *f1 = p1;
  const upb_msglayout_field *f2 = p2;
  return f1->number - f2->number;
}

static void assign_layout_indices(const upb_msgdef *m, upb_msglayout *l,
                                  upb_msglayout_field *fields) {
  int i;
  int n = upb_msgdef_numfields(m);
  int dense_below = 0;
  for (i = 0; i < n; i++) {
    upb_fielddef *f = (upb_fielddef*)upb_msgdef_itof(m, fields[i].number);
    UPB_ASSERT(f);
    f->layout_index = i;
    if (i < UINT8_MAX && fields[i].number == i + 1 &&
        (i == 0 || fields[i-1].number == i)) {
      dense_below = i + 1;
    }
  }
  l->dense_below = dense_below;
}

static uint8_t map_descriptortype(const upb_fielddef *f) {
  uint8_t type = upb_fielddef_descriptortype(f);
  /* See TableDescriptorType() in upbc/generator.cc for details and
   * rationale of these exceptions. */
  if (type == UPB_DTYPE_STRING && f->file->syntax == UPB_SYNTAX_PROTO2) {
    return UPB_DTYPE_BYTES;
  } else if (type == UPB_DTYPE_ENUM &&
             f->sub.enumdef->file->syntax == UPB_SYNTAX_PROTO3) {
    return UPB_DTYPE_INT32;
  }
  return type;
}

static void fill_fieldlayout(upb_msglayout_field *field, const upb_fielddef *f) {
  field->number = upb_fielddef_number(f);
  field->descriptortype = map_descriptortype(f);

  if (upb_fielddef_ismap(f)) {
    field->mode = _UPB_MODE_MAP | (_UPB_REP_PTR << _UPB_REP_SHIFT);
  } else if (upb_fielddef_isseq(f)) {
    field->mode = _UPB_MODE_ARRAY | (_UPB_REP_PTR << _UPB_REP_SHIFT);
  } else {
    /* Maps descriptor type -> elem_size_lg2.  */
    static const uint8_t sizes[] = {
        -1,               /* invalid descriptor type */
        _UPB_REP_8BYTE,  /* DOUBLE */
        _UPB_REP_4BYTE,   /* FLOAT */
        _UPB_REP_8BYTE,   /* INT64 */
        _UPB_REP_8BYTE,  /* UINT64 */
        _UPB_REP_4BYTE,   /* INT32 */
        _UPB_REP_8BYTE,  /* FIXED64 */
        _UPB_REP_4BYTE,  /* FIXED32 */
        _UPB_REP_1BYTE,    /* BOOL */
        _UPB_REP_STRVIEW,  /* STRING */
        _UPB_REP_PTR,  /* GROUP */
        _UPB_REP_PTR,  /* MESSAGE */
        _UPB_REP_STRVIEW,  /* BYTES */
        _UPB_REP_4BYTE,  /* UINT32 */
        _UPB_REP_4BYTE,    /* ENUM */
        _UPB_REP_4BYTE,   /* SFIXED32 */
        _UPB_REP_8BYTE,   /* SFIXED64 */
        _UPB_REP_4BYTE,   /* SINT32 */
        _UPB_REP_8BYTE,   /* SINT64 */
    };
    field->mode =
        _UPB_MODE_SCALAR | (sizes[field->descriptortype] << _UPB_REP_SHIFT);
  }

  if (upb_fielddef_packed(f)) {
    field->mode |= _UPB_MODE_IS_PACKED;
  }

  if (upb_fielddef_isextension(f)) {
    field->mode |= _UPB_MODE_IS_EXTENSION;
  }
}

/* This function is the dynamic equivalent of message_layout.{cc,h} in upbc.
 * It computes a dynamic layout for all of the fields in |m|. */
static void make_layout(symtab_addctx *ctx, const upb_msgdef *m) {
  upb_msglayout *l = (upb_msglayout*)m->layout;
  size_t field_count = upb_msgdef_numfields(m);
  size_t sublayout_count = 0;
  upb_msglayout_sub *subs;
  upb_msglayout_field *fields;

  memset(l, 0, sizeof(*l) + sizeof(_upb_fasttable_entry));

  /* Count sub-messages. */
  for (size_t i = 0; i < field_count; i++) {
    const upb_fielddef *f = &m->fields[i];
    if (upb_fielddef_issubmsg(f)) {
      sublayout_count++;
    }
    if (upb_fielddef_type(f) == UPB_TYPE_ENUM &&
        f->sub.enumdef->file->syntax == UPB_SYNTAX_PROTO2) {
      sublayout_count++;
    }
  }

  fields = symtab_alloc(ctx, field_count * sizeof(*fields));
  subs = symtab_alloc(ctx, sublayout_count * sizeof(*subs));

  l->field_count = upb_msgdef_numfields(m);
  l->fields = fields;
  l->subs = subs;
  l->table_mask = 0;
  l->required_count = 0;

  if (upb_msgdef_extrangecount(m) > 0) {
    if (google_protobuf_MessageOptions_message_set_wire_format(m->opts)) {
      l->ext = _UPB_MSGEXT_MSGSET;
    } else {
      l->ext = _UPB_MSGEXT_EXTENDABLE;
    }
  } else {
    l->ext = _UPB_MSGEXT_NONE;
  }

  /* TODO(haberman): initialize fast tables so that reflection-based parsing
   * can get the same speeds as linked-in types. */
  l->fasttable[0].field_parser = &fastdecode_generic;
  l->fasttable[0].field_data = 0;

  if (upb_msgdef_mapentry(m)) {
    /* TODO(haberman): refactor this method so this special case is more
     * elegant. */
    const upb_fielddef *key = upb_msgdef_itof(m, 1);
    const upb_fielddef *val = upb_msgdef_itof(m, 2);
    fields[0].number = 1;
    fields[1].number = 2;
    fields[0].mode = _UPB_MODE_SCALAR;
    fields[1].mode = _UPB_MODE_SCALAR;
    fields[0].presence = 0;
    fields[1].presence = 0;
    fields[0].descriptortype = map_descriptortype(key);
    fields[1].descriptortype = map_descriptortype(val);
    fields[0].offset = 0;
    fields[1].offset = sizeof(upb_strview);
    fields[1].submsg_index = 0;

    if (upb_fielddef_type(val) == UPB_TYPE_MESSAGE) {
      subs[0].submsg = upb_fielddef_msgsubdef(val)->layout;
    }

    upb_fielddef *fielddefs = (upb_fielddef*)&m->fields[0];
    UPB_ASSERT(fielddefs[0].number_ == 1);
    UPB_ASSERT(fielddefs[1].number_ == 2);
    fielddefs[0].layout_index = 0;
    fielddefs[1].layout_index = 1;

    l->field_count = 2;
    l->size = 2 * sizeof(upb_strview);
    l->size = UPB_ALIGN_UP(l->size, 8);
    l->dense_below = 2;
    return;
  }

  /* Allocate data offsets in three stages:
   *
   * 1. hasbits.
   * 2. regular fields.
   * 3. oneof fields.
   *
   * OPT: There is a lot of room for optimization here to minimize the size.
   */

  /* Assign hasbits for required fields first. */
  size_t hasbit = 0;

  for (int i = 0; i < m->field_count; i++) {
    const upb_fielddef* f = &m->fields[i];
    upb_msglayout_field *field = &fields[upb_fielddef_index(f)];
    if (upb_fielddef_label(f) == UPB_LABEL_REQUIRED) {
      field->presence = ++hasbit;
      if (hasbit >= 63) {
        symtab_errf(ctx, "Message with >=63 required fields: %s",
                    upb_msgdef_fullname(m));
      }
      l->required_count++;
    }
  }

  /* Allocate hasbits and set basic field attributes. */
  sublayout_count = 0;
  for (int i = 0; i < m->field_count; i++) {
    const upb_fielddef* f = &m->fields[i];
    upb_msglayout_field *field = &fields[upb_fielddef_index(f)];

    fill_fieldlayout(field, f);

    if (upb_fielddef_issubmsg(f)) {
      field->submsg_index = sublayout_count++;
      subs[field->submsg_index].submsg = upb_fielddef_msgsubdef(f)->layout;
    } else if (upb_fielddef_type(f) == UPB_TYPE_ENUM &&
               f->file->syntax == UPB_SYNTAX_PROTO2) {
      field->submsg_index = sublayout_count++;
      subs[field->submsg_index].subenum = upb_fielddef_enumsubdef(f)->layout;
      UPB_ASSERT(subs[field->submsg_index].subenum);
    }

    if (upb_fielddef_label(f) == UPB_LABEL_REQUIRED) {
      /* Hasbit was already assigned. */
    } else if (upb_fielddef_haspresence(f) && !upb_fielddef_realcontainingoneof(f)) {
      /* We don't use hasbit 0, so that 0 can indicate "no presence" in the
       * table. This wastes one hasbit, but we don't worry about it for now. */
      field->presence = ++hasbit;
    } else {
      field->presence = 0;
    }
  }

  /* Account for space used by hasbits. */
  l->size = div_round_up(hasbit, 8);

  /* Allocate non-oneof fields. */
  for (int i = 0; i < m->field_count; i++) {
    const upb_fielddef* f = &m->fields[i];
    size_t field_size = upb_msg_fielddefsize(f);
    size_t index = upb_fielddef_index(f);

    if (upb_fielddef_realcontainingoneof(f)) {
      /* Oneofs are handled separately below. */
      continue;
    }

    fields[index].offset = upb_msglayout_place(ctx, l, field_size, m);
  }

  /* Allocate oneof fields.  Each oneof field consists of a uint32 for the case
   * and space for the actual data. */
  for (int i = 0; i < m->oneof_count; i++) {
    const upb_oneofdef* o = &m->oneofs[i];
    upb_oneof_iter fit;

    size_t case_size = sizeof(uint32_t);  /* Could potentially optimize this. */
    size_t field_size = 0;
    uint32_t case_offset;
    uint32_t data_offset;

    if (upb_oneofdef_issynthetic(o)) continue;

    /* Calculate field size: the max of all field sizes. */
    for (upb_oneof_begin(&fit, o);
         !upb_oneof_done(&fit);
         upb_oneof_next(&fit)) {
      const upb_fielddef* f = upb_oneof_iter_field(&fit);
      field_size = UPB_MAX(field_size, upb_msg_fielddefsize(f));
    }

    /* Align and allocate case offset. */
    case_offset = upb_msglayout_place(ctx, l, case_size, m);
    data_offset = upb_msglayout_place(ctx, l, field_size, m);

    for (int i = 0; i < o->field_count; i++) {
      const upb_fielddef* f = o->fields[i];
      fields[upb_fielddef_index(f)].offset = data_offset;
      fields[upb_fielddef_index(f)].presence = ~case_offset;
    }
  }

  /* Size of the entire structure should be a multiple of its greatest
   * alignment.  TODO: track overall alignment for real? */
  l->size = UPB_ALIGN_UP(l->size, 8);

  /* Sort fields by number. */
  qsort(fields, upb_msgdef_numfields(m), sizeof(*fields), field_number_cmp);
  assign_layout_indices(m, l, fields);
}

static char *strviewdup(symtab_addctx *ctx, upb_strview view) {
  char *ret = upb_strdup2(view.data, view.size, ctx->arena);
  CHK_OOM(ret);
  return ret;
}

static bool streql2(const char *a, size_t n, const char *b) {
  return n == strlen(b) && memcmp(a, b, n) == 0;
}

static bool streql_view(upb_strview view, const char *b) {
  return streql2(view.data, view.size, b);
}

static const char *makefullname(symtab_addctx *ctx, const char *prefix,
                                upb_strview name) {
  if (prefix) {
    /* ret = prefix + '.' + name; */
    size_t n = strlen(prefix);
    char *ret = symtab_alloc(ctx, n + name.size + 2);
    strcpy(ret, prefix);
    ret[n] = '.';
    memcpy(&ret[n + 1], name.data, name.size);
    ret[n + 1 + name.size] = '\0';
    return ret;
  } else {
    return strviewdup(ctx, name);
  }
}

static void finalize_oneofs(symtab_addctx *ctx, upb_msgdef *m) {
  int i;
  int synthetic_count = 0;
  upb_oneofdef *mutable_oneofs = (upb_oneofdef*)m->oneofs;

  for (i = 0; i < m->oneof_count; i++) {
    upb_oneofdef *o = &mutable_oneofs[i];

    if (o->synthetic && o->field_count != 1) {
      symtab_errf(ctx, "Synthetic oneofs must have one field, not %d: %s",
                  o->field_count, upb_oneofdef_name(o));
    }

    if (o->synthetic) {
      synthetic_count++;
    } else if (synthetic_count != 0) {
      symtab_errf(ctx, "Synthetic oneofs must be after all other oneofs: %s",
                  upb_oneofdef_name(o));
    }

    o->fields = symtab_alloc(ctx, sizeof(upb_fielddef *) * o->field_count);
    o->field_count = 0;
  }

  for (i = 0; i < m->field_count; i++) {
    const upb_fielddef *f = &m->fields[i];
    upb_oneofdef *o = (upb_oneofdef*)f->oneof;
    if (o) {
      o->fields[o->field_count++] = f;
    }
  }

  m->real_oneof_count = m->oneof_count - synthetic_count;
}

size_t getjsonname(const char *name, char *buf, size_t len) {
  size_t src, dst = 0;
  bool ucase_next = false;

#define WRITE(byte) \
  ++dst; \
  if (dst < len) buf[dst - 1] = byte; \
  else if (dst == len) buf[dst - 1] = '\0'

  if (!name) {
    WRITE('\0');
    return 0;
  }

  /* Implement the transformation as described in the spec:
   *   1. upper case all letters after an underscore.
   *   2. remove all underscores.
   */
  for (src = 0; name[src]; src++) {
    if (name[src] == '_') {
      ucase_next = true;
      continue;
    }

    if (ucase_next) {
      WRITE(toupper(name[src]));
      ucase_next = false;
    } else {
      WRITE(name[src]);
    }
  }

  WRITE('\0');
  return dst;

#undef WRITE
}

static char* makejsonname(symtab_addctx *ctx, const char* name) {
  size_t size = getjsonname(name, NULL, 0);
  char* json_name = symtab_alloc(ctx, size);
  getjsonname(name, json_name, size);
  return json_name;
}

/* Adds a symbol |v| to the symtab, which must be a def pointer previously packed
 * with pack_def().  The def's pointer to upb_filedef* must be set before adding,
 * so we know which entries to remove if building this file fails. */
static void symtab_add(symtab_addctx *ctx, const char *name, upb_value v) {
  // TODO: table should support an operation "tryinsert" to avoid the double
  // lookup.
  if (upb_strtable_lookup(&ctx->symtab->syms, name, NULL)) {
    symtab_errf(ctx, "duplicate symbol '%s'", name);
  }
  size_t len = strlen(name);
  CHK_OOM(upb_strtable_insert(&ctx->symtab->syms, name, len, v,
                              ctx->symtab->arena));
}

static bool remove_component(char *base, size_t *len) {
  if (*len == 0) return false;

  for (size_t i = *len - 1; i > 0; i--) {
    if (base[i] == '.') {
      *len = i;
      return true;
    }
  }

  *len = 0;
  return true;
}

/* Given a symbol and the base symbol inside which it is defined, find the
 * symbol's definition in t. */
static const void *symtab_resolveany(symtab_addctx *ctx,
                                     const char *from_name_dbg,
                                     const char *base, upb_strview sym,
                                     upb_deftype_t *type) {
  const upb_strtable *t = &ctx->symtab->syms;
  if(sym.size == 0) goto notfound;
  upb_value v;
  if(sym.data[0] == '.') {
    /* Symbols starting with '.' are absolute, so we do a single lookup.
     * Slice to omit the leading '.' */
    if (!upb_strtable_lookup2(t, sym.data + 1, sym.size - 1, &v)) {
      goto notfound;
    }
  } else {
    /* Remove components from base until we find an entry or run out. */
    size_t baselen = strlen(base);
    char *tmp = malloc(sym.size + strlen(base) + 1);
    while (1) {
      char *p = tmp;
      if (baselen) {
        memcpy(p, base, baselen);
        p[baselen] = '.';
        p += baselen + 1;
      }
      memcpy(p, sym.data, sym.size);
      p += sym.size;
      if (upb_strtable_lookup2(t, tmp, p - tmp, &v)) {
        break;
      }
      if (!remove_component(tmp, &baselen)) {
        free(tmp);
        goto notfound;
      }
    }
    free(tmp);
  }

  *type = deftype(v);
  return unpack_def(v, *type);

notfound:
  symtab_errf(ctx, "couldn't resolve name '" UPB_STRVIEW_FORMAT "'",
              UPB_STRVIEW_ARGS(sym));
}

static const void *symtab_resolve(symtab_addctx *ctx, const char *from_name_dbg,
                                  const char *base, upb_strview sym,
                                  upb_deftype_t type) {
  upb_deftype_t found_type;
  const void *ret =
      symtab_resolveany(ctx, from_name_dbg, base, sym, &found_type);
  if (ret && found_type != type) {
    symtab_errf(
        ctx,
        "type mismatch when resolving %s: couldn't find name %s with type=%d",
        from_name_dbg, sym.data, (int)type);
  }
  return ret;
}

static void create_oneofdef(
    symtab_addctx *ctx, upb_msgdef *m,
    const google_protobuf_OneofDescriptorProto *oneof_proto,
    const upb_oneofdef *_o) {
  upb_oneofdef *o = (upb_oneofdef *)_o;
  upb_strview name = google_protobuf_OneofDescriptorProto_name(oneof_proto);
  upb_value v;

  o->parent = m;
  o->full_name = makefullname(ctx, m->full_name, name);
  o->field_count = 0;
  o->synthetic = false;

  SET_OPTIONS(o->opts, OneofDescriptorProto, OneofOptions, oneof_proto);

  v = pack_def(o, UPB_DEFTYPE_ONEOF);
  CHK_OOM(upb_strtable_insert(&m->ntof, name.data, name.size, v, ctx->arena));

  CHK_OOM(upb_inttable_init(&o->itof, ctx->arena));
  CHK_OOM(upb_strtable_init(&o->ntof, 4, ctx->arena));
}

static str_t *newstr(symtab_addctx *ctx, const char *data, size_t len) {
  str_t *ret = symtab_alloc(ctx, sizeof(*ret) + len);
  if (!ret) return NULL;
  ret->len = len;
  if (len) memcpy(ret->str, data, len);
  ret->str[len] = '\0';
  return ret;
}

static void parse_default(symtab_addctx *ctx, const char *str, size_t len,
                          upb_fielddef *f) {
  char *end;
  char nullz[64];
  errno = 0;

  switch (upb_fielddef_type(f)) {
    case UPB_TYPE_INT32:
    case UPB_TYPE_INT64:
    case UPB_TYPE_UINT32:
    case UPB_TYPE_UINT64:
    case UPB_TYPE_DOUBLE:
    case UPB_TYPE_FLOAT:
      /* Standard C number parsing functions expect null-terminated strings. */
      if (len >= sizeof(nullz) - 1) {
        symtab_errf(ctx, "Default too long: %.*s", (int)len, str);
      }
      memcpy(nullz, str, len);
      nullz[len] = '\0';
      str = nullz;
      break;
    default:
      break;
  }

  switch (upb_fielddef_type(f)) {
    case UPB_TYPE_INT32: {
      long val = strtol(str, &end, 0);
      if (val > INT32_MAX || val < INT32_MIN || errno == ERANGE || *end) {
        goto invalid;
      }
      f->defaultval.sint = val;
      break;
    }
    case UPB_TYPE_ENUM: {
      const upb_enumdef *e = f->sub.enumdef;
      const upb_enumvaldef *ev = upb_enumdef_lookupname(e, str, len);
      if (!ev) {
        goto invalid;
      }
      f->defaultval.sint = ev->number;
      break;
    }
    case UPB_TYPE_INT64: {
      long long val = strtoll(str, &end, 0);
      if (val > INT64_MAX || val < INT64_MIN || errno == ERANGE || *end) {
        goto invalid;
      }
      f->defaultval.sint = val;
      break;
    }
    case UPB_TYPE_UINT32: {
      unsigned long val = strtoul(str, &end, 0);
      if (val > UINT32_MAX || errno == ERANGE || *end) {
        goto invalid;
      }
      f->defaultval.uint = val;
      break;
    }
    case UPB_TYPE_UINT64: {
      unsigned long long val = strtoull(str, &end, 0);
      if (val > UINT64_MAX || errno == ERANGE || *end) {
        goto invalid;
      }
      f->defaultval.uint = val;
      break;
    }
    case UPB_TYPE_DOUBLE: {
      double val = strtod(str, &end);
      if (errno == ERANGE || *end) {
        goto invalid;
      }
      f->defaultval.dbl = val;
      break;
    }
    case UPB_TYPE_FLOAT: {
      float val = strtof(str, &end);
      if (errno == ERANGE || *end) {
        goto invalid;
      }
      f->defaultval.flt = val;
      break;
    }
    case UPB_TYPE_BOOL: {
      if (streql2(str, len, "false")) {
        f->defaultval.boolean = false;
      } else if (streql2(str, len, "true")) {
        f->defaultval.boolean = true;
      } else {
        goto invalid;
      }
      break;
    }
    case UPB_TYPE_STRING:
      f->defaultval.str = newstr(ctx, str, len);
      break;
    case UPB_TYPE_BYTES:
      /* XXX: need to interpret the C-escaped value. */
      f->defaultval.str = newstr(ctx, str, len);
      break;
    case UPB_TYPE_MESSAGE:
      /* Should not have a default value. */
      symtab_errf(ctx, "Message should not have a default (%s)",
                  upb_fielddef_fullname(f));
  }

  return;

invalid:
  symtab_errf(ctx, "Invalid default '%.*s' for field %s of type %d", (int)len, str,
              upb_fielddef_fullname(f), (int)upb_fielddef_descriptortype(f));
}

static void set_default_default(symtab_addctx *ctx, upb_fielddef *f) {
  switch (upb_fielddef_type(f)) {
    case UPB_TYPE_INT32:
    case UPB_TYPE_INT64:
      f->defaultval.sint = 0;
      break;
    case UPB_TYPE_UINT64:
    case UPB_TYPE_UINT32:
      f->defaultval.uint = 0;
      break;
    case UPB_TYPE_DOUBLE:
    case UPB_TYPE_FLOAT:
      f->defaultval.dbl = 0;
      break;
    case UPB_TYPE_STRING:
    case UPB_TYPE_BYTES:
      f->defaultval.str = newstr(ctx, NULL, 0);
      break;
    case UPB_TYPE_BOOL:
      f->defaultval.boolean = false;
      break;
    case UPB_TYPE_ENUM:
      f->defaultval.sint = f->sub.enumdef->values[0].number;
    case UPB_TYPE_MESSAGE:
      break;
  }
}

static void create_fielddef(
    symtab_addctx *ctx, const char *prefix, upb_msgdef *m,
    const google_protobuf_FieldDescriptorProto *field_proto,
    const upb_fielddef *_f) {
  upb_fielddef *f = (upb_fielddef*)_f;
  upb_strview name;
  const char *full_name;
  const char *json_name;
  const char *shortname;
  int32_t field_number;

  f->file = ctx->file;  /* Must happen prior to symtab_add(). */

  if (!google_protobuf_FieldDescriptorProto_has_name(field_proto)) {
    symtab_errf(ctx, "field has no name (%s)", upb_msgdef_fullname(m));
  }

  name = google_protobuf_FieldDescriptorProto_name(field_proto);
  check_ident(ctx, name, false);
  full_name = makefullname(ctx, prefix, name);
  shortname = shortdefname(full_name);

  if (google_protobuf_FieldDescriptorProto_has_json_name(field_proto)) {
    json_name = strviewdup(
        ctx, google_protobuf_FieldDescriptorProto_json_name(field_proto));
    f->has_json_name_ = true;
  } else {
    json_name = makejsonname(ctx, shortname);
    f->has_json_name_ = false;
  }

  field_number = google_protobuf_FieldDescriptorProto_number(field_proto);

  f->full_name = full_name;
  f->json_name = json_name;
  f->label_ = (int)google_protobuf_FieldDescriptorProto_label(field_proto);
  f->number_ = field_number;
  f->oneof = NULL;
  f->proto3_optional_ =
      google_protobuf_FieldDescriptorProto_proto3_optional(field_proto);

  bool has_type = google_protobuf_FieldDescriptorProto_has_type(field_proto);
  bool has_type_name =
      google_protobuf_FieldDescriptorProto_has_type_name(field_proto);

  f->type_ = (int)google_protobuf_FieldDescriptorProto_type(field_proto);

  if (has_type) {
    switch (f->type_) {
      case UPB_DTYPE_MESSAGE:
      case UPB_DTYPE_GROUP:
      case UPB_DTYPE_ENUM:
        if (!has_type_name) {
          symtab_errf(ctx, "field of type %d requires type name (%s)",
                      (int)f->type_, full_name);
        }
        break;
      default:
        if (has_type_name) {
          symtab_errf(ctx, "invalid type for field with type_name set (%s, %d)",
                      full_name, (int)f->type_);
        }
    }
  } else if (has_type_name) {
    f->type_ = FIELD_TYPE_UNSPECIFIED;  // We'll fill this in in resolve_fielddef().
  }

  if (m) {
    /* direct message field. */
    upb_value v, field_v, json_v, existing_v;
    size_t json_size;

    if (field_number <= 0 || field_number > UPB_MAX_FIELDNUMBER) {
      symtab_errf(ctx, "invalid field number (%u)", field_number);
    }

    f->index_ = f - m->fields;
    f->msgdef = m;
    f->is_extension_ = false;

    field_v = pack_def(f, UPB_DEFTYPE_FIELD);
    json_v = pack_def(f, UPB_DEFTYPE_FIELD_JSONNAME);
    v = upb_value_constptr(f);
    json_size = strlen(json_name);

    if (upb_strtable_lookup(&m->ntof, shortname, &existing_v)) {
      if (ctx->symtab->allow_name_conflicts &&
          deftype(existing_v) == UPB_DEFTYPE_FIELD_JSONNAME) {
        // Field name takes precedence over json name.
        upb_strtable_remove(&m->ntof, shortname, NULL);
      } else {
        symtab_errf(ctx, "duplicate field name (%s)", shortname);
      }
    }

    CHK_OOM(upb_strtable_insert(&m->ntof, name.data, name.size, field_v,
                                ctx->arena));

    if (strcmp(shortname, json_name) != 0) {
      if (upb_strtable_lookup(&m->ntof, json_name, &v)) {
        if (!ctx->symtab->allow_name_conflicts) {
          symtab_errf(ctx, "duplicate json_name (%s)", json_name);
        }
      } else {
        CHK_OOM(upb_strtable_insert(&m->ntof, json_name, json_size, json_v,
                                    ctx->arena));
      }
    }

    if (upb_inttable_lookup(&m->itof, field_number, NULL)) {
      symtab_errf(ctx, "duplicate field number (%u)", field_number);
    }

    CHK_OOM(upb_inttable_insert(&m->itof, field_number, v, ctx->arena));

    if (ctx->layout) {
      const upb_msglayout_field *fields = m->layout->fields;
      int count = m->layout->field_count;
      bool found = false;
      for (int i = 0; i < count; i++) {
        if (fields[i].number == field_number) {
          f->layout_index = i;
          found = true;
          break;
        }
      }
      UPB_ASSERT(found);
    }
  } else {
    /* extension field. */
    f->is_extension_ = true;
    symtab_add(ctx, full_name, pack_def(f, UPB_DEFTYPE_EXT));
    f->layout_index = ctx->ext_count++;
    if (ctx->layout) {
      UPB_ASSERT(ctx->file->ext_layouts[f->layout_index]->field.number ==
                 field_number);
    }
  }

  if (f->type_ < UPB_DTYPE_DOUBLE || f->type_ > UPB_DTYPE_SINT64) {
    symtab_errf(ctx, "invalid type for field %s (%d)", f->full_name, f->type_);
  }

  if (f->label_ < UPB_LABEL_OPTIONAL || f->label_ > UPB_LABEL_REPEATED) {
    symtab_errf(ctx, "invalid label for field %s (%d)", f->full_name,
                f->label_);
  }

  /* We can't resolve the subdef or (in the case of extensions) the containing
   * message yet, because it may not have been defined yet.  We stash a pointer
   * to the field_proto until later when we can properly resolve it. */
  f->sub.unresolved = field_proto;

  if (f->label_ == UPB_LABEL_REQUIRED && f->file->syntax == UPB_SYNTAX_PROTO3) {
    symtab_errf(ctx, "proto3 fields cannot be required (%s)", f->full_name);
  }

  if (google_protobuf_FieldDescriptorProto_has_oneof_index(field_proto)) {
    int oneof_index =
        google_protobuf_FieldDescriptorProto_oneof_index(field_proto);
    upb_oneofdef *oneof;
    upb_value v = upb_value_constptr(f);

    if (upb_fielddef_label(f) != UPB_LABEL_OPTIONAL) {
      symtab_errf(ctx, "fields in oneof must have OPTIONAL label (%s)",
                  f->full_name);
    }

    if (!m) {
      symtab_errf(ctx, "oneof_index provided for extension field (%s)",
                  f->full_name);
    }

    if (oneof_index >= m->oneof_count) {
      symtab_errf(ctx, "oneof_index out of range (%s)", f->full_name);
    }

    oneof = (upb_oneofdef *)&m->oneofs[oneof_index];
    f->oneof = oneof;

    oneof->field_count++;
    if (f->proto3_optional_) {
      oneof->synthetic = true;
    }
    CHK_OOM(upb_inttable_insert(&oneof->itof, f->number_, v, ctx->arena));
    CHK_OOM(
        upb_strtable_insert(&oneof->ntof, name.data, name.size, v, ctx->arena));
  } else {
    f->oneof = NULL;
    if (f->proto3_optional_) {
      symtab_errf(ctx, "field with proto3_optional was not in a oneof (%s)",
                  f->full_name);
    }
  }

  SET_OPTIONS(f->opts, FieldDescriptorProto, FieldOptions, field_proto);

  if (google_protobuf_FieldOptions_has_packed(f->opts)) {
    f->packed_ = google_protobuf_FieldOptions_packed(f->opts);
  } else {
    /* Repeated fields default to packed for proto3 only. */
    f->packed_ = upb_fielddef_isprimitive(f) &&
        f->label_ == UPB_LABEL_REPEATED && f->file->syntax == UPB_SYNTAX_PROTO3;
  }
}

static void create_service(
    symtab_addctx *ctx, const google_protobuf_ServiceDescriptorProto *svc_proto,
    const upb_servicedef *_s) {
  upb_servicedef *s = (upb_servicedef*)_s;
  upb_strview name;
  const google_protobuf_MethodDescriptorProto *const *methods;
  size_t i, n;

  s->file = ctx->file;  /* Must happen prior to symtab_add. */

  name = google_protobuf_ServiceDescriptorProto_name(svc_proto);
  check_ident(ctx, name, false);
  s->full_name = makefullname(ctx, ctx->file->package, name);
  symtab_add(ctx, s->full_name, pack_def(s, UPB_DEFTYPE_SERVICE));

  methods = google_protobuf_ServiceDescriptorProto_method(svc_proto, &n);

  s->method_count = n;
  s->methods = symtab_alloc(ctx, sizeof(*s->methods) * n);

  SET_OPTIONS(s->opts, ServiceDescriptorProto, ServiceOptions, svc_proto);

  for (i = 0; i < n; i++) {
    const google_protobuf_MethodDescriptorProto *method_proto = methods[i];
    upb_methoddef *m = (upb_methoddef*)&s->methods[i];
    upb_strview name = google_protobuf_MethodDescriptorProto_name(method_proto);

    m->service = s;
    m->full_name = makefullname(ctx, s->full_name, name);
    m->client_streaming =
        google_protobuf_MethodDescriptorProto_client_streaming(method_proto);
    m->server_streaming =
        google_protobuf_MethodDescriptorProto_server_streaming(method_proto);
    m->input_type = symtab_resolve(
        ctx, m->full_name, m->full_name,
        google_protobuf_MethodDescriptorProto_input_type(method_proto),
        UPB_DEFTYPE_MSG);
    m->output_type = symtab_resolve(
        ctx, m->full_name, m->full_name,
        google_protobuf_MethodDescriptorProto_output_type(method_proto),
        UPB_DEFTYPE_MSG);

    SET_OPTIONS(m->opts, MethodDescriptorProto, MethodOptions, method_proto);
  }
}

static int count_bits_debug(uint64_t x) {
  // For assertions only, speed does not matter.
  int n = 0;
  while (x) {
    if (x & 1) n++;
    x >>= 1;
  }
  return n;
}

upb_enumlayout *create_enumlayout(symtab_addctx *ctx, const upb_enumdef *e) {
  int n = 0;
  uint64_t mask = 0;

  for (int i = 0; i < e->value_count; i++) {
    uint32_t val = (uint32_t)e->values[i].number;
    if (val < 64) {
      mask |= 1 << val;
    } else {
      n++;
    }
  }

  int32_t *values = symtab_alloc(ctx, sizeof(*values) * n);

  if (n) {
    int32_t *p = values;

    // Add values outside the bitmask range to the list, as described in the
    // comments for upb_enumlayout.
    for (int i = 0; i < e->value_count; i++) {
      int32_t val = e->values[i].number;
      if ((uint32_t)val >= 64) {
        *p++ = val;
      }
    }
    UPB_ASSERT(p == values + n);
  }

  UPB_ASSERT(upb_inttable_count(&e->iton) == n + count_bits_debug(mask));

  upb_enumlayout *layout = symtab_alloc(ctx, sizeof(*layout));
  layout->value_count = n;
  layout->mask = mask;
  layout->values = values;

  return layout;
}

static void create_enumvaldef(
    symtab_addctx *ctx, const char *prefix,
    const google_protobuf_EnumValueDescriptorProto *val_proto, upb_enumdef *e,
    int i) {
  upb_enumvaldef *val = (upb_enumvaldef *)&e->values[i];
  upb_strview name = google_protobuf_EnumValueDescriptorProto_name(val_proto);
  upb_value v = upb_value_constptr(val);

  val->enum_ = e;  /* Must happen prior to symtab_add(). */
  val->full_name = makefullname(ctx, prefix, name);
  val->number = google_protobuf_EnumValueDescriptorProto_number(val_proto);
  symtab_add(ctx, val->full_name, pack_def(val, UPB_DEFTYPE_ENUMVAL));

  SET_OPTIONS(val->opts, EnumValueDescriptorProto, EnumValueOptions, val_proto);

  if (i == 0 && e->file->syntax == UPB_SYNTAX_PROTO3 && val->number != 0) {
    symtab_errf(ctx, "for proto3, the first enum value must be zero (%s)",
                e->full_name);
  }

  CHK_OOM(upb_strtable_insert(&e->ntoi, name.data, name.size, v, ctx->arena));

  // Multiple enumerators can have the same number, first one wins.
  if (!upb_inttable_lookup(&e->iton, val->number, NULL)) {
    CHK_OOM(upb_inttable_insert(&e->iton, val->number, v, ctx->arena));
  }
}

static void create_enumdef(
    symtab_addctx *ctx, const char *prefix,
    const google_protobuf_EnumDescriptorProto *enum_proto,
    const upb_msgdef *containing_type,
    const upb_enumdef *_e) {
  upb_enumdef *e = (upb_enumdef*)_e;;
  const google_protobuf_EnumValueDescriptorProto *const *values;
  upb_strview name;
  size_t i, n;

  e->file = ctx->file;  /* Must happen prior to symtab_add() */
  e->containing_type = containing_type;

  name = google_protobuf_EnumDescriptorProto_name(enum_proto);
  check_ident(ctx, name, false);

  e->full_name = makefullname(ctx, prefix, name);
  symtab_add(ctx, e->full_name, pack_def(e, UPB_DEFTYPE_ENUM));

  values = google_protobuf_EnumDescriptorProto_value(enum_proto, &n);
  CHK_OOM(upb_strtable_init(&e->ntoi, n, ctx->arena));
  CHK_OOM(upb_inttable_init(&e->iton, ctx->arena));

  e->defaultval = 0;
  e->value_count = n;
  e->values = symtab_alloc(ctx, sizeof(*e->values) * n);

  if (n == 0) {
    symtab_errf(ctx, "enums must contain at least one value (%s)",
                e->full_name);
  }

  SET_OPTIONS(e->opts, EnumDescriptorProto, EnumOptions, enum_proto);

  for (i = 0; i < n; i++) {
    create_enumvaldef(ctx, prefix, values[i], e, i);
  }

  upb_inttable_compact(&e->iton, ctx->arena);

  if (e->file->syntax == UPB_SYNTAX_PROTO2) {
    if (ctx->layout) {
      UPB_ASSERT(ctx->enum_count < ctx->layout->enum_count);
      e->layout = ctx->layout->enums[ctx->enum_count++];
      UPB_ASSERT(n == e->layout->value_count + count_bits_debug(e->layout->mask));
    } else {
      e->layout = create_enumlayout(ctx, e);
    }
  } else {
    e->layout = NULL;
  }
}

static void create_msgdef(symtab_addctx *ctx, const char *prefix,
                          const google_protobuf_DescriptorProto *msg_proto,
                          const upb_msgdef *containing_type,
                          const upb_msgdef *_m) {
  upb_msgdef *m = (upb_msgdef*)_m;
  const google_protobuf_OneofDescriptorProto *const *oneofs;
  const google_protobuf_FieldDescriptorProto *const *fields;
  const google_protobuf_EnumDescriptorProto *const *enums;
  const google_protobuf_DescriptorProto *const *msgs;
  const google_protobuf_DescriptorProto_ExtensionRange *const *ext_ranges;
  size_t i, n_oneof, n_field, n_ext_range, n;
  upb_strview name;

  m->file = ctx->file;  /* Must happen prior to symtab_add(). */
  m->containing_type = containing_type;

  name = google_protobuf_DescriptorProto_name(msg_proto);
  check_ident(ctx, name, false);

  m->full_name = makefullname(ctx, prefix, name);
  symtab_add(ctx, m->full_name, pack_def(m, UPB_DEFTYPE_MSG));

  oneofs = google_protobuf_DescriptorProto_oneof_decl(msg_proto, &n_oneof);
  fields = google_protobuf_DescriptorProto_field(msg_proto, &n_field);
  ext_ranges =
      google_protobuf_DescriptorProto_extension_range(msg_proto, &n_ext_range);

  CHK_OOM(upb_inttable_init(&m->itof, ctx->arena));
  CHK_OOM(upb_strtable_init(&m->ntof, n_oneof + n_field, ctx->arena));

  if (ctx->layout) {
    /* create_fielddef() below depends on this being set. */
    UPB_ASSERT(ctx->msg_count < ctx->layout->msg_count);
    m->layout = ctx->layout->msgs[ctx->msg_count++];
    UPB_ASSERT(n_field == m->layout->field_count);
  } else {
    /* Allocate now (to allow cross-linking), populate later. */
    m->layout = symtab_alloc(
        ctx, sizeof(*m->layout) + sizeof(_upb_fasttable_entry));
  }

  SET_OPTIONS(m->opts, DescriptorProto, MessageOptions, msg_proto);

  m->oneof_count = n_oneof;
  m->oneofs = symtab_alloc(ctx, sizeof(*m->oneofs) * n_oneof);
  for (i = 0; i < n_oneof; i++) {
    create_oneofdef(ctx, m, oneofs[i], &m->oneofs[i]);
  }

  m->field_count = n_field;
  m->fields = symtab_alloc(ctx, sizeof(*m->fields) * n_field);
  for (i = 0; i < n_field; i++) {
    create_fielddef(ctx, m->full_name, m, fields[i], &m->fields[i]);
  }

  m->ext_range_count = n_ext_range;
  m->ext_ranges = symtab_alloc(ctx, sizeof(*m->ext_ranges) * n_ext_range);
  for (i = 0; i < n_ext_range; i++) {
    const google_protobuf_DescriptorProto_ExtensionRange *r = ext_ranges[i];
    upb_extrange *r_def = (upb_extrange*)&m->ext_ranges[i];
    int32_t start = google_protobuf_DescriptorProto_ExtensionRange_start(r);
    int32_t end = google_protobuf_DescriptorProto_ExtensionRange_end(r);
    int32_t max =
        google_protobuf_MessageOptions_message_set_wire_format(m->opts)
            ? INT32_MAX
            : UPB_MAX_FIELDNUMBER + 1;

    // A full validation would also check that each range is disjoint, and that
    // none of the fields overlap with the extension ranges, but we are just
    // sanity checking here.
    if (start < 1 || end <= start || end > max) {
      symtab_errf(ctx, "Extension range (%d, %d) is invalid, message=%s\n",
                  (int)start, (int)end, m->full_name);
    }

    r_def->start = start;
    r_def->end = end;
    SET_OPTIONS(r_def->opts, DescriptorProto_ExtensionRange,
                ExtensionRangeOptions, r);
  }

  finalize_oneofs(ctx, m);
  assign_msg_wellknowntype(m);
  upb_inttable_compact(&m->itof, ctx->arena);

  /* This message is built.  Now build nested entities. */

  enums = google_protobuf_DescriptorProto_enum_type(msg_proto, &n);
  m->nested_enum_count = n;
  m->nested_enums = symtab_alloc(ctx, sizeof(*m->nested_enums) * n);
  for (i = 0; i < n; i++) {
    m->nested_enum_count = i + 1;
    create_enumdef(ctx, m->full_name, enums[i], m, &m->nested_enums[i]);
  }

  fields = google_protobuf_DescriptorProto_extension(msg_proto, &n);
  m->nested_ext_count = n;
  m->nested_exts = symtab_alloc(ctx, sizeof(*m->nested_exts) * n);
  for (i = 0; i < n; i++) {
    create_fielddef(ctx, m->full_name, NULL, fields[i], &m->nested_exts[i]);
    ((upb_fielddef*)&m->nested_exts[i])->index_ = i;
  }

  msgs = google_protobuf_DescriptorProto_nested_type(msg_proto, &n);
  m->nested_msg_count = n;
  m->nested_msgs = symtab_alloc(ctx, sizeof(*m->nested_msgs) * n);
  for (i = 0; i < n; i++) {
    create_msgdef(ctx, m->full_name, msgs[i], m, &m->nested_msgs[i]);
  }
}

static void resolve_subdef(symtab_addctx *ctx, const char *prefix,
                           upb_fielddef *f) {
  const google_protobuf_FieldDescriptorProto *field_proto = f->sub.unresolved;
  upb_strview name =
      google_protobuf_FieldDescriptorProto_type_name(field_proto);
  bool has_name =
      google_protobuf_FieldDescriptorProto_has_type_name(field_proto);
  switch ((int)f->type_) {
    case FIELD_TYPE_UNSPECIFIED: {
      // Type was not specified and must be inferred.
      UPB_ASSERT(has_name);
      upb_deftype_t type;
      const void *def = 
          symtab_resolveany(ctx, f->full_name, prefix, name, &type);
      switch (type) {
        case UPB_DEFTYPE_ENUM:
          f->sub.enumdef = def;
          f->type_ = UPB_DTYPE_ENUM;
          break;
        case UPB_DEFTYPE_MSG:
          f->sub.msgdef = def;
          f->type_ = UPB_DTYPE_MESSAGE;  // It appears there is no way of this
                                         // being a group.
          break;
        default:
          symtab_errf(ctx, "Couldn't resolve type name for field %s",
                      f->full_name);
      }
    }
    case UPB_DTYPE_MESSAGE:
    case UPB_DTYPE_GROUP:
      UPB_ASSERT(has_name);
      f->sub.msgdef =
          symtab_resolve(ctx, f->full_name, prefix, name, UPB_DEFTYPE_MSG);
      break;
    case UPB_DTYPE_ENUM:
      UPB_ASSERT(has_name);
      f->sub.enumdef =
          symtab_resolve(ctx, f->full_name, prefix, name, UPB_DEFTYPE_ENUM);
      break;
    default:
      // No resolution necessary.
      break;
  }
}

static void resolve_extension(
    symtab_addctx *ctx, const char *prefix, upb_fielddef *f,
    const google_protobuf_FieldDescriptorProto *field_proto) {
  if (!google_protobuf_FieldDescriptorProto_has_extendee(field_proto)) {
    symtab_errf(ctx, "extension for field '%s' had no extendee",
                f->full_name);
  }

  upb_strview name = google_protobuf_FieldDescriptorProto_extendee(field_proto);
  const upb_msgdef *m =
      symtab_resolve(ctx, f->full_name, prefix, name, UPB_DEFTYPE_MSG);
  f->msgdef = m;

  bool found = false;

  for (int i = 0, n = m->ext_range_count; i < n; i++) {
    const upb_extrange *r = &m->ext_ranges[i];
    if (r->start <= f->number_ && f->number_ < r->end) {
      found = true;
      break;
    }
  }

  if (!found) {
    symtab_errf(ctx,
                "field number %u in extension %s has no extension range in "
                "message %s",
                (unsigned)f->number_, f->full_name, f->msgdef->full_name);
  }

  const upb_msglayout_ext *ext = ctx->file->ext_layouts[f->layout_index];
  if (ctx->layout) {
    UPB_ASSERT(upb_fielddef_number(f) == ext->field.number);
  } else {
    upb_msglayout_ext *mut_ext = (upb_msglayout_ext*)ext;
    fill_fieldlayout(&mut_ext->field, f);
    mut_ext->field.presence = 0;
    mut_ext->field.offset = 0;
    mut_ext->field.submsg_index = 0;
    mut_ext->extendee = f->msgdef->layout;
    mut_ext->sub.submsg = f->sub.msgdef->layout;
  }

  CHK_OOM(upb_inttable_insert(&ctx->symtab->exts, (uintptr_t)ext,
                              upb_value_constptr(f), ctx->arena));
}

static void resolve_default(
    symtab_addctx *ctx, upb_fielddef *f,
    const google_protobuf_FieldDescriptorProto *field_proto) {
  // Have to delay resolving of the default value until now because of the enum
  // case, since enum defaults are specified with a label.
  if (google_protobuf_FieldDescriptorProto_has_default_value(field_proto)) {
    upb_strview defaultval =
        google_protobuf_FieldDescriptorProto_default_value(field_proto);

    if (f->file->syntax == UPB_SYNTAX_PROTO3) {
      symtab_errf(ctx, "proto3 fields cannot have explicit defaults (%s)",
                  f->full_name);
    }

    if (upb_fielddef_issubmsg(f)) {
      symtab_errf(ctx, "message fields cannot have explicit defaults (%s)",
                  f->full_name);
    }

    parse_default(ctx, defaultval.data, defaultval.size, f);
    f->has_default = true;
  } else {
    set_default_default(ctx, f);
    f->has_default = false;
  }
}

static void resolve_fielddef(symtab_addctx *ctx, const char *prefix,
                             upb_fielddef *f) {
  // We have to stash this away since resolve_subdef() may overwrite it.
  const google_protobuf_FieldDescriptorProto *field_proto = f->sub.unresolved;

  resolve_subdef(ctx, prefix, f);
  resolve_default(ctx, f, field_proto);

  if (f->is_extension_) {
    resolve_extension(ctx, prefix, f, field_proto);
  }
}

static void resolve_msgdef(symtab_addctx *ctx, upb_msgdef *m) {
  for (int i = 0; i < m->field_count; i++) {
    resolve_fielddef(ctx, m->full_name, (upb_fielddef *)&m->fields[i]);
  }

  for (int i = 0; i < m->nested_ext_count; i++) {
    resolve_fielddef(ctx, m->full_name, (upb_fielddef *)&m->nested_exts[i]);
  }

  if (!ctx->layout) make_layout(ctx, m);

  m->in_message_set = false;
  if (m->nested_ext_count == 1) {
    const upb_fielddef *ext = &m->nested_exts[0];
    if (ext->type_ == UPB_DTYPE_MESSAGE && ext->label_ == UPB_LABEL_OPTIONAL &&
        ext->sub.msgdef == m &&
        google_protobuf_MessageOptions_message_set_wire_format(
            ext->msgdef->opts)) {
      m->in_message_set = true;
    }
  }

  for (int i = 0; i < m->nested_msg_count; i++) {
    resolve_msgdef(ctx, (upb_msgdef*)&m->nested_msgs[i]);
  }
}

static int count_exts_in_msg(const google_protobuf_DescriptorProto *msg_proto) {
  size_t n;
  google_protobuf_DescriptorProto_extension(msg_proto, &n);
  int ext_count = n;

  const google_protobuf_DescriptorProto *const *nested_msgs =
      google_protobuf_DescriptorProto_nested_type(msg_proto, &n);
  for (size_t i = 0; i < n; i++) {
    ext_count += count_exts_in_msg(nested_msgs[i]);
  }

  return ext_count;
}

static void build_filedef(
    symtab_addctx *ctx, upb_filedef *file,
    const google_protobuf_FileDescriptorProto *file_proto) {
  const google_protobuf_DescriptorProto *const *msgs;
  const google_protobuf_EnumDescriptorProto *const *enums;
  const google_protobuf_FieldDescriptorProto *const *exts;
  const google_protobuf_ServiceDescriptorProto *const *services;
  const upb_strview *strs;
  const int32_t *public_deps;
  const int32_t *weak_deps;
  size_t i, n;

  file->symtab = ctx->symtab;

  /* Count all extensions in the file, to build a flat array of layouts. */
  google_protobuf_FileDescriptorProto_extension(file_proto, &n);
  int ext_count = n;
  msgs = google_protobuf_FileDescriptorProto_message_type(file_proto, &n);
  for (int i = 0; i < n; i++) {
    ext_count += count_exts_in_msg(msgs[i]);
  }
  file->ext_count = ext_count;

  if (ctx->layout) {
    /* We are using the ext layouts that were passed in. */
    file->ext_layouts = ctx->layout->exts;
    if (ctx->layout->ext_count != file->ext_count) {
      symtab_errf(ctx, "Extension count did not match layout (%d vs %d)",
                  ctx->layout->ext_count, file->ext_count);
    }
  } else {
    /* We are building ext layouts from scratch. */
    file->ext_layouts =
        symtab_alloc(ctx, sizeof(*file->ext_layouts) * file->ext_count);
    upb_msglayout_ext *ext = symtab_alloc(ctx, sizeof(*ext) * file->ext_count);
    for (int i = 0; i < file->ext_count; i++) {
      file->ext_layouts[i] = &ext[i];
    }
  }

  if (!google_protobuf_FileDescriptorProto_has_name(file_proto)) {
    symtab_errf(ctx, "File has no name");
  }

  file->name =
      strviewdup(ctx, google_protobuf_FileDescriptorProto_name(file_proto));

  if (google_protobuf_FileDescriptorProto_has_package(file_proto)) {
    upb_strview package =
        google_protobuf_FileDescriptorProto_package(file_proto);
    check_ident(ctx, package, true);
    file->package = strviewdup(ctx, package);
  } else {
    file->package = NULL;
  }

  if (google_protobuf_FileDescriptorProto_has_syntax(file_proto)) {
    upb_strview syntax = google_protobuf_FileDescriptorProto_syntax(file_proto);

    if (streql_view(syntax, "proto2")) {
      file->syntax = UPB_SYNTAX_PROTO2;
    } else if (streql_view(syntax, "proto3")) {
      file->syntax = UPB_SYNTAX_PROTO3;
    } else {
      symtab_errf(ctx, "Invalid syntax '" UPB_STRVIEW_FORMAT "'",
                  UPB_STRVIEW_ARGS(syntax));
    }
  } else {
    file->syntax = UPB_SYNTAX_PROTO2;
  }

  /* Read options. */
  SET_OPTIONS(file->opts, FileDescriptorProto, FileOptions, file_proto);

  /* Verify dependencies. */
  strs = google_protobuf_FileDescriptorProto_dependency(file_proto, &n);
  file->dep_count = n;
  file->deps = symtab_alloc(ctx, sizeof(*file->deps) * n);

  for (i = 0; i < n; i++) {
    upb_strview str = strs[i];
    file->deps[i] = upb_symtab_lookupfile2(ctx->symtab, str.data, str.size);
    if (!file->deps[i]) {
      symtab_errf(ctx,
                  "Depends on file '" UPB_STRVIEW_FORMAT
                  "', but it has not been loaded",
                  UPB_STRVIEW_ARGS(str));
    }
  }

  public_deps =
      google_protobuf_FileDescriptorProto_public_dependency(file_proto, &n);
  file->public_dep_count = n;
  file->public_deps = symtab_alloc(ctx, sizeof(*file->public_deps) * n);
  int32_t *mutable_public_deps = (int32_t*)file->public_deps;
  for (i = 0; i < n; i++) {
    if (public_deps[i] >= file->dep_count) {
      symtab_errf(ctx, "public_dep %d is out of range", (int)public_deps[i]);
    }
    mutable_public_deps[i] = public_deps[i];
  }

  weak_deps =
      google_protobuf_FileDescriptorProto_weak_dependency(file_proto, &n);
  file->weak_dep_count = n;
  file->weak_deps = symtab_alloc(ctx, sizeof(*file->weak_deps) * n);
  int32_t *mutable_weak_deps = (int32_t*)file->weak_deps;
  for (i = 0; i < n; i++) {
    if (weak_deps[i] >= file->dep_count) {
      symtab_errf(ctx, "public_dep %d is out of range", (int)public_deps[i]);
    }
    mutable_weak_deps[i] = weak_deps[i];
  }

  /* Create enums. */
  enums = google_protobuf_FileDescriptorProto_enum_type(file_proto, &n);
  file->top_lvl_enum_count = n;
  file->top_lvl_enums = symtab_alloc(ctx, sizeof(*file->top_lvl_enums) * n);
  for (i = 0; i < n; i++) {
    create_enumdef(ctx, file->package, enums[i], NULL, &file->top_lvl_enums[i]);
  }

  /* Create extensions. */
  exts = google_protobuf_FileDescriptorProto_extension(file_proto, &n);
  file->top_lvl_ext_count = n;
  file->top_lvl_exts = symtab_alloc(ctx, sizeof(*file->top_lvl_exts) * n);
  for (i = 0; i < n; i++) {
    create_fielddef(ctx, file->package, NULL, exts[i], &file->top_lvl_exts[i]);
    ((upb_fielddef*)&file->top_lvl_exts[i])->index_ = i;
  }

  /* Create messages. */
  msgs = google_protobuf_FileDescriptorProto_message_type(file_proto, &n);
  file->top_lvl_msg_count = n;
  file->top_lvl_msgs = symtab_alloc(ctx, sizeof(*file->top_lvl_msgs) * n);
  for (i = 0; i < n; i++) {
    create_msgdef(ctx, file->package, msgs[i], NULL, &file->top_lvl_msgs[i]);
  }

  /* Create services. */
  services = google_protobuf_FileDescriptorProto_service(file_proto, &n);
  file->service_count = n;
  file->services = symtab_alloc(ctx, sizeof(*file->services) * n);
  for (i = 0; i < n; i++) {
    create_service(ctx, services[i], &file->services[i]);
    ((upb_servicedef*)&file->services[i])->index = i;
  }

  /* Now that all names are in the table, build layouts and resolve refs. */
  for (i = 0; i < (size_t)file->top_lvl_ext_count; i++) {
    resolve_fielddef(ctx, file->package, (upb_fielddef*)&file->top_lvl_exts[i]);
  }

  for (i = 0; i < (size_t)file->top_lvl_msg_count; i++) {
    resolve_msgdef(ctx, (upb_msgdef*)&file->top_lvl_msgs[i]);
  }

  if (file->ext_count) {
    CHK_OOM(_upb_extreg_add(ctx->symtab->extreg, file->ext_layouts,
                            file->ext_count));
  }
}

static void remove_filedef(upb_symtab *s, upb_filedef *file) {
  intptr_t iter = UPB_INTTABLE_BEGIN;
  upb_strview key;
  upb_value val;
  while (upb_strtable_next2(&s->syms, &key, &val, &iter)) {
    const upb_filedef *f;
    switch (deftype(val)) {
      case UPB_DEFTYPE_EXT:
        f = upb_fielddef_file(unpack_def(val, UPB_DEFTYPE_EXT));
        break;
      case UPB_DEFTYPE_MSG:
        f = upb_msgdef_file(unpack_def(val, UPB_DEFTYPE_MSG));
        break;
      case UPB_DEFTYPE_ENUM:
        f = upb_enumdef_file(unpack_def(val, UPB_DEFTYPE_ENUM));
        break;
      case UPB_DEFTYPE_ENUMVAL:
        f = upb_enumdef_file(
            upb_enumvaldef_enum(unpack_def(val, UPB_DEFTYPE_ENUMVAL)));
        break;
      case UPB_DEFTYPE_SERVICE:
        f = upb_servicedef_file(unpack_def(val, UPB_DEFTYPE_SERVICE));
        break;
      default:
        UPB_UNREACHABLE();
    }

    if (f == file) upb_strtable_removeiter(&s->syms, &iter);
  }
}

static const upb_filedef *_upb_symtab_addfile(
    upb_symtab *s, const google_protobuf_FileDescriptorProto *file_proto,
    const upb_msglayout_file *layout, upb_status *status) {
  symtab_addctx ctx;
  upb_strview name = google_protobuf_FileDescriptorProto_name(file_proto);
  upb_value v;

  if (upb_strtable_lookup2(&s->files, name.data, name.size, &v)) {
    if (unpack_def(v, UPB_DEFTYPE_FILE)) {
      upb_status_seterrf(status, "duplicate file name (%.*s)",
                        UPB_STRVIEW_ARGS(name));
      return NULL;
    }
    const upb_msglayout_file *registered = unpack_def(v, UPB_DEFTYPE_LAYOUT);
    UPB_ASSERT(registered);
    if (layout && layout != registered) {
      upb_status_seterrf(
          status, "tried to build with a different layout (filename=%.*s)",
          UPB_STRVIEW_ARGS(name));
      return NULL;
    }
    layout = registered;
  }

  ctx.symtab = s;
  ctx.layout = layout;
  ctx.msg_count = 0;
  ctx.enum_count = 0;
  ctx.ext_count = 0;
  ctx.status = status;
  ctx.file = NULL;
  ctx.arena = upb_arena_new();
  ctx.tmp_arena = upb_arena_new();

  if (!ctx.arena || !ctx.tmp_arena) {
    if (ctx.arena) upb_arena_free(ctx.arena);
    if (ctx.tmp_arena) upb_arena_free(ctx.tmp_arena);
    upb_status_setoom(status);
    return NULL;
  }

  if (UPB_UNLIKELY(UPB_SETJMP(ctx.err))) {
    UPB_ASSERT(!upb_ok(status));
    if (ctx.file) {
      remove_filedef(s, ctx.file);
      ctx.file = NULL;
    }
  } else {
    ctx.file = symtab_alloc(&ctx, sizeof(*ctx.file));
    build_filedef(&ctx, ctx.file, file_proto);
    upb_strtable_insert(&s->files, name.data, name.size,
                        pack_def(ctx.file, UPB_DEFTYPE_FILE), ctx.arena);
    UPB_ASSERT(upb_ok(status));
    upb_arena_fuse(s->arena, ctx.arena);
  }

  upb_arena_free(ctx.arena);
  upb_arena_free(ctx.tmp_arena);
  return ctx.file;
}

  const upb_filedef *upb_symtab_addfile(
      upb_symtab * s, const google_protobuf_FileDescriptorProto *file_proto,
      upb_status *status) {
    return _upb_symtab_addfile(s, file_proto, NULL, status);
  }

/* Include here since we want most of this file to be stdio-free. */
#include <stdio.h>

bool _upb_symtab_loaddefinit(upb_symtab *s, const upb_def_init *init) {
  /* Since this function should never fail (it would indicate a bug in upb) we
   * print errors to stderr instead of returning error status to the user. */
  upb_def_init **deps = init->deps;
  google_protobuf_FileDescriptorProto *file;
  upb_arena *arena;
  upb_status status;

  upb_status_clear(&status);

  if (upb_symtab_lookupfile(s, init->filename)) {
    return true;
  }

  arena = upb_arena_new();

  for (; *deps; deps++) {
    if (!_upb_symtab_loaddefinit(s, *deps)) goto err;
  }

  file = google_protobuf_FileDescriptorProto_parse_ex(
      init->descriptor.data, init->descriptor.size, NULL, kUpb_DecodeOption_AliasString,
      arena);
  s->bytes_loaded += init->descriptor.size;

  if (!file) {
    upb_status_seterrf(
        &status,
        "Failed to parse compiled-in descriptor for file '%s'. This should "
        "never happen.",
        init->filename);
    goto err;
  }

  if (!_upb_symtab_addfile(s, file, init->layout, &status)) {
    goto err;
  }

  upb_arena_free(arena);
  return true;

err:
  fprintf(stderr,
          "Error loading compiled-in descriptor for file '%s' (this should "
          "never happen): %s\n",
          init->filename, upb_status_errmsg(&status));
  upb_arena_free(arena);
  return false;
}

size_t _upb_symtab_bytesloaded(const upb_symtab *s) {
  return s->bytes_loaded;
}

upb_arena *_upb_symtab_arena(const upb_symtab *s) {
  return s->arena;
}

const upb_fielddef *_upb_symtab_lookupextfield(const upb_symtab *s,
                                               const upb_msglayout_ext *ext) {
  upb_value v;
  bool ok = upb_inttable_lookup(&s->exts, (uintptr_t)ext, &v);
  UPB_ASSERT(ok);
  return upb_value_getconstptr(v);
}

const upb_fielddef *upb_symtab_lookupextbynum(const upb_symtab *s,
                                              const upb_msgdef *m,
                                              int32_t fieldnum) {
  const upb_msglayout *l = upb_msgdef_layout(m);
  const upb_msglayout_ext *ext = _upb_extreg_get(s->extreg, l, fieldnum);
  return ext ? _upb_symtab_lookupextfield(s, ext) : NULL;
}

bool _upb_symtab_registerlayout(upb_symtab *s, const char *filename,
                                const upb_msglayout_file *file) {
  if (upb_symtab_lookupfile(s, filename)) return false;
  upb_value v = pack_def(file, UPB_DEFTYPE_LAYOUT);
  return upb_strtable_insert(&s->files, filename, strlen(filename), v,
                             s->arena);
}

const upb_extreg *upb_symtab_extreg(const upb_symtab *s) {
  return s->extreg;
}

const upb_fielddef **upb_symtab_getallexts(const upb_symtab *s,
                                           const upb_msgdef *m, size_t *count) {
  size_t n = 0;
  intptr_t iter = UPB_INTTABLE_BEGIN;
  uintptr_t key;
  upb_value val;
  // This is O(all exts) instead of O(exts for m).  If we need this to be
  // efficient we may need to make extreg into a two-level table, or have a
  // second per-message index.
  while (upb_inttable_next2(&s->exts, &key, &val, &iter)) {
    const upb_fielddef *f = upb_value_getconstptr(val);
    if (upb_fielddef_containingtype(f) == m) n++;
  }
  const upb_fielddef **exts = malloc(n * sizeof(*exts));
  iter = UPB_INTTABLE_BEGIN;
  size_t i = 0;
  while (upb_inttable_next2(&s->exts, &key, &val, &iter)) {
    const upb_fielddef *f = upb_value_getconstptr(val);
    if (upb_fielddef_containingtype(f) == m) exts[i++] = f;
  }
  *count = n;
  return exts;
}

#undef CHK_OOM

/** upb/reflection.c ************************************************************/

#include <string.h>


static size_t get_field_size(const upb_msglayout_field *f) {
  static unsigned char sizes[] = {
    0,/* 0 */
    8, /* UPB_DESCRIPTOR_TYPE_DOUBLE */
    4, /* UPB_DESCRIPTOR_TYPE_FLOAT */
    8, /* UPB_DESCRIPTOR_TYPE_INT64 */
    8, /* UPB_DESCRIPTOR_TYPE_UINT64 */
    4, /* UPB_DESCRIPTOR_TYPE_INT32 */
    8, /* UPB_DESCRIPTOR_TYPE_FIXED64 */
    4, /* UPB_DESCRIPTOR_TYPE_FIXED32 */
    1, /* UPB_DESCRIPTOR_TYPE_BOOL */
    sizeof(upb_strview), /* UPB_DESCRIPTOR_TYPE_STRING */
    sizeof(void*), /* UPB_DESCRIPTOR_TYPE_GROUP */
    sizeof(void*), /* UPB_DESCRIPTOR_TYPE_MESSAGE */
    sizeof(upb_strview), /* UPB_DESCRIPTOR_TYPE_BYTES */
    4, /* UPB_DESCRIPTOR_TYPE_UINT32 */
    4, /* UPB_DESCRIPTOR_TYPE_ENUM */
    4, /* UPB_DESCRIPTOR_TYPE_SFIXED32 */
    8, /* UPB_DESCRIPTOR_TYPE_SFIXED64 */
    4, /* UPB_DESCRIPTOR_TYPE_SINT32 */
    8, /* UPB_DESCRIPTOR_TYPE_SINT64 */
  };
  return _upb_repeated_or_map(f) ? sizeof(void *) : sizes[f->descriptortype];
}

/* Strings/bytes are special-cased in maps. */
static char _upb_fieldtype_to_mapsize[12] = {
  0,
  1,  /* UPB_TYPE_BOOL */
  4,  /* UPB_TYPE_FLOAT */
  4,  /* UPB_TYPE_INT32 */
  4,  /* UPB_TYPE_UINT32 */
  4,  /* UPB_TYPE_ENUM */
  sizeof(void*),  /* UPB_TYPE_MESSAGE */
  8,  /* UPB_TYPE_DOUBLE */
  8,  /* UPB_TYPE_INT64 */
  8,  /* UPB_TYPE_UINT64 */
  0,  /* UPB_TYPE_STRING */
  0,  /* UPB_TYPE_BYTES */
};

static const char _upb_fieldtype_to_sizelg2[12] = {
  0,
  0,  /* UPB_TYPE_BOOL */
  2,  /* UPB_TYPE_FLOAT */
  2,  /* UPB_TYPE_INT32 */
  2,  /* UPB_TYPE_UINT32 */
  2,  /* UPB_TYPE_ENUM */
  UPB_SIZE(2, 3),  /* UPB_TYPE_MESSAGE */
  3,  /* UPB_TYPE_DOUBLE */
  3,  /* UPB_TYPE_INT64 */
  3,  /* UPB_TYPE_UINT64 */
  UPB_SIZE(3, 4),  /* UPB_TYPE_STRING */
  UPB_SIZE(3, 4),  /* UPB_TYPE_BYTES */
};

/** upb_msg *******************************************************************/

upb_msg *upb_msg_new(const upb_msgdef *m, upb_arena *a) {
  return _upb_msg_new(upb_msgdef_layout(m), a);
}

static bool in_oneof(const upb_msglayout_field *field) {
  return field->presence < 0;
}

static upb_msgval _upb_msg_getraw(const upb_msg *msg, const upb_fielddef *f) {
  const upb_msglayout_field *field = upb_fielddef_layout(f);
  const char *mem = UPB_PTR_AT(msg, field->offset, char);
  upb_msgval val = {0};
  memcpy(&val, mem, get_field_size(field));
  return val;
}

bool upb_msg_has(const upb_msg *msg, const upb_fielddef *f) {
  if (upb_fielddef_isextension(f)) {
    const upb_msglayout_ext *ext = _upb_fielddef_extlayout(f);
    return _upb_msg_getext(msg, ext) != NULL;
  } else {
    const upb_msglayout_field *field = upb_fielddef_layout(f);
    if (in_oneof(field)) {
      return _upb_getoneofcase_field(msg, field) == field->number;
    } else if (field->presence > 0) {
      return _upb_hasbit_field(msg, field);
    } else {
      UPB_ASSERT(field->descriptortype == UPB_DESCRIPTOR_TYPE_MESSAGE ||
                 field->descriptortype == UPB_DESCRIPTOR_TYPE_GROUP);
      return _upb_msg_getraw(msg, f).msg_val != NULL;
    }
  }
}

const upb_fielddef *upb_msg_whichoneof(const upb_msg *msg,
                                       const upb_oneofdef *o) {
  const upb_fielddef *f = upb_oneofdef_field(o, 0);
  if (upb_oneofdef_issynthetic(o)) {
    UPB_ASSERT(upb_oneofdef_fieldcount(o) == 1);
    return upb_msg_has(msg, f) ? f : NULL;
  } else {
    const upb_msglayout_field *field = upb_fielddef_layout(f);
    uint32_t oneof_case = _upb_getoneofcase_field(msg, field);
    f = oneof_case ? upb_oneofdef_itof(o, oneof_case) : NULL;
    UPB_ASSERT((f != NULL) == (oneof_case != 0));
    return f;
  }
}

upb_msgval upb_msg_get(const upb_msg *msg, const upb_fielddef *f) {
  if (upb_fielddef_isextension(f)) {
    const upb_msg_ext *ext = _upb_msg_getext(msg, _upb_fielddef_extlayout(f));
    if (ext) {
      upb_msgval val;
      memcpy(&val, &ext->data, sizeof(val));
      return val;
    } else if (upb_fielddef_isseq(f)) {
      return (upb_msgval){.array_val = NULL};
    }
  } else if (!upb_fielddef_haspresence(f) || upb_msg_has(msg, f)) {
    return _upb_msg_getraw(msg, f);
  }
  return upb_fielddef_default(f);
}

upb_mutmsgval upb_msg_mutable(upb_msg *msg, const upb_fielddef *f,
                              upb_arena *a) {
  UPB_ASSERT(upb_fielddef_issubmsg(f) || upb_fielddef_isseq(f));
  if (upb_fielddef_haspresence(f) && !upb_msg_has(msg, f)) {
    // We need to skip the upb_msg_get() call in this case.
    goto make;
  }

  upb_msgval val = upb_msg_get(msg, f);
  if (val.array_val) {
    return (upb_mutmsgval){.array = (upb_array*)val.array_val};
  }

  upb_mutmsgval ret;
make:
  if (!a) return (upb_mutmsgval){.array = NULL};
  if (upb_fielddef_ismap(f)) {
    const upb_msgdef *entry = upb_fielddef_msgsubdef(f);
    const upb_fielddef *key = upb_msgdef_itof(entry, UPB_MAPENTRY_KEY);
    const upb_fielddef *value = upb_msgdef_itof(entry, UPB_MAPENTRY_VALUE);
    ret.map = upb_map_new(a, upb_fielddef_type(key), upb_fielddef_type(value));
  } else if (upb_fielddef_isseq(f)) {
    ret.array = upb_array_new(a, upb_fielddef_type(f));
  } else {
    UPB_ASSERT(upb_fielddef_issubmsg(f));
    ret.msg = upb_msg_new(upb_fielddef_msgsubdef(f), a);
  }

  val.array_val = ret.array;
  upb_msg_set(msg, f, val, a);

  return ret;
}

bool upb_msg_set(upb_msg *msg, const upb_fielddef *f, upb_msgval val,
                 upb_arena *a) {
  if (upb_fielddef_isextension(f)) {
    upb_msg_ext *ext =
        _upb_msg_getorcreateext(msg, _upb_fielddef_extlayout(f), a);
    if (!ext) return false;
    memcpy(&ext->data, &val, sizeof(val));
  } else {
    const upb_msglayout_field *field = upb_fielddef_layout(f);
    char *mem = UPB_PTR_AT(msg, field->offset, char);
    memcpy(mem, &val, get_field_size(field));
    if (field->presence > 0) {
      _upb_sethas_field(msg, field);
    } else if (in_oneof(field)) {
      *_upb_oneofcase_field(msg, field) = field->number;
    }
  }
  return true;
}

void upb_msg_clearfield(upb_msg *msg, const upb_fielddef *f) {
  if (upb_fielddef_isextension(f)) {
    _upb_msg_clearext(msg, _upb_fielddef_extlayout(f));
  } else {
    const upb_msglayout_field *field = upb_fielddef_layout(f);
    char *mem = UPB_PTR_AT(msg, field->offset, char);

    if (field->presence > 0) {
      _upb_clearhas_field(msg, field);
    } else if (in_oneof(field)) {
      uint32_t *oneof_case = _upb_oneofcase_field(msg, field);
      if (*oneof_case != field->number) return;
      *oneof_case = 0;
    }

    memset(mem, 0, get_field_size(field));
  }
}

void upb_msg_clear(upb_msg *msg, const upb_msgdef *m) {
  _upb_msg_clear(msg, upb_msgdef_layout(m));
}

bool upb_msg_next(const upb_msg *msg, const upb_msgdef *m,
                  const upb_symtab *ext_pool, const upb_fielddef **out_f,
                  upb_msgval *out_val, size_t *iter) {
  size_t i = *iter;
  size_t n = upb_msgdef_fieldcount(m);
  const upb_msgval zero = {0};
  UPB_UNUSED(ext_pool);

  /* Iterate over normal fields, returning the first one that is set. */
  while (++i < n) {
    const upb_fielddef *f = upb_msgdef_field(m, i);
    upb_msgval val = _upb_msg_getraw(msg, f);

    /* Skip field if unset or empty. */
    if (upb_fielddef_haspresence(f)) {
      if (!upb_msg_has(msg, f)) continue;
    } else {
      upb_msgval test = val;
      if (upb_fielddef_isstring(f) && !upb_fielddef_isseq(f)) {
        /* Clear string pointer, only size matters (ptr could be non-NULL). */
        test.str_val.data = NULL;
      }
      /* Continue if NULL or 0. */
      if (memcmp(&test, &zero, sizeof(test)) == 0) continue;

      /* Continue on empty array or map. */
      if (upb_fielddef_ismap(f)) {
        if (upb_map_size(test.map_val) == 0) continue;
      } else if (upb_fielddef_isseq(f)) {
        if (upb_array_size(test.array_val) == 0) continue;
      }
    }

    *out_val = val;
    *out_f = f;
    *iter = i;
    return true;
  }

  if (ext_pool) {
    /* Return any extensions that are set. */
    size_t count;
    const upb_msg_ext *ext = _upb_msg_getexts(msg, &count);
    if (i - n < count) {
      ext += count - 1 - (i - n);
      memcpy(out_val, &ext->data, sizeof(*out_val));
      *out_f = _upb_symtab_lookupextfield(ext_pool, ext->ext);
      *iter = i;
      return true;
    }
  }

  *iter = i;
  return false;
}

bool _upb_msg_discardunknown(upb_msg *msg, const upb_msgdef *m, int depth) {
  size_t iter = UPB_MSG_BEGIN;
  const upb_fielddef *f;
  upb_msgval val;
  bool ret = true;

  if (--depth == 0) return false;

  _upb_msg_discardunknown_shallow(msg);

  while (upb_msg_next(msg, m, NULL /*ext_pool*/, &f, &val, &iter)) {
    const upb_msgdef *subm = upb_fielddef_msgsubdef(f);
    if (!subm) continue;
    if (upb_fielddef_ismap(f)) {
      const upb_fielddef *val_f = upb_msgdef_itof(subm, 2);
      const upb_msgdef *val_m = upb_fielddef_msgsubdef(val_f);
      upb_map *map = (upb_map*)val.map_val;
      size_t iter = UPB_MAP_BEGIN;

      if (!val_m) continue;

      while (upb_mapiter_next(map, &iter)) {
        upb_msgval map_val = upb_mapiter_value(map, iter);
        if (!_upb_msg_discardunknown((upb_msg*)map_val.msg_val, val_m, depth)) {
          ret = false;
        }
      }
    } else if (upb_fielddef_isseq(f)) {
      const upb_array *arr = val.array_val;
      size_t i, n = upb_array_size(arr);
      for (i = 0; i < n; i++) {
        upb_msgval elem = upb_array_get(arr, i);
        if (!_upb_msg_discardunknown((upb_msg*)elem.msg_val, subm, depth)) {
          ret = false;
        }
      }
    } else {
      if (!_upb_msg_discardunknown((upb_msg*)val.msg_val, subm, depth)) {
        ret = false;
      }
    }
  }

  return ret;
}

bool upb_msg_discardunknown(upb_msg *msg, const upb_msgdef *m, int maxdepth) {
  return _upb_msg_discardunknown(msg, m, maxdepth);
}

/** upb_array *****************************************************************/

upb_array *upb_array_new(upb_arena *a, upb_fieldtype_t type) {
  return _upb_array_new(a, 4, _upb_fieldtype_to_sizelg2[type]);
}

size_t upb_array_size(const upb_array *arr) {
  return arr->len;
}

upb_msgval upb_array_get(const upb_array *arr, size_t i) {
  upb_msgval ret;
  const char* data = _upb_array_constptr(arr);
  int lg2 = arr->data & 7;
  UPB_ASSERT(i < arr->len);
  memcpy(&ret, data + (i << lg2), 1 << lg2);
  return ret;
}

void upb_array_set(upb_array *arr, size_t i, upb_msgval val) {
  char* data = _upb_array_ptr(arr);
  int lg2 = arr->data & 7;
  UPB_ASSERT(i < arr->len);
  memcpy(data + (i << lg2), &val, 1 << lg2);
}

bool upb_array_append(upb_array *arr, upb_msgval val, upb_arena *arena) {
  if (!upb_array_resize(arr, arr->len + 1, arena)) {
    return false;
  }
  upb_array_set(arr, arr->len - 1, val);
  return true;
}

void upb_array_move(upb_array* arr, size_t dst_idx, size_t src_idx,
                    size_t count) {
  char* data = _upb_array_ptr(arr);
  int lg2 = arr->data & 7;
  memmove(&data[dst_idx << lg2], &data[src_idx << lg2], count << lg2);
}

bool upb_array_insert(upb_array *arr, size_t i, size_t count,
                      upb_arena *arena) {
  UPB_ASSERT(i <= arr->len);
  UPB_ASSERT(count + arr->len >= count);
  size_t oldsize = arr->len;
  if (!upb_array_resize(arr, arr->len + count, arena)) {
    return false;
  }
  upb_array_move(arr, i + count, i, oldsize - i);
  return true;
}

/*
 *              i        end      arr->len
 * |------------|XXXXXXXX|--------|
 */
void upb_array_delete(upb_array *arr, size_t i, size_t count) {
  size_t end = i + count;
  UPB_ASSERT(i <= end);
  UPB_ASSERT(end <= arr->len);
  upb_array_move(arr, i, end, arr->len - end);
  arr->len -= count;
}

bool upb_array_resize(upb_array *arr, size_t size, upb_arena *arena) {
  return _upb_array_resize(arr, size, arena);
}

/** upb_map *******************************************************************/

upb_map *upb_map_new(upb_arena *a, upb_fieldtype_t key_type,
                     upb_fieldtype_t value_type) {
  return _upb_map_new(a, _upb_fieldtype_to_mapsize[key_type],
                      _upb_fieldtype_to_mapsize[value_type]);
}

size_t upb_map_size(const upb_map *map) {
  return _upb_map_size(map);
}

bool upb_map_get(const upb_map *map, upb_msgval key, upb_msgval *val) {
  return _upb_map_get(map, &key, map->key_size, val, map->val_size);
}

void upb_map_clear(upb_map *map) {
  _upb_map_clear(map);
}

bool upb_map_set(upb_map *map, upb_msgval key, upb_msgval val,
                 upb_arena *arena) {
  return _upb_map_set(map, &key, map->key_size, &val, map->val_size, arena);
}

bool upb_map_delete(upb_map *map, upb_msgval key) {
  return _upb_map_delete(map, &key, map->key_size);
}

bool upb_mapiter_next(const upb_map *map, size_t *iter) {
  return _upb_map_next(map, iter);
}

bool upb_mapiter_done(const upb_map *map, size_t iter) {
  upb_strtable_iter i;
  UPB_ASSERT(iter != UPB_MAP_BEGIN);
  i.t = &map->table;
  i.index = iter;
  return upb_strtable_done(&i);
}

/* Returns the key and value for this entry of the map. */
upb_msgval upb_mapiter_key(const upb_map *map, size_t iter) {
  upb_strtable_iter i;
  upb_msgval ret;
  i.t = &map->table;
  i.index = iter;
  _upb_map_fromkey(upb_strtable_iter_key(&i), &ret, map->key_size);
  return ret;
}

upb_msgval upb_mapiter_value(const upb_map *map, size_t iter) {
  upb_strtable_iter i;
  upb_msgval ret;
  i.t = &map->table;
  i.index = iter;
  _upb_map_fromvalue(upb_strtable_iter_value(&i), &ret, map->val_size);
  return ret;
}

/* void upb_mapiter_setvalue(upb_map *map, size_t iter, upb_msgval value); */

/** third_party/utf8_range/utf8_range.c ************************************************************/
/*
 * http://www.unicode.org/versions/Unicode6.0.0/ch03.pdf - page 94
 *
 * Table 3-7. Well-Formed UTF-8 Byte Sequences
 *
 * +--------------------+------------+-------------+------------+-------------+
 * | Code Points        | First Byte | Second Byte | Third Byte | Fourth Byte |
 * +--------------------+------------+-------------+------------+-------------+
 * | U+0000..U+007F     | 00..7F     |             |            |             |
 * +--------------------+------------+-------------+------------+-------------+
 * | U+0080..U+07FF     | C2..DF     | 80..BF      |            |             |
 * +--------------------+------------+-------------+------------+-------------+
 * | U+0800..U+0FFF     | E0         | A0..BF      | 80..BF     |             |
 * +--------------------+------------+-------------+------------+-------------+
 * | U+1000..U+CFFF     | E1..EC     | 80..BF      | 80..BF     |             |
 * +--------------------+------------+-------------+------------+-------------+
 * | U+D000..U+D7FF     | ED         | 80..9F      | 80..BF     |             |
 * +--------------------+------------+-------------+------------+-------------+
 * | U+E000..U+FFFF     | EE..EF     | 80..BF      | 80..BF     |             |
 * +--------------------+------------+-------------+------------+-------------+
 * | U+10000..U+3FFFF   | F0         | 90..BF      | 80..BF     | 80..BF      |
 * +--------------------+------------+-------------+------------+-------------+
 * | U+40000..U+FFFFF   | F1..F3     | 80..BF      | 80..BF     | 80..BF      |
 * +--------------------+------------+-------------+------------+-------------+
 * | U+100000..U+10FFFF | F4         | 80..8F      | 80..BF     | 80..BF      |
 * +--------------------+------------+-------------+------------+-------------+
 */

/* Return 0 - success,  >0 - index(1 based) of first error char */
int utf8_naive(const unsigned char *data, int len)
{
    int err_pos = 1;

    while (len) {
        int bytes;
        const unsigned char byte1 = data[0];

        /* 00..7F */
        if (byte1 <= 0x7F) {
            bytes = 1;
        /* C2..DF, 80..BF */
        } else if (len >= 2 && byte1 >= 0xC2 && byte1 <= 0xDF &&
                (signed char)data[1] <= (signed char)0xBF) {
            bytes = 2;
        } else if (len >= 3) {
            const unsigned char byte2 = data[1];

            /* Is byte2, byte3 between 0x80 ~ 0xBF */
            const int byte2_ok = (signed char)byte2 <= (signed char)0xBF;
            const int byte3_ok = (signed char)data[2] <= (signed char)0xBF;

            if (byte2_ok && byte3_ok &&
                     /* E0, A0..BF, 80..BF */
                    ((byte1 == 0xE0 && byte2 >= 0xA0) ||
                     /* E1..EC, 80..BF, 80..BF */
                     (byte1 >= 0xE1 && byte1 <= 0xEC) ||
                     /* ED, 80..9F, 80..BF */
                     (byte1 == 0xED && byte2 <= 0x9F) ||
                     /* EE..EF, 80..BF, 80..BF */
                     (byte1 >= 0xEE && byte1 <= 0xEF))) {
                bytes = 3;
            } else if (len >= 4) {
                /* Is byte4 between 0x80 ~ 0xBF */
                const int byte4_ok = (signed char)data[3] <= (signed char)0xBF;

                if (byte2_ok && byte3_ok && byte4_ok &&
                         /* F0, 90..BF, 80..BF, 80..BF */
                        ((byte1 == 0xF0 && byte2 >= 0x90) ||
                         /* F1..F3, 80..BF, 80..BF, 80..BF */
                         (byte1 >= 0xF1 && byte1 <= 0xF3) ||
                         /* F4, 80..8F, 80..BF, 80..BF */
                         (byte1 == 0xF4 && byte2 <= 0x8F))) {
                    bytes = 4;
                } else {
                    return err_pos;
                }
            } else {
                return err_pos;
            }
        } else {
            return err_pos;
        }

        len -= bytes;
        err_pos += bytes;
        data += bytes;
    }

    return 0;
}

#ifdef __SSE4_1__

#include <stdio.h>
#include <stdint.h>
#include <x86intrin.h>

int utf8_naive(const unsigned char *data, int len);

static const int8_t _first_len_tbl[] = {
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 2, 3,
};

static const int8_t _first_range_tbl[] = {
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 8, 8, 8, 8,
};

static const int8_t _range_min_tbl[] = {
    0x00, 0x80, 0x80, 0x80, 0xA0, 0x80, 0x90, 0x80,
    0xC2, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F,
};
static const int8_t _range_max_tbl[] = {
    0x7F, 0xBF, 0xBF, 0xBF, 0xBF, 0x9F, 0xBF, 0x8F,
    0xF4, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80,
};

static const int8_t _df_ee_tbl[] = {
    0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 3, 0,
};
static const int8_t _ef_fe_tbl[] = {
    0, 3, 0, 0, 0, 4, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
};

/* Return 0 on success, -1 on error */
int utf8_range2(const unsigned char *data, int len)
{
    if (len >= 32) {
        __m128i prev_input = _mm_set1_epi8(0);
        __m128i prev_first_len = _mm_set1_epi8(0);

        const __m128i first_len_tbl =
            _mm_loadu_si128((const __m128i *)_first_len_tbl);
        const __m128i first_range_tbl =
            _mm_loadu_si128((const __m128i *)_first_range_tbl);
        const __m128i range_min_tbl =
            _mm_loadu_si128((const __m128i *)_range_min_tbl);
        const __m128i range_max_tbl =
            _mm_loadu_si128((const __m128i *)_range_max_tbl);
        const __m128i df_ee_tbl =
            _mm_loadu_si128((const __m128i *)_df_ee_tbl);
        const __m128i ef_fe_tbl =
            _mm_loadu_si128((const __m128i *)_ef_fe_tbl);

        __m128i error = _mm_set1_epi8(0);

        while (len >= 32) {
            /***************************** block 1 ****************************/
            const __m128i input_a = _mm_loadu_si128((const __m128i *)data);

            __m128i high_nibbles =
                _mm_and_si128(_mm_srli_epi16(input_a, 4), _mm_set1_epi8(0x0F));

            __m128i first_len_a = _mm_shuffle_epi8(first_len_tbl, high_nibbles);

            __m128i range_a = _mm_shuffle_epi8(first_range_tbl, high_nibbles);

            range_a = _mm_or_si128(
                    range_a, _mm_alignr_epi8(first_len_a, prev_first_len, 15));

            __m128i tmp;
            tmp = _mm_alignr_epi8(first_len_a, prev_first_len, 14);
            tmp = _mm_subs_epu8(tmp, _mm_set1_epi8(1));
            range_a = _mm_or_si128(range_a, tmp);

            tmp = _mm_alignr_epi8(first_len_a, prev_first_len, 13);
            tmp = _mm_subs_epu8(tmp, _mm_set1_epi8(2));
            range_a = _mm_or_si128(range_a, tmp);

            __m128i shift1, pos, range2;
            shift1 = _mm_alignr_epi8(input_a, prev_input, 15);
            pos = _mm_sub_epi8(shift1, _mm_set1_epi8(0xEF));
            tmp = _mm_subs_epu8(pos, _mm_set1_epi8(0xF0));
            range2 = _mm_shuffle_epi8(df_ee_tbl, tmp);
            tmp = _mm_adds_epu8(pos, _mm_set1_epi8(0x70));
            range2 = _mm_add_epi8(range2, _mm_shuffle_epi8(ef_fe_tbl, tmp));

            range_a = _mm_add_epi8(range_a, range2);

            __m128i minv = _mm_shuffle_epi8(range_min_tbl, range_a);
            __m128i maxv = _mm_shuffle_epi8(range_max_tbl, range_a);

            tmp = _mm_or_si128(
                      _mm_cmplt_epi8(input_a, minv),
                      _mm_cmpgt_epi8(input_a, maxv)
                  );
            error = _mm_or_si128(error, tmp);

            /***************************** block 2 ****************************/
            const __m128i input_b = _mm_loadu_si128((const __m128i *)(data+16));

            high_nibbles =
                _mm_and_si128(_mm_srli_epi16(input_b, 4), _mm_set1_epi8(0x0F));

            __m128i first_len_b = _mm_shuffle_epi8(first_len_tbl, high_nibbles);

            __m128i range_b = _mm_shuffle_epi8(first_range_tbl, high_nibbles);

            range_b = _mm_or_si128(
                    range_b, _mm_alignr_epi8(first_len_b, first_len_a, 15));


            tmp = _mm_alignr_epi8(first_len_b, first_len_a, 14);
            tmp = _mm_subs_epu8(tmp, _mm_set1_epi8(1));
            range_b = _mm_or_si128(range_b, tmp);

            tmp = _mm_alignr_epi8(first_len_b, first_len_a, 13);
            tmp = _mm_subs_epu8(tmp, _mm_set1_epi8(2));
            range_b = _mm_or_si128(range_b, tmp);

            shift1 = _mm_alignr_epi8(input_b, input_a, 15);
            pos = _mm_sub_epi8(shift1, _mm_set1_epi8(0xEF));
            tmp = _mm_subs_epu8(pos, _mm_set1_epi8(0xF0));
            range2 = _mm_shuffle_epi8(df_ee_tbl, tmp);
            tmp = _mm_adds_epu8(pos, _mm_set1_epi8(0x70));
            range2 = _mm_add_epi8(range2, _mm_shuffle_epi8(ef_fe_tbl, tmp));

            range_b = _mm_add_epi8(range_b, range2);

            minv = _mm_shuffle_epi8(range_min_tbl, range_b);
            maxv = _mm_shuffle_epi8(range_max_tbl, range_b);


            tmp = _mm_or_si128(
                      _mm_cmplt_epi8(input_b, minv),
                      _mm_cmpgt_epi8(input_b, maxv)
                  );
            error = _mm_or_si128(error, tmp);

            /************************ next iteration **************************/
            prev_input = input_b;
            prev_first_len = first_len_b;

            data += 32;
            len -= 32;
        }

        if (!_mm_testz_si128(error, error))
            return -1;

        int32_t token4 = _mm_extract_epi32(prev_input, 3);
        const int8_t *token = (const int8_t *)&token4;
        int lookahead = 0;
        if (token[3] > (int8_t)0xBF)
            lookahead = 1;
        else if (token[2] > (int8_t)0xBF)
            lookahead = 2;
        else if (token[1] > (int8_t)0xBF)
            lookahead = 3;

        data -= lookahead;
        len += lookahead;
    }

    return utf8_naive(data, len);
}

#endif

#ifdef __ARM_NEON

#include <stdio.h>
#include <stdint.h>
#include <arm_neon.h>

int utf8_naive(const unsigned char *data, int len);

static const uint8_t _first_len_tbl[] = {
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 2, 3,
};

static const uint8_t _first_range_tbl[] = {
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 8, 8, 8, 8,
};

static const uint8_t _range_min_tbl[] = {
    0x00, 0x80, 0x80, 0x80, 0xA0, 0x80, 0x90, 0x80,
    0xC2, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
};
static const uint8_t _range_max_tbl[] = {
    0x7F, 0xBF, 0xBF, 0xBF, 0xBF, 0x9F, 0xBF, 0x8F,
    0xF4, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
};

static const uint8_t _range_adjust_tbl[] = {
    2, 3, 0, 0, 0, 0, 0, 0, 0, 4, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 3, 0, 0, 0, 0, 0,
};

/* Return 0 on success, -1 on error */
int utf8_range2(const unsigned char *data, int len)
{
    if (len >= 32) {
        uint8x16_t prev_input = vdupq_n_u8(0);
        uint8x16_t prev_first_len = vdupq_n_u8(0);

        const uint8x16_t first_len_tbl = vld1q_u8(_first_len_tbl);
        const uint8x16_t first_range_tbl = vld1q_u8(_first_range_tbl);
        const uint8x16_t range_min_tbl = vld1q_u8(_range_min_tbl);
        const uint8x16_t range_max_tbl = vld1q_u8(_range_max_tbl);
        const uint8x16x2_t range_adjust_tbl = vld2q_u8(_range_adjust_tbl);

        const uint8x16_t const_1 = vdupq_n_u8(1);
        const uint8x16_t const_2 = vdupq_n_u8(2);
        const uint8x16_t const_e0 = vdupq_n_u8(0xE0);

        uint8x16_t error1 = vdupq_n_u8(0);
        uint8x16_t error2 = vdupq_n_u8(0);
        uint8x16_t error3 = vdupq_n_u8(0);
        uint8x16_t error4 = vdupq_n_u8(0);

        while (len >= 32) {
            /******************* two blocks interleaved **********************/

#if defined(__GNUC__) && !defined(__clang__) && (__GNUC__ < 8)
            /* gcc doesn't support vldq1_u8_x2 until version 8 */
            const uint8x16_t input_a = vld1q_u8(data);
            const uint8x16_t input_b = vld1q_u8(data + 16);
#else
            /* Forces a double load on Clang */
            const uint8x16x2_t input_pair = vld1q_u8_x2(data);
            const uint8x16_t input_a = input_pair.val[0];
            const uint8x16_t input_b = input_pair.val[1];
#endif

            const uint8x16_t high_nibbles_a = vshrq_n_u8(input_a, 4);
            const uint8x16_t high_nibbles_b = vshrq_n_u8(input_b, 4);

            const uint8x16_t first_len_a =
                vqtbl1q_u8(first_len_tbl, high_nibbles_a);
            const uint8x16_t first_len_b =
                vqtbl1q_u8(first_len_tbl, high_nibbles_b);

            uint8x16_t range_a = vqtbl1q_u8(first_range_tbl, high_nibbles_a);
            uint8x16_t range_b = vqtbl1q_u8(first_range_tbl, high_nibbles_b);

            range_a =
                vorrq_u8(range_a, vextq_u8(prev_first_len, first_len_a, 15));
            range_b =
                vorrq_u8(range_b, vextq_u8(first_len_a, first_len_b, 15));

            uint8x16_t tmp1_a, tmp2_a, tmp1_b, tmp2_b;
            tmp1_a = vextq_u8(prev_first_len, first_len_a, 14);
            tmp1_a = vqsubq_u8(tmp1_a, const_1);
            range_a = vorrq_u8(range_a, tmp1_a);

            tmp1_b = vextq_u8(first_len_a, first_len_b, 14);
            tmp1_b = vqsubq_u8(tmp1_b, const_1);
            range_b = vorrq_u8(range_b, tmp1_b);

            tmp2_a = vextq_u8(prev_first_len, first_len_a, 13);
            tmp2_a = vqsubq_u8(tmp2_a, const_2);
            range_a = vorrq_u8(range_a, tmp2_a);

            tmp2_b = vextq_u8(first_len_a, first_len_b, 13);
            tmp2_b = vqsubq_u8(tmp2_b, const_2);
            range_b = vorrq_u8(range_b, tmp2_b);

            uint8x16_t shift1_a = vextq_u8(prev_input, input_a, 15);
            uint8x16_t pos_a = vsubq_u8(shift1_a, const_e0);
            range_a = vaddq_u8(range_a, vqtbl2q_u8(range_adjust_tbl, pos_a));

            uint8x16_t shift1_b = vextq_u8(input_a, input_b, 15);
            uint8x16_t pos_b = vsubq_u8(shift1_b, const_e0);
            range_b = vaddq_u8(range_b, vqtbl2q_u8(range_adjust_tbl, pos_b));

            uint8x16_t minv_a = vqtbl1q_u8(range_min_tbl, range_a);
            uint8x16_t maxv_a = vqtbl1q_u8(range_max_tbl, range_a);

            uint8x16_t minv_b = vqtbl1q_u8(range_min_tbl, range_b);
            uint8x16_t maxv_b = vqtbl1q_u8(range_max_tbl, range_b);

            error1 = vorrq_u8(error1, vcltq_u8(input_a, minv_a));
            error2 = vorrq_u8(error2, vcgtq_u8(input_a, maxv_a));

            error3 = vorrq_u8(error3, vcltq_u8(input_b, minv_b));
            error4 = vorrq_u8(error4, vcgtq_u8(input_b, maxv_b));

            /************************ next iteration *************************/
            prev_input = input_b;
            prev_first_len = first_len_b;

            data += 32;
            len -= 32;
        }
        error1 = vorrq_u8(error1, error2);
        error1 = vorrq_u8(error1, error3);
        error1 = vorrq_u8(error1, error4);

        if (vmaxvq_u8(error1))
            return -1;

        uint32_t token4;
        vst1q_lane_u32(&token4, vreinterpretq_u32_u8(prev_input), 3);

        const int8_t *token = (const int8_t *)&token4;
        int lookahead = 0;
        if (token[3] > (int8_t)0xBF)
            lookahead = 1;
        else if (token[2] > (int8_t)0xBF)
            lookahead = 2;
        else if (token[1] > (int8_t)0xBF)
            lookahead = 3;

        data -= lookahead;
        len += lookahead;
    }

    return utf8_naive(data, len);
}

#endif

/** upb/port_undef.inc ************************************************************/
/* See port_def.inc.  This should #undef all macros #defined there. */

#undef UPB_SIZE
#undef UPB_PTR_AT
#undef UPB_READ_ONEOF
#undef UPB_WRITE_ONEOF
#undef UPB_MAPTYPE_STRING
#undef UPB_INLINE
#undef UPB_ALIGN_UP
#undef UPB_ALIGN_DOWN
#undef UPB_ALIGN_MALLOC
#undef UPB_ALIGN_OF
#undef UPB_LIKELY
#undef UPB_UNLIKELY
#undef UPB_FORCEINLINE
#undef UPB_NOINLINE
#undef UPB_NORETURN
#undef UPB_PRINTF
#undef UPB_MAX
#undef UPB_MIN
#undef UPB_UNUSED
#undef UPB_ASSUME
#undef UPB_ASSERT
#undef UPB_UNREACHABLE
#undef UPB_SETJMP
#undef UPB_LONGJMP
#undef UPB_PTRADD
#undef UPB_MUSTTAIL
#undef UPB_FASTTABLE_SUPPORTED
#undef UPB_FASTTABLE
#undef UPB_FASTTABLE_INIT
#undef UPB_POISON_MEMORY_REGION
#undef UPB_UNPOISON_MEMORY_REGION
#undef UPB_ASAN