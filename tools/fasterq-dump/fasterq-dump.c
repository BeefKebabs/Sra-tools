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

#ifndef _h_err_msg_
#include "err_msg.h"
#endif

#ifndef _h_arg_helper_
#include "arg_helper.h"
#endif

#ifndef _h_tool_ctx_
#include "tool_ctx.h"
#endif

#ifndef _h_inspector_
#include "inspector.h"
#endif

#ifndef _h_file_tools_
#include "file_tools.h"
#endif

#ifndef _h_sorter_
#include "sorter.h"
#endif

#ifndef _h_join_
#include "join.h"
#endif

#ifndef _h_tbl_join_
#include "tbl_join.h"
#endif

#ifndef _h_kapp_main_
#include <kapp/main.h>
#endif

#ifndef _h_kapp_args_
#include <kapp/args.h>
#endif

#ifndef _h_kfg_config_
#include <kfg/config.h> /* KConfigSetNgcFile */
#endif

#ifndef _h_klib_out_
#include <klib/out.h>
#endif

#ifndef _h_klib_printf_
#include <klib/printf.h>
#endif

/* ---------------------------------------------------------------------------------- */

static const char * format_usage[] = { "format (special, fastq, default=fastq)", NULL };
#define OPTION_FORMAT   "format"
#define ALIAS_FORMAT    "F"

static const char * outputf_usage[] = { "output-file", NULL };
#define OPTION_OUTPUT_F "outfile"
#define ALIAS_OUTPUT_F  "o"

static const char * outputd_usage[] = { "output-dir", NULL };
#define OPTION_OUTPUT_D "outdir"
#define ALIAS_OUTPUT_D  "O"

static const char * progress_usage[] = { "show progress", NULL };
#define OPTION_PROGRESS "progress"
#define ALIAS_PROGRESS  "p"

static const char * bufsize_usage[] = { "size of file-buffer dflt=1MB", NULL };
#define OPTION_BUFSIZE  "bufsize"
#define ALIAS_BUFSIZE   "b"

static const char * curcache_usage[] = { "size of cursor-cache dflt=10MB", NULL };
#define OPTION_CURCACHE "curcache"
#define ALIAS_CURCACHE  "c"

static const char * mem_usage[] = { "memory limit for sorting dflt=100MB", NULL };
#define OPTION_MEM      "mem"
#define ALIAS_MEM       "m"

static const char * temp_usage[] = { "where to put temp. files dflt=curr dir", NULL };
#define OPTION_TEMP     "temp"
#define ALIAS_TEMP      "t"

static const char * threads_usage[] = { "how many thread dflt=6", NULL };
#define OPTION_THREADS  "threads"
#define ALIAS_THREADS   "e"

static const char * detail_usage[] = { "print details", NULL };
#define OPTION_DETAILS  "details"
#define ALIAS_DETAILS    "x"

static const char * split_spot_usage[] = { "split spots into reads", NULL };
#define OPTION_SPLIT_SPOT "split-spot"
#define ALIAS_SPLIT_SPOT  "s"

static const char * split_file_usage[] = { "write reads into different files", NULL };
#define OPTION_SPLIT_FILE "split-files"
#define ALIAS_SPLIT_FILE  "S"

static const char * split_3_usage[] = { "writes single reads in special file", NULL };
#define OPTION_SPLIT_3   "split-3"
#define ALIAS_SPLIT_3    "3"

static const char * whole_spot_usage[] = { "writes whole spots into one file", NULL };
#define OPTION_WHOLE_SPOT   "concatenate-reads"

static const char * stdout_usage[] = { "print output to stdout", NULL };
#define OPTION_STDOUT    "stdout"
#define ALIAS_STDOUT     "Z"

static const char * force_usage[] = { "force to overwrite existing file(s)", NULL };
#define OPTION_FORCE     "force"
#define ALIAS_FORCE      "f"

static const char * skip_tech_usage[] = { "skip technical reads", NULL };
#define OPTION_SKIP_TECH      "skip-technical"

static const char * incl_tech_usage[] = { "include technical reads", NULL };
#define OPTION_INCL_TECH      "include-technical"

static const char * min_rl_usage[] = { "filter by sequence-len", NULL };
#define OPTION_MINRDLEN  "min-read-len"
#define ALIAS_MINRDLEN   "M"

static const char * base_flt_usage[] = { "filter by bases", NULL };
#define OPTION_BASE_FLT  "bases"
#define ALIAS_BASE_FLT   "B"

