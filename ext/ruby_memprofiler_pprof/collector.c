#include <pthread.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include <ruby.h>
#include <ruby/debug.h>
#include <vm_core.h>
#include <iseq.h>

#include "ruby_memprofiler_pprof.h"

struct collector_cdata {
    // Internal, cross-ractor lock for this data
    pthread_mutex_t lock;

    // Global variables we need to keep a hold of
    VALUE cCollector;
    VALUE cProfileData;
    VALUE mMemprofilerPprof;

    // Ruby Tracepoint objects for our hooks
    VALUE newobj_trace;
    VALUE freeobj_trace;
    VALUE creturn_trace;

    // How often (as a fraction of UINT32_MAX) we should sample allocations;
    // Must be accessed through atomics
    uint32_t u32_sample_rate;
    // How often (as a fraction of UINT32_MAX) we should retain allocations, to profile allocations
    // as well as just heap usage.
    // Does _NOT_ need to be accessed through atomics.
    uint32_t u32_allocation_retain_rate;
    // This flag is used to make sure we detach our tracepoints as we're getting GC'd.
    bool is_tracing;

    // ======== Allocation samples ========
    // A linked list of samples, added to each time memory is allocated and cleared when
    // #flush is called.
    struct mpp_sample *allocation_samples;
    // Number of elements currently in the list
    int64_t allocation_samples_count;
    // How big the linked list can grow
    int64_t max_allocation_samples;
    // When objects are first allocated, we won't actually know their _real_ size; the object is not
    // in a state where calling rb_obj_memsize_of() on it is well-defined. Among other things, if the
    // object is T_CLASS, the ivar table won't be initialized yet, and trying to get its size will crash.
    // Even if it _did_ work (which it did, in versions of Ruby before variable-sized RValues), calling
    // rb_obj_memsize_of() will return sizeof(RVALUE). If e.g. a T_STRING is being allocated,
    // the heap memory for that is actually only allocated _after_ the newobj tracepoint fires.
    //
    // To make sure we can see the "real" size of these, we add a tracepoint on CRETURN. When that hook
    // fires, we check the size of all (still-live) objects recently allocated, and store _that_ as
    // the allocation size. This works well for T_STRING, T_DATA, T_STRUCT's etc that are allocated
    // inside C and then immediately filled; the correct memsize will be recorded on them before the
    // Ruby backtrace even changes.
    // This counter therefore keeps track of how many elements of *allocation_samples have yet to have
    // this hook called on them.
    int64_t pending_size_count;

    // ======== Heap samples ========
    // A hash-table keying live VALUEs to their allocation sample. This is _not_ cleared
    // when #flush is called; instead, elements are deleted when they are free'd. This is
    // used for building heap profiles.
    st_table *heap_samples;
    // Number of elements currently in the heap profile hash
    int64_t heap_samples_count;
    // How big the sample table can grow
    int64_t max_heap_samples;

    // ======== Sample drop counters ========
    // These are all accessed via atomics; how else would we have a counter for how often we failed
    // to acquire the lock?
    //
    // Number of samples dropped for want of obtaining the lock.
    int64_t dropped_samples_nolock;
    // Number of samples dropped for want of space in the allocation buffer.
    int64_t dropped_samples_allocation_bufsize;
    // Number of samples dropped for want of space in the heap allocation table.
    int64_t dropped_samples_heap_bufsize;

    // String interning table used to keep constant pointers to every string; this saves memory
    // used in backtraces, and also helps us efficiently build up the pprof protobuf format (since that
    // _requires_ that strings are interned in a string table).
    struct mpp_strtab *string_tab;
    // Same thing, but for backtrace locations.
    struct mpp_rb_loctab *loctab;

    // Which method to use for getting backtraces
    int bt_method;

    // This we need to know so we can at least give a non-zero size for new objects.
    size_t rvalue_size;
};

// We need a global list of all collectors, so that, in our atfork handler, we can correctly lock/unlock
// all of their mutexes and guarantee correctness across forks.
static st_table *global_collectors;
static pthread_mutex_t global_collectors_lock;

static void internal_sample_decrement_refcount(struct collector_cdata *cd, struct mpp_sample *s) {
    s->refcount--;
    if (!s->refcount) {
        mpp_rb_backtrace_destroy(cd->loctab, s->bt);
        mpp_free(s);
    }
}

static int collector_cdata_gc_decrement_live_object_refcounts(st_data_t key, st_data_t value, st_data_t arg) {
    struct mpp_sample *s = (struct mpp_sample *)value;
    struct collector_cdata *cd = (struct collector_cdata *)arg;
    internal_sample_decrement_refcount(cd, s);
    return ST_CONTINUE;
}

static void collector_cdata_gc_free_heap_samples(struct collector_cdata *cd) {
    if (cd->heap_samples) {
        st_foreach(cd->heap_samples, collector_cdata_gc_decrement_live_object_refcounts, (st_data_t)cd);
        st_free_table(cd->heap_samples);
    }
    cd->heap_samples = NULL;
}

static void internal_sample_list_decrement_refcount(struct collector_cdata *cd, struct mpp_sample *s) {
    while (s) {
        struct mpp_sample *next_s = s->next_alloc;
        internal_sample_decrement_refcount(cd, s);
        s = next_s;
    }
}

static void collector_cdata_gc_free_allocation_samples(struct collector_cdata *cd) {
    internal_sample_list_decrement_refcount(cd, cd->allocation_samples);
    cd->allocation_samples = NULL;
}

static int collector_cdata_gc_memsize_live_objects(st_data_t key, st_data_t value, st_data_t arg) {
    size_t *acc_ptr = (size_t *)arg;
    struct mpp_sample *s = (struct mpp_sample *)value;

    // Only consider the live object list to be holding the backtrace, for accounting purposes, if it's
    // not also in the allocation sample list.
    if (s->refcount == 1) {
        *acc_ptr += sizeof(*s);
        *acc_ptr += mpp_rb_backtrace_memsize(s->bt);
    }
    return ST_CONTINUE;
}

