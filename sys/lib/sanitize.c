/*
 * Copyright (c) 2014, 2015, 2024 Jonas 'Sortie' Termansen.
 * Copyright (c) 2026 Aggelos Tselios
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

/****************************************************************************************************
 * This file is based on the Sortix operating system's (gitlab.com/sortix/Sortix) implementation,
 *provided under the license seen above. The combined work (DragonWare) is distributed under the GNU
 *General Public License, found in the file COPYING in the toplevel tree, but the original license
 *is still used for this file. The original file can be found here:
 * https://gitlab.com/sortix/sortix/-/blob/main/libc/ubsan/ubsan.c
 *
 * Main changes are:
 * - Use DragonWare coding style and formatting
 * - Use kernel-provided facilities like FatalError
 * - Remove old GCC compatibility hacks in the source (We need newer GCC versions to support C23
 * anyways)
 * - Code cleanups like removing stray semicolons
 ***************************************************************************************************/

#include <ktypes.h>

#include "panic.h"

typedef struct _UBSanSourceLocation {
        const char* filename;
        u32         line;
        u32         column;
} UBSanSourceLocation;

typedef struct _UBSanTypeDescriptor {
        u16  type_kind;
        u16  type_info;
        char type_name[];
} UBSanTypeDescriptor;

typedef uintptr_t UBSanValueHandle;

static const UBSanSourceLocation unknown_location = {
        "<unknown file>",
        0,
        0,
};

[[noreturn]]
static void KernelUBSanAbort(const char* violation, const char* filename, u32 line, u32 column) {
        FatalError("** SANITIZER VIOLATION ** :: %s (%s:%d-%d)", violation, filename, line, column);
        __builtin_unreachable();
}

[[noreturn]]
static void UBSanAbort(const UBSanSourceLocation* location, const char* violation) {
        if (!location || !location->filename) location = &unknown_location;
        KernelUBSanAbort(violation, location->filename, location->line, location->column);
}

#define ABORT_VARIANT(name, params, call)           \
        [[noreturn]]                                \
        void __ubsan_handle_##name##_abort params { \
                __ubsan_handle_##name call;         \
                __builtin_unreachable();            \
        }

#define ABORT_VARIANT_VP(name)       ABORT_VARIANT(name, (void* a), (a))
#define ABORT_VARIANT_VP_VP(name)    ABORT_VARIANT(name, (void* a, void* b), (a, b))
#define ABORT_VARIANT_VP_IP(name)    ABORT_VARIANT(name, (void* a, intptr_t b), (a, b))
#define ABORT_VARIANT_VP_VP_VP(name) ABORT_VARIANT(name, (void* a, void* b, void* c), (a, b, c))
#define ABORT_VARIANT_VP_VP_UP(name) ABORT_VARIANT(name, (void* a, void* b, uintptr_t c), (a, b, c))
#define ABORT_VARIANT_VP_VP_VP_VP(name) \
        ABORT_VARIANT(name, (void* a, void* b, void* c, void* d), (a, b, c, d))
#define ABORT_VARIANT_VP_VP_UP_VP(name) \
        ABORT_VARIANT(name, (void* a, void* b, uintptr_t c, void* d), (a, b, c, d))

struct ubsan_type_mismatch_v1_data {
        UBSanSourceLocation           location;
        struct ubsan_type_descriptor* type;
        unsigned char                 log_alignment;
        unsigned char                 type_check_kind;
};

void __ubsan_handle_type_mismatch_v1(void* data_raw, void* pointer_raw) {
        struct ubsan_type_mismatch_v1_data* data    = (struct ubsan_type_mismatch_v1_data*)data_raw;
        UBSanValueHandle                    pointer = (UBSanValueHandle)pointer_raw;
        uintptr_t                           alignment = (uintptr_t)1UL << data->log_alignment;
        const char*                         violation = "type mismatch";
        if (!pointer)
                violation = "null pointer access";
        else if (alignment && (pointer & (alignment - 1)))
                violation = "unaligned pointer access";
        UBSanAbort(&data->location, violation);
}

ABORT_VARIANT_VP_VP(type_mismatch_v1)

struct ubsan_alignment_assumption_data {
        UBSanSourceLocation           location;
        UBSanSourceLocation           assumption_location;
        struct ubsan_type_descriptor* type;
};