static const char * table_usage[] = { "which seq-table to use in case of pacbio", NULL };
#define OPTION_TABLE    "table"

static const char * append_usage[] = { "append to output-file", NULL };
#define OPTION_APPEND           "append"
#define ALIAS_APPEND            "A"

static const char * fasta_usage[] = { "produce FASTA output", NULL };
#define OPTION_FASTA            "fasta"

static const char * fasta_us_usage[] = { "produce FASTA output, unsorted", NULL };
#define OPTION_FASTA_US         "fasta-unsorted"

static const char * seq_defline_usage[] = { "custom defline for sequence: ",
                                            "$ac=accession, $sn=spot-name, ",
                                            "$sg=spot-group, $si=spot-id, ",
                                            "$ri=read-id, $rl=read-length",                                            
                                            NULL };
#define OPTION_SEQ_DEFLINE      "seq-defline"

static const char * qual_defline_usage[] = { "custom defline for qualities: ",
                                             "same as seq-defline",
                                             NULL };
#define OPTION_QUAL_DEFLINE     "qual-defline"

static const char * only_un_usage[] = { "process only unaligned reads", NULL };
#define OPTION_ONLY_UN          "only-unaligned"
#define ALIAS_ONLY_UN           "U"

static const char * only_a_usage[] = { "process only aligned reads", NULL };
#define OPTION_ONLY_ALIG        "only-aligned"
#define ALIAS_ONLY_ALIG         "a"

static const char * row_limit_usage[] = { "limit rowcount per thread", NULL };
#define OPTION_ROW_LIMIT        "row-limit"
#define ALIAS_ROW_LIMIT         "l"

static const char * check_usage[] = { "switch to control:",
                                      "on=perform size-check (default), ",
                                      "off=do not perform size-check, ",
                                      "only=perform size-check only",
                                      NULL };
#define OPTION_CHECK            "size-check"

static const char * disk_limit_out_usage[] = { "explicitly set disk-limit", NULL };
#define OPTION_DISK_LIMIT_OUT   "disk-limit"

static const char * disk_limit_tmp_usage[] = { "explicitly set disk-limit for temp. files", NULL };
#define OPTION_DISK_LIMIT_TMP   "disk-limit-tmp"

static const char * ngc_usage[] = { "PATH to ngc file", NULL };
#define OPTION_NGC              "ngc"

/* ---------------------------------------------------------------------------------- */

OptDef ToolOptions[] = {
    { OPTION_FORMAT,        ALIAS_FORMAT,       NULL, format_usage,         1, true,   false },
    { OPTION_OUTPUT_F,      ALIAS_OUTPUT_F,     NULL, outputf_usage,        1, true,   false },
    { OPTION_OUTPUT_D,      ALIAS_OUTPUT_D,     NULL, outputd_usage,        1, true,   false },
    { OPTION_BUFSIZE,       ALIAS_BUFSIZE,      NULL, bufsize_usage,        1, true,   false },
    { OPTION_CURCACHE,      ALIAS_CURCACHE,     NULL, curcache_usage,       1, true,   false },
    { OPTION_MEM,           ALIAS_MEM,          NULL, mem_usage,            1, true,   false },
    { OPTION_TEMP,          ALIAS_TEMP,         NULL, temp_usage,           1, true,   false },
    { OPTION_THREADS,       ALIAS_THREADS,      NULL, threads_usage,        1, true,   false },
    { OPTION_PROGRESS,      ALIAS_PROGRESS,     NULL, progress_usage,       1, false,  false },
    { OPTION_DETAILS,       ALIAS_DETAILS,      NULL, detail_usage,         1, false,  false },
    { OPTION_SPLIT_SPOT,    ALIAS_SPLIT_SPOT,   NULL, split_spot_usage,     1, false,  false },
    { OPTION_SPLIT_FILE,    ALIAS_SPLIT_FILE,   NULL, split_file_usage,     1, false,  false },
    { OPTION_SPLIT_3,       ALIAS_SPLIT_3,      NULL, split_3_usage,        1, false,  false },
    { OPTION_WHOLE_SPOT,    NULL,               NULL, whole_spot_usage,     1, false,  false },    
    { OPTION_STDOUT,        ALIAS_STDOUT,       NULL, stdout_usage,         1, false,  false },
    { OPTION_FORCE,         ALIAS_FORCE,        NULL, force_usage,          1, false,  false },
    { OPTION_SKIP_TECH,     NULL,               NULL, skip_tech_usage,      1, false,  false },
    { OPTION_INCL_TECH,     NULL,               NULL, incl_tech_usage,      1, false,  false },
    { OPTION_MINRDLEN,      ALIAS_MINRDLEN,     NULL, min_rl_usage,         1, true,   false },
    { OPTION_TABLE,         NULL,               NULL, table_usage,          1, true,   false },
    { OPTION_BASE_FLT,      ALIAS_BASE_FLT,     NULL, base_flt_usage,       10, true,  false },
    { OPTION_APPEND,        ALIAS_APPEND,       NULL, append_usage,         1, false,  false },
    { OPTION_FASTA,         NULL,               NULL, fasta_usage,          1, false,  false },
    { OPTION_FASTA_US,      NULL,               NULL, fasta_us_usage,       1, false,  false },
    { OPTION_SEQ_DEFLINE,   NULL,               NULL, seq_defline_usage,    1, true,   false },
    { OPTION_QUAL_DEFLINE,  NULL,               NULL, qual_defline_usage,   1, true,   false },
    { OPTION_ONLY_UN,       ALIAS_ONLY_UN,      NULL, only_un_usage,        1, false,  false },
    { OPTION_ONLY_ALIG,     ALIAS_ONLY_ALIG,    NULL, only_a_usage,         1, false,  false },
    { OPTION_ROW_LIMIT,     ALIAS_ROW_LIMIT,    NULL, row_limit_usage,      1, true,   false },
    { OPTION_DISK_LIMIT_OUT,NULL,               NULL, disk_limit_out_usage, 1, true,   false },
    { OPTION_DISK_LIMIT_TMP,NULL,               NULL, disk_limit_tmp_usage, 1, true,   false },    
    { OPTION_CHECK,         NULL,               NULL, check_usage,          1, true,   false },
    { OPTION_NGC,           NULL,               NULL, ngc_usage,            1, true,   false }
};