static void collector_cdata_gc_free_loctab(struct collector_cdata *cd) {
    if (cd->loctab) {
        mpp_rb_loctab_destroy(cd->loctab);
    }
}

static void collector_cdata_gc_free_strtab(struct collector_cdata *cd) {
    if (cd->string_tab) {
        mpp_strtab_destroy(cd->string_tab);
    }
}

static void collector_cdata_gc_mark(void *ptr) {
    struct collector_cdata *cd = (struct collector_cdata *)ptr;
    rb_gc_mark_movable(cd->newobj_trace);
    rb_gc_mark_movable(cd->freeobj_trace);
    rb_gc_mark_movable(cd->creturn_trace);
    rb_gc_mark_movable(cd->mMemprofilerPprof);
    rb_gc_mark_movable(cd->cCollector);
    rb_gc_mark_movable(cd->cProfileData);
}

static void collector_cdata_gc_free(void *ptr) {
    struct collector_cdata *cd = (struct collector_cdata *)ptr;
    if (cd->is_tracing) {
        if (cd->newobj_trace) {
            rb_tracepoint_disable(cd->newobj_trace);
        }
        if (cd->freeobj_trace) {
            rb_tracepoint_disable(cd->freeobj_trace);
        }
    }

    // Needed in case there are any in-flight tracepoints we just disabled above.
    mpp_pthread_mutex_lock(&cd->lock);

    collector_cdata_gc_free_heap_samples(cd);
    collector_cdata_gc_free_allocation_samples(cd);
    collector_cdata_gc_free_loctab(cd);
    collector_cdata_gc_free_strtab(cd);

    // Remove from global collectors list.
    mpp_pthread_mutex_lock(&global_collectors_lock);
    st_data_t cd_key = (st_data_t)cd;
    st_delete(global_collectors, &cd_key, NULL);
    mpp_pthread_mutex_unlock(&global_collectors_lock);

    mpp_pthread_mutex_unlock(&cd->lock);
    mpp_pthread_mutex_destroy(&cd->lock);

    ruby_xfree(ptr);
}

static size_t collector_cdata_memsize(const void *ptr) {
    struct collector_cdata *cd = (struct collector_cdata *)ptr;
    size_t sz = sizeof(*cd);
    if (cd->heap_samples) {
        st_foreach(cd->heap_samples, collector_cdata_gc_memsize_live_objects, (st_data_t)&sz);
        sz += st_memsize(cd->heap_samples);
    }
    if (cd->string_tab) {
        sz += mpp_strtab_memsize(cd->string_tab);
    }
    if (cd->loctab) {
        sz += mpp_rb_loctab_memsize(cd->loctab);
    }
    struct mpp_sample *s = cd->allocation_samples;
    while (s) {
        sz += sizeof(*s);
        sz += mpp_rb_backtrace_memsize(s->bt);
        s = s->next_alloc;
    }

    return sz;
}

#ifdef HAVE_RB_GC_MARK_MOVABLE
// Support VALUES we're tracking being moved away in Ruby 2.7+ with GC.compact
static int collector_move_each_live_object(st_data_t key, st_data_t value, st_data_t arg) {
    struct collector_cdata *cd = (struct collector_cdata *)arg;
    struct mpp_sample *sample = (struct mpp_sample *)value;

    if (rb_gc_location(sample->allocated_value_weak) == sample->allocated_value_weak) {
        return ST_CONTINUE;
    } else {
        sample->allocated_value_weak = rb_gc_location(sample->allocated_value_weak);
        st_insert(cd->heap_samples, sample->allocated_value_weak, (st_data_t)sample);
        return ST_DELETE;
    }
}

static void collector_cdata_gc_compact(void *ptr) {
    struct collector_cdata *cd = (struct collector_cdata *)ptr;
    cd->newobj_trace = rb_gc_location(cd->newobj_trace);
    cd->freeobj_trace = rb_gc_location(cd->freeobj_trace);
    cd->creturn_trace = rb_gc_location(cd->creturn_trace);
    cd->mMemprofilerPprof = rb_gc_location(cd->mMemprofilerPprof);
    cd->cCollector = rb_gc_location(cd->cCollector);
    cd->cProfileData = rb_gc_location(cd->cProfileData);
    st_foreach(cd->heap_samples, collector_move_each_live_object, (st_data_t)cd);
}
#endif

static const rb_data_type_t collector_cdata_type = {
    "collector_cdata",
    {
        collector_cdata_gc_mark, collector_cdata_gc_free, collector_cdata_memsize,
#ifdef HAVE_RB_GC_MARK_MOVABLE
        collector_cdata_gc_compact,
#endif
        { 0 }, /* reserved */
    },
    /* parent, data, [ flags ] */
    NULL, NULL, 0
};

static struct collector_cdata *collector_cdata_get(VALUE self) {
    struct collector_cdata *a;
    TypedData_Get_Struct(self, struct collector_cdata, &collector_cdata_type, a);
    return a;
}

