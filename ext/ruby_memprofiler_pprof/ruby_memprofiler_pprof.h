#ifndef __RUBY_MEMPROFILER_PPROF_H
#define __RUBY_MEMPROFILER_PPROF_H

#include <pthread.h>
#include <stdint.h>

#include <ruby.h>

// UPB header files trip up a BUNCH of -Wshorten-64-to-32
// Also ignore -Wpragmas so that if -Wshorten-64-to-32 isn't present
// (it's a clang only thing), GCC doesn't warn about the unknown warning.
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpragmas"
#pragma GCC diagnostic ignored "-Wshorten-64-to-32"
#include <upb/upb.h>
#include "pprof.upb.h"
#pragma GCC diagnostic pop

// ======== COMPAT DECLARATIONS ========

// For handling differences in ruby versions
#ifndef HAVE_RB_GC_MARK_MOVABLE
#define rb_gc_mark_movable(v) rb_gc_mark(v)
#endif

#ifndef RB_PASS_KEYWORDS
#define RB_SCAN_ARGS_LAST_HASH_KEYWORDS 3
#define rb_scan_args_kw(kw, c, v, s, ...) rb_scan_args(c, v, s, __VA_ARGS__)
#endif

#ifndef HAVE_RB_EXT_RACTOR_SAFE
#define rb_ext_ractor_safe(x) do {} while (0)
#endif

// Apparently "I just want a random number, without thinking about whether it's
// threadsafe, without thinking about whether some other part of the process needs
// the global seed to be set to some deterministic value, and without calling into
// the kernel every time" is... too much to ask for.
// BSD has arc4random(3) for this, but for glibc we have to use one of the threadsafe
// RNG's and seed a global instance of it, guarded by a mutex....
// These methods wrap all that rubbish up.
uint32_t mpp_rand();
void mpp_rand_init();


// These declarations just wrap some things from the standard library that should "always succeed", but call
// our assertion macro if they fail to abort the program.
void *mpp_xmalloc(size_t sz);
void *mpp_realloc(void *mem, size_t newsz);
void mpp_free(void *mem);
void mpp_pthread_mutex_lock(pthread_mutex_t *m);
void mpp_pthread_mutex_unlock(pthread_mutex_t *m);
int mpp_pthread_mutex_trylock(pthread_mutex_t *m);
void mpp_pthread_mutex_init(pthread_mutex_t *m, const pthread_mutexattr_t *attr);
void mpp_pthread_mutex_destroy(pthread_mutex_t *m);
void mpp_pthread_mutexattr_init(pthread_mutexattr_t *a);
void mpp_pthread_mutexattr_destroy(pthread_mutexattr_t *a);
void mpp_pthread_mutexattr_settype(pthread_mutexattr_t *a, int type);