/* ----------------------------------------------------------------------------------- */

const char UsageDefaultName[] = "fasterq-dump";

/* ----------------------------------------------------------------------------------- */

rc_t CC UsageSummary( const char * progname ) {
    return KOutMsg( "\n"
                     "Usage:\n"
                     "  %s <path> [options]\n"
                     "\n", progname );
}

/* ----------------------------------------------------------------------------------- */

rc_t CC Usage ( const Args * args ) {
    rc_t rc;
    uint32_t idx, count = ( sizeof ToolOptions ) / ( sizeof ToolOptions[ 0 ] );
    const char * progname = UsageDefaultName;
    const char * fullpath = UsageDefaultName;

    if ( NULL == args ) {
        rc = RC( rcApp, rcArgv, rcAccessing, rcSelf, rcNull );
    } else {
        rc = ArgsProgram( args, &fullpath, &progname );
    }

    if ( 0 != rc ) {
        progname = fullpath = UsageDefaultName;
    }

    UsageSummary( progname );

    KOutMsg( "Options:\n" );
    for ( idx = 1; idx < count; ++idx ) { /* start with 1, do not advertize row-range-option*/
        const OptDef * opt = &ToolOptions[ idx ];
        const char * param = NULL;

        assert( opt );
        if ( 0 == strcmp( opt -> name, OPTION_NGC ) ) { param = "PATH"; }
        HelpOptionLine( opt -> aliases, opt -> name, param, opt -> help );
    }
    
    KOutMsg( "\n" );
    HelpOptionsStandard();

    KOutMsg( "for more information visit:\n" );
    KOutMsg( "   https://github.com/ncbi/sra-tools/wiki/HowTo:-fasterq-dump\n" );
    KOutMsg( "   https://github.com/ncbi/sra-tools/wiki/08.-prefetch-and-fasterq-dump\n" );

    HelpVersion( fullpath, KAppVersion() );

    return rc;
}

/* -------------------------------------------------------------------------------------------- */

static const char * dflt_seq_tabl_name = "SEQUENCE";