static VALUE collector_alloc(VALUE klass) {
    struct collector_cdata *cd;
    VALUE v = TypedData_Make_Struct(klass, struct collector_cdata, &collector_cdata_type, cd);

    cd->newobj_trace = Qnil;
    cd->freeobj_trace = Qnil;
    cd->creturn_trace = Qnil;

    __atomic_store_n(&cd->u32_sample_rate, 0, __ATOMIC_SEQ_CST);
    cd->is_tracing = false;

    cd->allocation_samples = NULL;
    cd->allocation_samples_count = 0;
    cd->max_allocation_samples = 0;
    cd->pending_size_count = 0;

    cd->heap_samples = NULL;
    cd->heap_samples_count = 0;
    cd->max_heap_samples = 0;

    __atomic_store_n(&cd->dropped_samples_allocation_bufsize, 0, __ATOMIC_SEQ_CST);
    __atomic_store_n(&cd->dropped_samples_heap_bufsize, 0, __ATOMIC_SEQ_CST);
    __atomic_store_n(&cd->dropped_samples_nolock, 0, __ATOMIC_SEQ_CST);

    cd->string_tab = NULL;
    cd->loctab = NULL;

    // Initialize the mutex.
    // It really does need to be recursive - if we call a rb_* function while holding
    // the lock, that could trigger the GC to run and call our freeobj tracepoint,
    // which _also_ needs the lock.
    pthread_mutexattr_t mutex_attr;
    mpp_pthread_mutexattr_init(&mutex_attr);
    mpp_pthread_mutexattr_settype(&mutex_attr, PTHREAD_MUTEX_RECURSIVE);
    mpp_pthread_mutex_init(&cd->lock, &mutex_attr);
    mpp_pthread_mutexattr_destroy(&mutex_attr);

    // Add us to the global list of collectors, to handle pthread_atfork.
    mpp_pthread_mutex_lock(&global_collectors_lock);
    st_insert(global_collectors, (st_data_t)cd, (st_data_t)cd);
    mpp_pthread_mutex_unlock(&global_collectors_lock);
    return v;
}

struct initialize_protected_args {
    int argc;
    VALUE *argv;
    VALUE self;
    struct collector_cdata *cd;
};

static VALUE collector_initialize_protected(VALUE vargs) {
    struct initialize_protected_args *args = (struct initialize_protected_args *)vargs;
    struct collector_cdata *cd = args->cd;

    // Save constants
    cd->mMemprofilerPprof = rb_const_get(rb_cObject, rb_intern("MemprofilerPprof"));
    cd->cCollector = rb_const_get(cd->mMemprofilerPprof, rb_intern("Collector"));
    cd->cProfileData = rb_const_get(cd->mMemprofilerPprof, rb_intern("ProfileData"));

    // Argument parsing
    VALUE kwargs_hash = Qnil;
    rb_scan_args_kw(RB_SCAN_ARGS_LAST_HASH_KEYWORDS, args->argc, args->argv, "00:", &kwargs_hash);
    VALUE kwarg_values[5];
    ID kwarg_ids[5];
    kwarg_ids[0] = rb_intern("sample_rate");
    kwarg_ids[1] = rb_intern("max_allocation_samples");
    kwarg_ids[2] = rb_intern("max_heap_samples");
    kwarg_ids[3] = rb_intern("bt_method");
    kwarg_ids[4] = rb_intern("allocation_retain_rate");
    rb_get_kwargs(kwargs_hash, kwarg_ids, 0, 5, kwarg_values);

    // Default values...
    if (kwarg_values[0] == Qundef) kwarg_values[0] = DBL2NUM(0.01);
    if (kwarg_values[1] == Qundef) kwarg_values[1] = LONG2NUM(10000);
    if (kwarg_values[2] == Qundef) kwarg_values[2] = LONG2NUM(50000);
    if (kwarg_values[3] == Qundef) kwarg_values[3] = rb_id2sym(rb_intern("cfp"));
    if (kwarg_values[4] == Qundef) kwarg_values[4] = DBL2NUM(1);

    rb_funcall(args->self, rb_intern("sample_rate="), 1, kwarg_values[0]);
    rb_funcall(args->self, rb_intern("max_allocation_samples="), 1, kwarg_values[1]);
    rb_funcall(args->self, rb_intern("max_heap_samples="), 1, kwarg_values[2]);
    rb_funcall(args->self, rb_intern("bt_method="), 1, kwarg_values[3]);
    rb_funcall(args->self, rb_intern("allocation_retain_rate="), 1, kwarg_values[4]);

    cd->string_tab = mpp_strtab_new();
    cd->loctab = mpp_rb_loctab_new(cd->string_tab);
    cd->allocation_samples = NULL;
    cd->allocation_samples_count = 0;
    cd->pending_size_count = 0;
    cd->heap_samples = st_init_numtable();
    cd->heap_samples_count = 0;

    VALUE internal_constants = rb_const_get(rb_mGC, rb_intern("INTERNAL_CONSTANTS"));
    cd->rvalue_size = NUM2LONG(rb_hash_aref(internal_constants, rb_id2sym(rb_intern("RVALUE_SIZE"))));

    return Qnil;
}

static VALUE collector_initialize(int argc, VALUE *argv, VALUE self) {
    // Need to do this rb_protect dance to ensure that all access to collector_cdata is through the mutex.
    struct initialize_protected_args args;
    args.argc = argc;
    args.argv = argv;
    args.self = self;
    args.cd = collector_cdata_get(self);

    mpp_pthread_mutex_lock(&args.cd->lock);
    int jump_tag = 0;
    VALUE r = rb_protect(collector_initialize_protected, (VALUE)&args, &jump_tag);
    mpp_pthread_mutex_unlock(&args.cd->lock);
    if (jump_tag) rb_jump_tag(jump_tag);
    return r;
}

static VALUE collector_get_sample_rate(VALUE self) {
    struct collector_cdata *cd = collector_cdata_get(self);
    uint32_t sample_rate = __atomic_load_n(&cd->u32_sample_rate, __ATOMIC_SEQ_CST);
    return DBL2NUM(((double)sample_rate)/UINT32_MAX);
}

static VALUE collector_set_sample_rate(VALUE self, VALUE newval) {
    struct collector_cdata *cd = collector_cdata_get(self);
    double dbl_sample_rate = NUM2DBL(newval);
    // Convert the double sample rate (between 0 and 1) to a value between 0 and UINT32_MAX
    uint32_t new_sample_rate_uint = UINT32_MAX * dbl_sample_rate;
    __atomic_store_n(&cd->u32_sample_rate, new_sample_rate_uint, __ATOMIC_SEQ_CST);
    return newval;
}

