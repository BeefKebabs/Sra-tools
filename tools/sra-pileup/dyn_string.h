/*===========================================================================
*
*                            PUBLIC DOMAIN NOTICE
*               National Center for Biotechnology Information
*
*  This software/database is a "United States Government Work" under the
*  terms of the United States Copyright Act.  It was written as part of
*  the author's official duties as a United States Government employee and
*  thus cannot be copyrighted.  This software/database is freely available
*  to the public for use. The National Library of Medicine and the U.S.
*  Government have not placed any restriction on its use or reproduction.
*
*  Although all reasonable efforts have been taken to ensure the accuracy
*  and reliability of the software and data, the NLM and the U.S.
*  Government do not and cannot warrant the performance or results that
*  may be obtained by using this software or data. The NLM and the U.S.
*  Government disclaim all warranties, express or implied, including
*  warranties of performance, merchantability or fitness for any particular
*  purpose.
*
*  Please cite the author in any work or product based on this material.
*
* ===========================================================================
*
*/

#ifndef _h_dyn_string_
#define _h_dyn_string_

#ifdef __cplusplus
extern "C" {
#endif

#ifndef _h_klib_rc_
#include <klib/rc.h>
#endif

struct dyn_string;

rc_t ds_allocate( struct dyn_string **self, size_t size );
void ds_free( struct dyn_string *self );

void ds_reset( struct dyn_string *self );
rc_t ds_expand( struct dyn_string *self, size_t new_size );
rc_t ds_add_char( struct dyn_string *self, const char c );
rc_t ds_repeat_char( struct dyn_string *self, const char c, uint32_t n );
char * ds_get_char( struct dyn_string *self, uint32_t idx );
rc_t ds_add_str( struct dyn_string *self, const char * s );
rc_t ds_add_ds( struct dyn_string *self, struct dyn_string *other );
rc_t ds_add_fmt( struct dyn_string * self, const char *fmt, ... );
rc_t ds_print( struct dyn_string * self );
size_t ds_len( struct dyn_string * self );
rc_t ds_print_char_n( struct dyn_string *self, const char c, uint32_t n );

#ifdef __cplusplus
}
#endif

#endif /* dyn_string_ */
