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

#include "sam-hdr1.h"

#ifndef _h_klib_printf_
#include <klib/printf.h>
#endif

#ifndef _h_kfs_filetools_
#include <kfs/filetools.h>
#endif

#ifndef _h_kdb_meta_
#include <kdb/meta.h>
#endif

#ifndef _h_kdb_namelist_
#include <kdb/namelist.h>
#endif

#ifndef _h_klib_out_
#include <klib/out.h>
#endif

#ifndef _h_klib_log_
#include <klib/log.h>
#endif

typedef struct headers {
    VNamelist * SQ_Lines_1;
    VNamelist * SQ_Lines_2;
    VNamelist * SQ_Lines_3;
    
    VNamelist * RG_Lines_1;
    VNamelist * RG_Lines_2;
    VNamelist * RG_Lines_3;
    
    VNamelist * Other_Lines;
    VNamelist * HD_Lines;
} headers;

static void release_lines( VNamelist ** lines ) {
    if ( *lines != NULL ) {
        VNamelistRelease ( *lines );
        lines = NULL;
    }
}

static void release_headers( headers * h ) {
    release_lines( &h->HD_Lines );
    release_lines( &h->Other_Lines );
    
    release_lines( &h->RG_Lines_1 );
    release_lines( &h->RG_Lines_2 );    
    release_lines( &h->RG_Lines_3 );
    
    release_lines( &h->SQ_Lines_1 );
    release_lines( &h->SQ_Lines_2 );
    release_lines( &h->SQ_Lines_3 );
}

static rc_t init_headers( headers * h, uint32_t blocksize ) {
    rc_t rc;
    
    h->SQ_Lines_1 = NULL;
    h->SQ_Lines_2 = NULL;
    h->SQ_Lines_3 = NULL;
    h->RG_Lines_1 = NULL;
    h->RG_Lines_2 = NULL;
    h->RG_Lines_3 = NULL;
    h->Other_Lines = NULL;
    h->HD_Lines = NULL;

    rc = VNamelistMake( &h->SQ_Lines_1, blocksize );
    if ( rc == 0 ) { rc = VNamelistMake( &h->SQ_Lines_2, blocksize ); }
    if ( rc == 0 ) { rc = VNamelistMake( &h->SQ_Lines_3, blocksize ); }
    if ( rc == 0 ) { rc = VNamelistMake( &h->RG_Lines_1, blocksize ); }
    if ( rc == 0 ) { rc = VNamelistMake( &h->RG_Lines_2, blocksize ); }
    if ( rc == 0 ) { rc = VNamelistMake( &h->RG_Lines_3, blocksize ); }
    if ( rc == 0 ) { rc = VNamelistMake( &h->Other_Lines, blocksize ); }
    if ( rc == 0 ) { rc = VNamelistMake( &h->HD_Lines, blocksize ); }
    if ( rc != 0 ) { release_headers( h ); }
    return rc;
}


static void process_line( headers * h, int hdr_idx, const char * line, size_t len ) {
    if ( len > 3 && line[ 0 ] == '@' ) {
        if ( line[ 1 ] == 'S' && line[ 2 ] == 'Q' ) {
            switch( hdr_idx ) {
                case 1 : VNamelistAppend( h->SQ_Lines_1, line ); break;
                case 2 : VNamelistAppend( h->SQ_Lines_2, line ); break;
                case 3 : VNamelistAppend( h->SQ_Lines_3, line ); break;
            }
        } else if ( line[ 1 ] == 'R' && line[ 2 ] == 'G' ) {
            switch( hdr_idx ) {
                case 1 : VNamelistAppend( h->RG_Lines_1, line ); break;
                case 2 : VNamelistAppend( h->RG_Lines_2, line ); break;
                case 3 : VNamelistAppend( h->RG_Lines_3, line ); break;
            }
        } else if ( line[ 1 ] == 'H' && line[ 2 ] == 'D' ) {
            VNamelistAppend( h->HD_Lines, line );
        } else {
            VNamelistAppend( h->Other_Lines, line );
        }
    }
}

