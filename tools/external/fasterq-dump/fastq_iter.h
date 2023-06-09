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

#ifndef _h_fastq_iter_
#define _h_fastq_iter_

#ifdef __cplusplus
extern "C" {
#endif

#ifndef _h_klib_rc_
#include <klib/rc.h>
#endif

#ifndef _h_klib_text_
#include <klib/text.h>
#endif

#ifndef _h_cmn_iter_
#include "cmn_iter.h"
#endif

typedef struct fastq_iter_opt
{
   bool with_read_len;
   bool with_name;
   bool with_read_type;
   bool with_cmp_read;
   bool with_quality;
   bool with_spotgroup;
} fastq_iter_opt_t;

typedef struct fastq_rec_t
{
    int64_t row_id;
    uint32_t num_alig_id;
    uint64_t prim_alig_id[ 2 ];
    uint32_t num_read_len;
    uint32_t * read_len;
    uint32_t num_read_type;
    uint8_t * read_type;
    String name;
    String read;
    String quality;
    String spotgroup;
} fastq_rec_t;

struct fastq_csra_iter_t;

void destroy_fastq_csra_iter( struct fastq_csra_iter_t * self );
rc_t make_fastq_csra_iter( const cmn_iter_params_t * params,
                           fastq_iter_opt_t opt,
                           struct fastq_csra_iter_t ** iter );
                         
bool get_from_fastq_csra_iter( struct fastq_csra_iter_t * self, fastq_rec_t * rec, rc_t * rc );
uint64_t get_row_count_of_fastq_csra_iter( struct fastq_csra_iter_t * self );

struct fastq_sra_iter_t;

void destroy_fastq_sra_iter( struct fastq_sra_iter_t * self );
rc_t make_fastq_sra_iter( const cmn_iter_params_t * params,
                          fastq_iter_opt_t opt,
                          const char * tbl_name,
                          struct fastq_sra_iter_t ** iter );

bool get_from_fastq_sra_iter( struct fastq_sra_iter_t * self, fastq_rec_t * rec, rc_t * rc );
uint64_t get_row_count_of_fastq_sra_iter( struct fastq_sra_iter_t * self );

/* --------------------------------------------------------------------------------- */

typedef struct align_rec_t
{
    int64_t row_id;
    uint64_t spot_id;
    String read;
} align_rec_t;

struct align_iter_t;

void destroy_align_iter( struct align_iter_t * self );

rc_t make_align_iter( const cmn_iter_params_t * params,
                      struct align_iter_t ** iter );

bool get_from_align_iter( struct align_iter_t * self, align_rec_t * rec, rc_t * rc );
uint64_t get_row_count_of_align_iter( struct align_iter_t * self );

#ifdef __cplusplus
}
#endif

#endif