static VALUE collector_get_allocation_retain_rate(VALUE self) {
    struct collector_cdata *cd = collector_cdata_get(self);
    mpp_pthread_mutex_lock(&cd->lock);
    uint32_t retain_rate_u32 = cd->u32_allocation_retain_rate;
    mpp_pthread_mutex_unlock(&cd->lock);
    return DBL2NUM(((double)retain_rate_u32)/UINT32_MAX);
}

static VALUE collector_set_allocation_retain_rate(VALUE self, VALUE newval) {
    struct collector_cdata *cd = collector_cdata_get(self);
    uint32_t retain_rate_u32 = UINT32_MAX * NUM2DBL(newval);
    mpp_pthread_mutex_lock(&cd->lock);
    cd->u32_allocation_retain_rate = retain_rate_u32;
    mpp_pthread_mutex_unlock(&cd->lock);
    return newval;
}

static VALUE collector_get_max_allocation_samples(VALUE self) {
    struct collector_cdata *cd = collector_cdata_get(self);
    mpp_pthread_mutex_lock(&cd->lock);
    int64_t v = cd->max_allocation_samples;
    mpp_pthread_mutex_unlock(&cd->lock);
    return LONG2NUM(v);
}

static VALUE collector_set_max_allocation_samples(VALUE self, VALUE newval) {
    struct collector_cdata *cd = collector_cdata_get(self);
    int64_t v = NUM2LONG(newval);
    mpp_pthread_mutex_lock(&cd->lock);
    cd->max_allocation_samples = v;
    mpp_pthread_mutex_unlock(&cd->lock);
    return newval;
}

static VALUE collector_get_max_heap_samples(VALUE self) {
    struct collector_cdata *cd = collector_cdata_get(self);
    mpp_pthread_mutex_lock(&cd->lock);
    int64_t v = cd->max_heap_samples;
    mpp_pthread_mutex_unlock(&cd->lock);
    return LONG2NUM(v);
}

static VALUE collector_set_max_heap_samples(VALUE self, VALUE newval) {
    struct collector_cdata *cd = collector_cdata_get(self);
    int64_t v = NUM2LONG(newval);
    mpp_pthread_mutex_lock(&cd->lock);
    cd->max_heap_samples = v;
    mpp_pthread_mutex_unlock(&cd->lock);
    return newval;
}

static void collector_mark_sample_as_freed(struct collector_cdata *cd, VALUE freed_obj) {
    struct mpp_sample *sample;
    if (st_delete(cd->heap_samples, (st_data_t *)&freed_obj, (st_data_t *)&sample)) {
        // Clear out the reference to it
        sample->allocated_value_weak = Qundef;
        // We deleted it out of live objects; decrement its refcount.
        internal_sample_decrement_refcount(cd, sample);
        cd->heap_samples_count--;
    }
}


struct newobj_impl_args {
    struct collector_cdata *cd;
    struct mpp_rb_backtrace *bt;
    VALUE tpval;
    VALUE newobj;
    size_t allocation_size;
};

// Collects all the parts of collector_tphook_newobj that could throw.
static VALUE collector_tphook_newobj_protected(VALUE args_as_uintptr) {
    struct newobj_impl_args *args = (struct newobj_impl_args*)args_as_uintptr;
    struct collector_cdata *cd = args->cd;
    if (cd->bt_method == MPP_BT_METHOD_CFP) {
        mpp_rb_backtrace_capture(cd->loctab, &args->bt);
    } else if (cd->bt_method == MPP_BT_METHOD_SLOWRB) {
        mpp_rb_backtrace_capture_slowrb(cd->loctab, &args->bt);
    } else {
        MPP_ASSERT_FAIL("unknown bt_method");
    }
    args->allocation_size = cd->rvalue_size;
    return Qnil;
}