static rc_t process_lines( headers * h, int hdr_idx, VNamelist * content, const char * identifier ) {
    uint32_t i, count;
    rc_t rc = VNameListCount( content, &count );
    if ( rc != 0 ) {
        (void)PLOGERR( klogErr, ( klogErr, rc, "cant get count for content of '$(t)'", "t=%s", identifier ) );
    } else {
        const char * line = NULL;
        for ( i = 0; i < count && rc == 0; ++i ) {
            rc = VNameListGet( content, i, &line );
            if ( rc != 0 ) {
                (void)PLOGERR( klogErr, ( klogErr, rc, "cant get line #$(t) from content", "t=%u", i ) );
            } else {
                process_line( h, hdr_idx, line, string_measure( line, NULL ) );
            }
        }
    }
    return rc;
}

#define STATE_ALPHA 0
#define STATE_LF 1
#define STATE_NL 2

typedef struct buffer_range {
    const char * buffer;
    size_t size, state;
} buffer_range;

static const char empty_str[ 2 ] = { ' ', 0 };

static void LoadFromBuffer( VNamelist * nl, buffer_range * range ) {
    size_t idx;
    const char * p = range->buffer;
    String S;

    S.addr = p;
    S.len = S.size = 0;
    for ( idx = 0; idx < range->size; ++idx ) {
        switch( p[ idx ] ) {
            case 0x0A : switch( range->state ) {
                            case STATE_ALPHA : /* ALPHA --> LF */
                                                VNamelistAppendString ( nl, &S );
                                                range->state = STATE_LF;
                                                break;

                            case STATE_LF : /* LF --> LF */
                                             VNamelistAppend ( nl, empty_str );
                                             break;

                            case STATE_NL : /* NL --> LF */
                                             VNamelistAppend ( nl, empty_str );
                                             range->state = STATE_LF;
                                             break;
                        }
                        break;

            case 0x0D : switch( range->state ) {
                            case STATE_ALPHA : /* ALPHA --> NL */
                                                VNamelistAppendString ( nl, &S );
                                                range->state = STATE_NL;
                                                break;

                            case STATE_LF : /* LF --> NL */
                                             range->state = STATE_NL;
                                             break;

                            case STATE_NL : /* NL --> NL */
                                             VNamelistAppend ( nl, empty_str );
                                             break;
                        }
                        break;

            default   : switch( range->state ) {
                            case STATE_ALPHA : /* ALPHA --> ALPHA */
                                                S.len++; S.size++;
                                                break;

                            case STATE_LF : /* LF --> ALPHA */
                                             S.addr = &p[ idx ]; S.len = S.size = 1;
                                             range->state = STATE_ALPHA;
                                             break;

                            case STATE_NL : /* NL --> ALPHA */
                                             S.addr = &p[ idx ]; S.len = S.size = 1;
                                             range->state = STATE_ALPHA;
                                             break;
                        }
                        break;
        }
    }
    if ( range->state == STATE_ALPHA && S.len > 0 ) {
        VNamelistAppendString ( nl, &S );
    }
}

static rc_t Load_Namelist_From_Node( VNamelist * dst, const KMDataNode * node ) {
    size_t num_read, remaining;
    char b[ 10 ];
    rc_t rc = KMDataNodeRead( node, 0, b, sizeof( b ), &num_read, &remaining );
    if ( rc == 0 ) {
        size_t bsize = num_read + remaining;
        buffer_range range;
        range.buffer = malloc( bsize );
        if ( range.buffer != NULL ) {
            rc = KMDataNodeRead( node, 0, ( void * )range.buffer, bsize, &range.size, &remaining );
            if ( rc == 0 ) {
                range.state = STATE_ALPHA;
                LoadFromBuffer( dst, &range );
            }
            free( ( void * ) range.buffer );
        }
    }
    return rc;
}