void __ubsan_handle_alignment_assumption(void* data_raw, void* pointer_raw, void* alignment_raw,
                                         void* offset_raw) {
        struct ubsan_alignment_assumption_data* data =
                (struct ubsan_alignment_assumption_data*)data_raw;
        UBSanValueHandle pointer   = (UBSanValueHandle)pointer_raw;
        UBSanValueHandle alignment = (UBSanValueHandle)alignment_raw;
        UBSanValueHandle offset    = (UBSanValueHandle)offset_raw;
        (void)pointer;
        (void)alignment;
        (void)offset;
        const char* violation = "alignment assumption failed";
        UBSanAbort(&data->location, violation);
}

ABORT_VARIANT_VP_VP_VP_VP(alignment_assumption)

struct ubsan_overflow_data {
        UBSanSourceLocation           location;
        struct ubsan_type_descriptor* type;
};

void __ubsan_handle_add_overflow(void* data_raw, void* lhs_raw, void* rhs_raw) {
        struct ubsan_overflow_data* data = (struct ubsan_overflow_data*)data_raw;
        UBSanValueHandle            lhs  = (UBSanValueHandle)lhs_raw;
        UBSanValueHandle            rhs  = (UBSanValueHandle)rhs_raw;
        (void)lhs;
        (void)rhs;
        UBSanAbort(&data->location, "addition overflow");
}

ABORT_VARIANT_VP_VP_VP(add_overflow)

void __ubsan_handle_sub_overflow(void* data_raw, void* lhs_raw, void* rhs_raw) {
        struct ubsan_overflow_data* data = (struct ubsan_overflow_data*)data_raw;
        UBSanValueHandle            lhs  = (UBSanValueHandle)lhs_raw;
        UBSanValueHandle            rhs  = (UBSanValueHandle)rhs_raw;
        (void)lhs;
        (void)rhs;
        UBSanAbort(&data->location, "subtraction overflow");
}

ABORT_VARIANT_VP_VP_VP(sub_overflow)

void __ubsan_handle_mul_overflow(void* data_raw, void* lhs_raw, void* rhs_raw) {
        struct ubsan_overflow_data* data = (struct ubsan_overflow_data*)data_raw;
        UBSanValueHandle            lhs  = (UBSanValueHandle)lhs_raw;
        UBSanValueHandle            rhs  = (UBSanValueHandle)rhs_raw;
        (void)lhs;
        (void)rhs;
        UBSanAbort(&data->location, "multiplication overflow");
}

ABORT_VARIANT_VP_VP_VP(mul_overflow)

void __ubsan_handle_negate_overflow(void* data_raw, void* old_value_raw) {
        struct ubsan_overflow_data* data      = (struct ubsan_overflow_data*)data_raw;
        UBSanValueHandle            old_value = (UBSanValueHandle)old_value_raw;
        (void)old_value;
        UBSanAbort(&data->location, "negation overflow");
}

ABORT_VARIANT_VP_VP(negate_overflow)

void __ubsan_handle_divrem_overflow(void* data_raw, void* lhs_raw, void* rhs_raw) {
        struct ubsan_overflow_data* data = (struct ubsan_overflow_data*)data_raw;
        UBSanValueHandle            lhs  = (UBSanValueHandle)lhs_raw;
        UBSanValueHandle            rhs  = (UBSanValueHandle)rhs_raw;
        (void)lhs;
        (void)rhs;
        UBSanAbort(&data->location, "division remainder overflow");
}

ABORT_VARIANT_VP_VP_VP(divrem_overflow)

struct ubsan_shift_out_of_bounds_data {
        UBSanSourceLocation           location;
        struct ubsan_type_descriptor* lhs_type;
        struct ubsan_type_descriptor* rhs_type;
};

void __ubsan_handle_shift_out_of_bounds(void* data_raw, void* lhs_raw, void* rhs_raw) {
        struct ubsan_shift_out_of_bounds_data* data =
                (struct ubsan_shift_out_of_bounds_data*)data_raw;
        UBSanValueHandle lhs = (UBSanValueHandle)lhs_raw;
        UBSanValueHandle rhs = (UBSanValueHandle)rhs_raw;
        (void)lhs;
        (void)rhs;
        UBSanAbort(&data->location, "shift out of bounds");
}

ABORT_VARIANT_VP_VP_VP(shift_out_of_bounds)

struct ubsan_out_of_bounds_data {
        UBSanSourceLocation           location;
        struct ubsan_type_descriptor* array_type;
        struct ubsan_type_descriptor* index_type;
};

void __ubsan_handle_out_of_bounds(void* data_raw, void* index_raw) {
        struct ubsan_out_of_bounds_data* data  = (struct ubsan_out_of_bounds_data*)data_raw;
        UBSanValueHandle                 index = (UBSanValueHandle)index_raw;
        (void)index;
        UBSanAbort(&data->location, "out of bounds access");
}

