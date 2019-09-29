#pragma once

C_HEADER_START
#include <sal.h>

#define PTR_SUCCESS                                 _Success_(NULL != return)
#define SIZE_SUCCESS                                _Success_(MAX_DWORD != return)
#define BOOL_SUCCESS                                _Success_(TRUE == return)

// The IN arguments must always be constants :)
#ifndef IN
#define IN                                          _In_ const
#endif // IN

#define IN_Z                                        _In_z_ const
#define IN_OPT                                      _In_opt_ const
#define IN_OPT_Z                                    _In_opt_z_ const
#define IN_READS(x)                                 _In_reads_((x)) const
#define IN_READS_BYTES(x)                           _In_reads_bytes_((x)) const
#define IN_READS_Z(x)                               _In_reads_or_z_((x)) const
#define IN_READS_OPT_Z(x)                           _In_reads_opt_z_((x)) const
#define IN_RANGE(a,b)                               IN _In_range_((a),(b))
#define IN_RANGE_UPPER(b)                           IN_RANGE(0,(b))
#define IN_RANGE_LOWER(a)                           IN_RANGE((a),MAX_DWORD)
#define IN_RANGE_LOWER64(a)                         IN_RANGE((a),MAX_QWORD)

#define INOUT                                       _Inout_
#define INOUT_OPT                                   _Inout_opt_
#define INOUT_UPDATES(x)                            _Inout_updates_((x))
#define INOUT_UPDATES_TO(x,y)                       _Inout_updates_to_((x),(y))
#define INOUT_UPDATES_ALL(x)                        _Inout_updates_all_((x))

#ifndef OUT
#define OUT                                         _Out_
#endif // OUT

#define OUT_Z                                       __out_z
#define OUT_PTR                                     _Outptr_
#define OUT_PTR_MAYBE_NULL                          _Outptr_result_maybenull_
#define OUT_OPT                                     _Out_opt_
#define OUT_OPT_PTR                                 _Outptr_opt_
#define OUT_OPT_PTR_MAYBE_NULL                      _Outptr_opt_result_maybenull_
#define OUT_WRITES(x)                               _Out_writes_((x))
#define OUT_WRITES_Z(x)                             _Out_writes_z_((x))
#define OUT_WRITES_OPT(x)                           _Out_writes_opt_((x))
#define OUT_WRITES_ALL(x)                           _Out_writes_all_((x))
#define OUT_WRITES_ALL_OPT(x)                       _Out_writes_all_opt_((x))
#define OUT_WRITES_BYTES(x)                         _Out_writes_bytes_((x))
#define OUT_WRITES_BYTES_OPT(x)                     _Out_writes_bytes_opt_((x))
#define OUT_WRITES_BYTES_ALL(x)                     _Out_writes_bytes_all_((x))
#define OUT_WRITES_BYTES_ALL_OPT(x)                 _Out_writes_bytes_all_opt_((x))

// locking annotations
#define ACQUIRES_EXCL_AND_NON_REENTRANT_LOCK(x)     _Acquires_exclusive_lock_((x)) \
                                                    _Acquires_nonreentrant_lock_((x))
#define ACQUIRES_SHARED_AND_NON_REENTRANT_LOCK(x)    _Acquires_shared_lock_((x)) \
                                                    _Acquires_nonreentrant_lock_((x))
#define ACQUIRES_EXCL_AND_REENTRANT_LOCK(x)         _Acquires_exclusive_lock_((x))

#define RELEASES_EXCL_AND_NON_REENTRANT_LOCK(x)     _Releases_exclusive_lock_((x)) \
                                                    _Releases_nonreentrant_lock_((x))
#define RELEASES_SHARED_AND_NON_REENTRANT_LOCK(x)   _Releases_shared_lock_((x)) \
                                                    _Releases_nonreentrant_lock_((x))
#define RELEASES_EXCL_AND_REENTRANT_LOCK(x)         _Releases_exclusive_lock_((x))

#define REQUIRES_EXCL_LOCK(x)                       _Requires_exclusive_lock_held_((x))
#define REQUIRES_SHARED_LOCK(x)                     _Requires_shared_lock_held_((x))
#define REQUIRES_NOT_HELD_LOCK(x)                   _Requires_lock_not_held_((x))

// return types
#define RET_NOT_NULL                                _Ret_notnull_

#include "sal_intrinsic.h"
C_HEADER_END