static rc_t collect_from_BAM_HEADER( headers * h, int hdr_idx, const input_files * ifs ) {
    rc_t rc = 0;
    if ( ifs->database_count > 0 ) {
        uint32_t idx;
        for ( idx = 0; idx < ifs->database_count && rc == 0; ++idx ) {
            input_database * id = VectorGet( &ifs->dbs, idx );
            if ( id != NULL ) {
                const KMetadata * meta;
                rc = VDatabaseOpenMetadataRead( id->db, &meta );
                if ( rc == 0 ) {
                    const KMDataNode * node;
                    rc = KMetadataOpenNodeRead( meta, &node, "BAM_HEADER" );
                    if ( rc == 0 ) {
                        VNamelist * content;
                        rc = VNamelistMake ( &content, 25 );
                        if ( rc != 0 ) {
                            (void)PLOGERR( klogErr, ( klogErr, rc, "cant create container for '$(t)'", "t=%s", id->path ) );
                        } else {
                            rc = Load_Namelist_From_Node( content, node );
                            if ( rc == 0 ) {
                                rc = process_lines( h, hdr_idx, content, id->path );
                            }
                            VNamelistRelease( content );
                        }
                        KMDataNodeRelease( node );
                    } else {
                        rc = 0;
                    }
                    KMetadataRelease( meta );
                }
            }
        }
    }
    return rc;
}

static rc_t collect_from_spotgroup_stats( headers * h, int hdr_idx, const KMDataNode * node, const KNamelist * spot_groups ) {
    uint32_t count;
    rc_t rc = KNamelistCount( spot_groups, &count );
    if ( rc == 0 && count > 0 ) {
        uint32_t i;
        for ( i = 0; i < count && rc == 0; ++i ) {
            const char * name = NULL; /* this is the name of the node !!!NOT!!! the name of the spotgroup! */
            rc = KNamelistGet( spot_groups, i, &name );
            if ( rc == 0 && name != NULL ) {
                const KMDataNode * spot_count_node;
                rc = KMDataNodeOpenNodeRead( node, &spot_count_node, "%s/SPOT_COUNT", name );
                if ( rc == 0 ) {
                    uint64_t spot_count = 0;
                    rc = KMDataNodeReadAsU64( spot_count_node, &spot_count );
                    if ( rc == 0 ) {
                        if ( spot_count > 0 ) {
                            const KMDataNode * spot_group_node;
                            rc = KMDataNodeOpenNodeRead( node, &spot_group_node, name );            
                            if ( rc == 0 ) {
                                char name_attr[ 2048 ];
                                char buffer[ 2048 ];
                                size_t num_writ;
                                rc = KMDataNodeReadAttr( spot_group_node, "name", name_attr, sizeof name_attr, &num_writ );
                                rc = string_printf( buffer, sizeof buffer, &num_writ,
                                            "@RG\tID:%s", rc == 0 ? name_attr : name );
                                if ( rc == 0 ) {
                                    process_line( h, hdr_idx, buffer, num_writ );
                                }
                                KMDataNodeRelease( spot_group_node );                
                            }
                        }
                    } else {
                        rc = 0;
                    }
                    KMDataNodeRelease( spot_count_node );
                }
            }
        }
    }
    return rc;
}

static rc_t collect_from_stats( headers * h, int hdr_idx, const input_files * ifs ) {
    rc_t rc = 0;
    if ( ifs->database_count > 0 ) {
        uint32_t idx;
        for ( idx = 0; idx < ifs->database_count && rc == 0; ++idx ) {
            input_database * id = VectorGet( &ifs->dbs, idx );
            if ( id != NULL ) {
                const VTable * seq_table = NULL;
                rc = VDatabaseOpenTableRead( id->db, &seq_table, "SEQUENCE" );
                if ( rc == 0 ) {
                    const KMetadata * meta;
                    rc = VTableOpenMetadataRead( seq_table, &meta );
                    if ( rc == 0 ) {
                        const KMDataNode * node;
                        rc = KMetadataOpenNodeRead( meta, &node, "STATS/SPOT_GROUP" );
                        if ( rc == 0 ) {
                            KNamelist * spot_groups;
                            rc = KMDataNodeListChildren( node, &spot_groups );
                            if ( rc == 0 ) {
                                rc = collect_from_spotgroup_stats( h, hdr_idx, node, spot_groups );
                                KNamelistRelease( spot_groups );
                            }
                            KMDataNodeRelease( node );
                        }
                        KMetadataRelease( meta );
                    }
                    VTableRelease( seq_table );
                }
            }
        }
    }
    return rc;
}