static void collector_tphook_newobj(VALUE tpval, void *data) {
    struct collector_cdata *cd = (struct collector_cdata *)data;
    struct newobj_impl_args args;
    args.cd = cd;
    args.tpval = tpval;
    args.bt = NULL;
    rb_trace_arg_t *tparg = rb_tracearg_from_tracepoint(tpval);
    args.newobj = rb_tracearg_object(tparg);
    int jump_tag = 0;
    VALUE original_errinfo = Qundef;

    mpp_pthread_mutex_lock(&cd->lock);

    // For every new object that is created, we _MUST_ check if there is already another VALUE with the same,
    // well, value, in our heap profiling table of live objects. This is because Ruby reserves the right to
    // simply free some kinds of internal objects (such as T_IMEMOs) by simply setting the flags value on it
    // to zero, without invoking the GC and without calling any kind of hook. So, we need to detect when such
    // an object is freed and then the RVALUE is re-used for a new object to track it appropriately.
    collector_mark_sample_as_freed(cd, args.newobj);

    // Skip the rest of this method if we're not sampling.
    uint32_t sample_rate = __atomic_load_n(&cd->u32_sample_rate, __ATOMIC_SEQ_CST);
    if (mpp_rand() > sample_rate) {
       goto out;
    }

    // Make sure there's enough space in our buffers.
    if (cd->allocation_samples_count >= cd->max_allocation_samples) {
        __atomic_add_fetch(&cd->dropped_samples_allocation_bufsize, 1, __ATOMIC_SEQ_CST);
        goto out;
    }
    if (cd->heap_samples_count >= cd->max_heap_samples) {
        __atomic_add_fetch(&cd->dropped_samples_heap_bufsize, 1, __ATOMIC_SEQ_CST);
        goto out;
    }

    // OK - run our code in here under rb_protect now so that it cannot longjmp out
    original_errinfo = rb_errinfo();
    rb_protect(collector_tphook_newobj_protected, (VALUE)&args, &jump_tag);
    if (jump_tag) goto out;

    // This looks super redundant, _BUT_ there is a narrow possibility that some of the code we invoke
    // inside the rb_protect actually does RVALUE allocations itself, and so recursively runs this hook
    // (which will work, because the &cd->lock mutex is recursive). So, we need to actually check
    // our buffer sizes _again_.
    if (cd->allocation_samples_count >= cd->max_allocation_samples) {
        __atomic_add_fetch(&cd->dropped_samples_allocation_bufsize, 1, __ATOMIC_SEQ_CST);
        goto out;
    }
    if (cd->heap_samples_count >= cd->max_heap_samples) {
        __atomic_add_fetch(&cd->dropped_samples_heap_bufsize, 1, __ATOMIC_SEQ_CST);
        goto out;
    }

    // OK, now it's time to add to our sample buffers.
    struct mpp_sample *sample = mpp_xmalloc(sizeof(struct mpp_sample));
    // Set the sample refcount to two. Once because it's going in the allocation sampling buffer,
    // and once because it's going in the heap profiling set.
    sample->refcount = 2;
    sample->bt = args.bt;
    sample->allocation_size = args.allocation_size;
    sample->current_size = args.allocation_size;
    sample->allocated_value_weak = args.newobj;

    // Insert into allocation profiling list.
    sample->next_alloc = cd->allocation_samples;
    cd->allocation_samples = sample;
    cd->allocation_samples_count++;
    cd->pending_size_count++;

    // Also insert into live object list
    st_insert(cd->heap_samples, args.newobj, (st_data_t)sample);
    cd->heap_samples_count++;

    // Clear args.bt so it doesn't get free'd below.
    args.bt = NULL;

out:
    // If this wasn't cleared, we need to free it.
    if (args.bt) mpp_rb_backtrace_destroy(cd->loctab, args.bt);
    // If there was an exception, ignore it and restore the original errinfo.
    if (jump_tag && original_errinfo != Qundef) rb_set_errinfo(original_errinfo);

    mpp_pthread_mutex_unlock(&cd->lock);
}

static void collector_tphook_freeobj(VALUE tpval, void *data) {
    struct collector_cdata *cd = (struct collector_cdata *)data;

    // We unfortunately do really need the mutex here, because if we don't handle this, we might
    // leave an allocation kicking around in live_objects that has been freed.
    mpp_pthread_mutex_lock(&cd->lock);

    // Definitely do _NOT_ try and run any Ruby code in here. Any allocation will crash
    // the process.
    rb_trace_arg_t *tparg = rb_tracearg_from_tracepoint(tpval);
    VALUE freed_obj = rb_tracearg_object(tparg);
    collector_mark_sample_as_freed(cd, freed_obj);

    mpp_pthread_mutex_unlock(&cd->lock);
}

static VALUE collector_tphook_creturn_protected(VALUE cdataptr) {
    struct collector_cdata *cd = (struct collector_cdata *)cdataptr;

    struct mpp_sample *s = cd->allocation_samples;
    struct mpp_sample **prev_slot = &cd->allocation_samples;
    for (int64_t i = 0; i < cd->pending_size_count; i++) {
        MPP_ASSERT_MSG(s, "More pending size samples than samples in linked list??");
        // Ruby apparently has the right to free stuff that's used internally (like T_IMEMOs)
        // _without_ invoking the garbage collector (and thus, _without_ invoking our hook). When
        // it does that, it will set flags of the RVALUE to zero, which indicates that the object
        // is now free.
        // Detect this and consider it the same as free'ing an object. Otherwise, we might try and
        // memsize() it, which will cause an rb_bug to trigger
        if (RB_TYPE_P(s->allocated_value_weak, T_NONE)) {
            collector_mark_sample_as_freed(cd, s->allocated_value_weak);
            s->allocated_value_weak = Qundef;
        }
        if (s->allocated_value_weak != Qundef) {
            s->allocation_size = rb_obj_memsize_of(s->allocated_value_weak);
            s->current_size = s->allocation_size;
        }

        if (mpp_rand() > cd->u32_allocation_retain_rate) {
            // Drop this sample out of the allocation sample list. We've been asked to drop a certain
            // percentage of things out of this list, so we don't OOM with piles of short-lived objects.
            *prev_slot = s->next_alloc;

            // Annoying little dance here so we don't read s->next_alloc after freeing s.
            struct mpp_sample *next_s = s->next_alloc;
            internal_sample_decrement_refcount(cd, s);
            s = next_s;

            cd->allocation_samples_count--;
        } else {
            prev_slot = &s->next_alloc;
            s = s->next_alloc;
        }
    }
    return Qnil;
}

static void collector_tphook_creturn(VALUE tpval, void *data) {
    struct collector_cdata *cd = (struct collector_cdata *)data;
    int jump_tag = 0;
    VALUE original_errinfo;
    // If we can't get the lock this time round, we can just do it later.
    if (mpp_pthread_mutex_trylock(&cd->lock) != 0) {
        return;
    }
    if (cd->pending_size_count == 0) goto out;

    original_errinfo = rb_errinfo();
    rb_protect(collector_tphook_creturn_protected, (VALUE)cd, &jump_tag);
    cd->pending_size_count = 0;
    if (jump_tag) {
        rb_set_errinfo(original_errinfo);
    }

out:
    mpp_pthread_mutex_unlock(&cd->lock);
}