// Need a handy assertion macro. It would be nice to re-use rb_bug for some of this, but that actually
// requires the GVL (it walks the Ruby stack frames, for one) and we (want to) run some code outside
// the GVL, so this assertion macro has to be threadsafe. So we just implement it in pretty much a
// similar way to how stdlib's assert() works (plus some stuff to prefix the gem name to the abort message
// so users know who is at fault).
#define MPP_ASSERT_STRINGIFY1(x) #x
#define MPP_ASSERT_STRINGIFY2(x) MPP_ASSERT_STRINGIFY1(x)
#define MPP_ASSERT__LINE MPP_ASSERT_STRINGIFY2(__LINE__)
__attribute__ ((noreturn))
void mpp_assert_fail(const char *msg, const char *assertion, const char *file, const char *line, const char *fn);
#define MPP_ASSERT_MSG(expr, msg)                                                   \
    do {                                                                            \
        if ((expr) == 0) {                                                          \
            mpp_assert_fail((msg), #expr, __FILE__, MPP_ASSERT__LINE, __func__);    \
        }                                                                           \
    } while (0)
#define MPP_ASSERT_FAIL(expr) MPP_ASSERT_MSG(expr, 0)

// ======== STRTAB DECLARATIONS ========

#define MPP_STRTAB_USE_STRLEN (-1)
#define MPP_STRTAB_UNKNOWN_LITERAL "(unknown)"
#define MPP_STRTAB_UNKNOWN_LITERAL_LEN ((int)strlen(MPP_STRTAB_UNKNOWN_LITERAL))

// Specialisation of the st_hash for "strings with length".
struct mpp_strtab_key {
    const char *str;    // Pointer to the string; n.b. it does NOT need a null-terminator
    size_t str_len;     // Size of the string, not including any null terminator.
};

// I copied this magic number out of st.c from Ruby.
#define FNV1_32A_INIT 0x811c9dc5

struct mpp_strtab_el {
    // Pointer to null-terminated string that has been interned
    char *str;
    // Length of str, NOT INCLUDING the null terminator
    size_t str_len;
    // Number of times this string has been interned. When the refcount drops to zero, the string
    // is removed from the table and str is free'd.
    uint64_t refcount;
    // We cleverly keep the key _inside_ the value, so we don't need a separate bunch of malloc'd
    // memory for each _key_ as well as each _value_.
    struct mpp_strtab_key key;
};

struct mpp_strtab {
    // The actual table which contains a mapping of (string hash) -> (str_intern_tab_el *)
    st_table *table;
    // Number of entries in table.
    int64_t table_count;
    // (approximate) allocated size of the _entries_ in the st_table (but not the st_table
    // itself).
    size_t table_entry_size;
    // Value of "" interned in the table.
    const char *interned_empty_str;
};

struct mpp_strtab_index {
    // The table this index is a part of and came from.
    struct mpp_strtab *tab;
    // The list & length of interned strings.
    struct mpp_strtab_el **str_list;
    int64_t str_list_len;
    // This st_hash is used to convert already-interned pointers to index into str_list.
    // It's a map of (uintptr_t) -> (int64_t)
    st_table *pos_table;
};


// NOTE - there's better documentation of what these methods do in strtab.c itself.

// Initializes a new, empty string intern table. This will allocate memory that remains owned by the strtab
// module and saves it in tab. It also allocates memory for struct intern_tab itself.
struct mpp_strtab *mpp_strtab_new();

// Destroys a string intern table, including freeing the underlying memory used by tab, and freeing
// the memory pointed to by tab itself.
void mpp_strtab_destroy(struct mpp_strtab *tab);

// Get the size of all memory used by the table
size_t mpp_strtab_memsize(struct mpp_strtab *tab);

// Intern new strings (or increment the refcount of already-interned ones)
void mpp_strtab_intern(
    struct mpp_strtab *tab, const char *str, int str_len,
    const char **interned_str_out, size_t *interned_str_len_out
);
void mpp_strtab_intern_rbstr(
    struct mpp_strtab *tab, VALUE rbstr,
    const char **interned_str_out, size_t *interned_str_len_out
);
void mpp_strtab_intern_cstr(
    struct mpp_strtab *tab, const char *str,
    const char **interned_str_out, size_t *interned_str_len_out
);

// Decrement the refcount of elements in the intern table.
void mpp_strtab_release(struct mpp_strtab *tab, const char *str, size_t str_len);
void mpp_strtab_release_rbstr(struct mpp_strtab *tab, VALUE rbstr);

// Methods for building a zero=-based list of interned pointers, for building the final string table
// in the pprof protobuf.
struct mpp_strtab_index *mpp_strtab_index(struct mpp_strtab *tab);
void mpp_strtab_index_destroy(struct mpp_strtab_index *ix);
int64_t mpp_strtab_index_of(struct mpp_strtab_index *ix, const char *interned_ptr);
typedef void (*mpp_strtab_each_fn)(int64_t el_ix, const char *interned_str, size_t interned_str_len, void *ctx);
void mpp_strtab_each(struct mpp_strtab_index *ix, mpp_strtab_each_fn fn, void *ctx);

// ======== BACKTRACE DECLARATIONS ========
struct mpp_rb_backtrace {
    // The array of frames - most recent call FIRST.
    uint64_t *frame_locations;
    int64_t frames_count;
    // Memory size of frames, which might actually be bigger than
    // sizeof(frame_locations[0]) * frames_count
    size_t memsize;
};

struct mpp_rb_loctab_function {
    // Refcount
    int refcount;
    // Interned pointer to function name
    const char *function_name;
    size_t function_name_len;
    // Interned pointer to file name
    const char *file_name;
    size_t file_name_len;
    // Line number where the function starts
    int64_t line_number;
    // Function ID of self
    uint64_t id;
};

struct mpp_rb_loctab_location {
    // Refcount
    int refcount;
    // Function & line number. Note that we do _NOT_ hold a reference
    // to the function here. Instead, each backtrace frame holds a reference
    // to both the function and the location.
    struct mpp_rb_loctab_function *function;
    // Line number
    int64_t line_number;
    // Location id of self
    uint64_t id;
};

struct mpp_rb_loctab {
    // The table of (uint64_t location_id) -> (struct mpp_rb_loctab_location *)
    st_table *locations;
    int64_t location_count;
    // The table of (uint64_t function_id) -> (struct mpp_rb_locatab_function *)
    st_table *functions;
    int64_t function_count;
    // The string interning table that all the names are related to
    struct mpp_strtab *strtab;
};

struct mpp_rb_loctab *mpp_rb_loctab_new(struct mpp_strtab *strtab);
void mpp_rb_loctab_destroy(struct mpp_rb_loctab *loctab);
void mpp_rb_backtrace_capture(struct mpp_rb_loctab *locatb, struct mpp_rb_backtrace **bt_out);
void mpp_rb_backtrace_destroy(struct mpp_rb_loctab *locatb, struct mpp_rb_backtrace *bt);
size_t mpp_rb_backtrace_memsize(struct mpp_rb_backtrace *bt);
size_t mpp_rb_loctab_memsize(struct mpp_rb_loctab *loctab);
typedef int (*mpp_rb_loctab_each_location_cb)(struct mpp_rb_loctab *loctab, struct mpp_rb_loctab_location *loc, void *ctx);
void mpp_rb_loctab_each_location(struct mpp_rb_loctab *loctab, mpp_rb_loctab_each_location_cb cb, void *ctx);
typedef int (*mpp_rb_loctab_each_function_cb)(struct mpp_rb_loctab *loctab, struct mpp_rb_loctab_function *loc, void *ctx);
void mpp_rb_loctab_each_function(struct mpp_rb_loctab *loctab, mpp_rb_loctab_each_function_cb cb, void *ctx);


// ======= MAIN DATA STRUCTURE DECLARATIONS ========
struct mpp_sample {
    // The backtrace for this sample
    struct mpp_rb_backtrace *bt;
    // Sample has a refcount - because it's used both in the heap profiling and in the allocation profiling.
    int64_t refcount;
    // How big this allocation was.
    size_t allocation_size;
    // Weak reference to what was allocated. Validate that it's alive by consulting the live object table first.
    VALUE allocated_value_weak;
    // Next element in the allocation profiling sample list. DO NOT use this in the heap profiling table.
    struct mpp_sample *next_alloc;
};

// ======== PROTO SERIALIZATION ROUTINES ========
struct mpp_pprof_serctx {
    // Defines the allocation routine & memory arena used by this serialisation context. When the ctx
    // is destroyed, we free the entire arena, so no other (protobuf) memory needs to be individually
    // freed.
    upb_alloc allocator;
    upb_arena *arena;
    // Location table used for looking up fucntion & location IDs to strings.
    struct mpp_rb_loctab *loctab;
    // String intern index; recall that holding this object does _not_ require that we have exclusive
    // use of the underlying string intern table, so it's safe for us to use this in a separate thread.
    struct mpp_strtab_index *strindex;
    // The protobuf representation we are building up.
    perftools_profiles_Profile *profile_proto;

    // We need to keep interned copies of some strings that will wind up in the protobuf output.
    // This is so that we can put constant values like "allocations" and "count" into our pprof output
    // (the pprof format requires these strings to be in the string table along with the rest of them)
    const char *internstr_allocations;
    const char *internstr_count;
    const char *internstr_size;
    const char *internstr_bytes;
};

struct mpp_pprof_serctx *mpp_pprof_serctx_new();
void mpp_pprof_serctx_destroy(struct mpp_pprof_serctx *ctx);
int mpp_pprof_serctx_set_loctab(struct mpp_pprof_serctx *ctx, struct mpp_rb_loctab *loctab, char *errbuf, size_t errbuflen);
int mpp_pprof_serctx_add_sample(struct mpp_pprof_serctx *ctx, struct mpp_sample *sample, char *errbuf, size_t errbuflen);
int mpp_pprof_serctx_serialize(struct mpp_pprof_serctx *ctx, char **buf_out, size_t *buflen_out, char *errbuf, size_t errbuflen);

// ======== COLLECTOR RUBY CLASS ========
extern VALUE mMemprofilerPprof;
extern VALUE cCollector;
void mpp_setup_collector_class();

#endif