static rc_t collect_from_file( headers * h, int hdr_idx, const char * filename ) {
    KDirectory * dir;
    rc_t rc = KDirectoryNativeDir ( &dir );
    if ( rc != 0 ) {
        (void)PLOGERR( klogErr, ( klogErr, rc, "cant created native directory for file '$(t)'", "t=%s", filename ) );
    } else {
        const struct KFile * f;
        rc = KDirectoryOpenFileRead ( dir, &f, "%s", filename );
        if ( rc != 0 ) {
            (void)PLOGERR( klogErr, ( klogErr, rc, "cant open file '$(t)'", "t=%s", filename ) );
        } else {
            VNamelist * content;
            rc = VNamelistMake ( &content, 25 );
            if ( rc != 0 ) {
                (void)PLOGERR( klogErr, ( klogErr, rc, "cant create container for file '$(t)'", "t=%s", filename ) );
            } else {
                rc = LoadKFileToNameList( f, content );
                if ( rc != 0 ) {
                    (void)PLOGERR( klogErr, ( klogErr, rc, "cant load file '$(t)' into container", "t=%s", filename ) );
                } else {
                    rc = process_lines( h, hdr_idx, content, filename );
                }
                VNamelistRelease( content );
            }
            KFileRelease ( f );
        }
        KDirectoryRelease ( dir );
    }
    return rc;
}