static VALUE collector_start_protected(VALUE self) {
    struct collector_cdata *cd = collector_cdata_get(self);

    if (cd->newobj_trace == Qnil) {
        cd->newobj_trace = rb_tracepoint_new(
            0, RUBY_INTERNAL_EVENT_NEWOBJ, collector_tphook_newobj, cd
        );
    }
    if (cd->freeobj_trace == Qnil) {
        cd->freeobj_trace = rb_tracepoint_new(
            0, RUBY_INTERNAL_EVENT_FREEOBJ, collector_tphook_freeobj, cd
        );
    }
    if (cd->creturn_trace == Qnil) {
        cd->creturn_trace = rb_tracepoint_new(
            0, RUBY_EVENT_C_RETURN, collector_tphook_creturn, cd
        );
    }

    rb_tracepoint_enable(cd->newobj_trace);
    rb_tracepoint_enable(cd->freeobj_trace);
    rb_tracepoint_enable(cd->creturn_trace);
    return Qnil;
}

static VALUE collector_start(VALUE self) {
    int jump_tag = 0;
    struct collector_cdata *cd = collector_cdata_get(self);
    mpp_pthread_mutex_lock(&cd->lock);
    if (cd->is_tracing) goto out;

    // Don't needlessly double-initialize everything
    if (cd->heap_samples_count > 0) {
        collector_cdata_gc_free_heap_samples(cd);
        cd->heap_samples = st_init_numtable();
        cd->heap_samples_count = 0;
    }
    if (cd->allocation_samples_count > 0) {
        collector_cdata_gc_free_allocation_samples(cd);
        cd->allocation_samples = NULL;
        cd->allocation_samples_count = 0;
        cd->pending_size_count = 0;
    }
    cd->is_tracing = true;
    __atomic_store_n(&cd->dropped_samples_allocation_bufsize, 0, __ATOMIC_SEQ_CST);
    __atomic_store_n(&cd->dropped_samples_heap_bufsize, 0, __ATOMIC_SEQ_CST);
    __atomic_store_n(&cd->dropped_samples_nolock, 0, __ATOMIC_SEQ_CST);

    // Now do the things that might throw
    rb_protect(collector_start_protected, self, &jump_tag);

out:
    mpp_pthread_mutex_unlock(&cd->lock);
    if (jump_tag) {
        rb_jump_tag(jump_tag);
    }
    return Qnil;
}

static VALUE collector_stop_protected(VALUE self) {
    struct collector_cdata *cd = collector_cdata_get(self);
    rb_tracepoint_disable(cd->newobj_trace);
    rb_tracepoint_disable(cd->freeobj_trace);
    rb_tracepoint_disable(cd->creturn_trace);
    return Qnil;
}

static VALUE collector_stop(VALUE self) {
    int jump_tag = 0;
    struct collector_cdata *cd = collector_cdata_get(self);
    mpp_pthread_mutex_lock(&cd->lock);
    if (!cd->is_tracing) goto out;

    rb_protect(collector_stop_protected, self, &jump_tag);
    if (jump_tag) goto out;

    cd->is_tracing = false;
    // Don't clear any of our buffers - it's OK to access the profiling info after calling stop!
out:
    mpp_pthread_mutex_unlock(&cd->lock);
    if (jump_tag) {
        rb_jump_tag(jump_tag);
    }
    return Qnil;
}

static VALUE collector_is_running(VALUE self) {
    struct collector_cdata *cd = collector_cdata_get(self);
    mpp_pthread_mutex_lock(&cd->lock);
    bool running = cd->is_tracing;
    mpp_pthread_mutex_unlock(&cd->lock);
    return running ? Qtrue : Qfalse;
}

static int collector_heap_samples_each_calc_size(st_data_t key, st_data_t val, st_data_t arg) {
    struct mpp_sample *sample = (struct mpp_sample *)val;
    struct collector_cdata *cd = (struct collector_cdata *)arg;
    MPP_ASSERT_MSG(sample->allocated_value_weak != Qundef, "undef was in heap sample map");

    // Check that the sample is, in fact, still live. This can happen if an object is freed internally
    // by Ruby without firing our freeobj hook (which Ruby is allowed to do for some kinds of objects).
    // In that case, flags will be zero and so type will be T_NONE.
    // Note that if an object is freed and then the slot is subsequently re-used for a different object,
    // our newobj hook will fire in that case and do this too. So this method captures the sequence
    // allocate -> free -> flush, but the newobj hook handles the allocate -> free -> reuse -> flush case.
    if (RB_TYPE_P(sample->allocated_value_weak, T_NONE)) {
        sample->allocated_value_weak = Qundef;
        internal_sample_decrement_refcount(cd, sample);
        cd->heap_samples_count--;
        return ST_DELETE;
    }

    sample->current_size = rb_obj_memsize_of(sample->allocated_value_weak);
    return ST_CONTINUE;
}

struct collector_heap_samples_each_add_args {
    struct mpp_pprof_serctx *serctx;
    char *errbuf;
    size_t errbuf_len;
    int r;
};

static int collector_heap_samples_each_add(st_data_t key, st_data_t val, st_data_t arg) {
    struct mpp_sample *sample = (struct mpp_sample *)val;
    struct collector_heap_samples_each_add_args *args = (struct collector_heap_samples_each_add_args *)arg;

    int r = mpp_pprof_serctx_add_sample(args->serctx, sample, MPP_SAMPLE_TYPE_HEAP, args->errbuf, args->errbuf_len);
    if (r != 0) {
        args->r = r;
        return ST_STOP;
    }
    return ST_CONTINUE;
}

struct collector_flush_prepresult_args {
    const char *pprofbuf;
    size_t pprofbuf_len;
    VALUE cProfileData;

    // Extra struff that needs to go onto the struct.
    int64_t allocation_samples_count;
    int64_t heap_samples_count;
    int64_t dropped_samples_nolock;
    int64_t dropped_samples_allocation_bufsize;
    int64_t dropped_samples_heap_bufsize;
};

static VALUE collector_flush_protected_heap_sample_size(VALUE self) {
    struct collector_cdata *cd = collector_cdata_get(self);
    st_foreach(cd->heap_samples, collector_heap_samples_each_calc_size, (st_data_t)cd);
    return Qnil;
}