#define DFLT_CUR_CACHE ( 5 * 1024 * 1024 )
#define DFLT_BUF_SIZE ( 1024 * 1024 )
#define DFLT_MEM_LIMIT ( 1024L * 1024 * 50 )
#define DFLT_NUM_THREADS 6
static rc_t get_user_input( tool_ctx_t * tool_ctx, const Args * args ) {
    bool split_spot, split_file, split_3, whole_spot, fasta, fasta_us;

    rc_t rc = ArgsParamValue( args, 0, ( const void ** )&( tool_ctx -> accession_path ) );
    if ( 0 != rc ) {
        ErrMsg( "ArgsParamValue() -> %R", rc );
    }

    tool_ctx -> cursor_cache = get_size_t_option( args, OPTION_CURCACHE, DFLT_CUR_CACHE );
    tool_ctx -> show_progress = get_bool_option( args, OPTION_PROGRESS );
    tool_ctx -> show_details = get_bool_option( args, OPTION_DETAILS );
    tool_ctx -> requested_temp_path = get_str_option( args, OPTION_TEMP, NULL );
    tool_ctx -> force = get_bool_option( args, OPTION_FORCE );        
    tool_ctx -> output_filename = get_str_option( args, OPTION_OUTPUT_F, NULL );
    tool_ctx -> output_dirname = get_str_option( args, OPTION_OUTPUT_D, NULL );
    tool_ctx -> buf_size = get_size_t_option( args, OPTION_BUFSIZE, DFLT_BUF_SIZE );
    tool_ctx -> mem_limit = get_size_t_option( args, OPTION_MEM, DFLT_MEM_LIMIT );
    tool_ctx -> row_limit = get_uint64_t_option( args, OPTION_ROW_LIMIT, 0 );
    tool_ctx -> disk_limit_out_cmdl = get_size_t_option( args, OPTION_DISK_LIMIT_OUT, 0 );
    tool_ctx -> disk_limit_tmp_cmdl = get_size_t_option( args, OPTION_DISK_LIMIT_TMP, 0 );
    tool_ctx -> num_threads = get_uint32_t_option( args, OPTION_THREADS, DFLT_NUM_THREADS );
    
    /* join_options_t is defined in helper.h */
    tool_ctx -> join_options . rowid_as_name = false;
    tool_ctx -> join_options . skip_tech = !( get_bool_option( args, OPTION_INCL_TECH ) );
    tool_ctx -> join_options . min_read_len = get_uint32_t_option( args, OPTION_MINRDLEN, 0 );
    tool_ctx -> join_options . filter_bases = get_str_option( args, OPTION_BASE_FLT, NULL );

    split_spot = get_bool_option( args, OPTION_SPLIT_SPOT );
    split_file = get_bool_option( args, OPTION_SPLIT_FILE );
    split_3    = get_bool_option( args, OPTION_SPLIT_3 );
    whole_spot = get_bool_option( args, OPTION_WHOLE_SPOT );
    fasta      = get_bool_option( args, OPTION_FASTA );
    fasta_us   = get_bool_option( args, OPTION_FASTA_US );

    if ( 0 == rc && split_spot && split_file ) {
        rc = RC( rcExe, rcFile, rcPacking, rcName, rcInvalid );
        ErrMsg( "split-spot and split-file exclude each other -> %R", rc );
    }

    tool_ctx -> fmt = get_format_t( get_str_option( args, OPTION_FORMAT, NULL ),
                    split_spot, split_file, split_3, whole_spot, fasta, fasta_us ); /* helper.c */
    if ( ft_fastq_split_3 == tool_ctx -> fmt ) {
        tool_ctx -> join_options . skip_tech = true;
    }

    tool_ctx -> check_mode = get_check_mode_t( get_str_option( args, OPTION_CHECK, "on" ) );
    if ( 0 == rc && cmt_unknown == tool_ctx -> check_mode ) {
        rc = RC( rcExe, rcFile, rcPacking, rcName, rcUnknown  );
        ErrMsg( "invalid check-mode -> %R", rc );
    }
    
    tool_ctx -> requested_seq_tbl_name = get_str_option( args, OPTION_TABLE, dflt_seq_tabl_name );
    tool_ctx -> append = get_bool_option( args, OPTION_APPEND );
    tool_ctx -> use_stdout = get_bool_option( args, OPTION_STDOUT );

    tool_ctx -> seq_defline = get_str_option( args, OPTION_SEQ_DEFLINE, NULL );
    tool_ctx -> qual_defline = get_str_option( args, OPTION_QUAL_DEFLINE, NULL );    
    tool_ctx -> only_unaligned = get_bool_option( args, OPTION_ONLY_UN );
    tool_ctx -> only_aligned = get_bool_option( args, OPTION_ONLY_ALIG );
    
    {
        const char * ngc = get_str_option( args, OPTION_NGC, NULL );
        if ( NULL != ngc ) {
            KConfigSetNgcFile( ngc );
        }
    }
    return rc;
}