static rc_t collect_from_references( headers * h, int hdr_idx, const input_files * ifs , bool use_seqid) {
    rc_t rc = 0;
    uint32_t i;
    for ( i = 0; i < ifs->database_count && rc == 0; ++i ) {
        input_database * id = VectorGet( &ifs->dbs, i );
        if ( id != NULL ) {
            uint32_t rcount;
            rc = ReferenceList_Count( id->reflist, &rcount );
            if ( rc == 0 && rcount > 0 ) {
                uint32_t r_idx;
                for ( r_idx = 0; r_idx < rcount && rc == 0; ++r_idx ) {
                    const ReferenceObj * ref_obj;
                    rc = ReferenceList_Get( id->reflist, &ref_obj, r_idx );
                    if ( rc == 0 && ref_obj != NULL ) {
                        const char * seqid;
                        rc = ReferenceObj_SeqId( ref_obj, &seqid );
                        if ( rc == 0 ) {
                            const char * name;
                            rc = ReferenceObj_Name( ref_obj, &name );
                            if ( rc == 0 ) {
                                INSDC_coord_len seq_len;
                                rc = ReferenceObj_SeqLength( ref_obj, &seq_len );
                                if ( rc == 0 ) {
                                    char buffer[ 2048 ];
                                    size_t num_writ;
                                    rc = string_printf( buffer, sizeof buffer, &num_writ,
                                                        "@SQ\tSN:%s\tLN:%lu", use_seqid?seqid:name, seq_len );
                                    if ( rc == 0 ) {
                                        process_line( h, hdr_idx, buffer, num_writ );
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    return rc;
}

typedef struct hdr_tag {
    String key, value;
} hdr_tag;

typedef struct hdr_line {
    String line_key;
    hdr_tag tags[ 32 ];
    uint32_t n_tags;
} hdr_line;

static bool parse_hdr_line( hdr_line * hl, const char * line ) {
    size_t i, tl = 0, start = 3, len = string_size( line );
    bool res = ( len > start && line[ 0 ] == '@' );
    hl->n_tags = 0;
    if ( res ) {
        uint32_t colons = 0;
        StringInit( &hl->line_key, &line[ 1 ], 2, 2 );
        for( i = start; i < len; ++i ) {
            switch ( line[ i ] ) {
                case '\t' : if ( tl > 0 ) {
                                StringInit( &hl->tags[ hl->n_tags ].value, &line[ start ], tl, tl );
                                ( hl->n_tags )++;
                                tl = 0;
                            }
                            start = i + 1;
                            colons = 0;
                            break;

                case ':'  : if ( colons == 0 ) {
                                if ( tl > 0 ) {
                                    StringInit( &hl->tags[ hl->n_tags ].key, &line[ start ], tl, tl );
                                    tl = 0;
                                }
                                start = i + 1;
                            } else {
                                tl++;
                            }
                            colons++;
                            break;
                
                default :   tl++;
                            break;
            }
        }
        if ( tl > 0 ) {
            StringInit( &hl->tags[ hl->n_tags ].value, &line[ start ], tl, tl );
            ( hl->n_tags )++;
        }
    } else {
        StringInit( &hl->line_key, NULL, 0, 0 );
    }
    return res;
}

static rc_t print_hdr_line( char * buffer, size_t buflen, const hdr_line * hl ) {
    rc_t rc = 0;
    buffer[ 0 ] = 0;
    if ( hl->line_key.len > 0 ) {
        size_t num_writ, total_writ = 0, i;
        rc = string_printf( buffer, buflen, &num_writ, "@%S", &hl->line_key );
        for ( i = 0; i < hl->n_tags && rc == 0; ++i ) {
            total_writ += num_writ;
            if ( buflen > total_writ ) {
                rc = string_printf( &buffer[ total_writ ], buflen - total_writ, &num_writ,
                                "\t%S:%S", &hl->tags[ i ].key, &hl->tags[ i ].value );
            }
        }
    }
    return rc;
}

static bool sam_hdr_id( const hdr_line * hl1, const hdr_line * hl2 ) {
    bool res = ( hl1 != NULL && hl2 != NULL );
    if ( res ) {
        res = ( ( hl1->n_tags > 0 ) && ( hl2->n_tags > 0 ) );
    }
    if ( res ) {
        res = ( 0 == StringCompare( &hl1->tags[ 0 ].value, &hl2->tags[ 0 ].value ) );
    }
    return res;
}

static bool has_tag( const String * tag_id, hdr_line * dst ) {
    bool res = false;
    uint32_t i;
    for ( i = 0; i < dst->n_tags && !res; ++i ) {
        res = ( 0 == StringCompare( tag_id, &dst->tags[ i ].key ) );
    }
    return res;
}

static void merge_tag( const hdr_tag * tag, hdr_line * dst ) {
    if ( !has_tag( &tag->key, dst ) ) {
        dst->tags[ dst->n_tags ] = *tag;
        ( dst->n_tags )++;
    }
}

static void merge_hdr_line( const hdr_line * src, hdr_line * dst ) {
    if ( src != NULL && dst != NULL ) {
        uint32_t i;
        for ( i = 0; i < src->n_tags; ++i ) {
            merge_tag( &src->tags[ i ], dst );
        }
    }
}

static rc_t for_each_line( const VNamelist * src,
                           rc_t ( * f ) ( const char * line, void * context ),
                           void * context ) {
    uint32_t idx, count;
    rc_t rc = VNameListCount( src, &count );
    for ( idx = 0; idx < count && rc == 0; ++idx ) {
        const char * line = NULL;
        rc = VNameListGet( src, idx, &line );
        if ( rc == 0 ) {
            rc = f( line, context );
        }
    }
    return rc;
}

static rc_t merge_header_tags( const char * line, void * context ) {
    hdr_line * h_parent = context;
    hdr_line h;
    if ( parse_hdr_line( &h, line ) ) {
        /* merge the tags! */
        if ( sam_hdr_id( h_parent, &h ) ) {
            merge_hdr_line( h_parent, &h );
        }
    }
    return 0;
}

typedef struct merge_ctx {
    VNamelist * dst;
    const VNamelist * other;
    bool unique;
} merge_ctx;

static rc_t merge_header_tags_callback( const char * line, void * context ) {
    rc_t rc = 0;
    merge_ctx * mc = context;
    hdr_line h;
    if ( parse_hdr_line( &h, line ) ) {
        rc = for_each_line( mc->other, merge_header_tags, &h );
        if ( rc == 0 ) {
            char buffer[ 2048 ];
            rc = print_hdr_line( buffer, sizeof buffer, &h );
            if ( rc == 0 && buffer[ 0 ] != 0 ) {
                bool do_append = true;
                if ( mc->unique ) {
                    uint32_t found;
                    rc_t rc1 = VNamelistIndexOf( mc->dst, buffer, &found );
                    do_append = ( rc1 != 0 );
                }
                if ( do_append ) {
                    rc = VNamelistAppend( mc->dst, buffer );
                }
            }
        }
    }
    return rc;
}

/* SQ-lines have to be uniue by the SN-tag */
/* RG-lines have to be uniue by the ID-tag */
static rc_t merge_header_tags_of_2_lists( VNamelist ** lines_1, const VNamelist * lines_2, bool unique ) {
    rc_t rc;
    merge_ctx mc;
    memset( &mc, 0, sizeof mc );
    mc.unique = unique;
    rc = VNamelistMake( &mc.dst, 25 );
    if ( rc == 0 ) {
        mc.other = lines_2;
        rc = for_each_line( *lines_1, merge_header_tags_callback, &mc );
        if ( rc == 0 ) {
            VNamelistRelease( *lines_1 );
            *lines_1 = mc.dst;
        }
    }
    return rc;
}

static rc_t collect_from_bam_hdr( headers * dst, const input_files * ifs, bool use_seqid ) {
    uint32_t count;
    rc_t rc = collect_from_BAM_HEADER( dst, 1, ifs );
    if ( rc == 0 ) {
        rc = VNameListCount( dst->SQ_Lines_1, &count );
        if ( rc == 0 && ( count == 0 || use_seqid ) ) {
            rc = collect_from_references( dst, 2, ifs , use_seqid );
        }
    }
    if ( rc == 0 ) {
        rc = VNameListCount( dst->RG_Lines_1, &count );
        if ( rc == 0 && count == 0 ) {
            rc = collect_from_stats( dst, 1, ifs );    
        }
    }
    return rc;
}

static rc_t collect_by_recalc( headers * h, const input_files * ifs, bool use_seqid ) {
    rc_t rc = collect_from_references( h, 1, ifs, use_seqid );
    if ( rc == 0 ) {
        rc = collect_from_stats( h, 1, ifs );
    }
    return rc;
}

static rc_t collect_from_src_and_files( headers * h, const input_files * ifs, const char * filename, bool use_seqid ) {
    rc_t rc = collect_from_bam_hdr( h, ifs, use_seqid );
    if ( rc == 0 && filename != NULL ) {
        rc = collect_from_file( h, 3, filename );
    }
    return rc;
}

static rc_t print_HD_line( const VNamelist * lines ) {
    uint32_t count;
    rc_t rc = VNameListCount( lines, &count );
    if ( rc == 0 && count > 0 ) {
        const char * line = NULL;
        rc = VNameListGet( lines, 0, &line );
        if ( rc == 0 && line != NULL ) {
            rc = KOutMsg( "%s\n", line );
        } else {
            rc = KOutMsg( "@HD\tVN:1.2\tSO:coordinate\n" );
        }
    } else {
        rc = KOutMsg( "@HD\tVN:1.2\tSO:coordinate\n" );
    }
    return rc;
}

static rc_t print_callback( const char * line, void * context ) {
    return KOutMsg( "%s\n", line );
}

static rc_t merge_and_print( VNamelist ** L1, const VNamelist * L2, bool print_L2_if_only_src ) {
    uint32_t count1, count2;
    
    rc_t rc = VNameListCount( *L1, &count1 );
    if ( rc == 0 ) {
        rc = VNameListCount( L2, &count2 );
    }
    if ( rc == 0 ) {
        if ( count1 > 0 && count2 > 0 ) {
            if ( rc == 0 ) {
                rc = merge_header_tags_of_2_lists( L1, L2, true );
            }
            if ( rc == 0 ) {
                rc = for_each_line( *L1, print_callback, NULL );
            }
        } else if ( count1 > 0 ) {
            rc = for_each_line( *L1, print_callback, NULL );
        } else if ( print_L2_if_only_src && ( count2 > 0 ) ) {
            rc = for_each_line( L2, print_callback, NULL );
        }
    }
    return rc;
}

rc_t print_headers_1( const samdump_opts * opts, const input_files * ifs ) {
    headers h;
    rc_t rc = init_headers( &h, 25 );
    if ( rc == 0 ) {
        /* collect ... */
        switch( opts->header_mode ) {
            /* collect the headers that were written by the loader
               with special case if the user requested the seq-id to be used
               the bam-header-lines will be in list#1
               the recalculated-lines ( if requested via --seqid ) will be in list #2
            */
            case hm_dump    :  rc = collect_from_bam_hdr( &h, ifs, opts->use_seqid_as_refname );
                               if ( rc == 0 ) { rc = print_HD_line( h.HD_Lines ); }
                               if ( rc == 0 ) { rc = for_each_line( h.SQ_Lines_1, print_callback, NULL ); }
                               if ( rc == 0 ) { rc = for_each_line( h.SQ_Lines_2, print_callback, NULL ); }
                               if ( rc == 0 ) { rc = for_each_line( h.RG_Lines_1, print_callback, NULL ); }
                               if ( rc == 0 ) { rc = for_each_line( h.RG_Lines_2, print_callback, NULL ); }
                               break;

            /* collect the headers by iterating over the REFERENCE-table
               the recalculated-lines will be in list #1
            */
            case hm_recalc  : rc = collect_by_recalc( &h, ifs, opts->use_seqid_as_refname );
                              if ( rc == 0 ) { rc = print_HD_line( h.HD_Lines ); }
                              if ( rc == 0 ) { rc = for_each_line( h.SQ_Lines_1, print_callback, NULL ); }
                              if ( rc == 0 ) { rc = for_each_line( h.RG_Lines_1, print_callback, NULL ); }
                              break;

            /* collect the headers that were written by the loader ( list #1 )
               with special case if the user requested the seqid to be used ( list #2 )
               and merge header-fields with the ones from a user-supplied file ( list #3 )
            */
            case hm_file    : rc = collect_from_src_and_files( &h, ifs, opts->header_file, opts->use_seqid_as_refname );
                              if ( rc == 0 ) { rc = print_HD_line( h.HD_Lines ); }
                              if ( rc == 0 ) { rc = merge_and_print( &h.SQ_Lines_1, h.SQ_Lines_3, true ); }
                              if ( rc == 0 ) { rc = merge_and_print( &h.RG_Lines_1, h.RG_Lines_3, true ); }
                              if ( rc == 0 ) { rc = merge_and_print( &h.SQ_Lines_2, h.SQ_Lines_3, false ); }
                              if ( rc == 0 ) { rc = merge_and_print( &h.RG_Lines_2, h.RG_Lines_3, false ); }
                              break;

            case hm_none    : break; /* to not let the compiler complain about not handled enum */
        }

        /* all other lines collected: ( not HD,SQ,RG ) */
        if ( rc == 0 ) {
            rc = for_each_line( h.Other_Lines, print_callback, NULL );
        }
        release_headers( &h );
    }
    return rc;
}