static VALUE collector_flush_prepresult(VALUE vargs) {
    struct collector_flush_prepresult_args *args =
        (struct collector_flush_prepresult_args *)vargs;

    VALUE pprof_data = rb_str_new(args->pprofbuf, args->pprofbuf_len);
    VALUE profile_data = rb_class_new_instance(0, NULL, args->cProfileData);
    rb_funcall(profile_data, rb_intern("pprof_data="), 1, pprof_data);
    rb_funcall(profile_data, rb_intern("allocation_samples_count="), 1, LONG2NUM(args->allocation_samples_count));
    rb_funcall(profile_data, rb_intern("heap_samples_count="), 1, LONG2NUM(args->heap_samples_count));
    rb_funcall(profile_data, rb_intern("dropped_samples_nolock="), 1, LONG2NUM(args->dropped_samples_nolock));
    rb_funcall(
        profile_data, rb_intern("dropped_samples_allocation_bufsize="),
        1, LONG2NUM(args->dropped_samples_allocation_bufsize)
    );
    rb_funcall(
        profile_data, rb_intern("dropped_samples_heap_bufsize="),
        1, LONG2NUM(args->dropped_samples_heap_bufsize)
    );
    return profile_data;
}

static VALUE collector_flush(VALUE self) {
    struct collector_cdata *cd = collector_cdata_get(self);
    struct mpp_pprof_serctx *serctx = NULL;
    char *buf_out;
    size_t buflen_out;
    char errbuf[256];
    int jump_tag = 0;
    int r = 0;
    VALUE retval = Qundef;
    struct mpp_sample *sample_list = NULL;
    struct collector_flush_prepresult_args prepresult_args;
    int lock_held = 0;

    // Whilst under the GVL, we need to get the collector lock
    mpp_pthread_mutex_lock(&cd->lock);
    lock_held = 1;

    sample_list = cd->allocation_samples;
    cd->allocation_samples = NULL;
    prepresult_args.allocation_samples_count = cd->allocation_samples_count;
    prepresult_args.heap_samples_count = cd->heap_samples_count;
    cd->allocation_samples_count = 0;
    cd->pending_size_count = 0;

    prepresult_args.dropped_samples_nolock =
        __atomic_exchange_n(&cd->dropped_samples_nolock, 0, __ATOMIC_SEQ_CST);
    prepresult_args.dropped_samples_allocation_bufsize =
        __atomic_exchange_n(&cd->dropped_samples_allocation_bufsize, 0, __ATOMIC_SEQ_CST);
    prepresult_args.dropped_samples_heap_bufsize =
        __atomic_exchange_n(&cd->dropped_samples_heap_bufsize, 0, __ATOMIC_SEQ_CST);

    // Get the current size for everything in the live allocations table.
    rb_protect(collector_flush_protected_heap_sample_size, self, &jump_tag);
    if (jump_tag) goto out;

    serctx = mpp_pprof_serctx_new();
    MPP_ASSERT_MSG(serctx, "mpp_pprof_serctx_new failed??");
    r = mpp_pprof_serctx_set_loctab(serctx, cd->loctab, errbuf, sizeof(errbuf));
    if (r == -1) {
        goto out;
    }

    // Now that we have the samples (and have processed the stringtab) we can
    // yield the lock.
    mpp_pthread_mutex_unlock(&cd->lock);
    lock_held = 0;

    // Add the allocation samples
    struct mpp_sample *s = sample_list;
    while (s) {
        r = mpp_pprof_serctx_add_sample(serctx, s, MPP_SAMPLE_TYPE_ALLOCATION, errbuf, sizeof(errbuf));
        if (r == -1) {
            goto out;
        }
        s = s->next_alloc;
    }

    // Add the heap samples
    struct collector_heap_samples_each_add_args heap_add_args;
    heap_add_args.serctx = serctx;
    heap_add_args.errbuf = errbuf;
    heap_add_args.errbuf_len = sizeof(errbuf);
    heap_add_args.r = 0;
    st_foreach(cd->heap_samples, collector_heap_samples_each_add, (st_data_t)&heap_add_args);
    if (heap_add_args.r != 0) goto out;

    r = mpp_pprof_serctx_serialize(serctx, &buf_out, &buflen_out, errbuf, sizeof(errbuf));
    if ( r == -1) {
        goto out;
    }
    // Annoyingly, since rb_str_new could (in theory) throw, we have to rb_protect the whole construction
    // of our return value to ensure we don't leak serctx.
    prepresult_args.pprofbuf = buf_out;
    prepresult_args.pprofbuf_len = buflen_out;
    prepresult_args.cProfileData = cd->cProfileData;
    retval = rb_protect(collector_flush_prepresult, (VALUE)&prepresult_args, &jump_tag);

    // Do cleanup here now.
out:
    if (serctx) mpp_pprof_serctx_destroy(serctx);
    if (lock_held) mpp_pthread_mutex_unlock(&cd->lock);
    if (sample_list) internal_sample_decrement_refcount(cd, sample_list);

    // Now return-or-raise back to ruby.
    if (jump_tag) {
        rb_jump_tag(jump_tag);
    }
    if (retval == Qundef) {
        // Means we have an error to construct and throw
        rb_raise(rb_eRuntimeError, "ruby_memprofiler_pprof failed serializing pprof protobuf: %s", errbuf);
    }
    return retval;

    RB_GC_GUARD(self);
}

static VALUE collector_profile(VALUE self) {
    rb_need_block();

    rb_funcall(self, rb_intern("start!"), 0);
    rb_yield_values(0);
    VALUE profile_output = rb_funcall(self, rb_intern("flush"), 0);
    rb_funcall(self, rb_intern("stop!"), 0);

    return profile_output;
}