/* --------------------------------------------------------------------------------------------
    produce special-output ( SPOT_ID,READ,SPOT_GROUP ) by iterating over the SEQUENCE - table:
    produce fastq-output by iterating over the SEQUENCE - table:
   -------------------------------------------------------------------------------------------- 
   each thread iterates over a slice of the SEQUENCE-table
   for each SPOT it may look up an entry in the lookup-table to get the READ
   if it is not stored in the SEQ-tbl
-------------------------------------------------------------------------------------------- */

static const uint32_t queue_timeout = 200;  /* ms */

static rc_t produce_lookup_files( const tool_ctx_t * tool_ctx ) {
    rc_t rc = 0;
    struct bg_update_t * gap = NULL;                    /* merge_sorter.h */
    struct background_file_merger_t * bg_file_merger;   /* merge_sorter.h */
    struct background_vector_merger_t * bg_vec_merger;  /* merge_sorter.h */
    uint64_t align_row_count = tool_ctx -> insp_output . align . row_count;
    
    if ( tool_ctx -> show_progress ) {
        rc = bg_update_make( &gap, 0 );
    }
   
    /* the background-file-merger catches the files produced by
        the background-vector-merger */
    if ( 0 == rc ) {
        file_merger_args_t fm_args; /* merge_sorter.h */

        fm_args . dir = tool_ctx -> dir;
        fm_args . temp_dir = tool_ctx -> temp_dir;
        fm_args . cleanup_task = tool_ctx -> cleanup_task;
        fm_args . lookup_filename = tool_ctx -> lookup_filename;
        fm_args . index_filename = tool_ctx -> index_filename;
        fm_args . batch_size = tool_ctx -> num_threads;
        fm_args . wait_time = queue_timeout;
        fm_args . buf_size = tool_ctx -> buf_size;
        fm_args . gap = gap;

        rc = make_background_file_merger( &bg_file_merger, &fm_args ); /* merge_sorter.c */
    }

    /* the background-vector-merger catches the KVectors produced by
       the lookup-produceer */
    if ( 0 == rc ) {
        vector_merger_args_t vm_args; /* merge_sorter.c */
        
        vm_args . dir = tool_ctx -> dir;
        vm_args . temp_dir = tool_ctx -> temp_dir;
        vm_args . cleanup_task = tool_ctx -> cleanup_task;
        vm_args . file_merger = bg_file_merger;
        vm_args . batch_size = tool_ctx -> num_threads;
        vm_args . q_wait_time = queue_timeout;
        vm_args . buf_size = tool_ctx -> buf_size;
        vm_args . gap = gap;

        rc = make_background_vector_merger( &bg_vec_merger, &vm_args ); /* merge_sorter.c */
    }
   
/* --------------------------------------------------------------------------------------------
    produce the lookup-table by iterating over the PRIMARY_ALIGNMENT - table:
   -------------------------------------------------------------------------------------------- 
    reading SEQ_SPOT_ID, SEQ_READ_ID and RAW_READ
    SEQ_SPOT_ID and SEQ_READ_ID is merged into a 64-bit-key
    RAW_READ is read as 4na-unpacked ( Schema does not provide 4na-packed for this column )
    these key-pairs are temporarely stored in a KVector until a limit is reached
    after that limit is reached they are pushed to the background-vector-merger
    This KVector looks like this:
    content: [KEY][RAW_READ]
    KEY... 64-bit value as SEQ_SPOT_ID shifted left by 1 bit, zero-bit contains SEQ_READ_ID
    RAW_READ... 16-bit binary-chunk-lenght, followed by n bytes of packed 4na
-------------------------------------------------------------------------------------------- */
    /* the lookup-producer is the source of the chain */
    if ( 0 == rc ) {

        lookup_production_args_t args;

        args . dir = tool_ctx -> dir;
        args . vdb_mgr = tool_ctx -> vdb_mgr;
        args . accession_short = tool_ctx -> accession_short;
        args . accession_path = tool_ctx -> accession_path;        
        args . merger = bg_vec_merger;
        args . align_row_count = align_row_count;
        args . cursor_cache = tool_ctx -> cursor_cache;
        args . buf_size = tool_ctx -> buf_size;
        args . mem_limit = tool_ctx -> mem_limit;
        args . num_threads = tool_ctx -> num_threads;
        args . show_progress = tool_ctx -> show_progress;

        rc = execute_lookup_production( &args ); /* sorter.c */
    }
    bg_update_start( gap, "merge  : " ); /* progress_thread.c ...start showing the activity... */

    if ( 0 == rc ) {
        rc = wait_for_and_release_background_vector_merger( bg_vec_merger ); /* merge_sorter.c */
    }

    if ( 0 == rc ) {
        rc = wait_for_and_release_background_file_merger( bg_file_merger ); /* merge_sorter.c */
    }

    bg_update_release( gap );

    if ( 0 == rc ) {
        if ( tool_ctx -> show_details ) {
            uint64_t lookup_size = file_size( tool_ctx -> dir, tool_ctx -> lookup_filename ); /* file_tools.c */
            uint64_t index_size = file_size( tool_ctx -> dir, tool_ctx -> index_filename ); /* file_tools.c */
            KOutMsg( "lookup = %,lu bytes\nindex = %,lu bytes\n", lookup_size, index_size );
        }
    } else {
        ErrMsg( "fasterq-dump.c produce_lookup_files() -> %R", rc );
    }

    return rc;
}

