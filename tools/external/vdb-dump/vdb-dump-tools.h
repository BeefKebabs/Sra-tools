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

#ifndef _h_vdb_dump_tools_
#define _h_vdb_dump_tools_

#ifdef __cplusplus
extern "C" {
#endif

#ifndef _h_klib_defs_
#include <klib/defs.h>
#endif

#ifndef _h_vdb_dump_coldefs_
#include "vdb-dump-coldefs.h" /* because of p_col_def */
#endif

#ifndef _h_vdb_dump_context_
#include "vdb-dump-context.h" /* because of dump_format_t */
#endif

typedef struct dump_src
{
    const void *buf;
    uint32_t offset_in_bits;
    uint32_t element_idx;
    uint32_t number_of_elements;
    bool in_hex;
    bool print_dna_bases;
    bool without_sra_types;
    bool print_comma;
    bool translate_sra_values; /* legacy ??? */
    bool value_trans;   /* translate each value */
    bool group_trans;   /* translate a group of values */
    char c_boolean;
    dump_format_t output_format;
} dump_src;
typedef dump_src* p_dump_src;

rc_t vdt_format_cell_v1( const p_dump_src src, const p_col_def def, bool cell_debug );

rc_t vdt_format_cell_v2( const p_dump_src src, const p_col_def def, bool cell_debug );

#ifdef __cplusplus
}
#endif

#endif