static VALUE collector_live_heap_samples_count(VALUE self) {
    struct collector_cdata *cd = collector_cdata_get(self);

    mpp_pthread_mutex_lock(&cd->lock);
    int64_t counter = cd->heap_samples_count;
    mpp_pthread_mutex_unlock(&cd->lock);
    return LONG2NUM(counter);
}

static VALUE collector_bt_method_get(VALUE self) {
    struct collector_cdata *cd = collector_cdata_get(self);

    mpp_pthread_mutex_lock(&cd->lock);
    int method = cd->bt_method;
    mpp_pthread_mutex_unlock(&cd->lock);

    if (method == MPP_BT_METHOD_CFP) {
        return rb_id2sym(rb_intern("cfp"));
    } else if (method == MPP_BT_METHOD_SLOWRB) {
        return rb_id2sym(rb_intern("slowrb"));
    } else {
        MPP_ASSERT_FAIL("unknown bt_method");
        return Qundef;
    }
}

static VALUE collector_bt_method_set(VALUE self, VALUE newval) {
    struct collector_cdata *cd = collector_cdata_get(self);

    ID bt_method = rb_sym2id(newval);
    int method;
    if (bt_method == rb_intern("cfp")) {
        method = MPP_BT_METHOD_CFP;
    } else if (bt_method == rb_intern("slowrb")) {
        method = MPP_BT_METHOD_SLOWRB;
    } else {
        rb_raise(rb_eArgError, "passed value for bt_method was not recognised");
    }

    mpp_pthread_mutex_lock(&cd->lock);
    cd->bt_method = method;
    mpp_pthread_mutex_unlock(&cd->lock);

    return newval;
}

static int mpp_collector_atfork_lock_el(st_data_t key, st_data_t value, st_data_t arg) {
    struct collector_cdata *cd = (struct collector_cdata *)key;
    mpp_pthread_mutex_lock(&cd->lock);
    return ST_CONTINUE;
}

static int mpp_collector_atfork_unlock_el(st_data_t key, st_data_t value, st_data_t arg) {
    struct collector_cdata *cd = (struct collector_cdata *)key;
    mpp_pthread_mutex_unlock(&cd->lock);
    return ST_CONTINUE;
}

static int mpp_collector_atfork_replace_el(st_data_t key, st_data_t value, st_data_t arg) {
    struct collector_cdata *cd = (struct collector_cdata *)key;

    // In the parent process, we simply release the mutexes, but in the child process, we have
    // to _RECREATE_ them. This is because they're recursive mutexes, and must hold some kind of
    // thread ID in them somehow; unlocking them post-fork simply doesn't work it seems.
    // It's safe to re-create the mutex at this point, because no other thread can possibly be
    // holding it since we took it pre-fork
    mpp_pthread_mutex_destroy(&cd->lock);
    pthread_mutexattr_t mutex_attr;
    mpp_pthread_mutexattr_init(&mutex_attr);
    mpp_pthread_mutexattr_settype(&mutex_attr, PTHREAD_MUTEX_RECURSIVE);
    memset(&cd->lock, 0, sizeof(cd->lock));
    mpp_pthread_mutex_init(&cd->lock, &mutex_attr);
    mpp_pthread_mutexattr_destroy(&mutex_attr);

    return ST_CONTINUE;
}

static void mpp_collector_atfork_prepare() {
    mpp_pthread_mutex_lock(&global_collectors_lock);
    st_foreach(global_collectors, mpp_collector_atfork_lock_el, 0);
}

static void mpp_collector_atfork_release_parent() {
    st_foreach(global_collectors, mpp_collector_atfork_unlock_el, 0);
    mpp_pthread_mutex_unlock(&global_collectors_lock);
}

static void mpp_collector_atfork_release_child() {
    st_foreach(global_collectors, mpp_collector_atfork_replace_el, 0);
    mpp_pthread_mutex_unlock(&global_collectors_lock);
}


void mpp_setup_collector_class() {
    VALUE mMemprofilerPprof = rb_const_get(rb_cObject, rb_intern("MemprofilerPprof"));
    VALUE cCollector = rb_define_class_under(mMemprofilerPprof, "Collector", rb_cObject);
    rb_define_alloc_func(cCollector, collector_alloc);


    rb_define_method(cCollector, "initialize", collector_initialize, -1);
    rb_define_method(cCollector, "sample_rate", collector_get_sample_rate, 0);
    rb_define_method(cCollector, "sample_rate=", collector_set_sample_rate, 1);
    rb_define_method(cCollector, "max_allocation_samples", collector_get_max_allocation_samples, 0);
    rb_define_method(cCollector, "max_allocation_samples=", collector_set_max_allocation_samples, 1);
    rb_define_method(cCollector, "max_heap_samples", collector_get_max_heap_samples, 0);
    rb_define_method(cCollector, "max_heap_samples=", collector_set_max_heap_samples, 1);
    rb_define_method(cCollector, "bt_method", collector_bt_method_get, 0);
    rb_define_method(cCollector, "bt_method=", collector_bt_method_set, 1);
    rb_define_method(cCollector, "allocation_retain_rate", collector_get_allocation_retain_rate, 0);
    rb_define_method(cCollector, "allocation_retain_rate=", collector_set_allocation_retain_rate, 1);
    rb_define_method(cCollector, "running?", collector_is_running, 0);
    rb_define_method(cCollector, "start!", collector_start, 0);
    rb_define_method(cCollector, "stop!", collector_stop, 0);
    rb_define_method(cCollector, "flush", collector_flush, 0);
    rb_define_method(cCollector, "profile", collector_profile, 0);
    rb_define_method(cCollector, "live_heap_samples_count", collector_live_heap_samples_count, 0);

    global_collectors = st_init_numtable();
    mpp_pthread_mutex_init(&global_collectors_lock, NULL);
    mpp_pthread_atfork(mpp_collector_atfork_prepare, mpp_collector_atfork_release_parent, mpp_collector_atfork_release_child);
}