/* -------------------------------------------------------------------------------------------- */

static rc_t produce_final_db_output( const tool_ctx_t * tool_ctx ) {
    struct temp_registry_t * registry = NULL; /* temp_registry.h */
    join_stats_t stats; /* helper.h */
    execute_db_join_args_t args; /* join.h */

    rc_t rc = make_temp_registry( &registry, tool_ctx -> cleanup_task ); /* temp_registry.c */
    
    clear_join_stats( &stats ); /* helper.c */
    /* join SEQUENCE-table with lookup-table === this is the actual purpos of the tool === */
    
/* --------------------------------------------------------------------------------------------
    produce special-output ( SPOT_ID,READ,SPOT_GROUP ) by iterating over the SEQUENCE - table:
    produce fastq-output by iterating over the SEQUENCE - table:
   -------------------------------------------------------------------------------------------- 
   each thread iterates over a slice of the SEQUENCE-table
   for each SPOT it may look up an entry in the lookup-table to get the READ
   if it is not stored in the SEQ-tbl
-------------------------------------------------------------------------------------------- */

    args . dir = tool_ctx -> dir;
    args . vdb_mgr = tool_ctx -> vdb_mgr;
    args . accession_path = tool_ctx -> accession_path;
    args . accession_short = tool_ctx -> accession_short;
    args . seq_defline = tool_ctx -> seq_defline;
    args . qual_defline = tool_ctx -> qual_defline;
    args . lookup_filename = &( tool_ctx -> lookup_filename[ 0 ] );
    args . index_filename = &( tool_ctx -> index_filename[ 0 ] );
    args . stats = &stats;
    args . insp_output = &( tool_ctx -> insp_output );
    args . join_options = &( tool_ctx -> join_options );
    args . temp_dir = tool_ctx -> temp_dir;
    args . registry= registry;
    args . cursor_cache = tool_ctx -> cursor_cache;
    args . buf_size = tool_ctx -> buf_size;
    args . num_threads = tool_ctx -> num_threads;
    args . row_limit = tool_ctx -> row_limit;
    args . show_progress = tool_ctx -> show_progress;
    args . fmt = tool_ctx -> fmt;

    if ( rc == 0 ) {
        rc = execute_db_join( &args ); /* join.c */
    }

    /* from now on we do not need the lookup-file and it's index any more... */
    if ( 0 != tool_ctx -> lookup_filename[ 0 ] ) {
        KDirectoryRemove( tool_ctx -> dir, true, "%s", &tool_ctx -> lookup_filename[ 0 ] );
    }

    if ( 0 != tool_ctx -> index_filename[ 0 ] ) {
        KDirectoryRemove( tool_ctx -> dir, true, "%s", &tool_ctx -> index_filename[ 0 ] );
    }

    /* STEP 4 : concatenate output-chunks */
    if ( 0 == rc ) {
        if ( tool_ctx -> use_stdout ) {
            rc = temp_registry_to_stdout( registry,
                                          tool_ctx -> dir,
                                          tool_ctx -> buf_size ); /* temp_registry.c */
        } else {
            rc = temp_registry_merge( registry,
                              tool_ctx -> dir,
                              tool_ctx -> output_filename,
                              tool_ctx -> buf_size,
                              tool_ctx -> show_progress,
                              tool_ctx -> force,
                              tool_ctx -> append ); /* temp_registry.c */
        }
    }

    /* in case some of the partial results have not been deleted be the concatenator */
    if ( NULL != registry ) {
        destroy_temp_registry( registry ); /* temp_registry.c */
    }

    print_stats( &stats ); /* above */

    return rc;
}