ABORT_VARIANT_VP_VP(out_of_bounds)

struct ubsan_unreachable_data {
        UBSanSourceLocation location;
};

__attribute__((noreturn)) void __ubsan_handle_builtin_unreachable(void* data_raw) {
        struct ubsan_unreachable_data* data = (struct ubsan_unreachable_data*)data_raw;
        UBSanAbort(&data->location, "reached unreachable");
}

__attribute__((noreturn)) void __ubsan_handle_missing_return(void* data_raw) {
        struct ubsan_unreachable_data* data = (struct ubsan_unreachable_data*)data_raw;
        UBSanAbort(&data->location, "missing return");
}

struct ubsan_vla_bound_data {
        UBSanSourceLocation           location;
        struct ubsan_type_descriptor* type;
};

void __ubsan_handle_vla_bound_not_positive(void* data_raw, void* bound_raw) {
        struct ubsan_vla_bound_data* data  = (struct ubsan_vla_bound_data*)data_raw;
        UBSanValueHandle             bound = (UBSanValueHandle)bound_raw;
        (void)bound;
        UBSanAbort(&data->location, "negative variable array length");
}

ABORT_VARIANT_VP_VP(vla_bound_not_positive)

struct ubsan_float_cast_overflow_data {
        UBSanSourceLocation           location;
        struct ubsan_type_descriptor* from_type;
        struct ubsan_type_descriptor* to_type;
};

void __ubsan_handle_float_cast_overflow(void* data_raw, void* from_raw) {
        struct ubsan_float_cast_overflow_data* data =
                (struct ubsan_float_cast_overflow_data*)data_raw;
        (void)from_raw;
        UBSanAbort(&data->location, "float cast overflow");
}

ABORT_VARIANT_VP_VP(float_cast_overflow)

struct ubsan_invalid_value_data {
        UBSanSourceLocation           location;
        struct ubsan_type_descriptor* type;
};

void __ubsan_handle_load_invalid_value(void* data_raw, void* value_raw) {
        struct ubsan_invalid_value_data* data  = (struct ubsan_invalid_value_data*)data_raw;
        UBSanValueHandle                 value = (UBSanValueHandle)value_raw;
        (void)value;
        UBSanAbort(&data->location, "invalid value load");
}

ABORT_VARIANT_VP_VP(load_invalid_value)

struct ubsan_implicit_conversion_data {
        UBSanSourceLocation           location;
        struct ubsan_type_descriptor* type;
        struct ubsan_type_descriptor* from_type;
        struct ubsan_type_descriptor* to_type;
        unsigned char                 kind;
};

void __ubsan_handle_implicit_conversion(void* data_raw, void* src_raw, void* dst_raw) {
        struct ubsan_implicit_conversion_data* data =
                (struct ubsan_implicit_conversion_data*)data_raw;
        UBSanValueHandle src = (UBSanValueHandle)src_raw;
        UBSanValueHandle dst = (UBSanValueHandle)dst_raw;
        (void)src;
        (void)dst;
        UBSanAbort(&data->location, "implicit conversion");
}

ABORT_VARIANT_VP_VP_VP(implicit_conversion)

struct ubsan_invalid_builtin_data {
        UBSanSourceLocation location;
        unsigned char       kind;
};

void __ubsan_handle_invalid_builtin(void* data_raw) {
        struct ubsan_invalid_builtin_data* data = (struct ubsan_invalid_builtin_data*)data_raw;
        UBSanAbort(&data->location, "invalid builtin");
}

ABORT_VARIANT_VP(invalid_builtin)

struct ubsan_invalid_objc_cast_data {
        UBSanSourceLocation           location;
        struct ubsan_type_descriptor* expected_type;
};

void __ubsan_handle_invalid_objc_cast(void* data_raw, void* pointer_raw) {
        struct ubsan_invalid_builtin_data* data    = (struct ubsan_invalid_builtin_data*)data_raw;
        UBSanValueHandle                   pointer = (UBSanValueHandle)pointer_raw;
        (void)pointer;
        UBSanAbort(&data->location, "invalid objc cast");
}

ABORT_VARIANT_VP_VP(invalid_objc_cast)

struct ubsan_function_type_mismatch_data {
        UBSanSourceLocation           location;
        struct ubsan_type_descriptor* type;
};