static rc_t process_csra_fasta_unsorted( const tool_ctx_t * tool_ctx ) {
    rc_t rc;

    join_stats_t stats; /* helper.h */
    execute_unsorted_fasta_db_join_args_t args; /* join.h */

    clear_join_stats( &stats ); /* helper.c */

    args . dir = tool_ctx -> dir;
    args . vdb_mgr = tool_ctx -> vdb_mgr;
    args . accession_short = tool_ctx -> accession_short;
    args . accession_path = tool_ctx -> accession_path;
    args . output_filename = tool_ctx -> use_stdout ? NULL : tool_ctx -> output_filename;
    args . seq_defline = tool_ctx -> seq_defline;
    args . stats = &stats;
    args . insp_output = &( tool_ctx -> insp_output );
    args . join_options = &( tool_ctx -> join_options );
    args . cur_cache = tool_ctx -> cursor_cache;
    args . buf_size = tool_ctx -> buf_size;
    args . num_threads = tool_ctx -> num_threads;
    args . row_limit = tool_ctx -> row_limit;
    args . show_progress = tool_ctx -> show_progress;
    args . force = tool_ctx -> force;
    args . only_unaligned = tool_ctx -> only_unaligned;
    args . only_aligned = tool_ctx -> only_aligned;

    rc = execute_unsorted_fasta_db_join( &args ); /* join.c */

    print_stats( &stats ); /* helper.c */

    return rc;
}

/* ============================================================================================
    >>>>> cSRA <<<<<
   ============================================================================================ */

static rc_t process_csra( const tool_ctx_t * tool_ctx ) {
    rc_t rc;
    
    if ( tool_ctx -> fmt != ft_fasta_us_split_spot ) {

        /* the common case the other cominations of FASTA/FASTQ : */
        rc = produce_lookup_files( tool_ctx ); /* above */
        if ( 0 == rc ) {
            rc = produce_final_db_output( tool_ctx ); /* above */
        }

    } else {
        
        /* the special case of fasta-unsorted and split-spot : */
        rc = process_csra_fasta_unsorted( tool_ctx ); /* above */
    }
    return rc;
}

/* ============================================================================================ */

static rc_t process_table_in_seq_order( const tool_ctx_t * tool_ctx, const char * tbl_name ) {
    rc_t rc = 0;
    join_stats_t stats; /* helper.h */
    struct temp_registry_t * registry = NULL;   /* temp_registry.h */
        
    clear_join_stats( &stats ); /* helper.c */

    rc = make_temp_registry( &registry, tool_ctx -> cleanup_task ); /* temp_registry.c */

    if ( 0 == rc ) {
        
        execute_tbl_join_args_t args; /* tbl_join.h */

        args . dir = tool_ctx -> dir;
        args . vdb_mgr = tool_ctx -> vdb_mgr;
        args . accession_short = tool_ctx -> accession_short;
        args . accession_path = tool_ctx -> accession_path;
        args . seq_defline = tool_ctx -> seq_defline;
        args . qual_defline = tool_ctx -> qual_defline;
        args . tbl_name = tbl_name;
        args . stats = &stats;
        args . insp_output = &( tool_ctx -> insp_output );
        args . join_options = &( tool_ctx -> join_options );
        args . temp_dir = tool_ctx -> temp_dir;
        args . registry = registry;
        args . cursor_cache = tool_ctx -> cursor_cache;
        args . buf_size = tool_ctx -> buf_size;
        args . num_threads = tool_ctx -> num_threads;
        args . show_progress = tool_ctx -> show_progress;
        args . fmt = tool_ctx -> fmt;
        args . row_limit = tool_ctx -> row_limit;

        rc = execute_tbl_join( &args ); /* tbl_join.c */
    }

    if ( 0 == rc ) {
        if ( tool_ctx -> use_stdout ) {
            rc = temp_registry_to_stdout( registry,
                                        tool_ctx -> dir,
                                        tool_ctx -> buf_size ); /* temp_registry.c */
        } else {
            rc = temp_registry_merge( registry,
                            tool_ctx -> dir,
                            tool_ctx -> output_filename,
                            tool_ctx -> buf_size,
                            tool_ctx -> show_progress,
                            tool_ctx -> force,
                            tool_ctx -> append ); /* temp_registry.c */
        }
    }
    
    if ( NULL != registry ) {
        destroy_temp_registry( registry ); /* temp_registry.c */
    }

    print_stats( &stats ); /* helper.c */

    return rc;
}

static rc_t process_table_fasta_unsorted( const tool_ctx_t * tool_ctx, const char * tbl_name ) {
    rc_t rc = 0;
    join_stats_t stats; /* helper.h */
    execute_fasta_tbl_join_args_t args; /* tbl_join.h */
        
    clear_join_stats( &stats ); /* helper.c */

    args . dir = tool_ctx -> dir;
    args . vdb_mgr = tool_ctx -> vdb_mgr;
    args . accession_short = tool_ctx -> accession_short;
    args . accession_path = tool_ctx -> accession_path;
    args . output_filename = tool_ctx -> use_stdout ? NULL : tool_ctx -> output_filename;
    args . seq_defline = tool_ctx -> seq_defline;
    args . tbl_name = tbl_name;
    args . stats = &stats;
    args . insp_output = &( tool_ctx -> insp_output );
    args . join_options = &( tool_ctx -> join_options );
    args . cursor_cache = tool_ctx -> cursor_cache;
    args . buf_size = tool_ctx -> buf_size;
    args . num_threads = tool_ctx -> num_threads;
    args . show_progress = tool_ctx -> show_progress;
    args . force = tool_ctx -> force;
    args . row_limit = tool_ctx -> row_limit;
    
    rc = execute_unsorted_fasta_tbl_join( &args ); /* tbl_join.c */

    print_stats( &stats ); /* helper.c */

    return rc;
}

/* ============================================================================================
    >>>>> flat table <<<<< ( tbl_name is NULL for raw-table, otherwise db with a table only ! )
   ============================================================================================ */

static rc_t process_table( const tool_ctx_t * tool_ctx, const char * tbl_name ) {
    rc_t rc = 0;
    
    if ( !tool_ctx -> only_aligned ) {

        if ( tool_ctx -> fmt != ft_fasta_us_split_spot ) {
            rc = process_table_in_seq_order( tool_ctx, tbl_name ); /* above */
        } else {
            rc = process_table_fasta_unsorted( tool_ctx, tbl_name ); /* above */
        }
    }

    return rc;
}

/* ============================================================================================ */

rc_t CC KMain ( int argc, char *argv [] ) {
    Args * args;
    uint32_t num_options = sizeof ToolOptions / sizeof ToolOptions [ 0 ];

    rc_t rc = ArgsMakeAndHandle ( &args, argc, argv, 1, ToolOptions, num_options );
    if ( 0 != rc ) {
        ErrMsg( "ArgsMakeAndHandle() -> %R", rc );
    } else {
        uint32_t param_count;
        rc = ArgsParamCount( args, &param_count );
        if ( 0 != rc ) {
            ErrMsg( "ArgsParamCount() -> %R", rc );
        } else {
            /* in case we are given no or more than one accessions/files to process */
            if ( param_count == 0 || param_count > 1 ) {
                Usage ( args );
                /* will make the caller of this function aka KMane() in man.c return
                error code of 3 */
                rc = 3;
            } else {
                tool_ctx_t tool_ctx;    /* tool_ctx.h */

                memset( &tool_ctx, 0, sizeof tool_ctx );

                rc = get_user_input( &tool_ctx, args ); /* above: get argument and options from args */
                if ( 0 == rc ) {
                    rc = populate_tool_ctx( &tool_ctx ); /* tool_ctx.c */
                }

                if ( 0 == rc && !( cmt_only == tool_ctx . check_mode ) ) {
                    switch( tool_ctx . insp_output . acc_type ) {
                        /* a cSRA-database with alignments */
                        case acc_csra       : rc = process_csra( &tool_ctx ); break; /* above */

                        /* a PACBIO-database */
                        case acc_pacbio     : ErrMsg( "accession '%s' is PACBIO, please use fastq-dump instead", tool_ctx . accession_path );
                                                rc = 3; /* signal to main() that the accession is not-processed */
                                                break;

                        /* a flat SRA-table */
                        case acc_sra_flat   : rc = process_table( &tool_ctx, NULL ); break; /* above */

                        /* a SRA-database, containing only unaligned data */
                        case acc_sra_db     : rc = process_table( &tool_ctx, tool_ctx . insp_output . seq . tbl_name ); /* above */
                                                break;

                        default             : ErrMsg( "invalid accession '%s'", tool_ctx . accession_path );
                                                rc = 3; /* signal to main() that the accession is not-found/invalid */
                                                break;
                    }
                }

                rc = release_tool_ctx( &tool_ctx, rc ); /* tool_ctx.c */
            }
        }
    }
    unread_rc_info( false ); /* inspectory.c */
    return rc;
}