void __ubsan_handle_function_type_mismatch(void* data_raw, void* value_raw) {
        struct ubsan_function_type_mismatch_data* data =
                (struct ubsan_function_type_mismatch_data*)data_raw;
        UBSanValueHandle value = (UBSanValueHandle)value_raw;
        (void)value;
        UBSanAbort(&data->location, "function type mismatch");
}

ABORT_VARIANT_VP_VP(function_type_mismatch)

struct ubsan_nonnull_return_v1_data {
        UBSanSourceLocation attr_location;
};

void __ubsan_handle_nonnull_return_v1(void* data_raw, void* location_raw) {
        struct ubsan_nonnull_return_v1_data* data = (struct ubsan_nonnull_return_v1_data*)data_raw;
        (void)data;
        UBSanSourceLocation* location = location_raw;
        UBSanAbort(location, "null return");
}

void __ubsan_handle_nullability_return_v1(void* data_raw, void* location_raw) {
        struct ubsan_nonnull_return_v1_data* data = (struct ubsan_nonnull_return_v1_data*)data_raw;
        (void)data;
        UBSanSourceLocation* location = location_raw;
        UBSanAbort(location, "nullability return");
}

ABORT_VARIANT_VP_VP(nonnull_return_v1)
ABORT_VARIANT_VP_VP(nullability_return_v1)

struct ubsan_nonnull_arg_data {
        UBSanSourceLocation location;
        UBSanSourceLocation attr_location;
        int                 arg_index;
};

void __ubsan_handle_nonnull_arg(void* data_raw) {
        struct ubsan_nonnull_arg_data* data = (struct ubsan_nonnull_arg_data*)data_raw;
        UBSanAbort(&data->location, "null argument");
}

void __ubsan_handle_nullability_arg(void* data_raw) {
        struct ubsan_nonnull_arg_data* data = (struct ubsan_nonnull_arg_data*)data_raw;
        UBSanAbort(&data->location, "nullability argument");
}

ABORT_VARIANT_VP(nonnull_arg)
ABORT_VARIANT_VP(nullability_arg)

struct ubsan_pointer_overflow_data {
        UBSanSourceLocation location;
};

void __ubsan_handle_pointer_overflow(void* data_raw, void* base_raw, void* result_raw) {
        struct ubsan_pointer_overflow_data* data   = (struct ubsan_pointer_overflow_data*)data_raw;
        UBSanValueHandle                    base   = (UBSanValueHandle)base_raw;
        UBSanValueHandle                    result = (UBSanValueHandle)result_raw;
        (void)base;
        (void)result;
        UBSanAbort(&data->location, "pointer overflow");
}

ABORT_VARIANT_VP_VP_VP(pointer_overflow)

struct ubsan_cfi_bad_icall_data {
        UBSanSourceLocation           location;
        struct ubsan_type_descriptor* type;
};

void __ubsan_handle_cfi_bad_icall(void* data_raw, void* value_raw) {
        struct ubsan_cfi_bad_icall_data* data  = (struct ubsan_cfi_bad_icall_data*)data_raw;
        UBSanValueHandle                 value = (UBSanValueHandle)value_raw;
        (void)value;
        UBSanAbort(&data->location, "control flow integrity check failure during indirect call");
}

ABORT_VARIANT_VP_VP(cfi_bad_icall)

struct ubsan_cfi_check_fail_data {
        unsigned char                 check_kind;
        UBSanSourceLocation           location;
        struct ubsan_type_descriptor* type;
};

void __ubsan_handle_cfi_check_fail(void* data_raw, void* function_raw, uintptr_t vtable_is_valid) {
        struct ubsan_cfi_check_fail_data* data     = (struct ubsan_cfi_check_fail_data*)data_raw;
        UBSanValueHandle                  function = (UBSanValueHandle)function_raw;
        (void)function;
        (void)vtable_is_valid;
        UBSanAbort(&data->location, "control flow integrity check failure");
}

ABORT_VARIANT_VP_VP_UP(cfi_check_fail)

void __ubsan_handle_cfi_bad_type(void* data_raw, void* function_raw, uintptr_t vtable_is_valid,
                                 void* report_options_raw) {
        struct ubsan_cfi_check_fail_data* data     = (struct ubsan_cfi_check_fail_data*)data_raw;
        UBSanValueHandle                  function = (UBSanValueHandle)function_raw;
        (void)function;
        (void)vtable_is_valid;
        (void)report_options_raw;
        UBSanAbort(&data->location, "control flow integrity bad type");
}

ABORT_VARIANT_VP_VP_UP_VP(cfi_bad_type)
