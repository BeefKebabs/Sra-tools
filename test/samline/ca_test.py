#!/bin/env python
import sys, sam

def dump( acc, with_sec = False ) :
    print( f"{acc}.SEQ" )
    sam.vdb_dump( acc, "-C SPOT_ID,READ -l0 -f tab" )
    print( f"{acc}.PRIM" )
    sam.vdb_dump( acc, "-T PRIM -C ALIGN_ID,READ -l0, -f tab" )
    if with_sec :
        print( f"{acc}.SEC" )
        sam.vdb_dump( acc, "-T SEC -C ALIGN_ID,READ -l0 -f tab" )

REF         = "NC_011752.1"
REF_ALIAS   = "c1"

CSRA1       = "AFTER_BAM_LOAD.CSRA"
CSRA2       = "AFTER_SRA_SORT.CSRA"

def load_sort( L ) :
    res_1 = sam.bam_load( L, CSRA1, "--make-spots-with-secondary -L 3 -E0 -Q0" )
    if res_1 == 1 :
        print( "bam-load = OK" )
        res_2 = sam.sra_sort( CSRA1, CSRA2 )
        if res_2 == 1 :
            print( "sra-sort = OK" )
            return True
        else :
            print( "sra-sort = FAILED" )
    else :
        print( "bam-load = FAILED" )
    return False

def load_sort_print( L, with_sec = False ) :
    if load_sort( L ) :
        dump( CSRA1, with_sec )
        dump( CSRA2, with_sec )
        return True
    return False

def test1() :
    print( "test #1 --------------------------------------------------" )
    print( "...having a single secondary alignmnet without a primary it belongs to" )
    A1 = sam.make_prim( "A1", 0, REF, REF_ALIAS, 17000, 20, "60M" )
    A2 = sam.make_prim( "A2", 0, REF, REF_ALIAS, 12500, 20, "50M", A1 )
    A3 = sam.make_prim( "A3", 0, REF, REF_ALIAS, 33000, 20, "60M" )
    U1 = sam.make_unaligned( "U1", 0, "ACTTTAGTAAGGGGTTNN" )
    A4 = sam.make_sec( "A4", 0, REF, REF_ALIAS, 19000, 20, "60M", A1 )
    A5 = sam.make_sec( "A5", 0, REF, REF_ALIAS, 22000, 20, "30M" )
    return load_sort_print( [ A1, A2, A3, A4, U1, A5 ], True )

# the resulting X.CSRA and S.CSRA produce errors in seq_restore_read_impl2
def test2() :
    print( "test #2 --------------------------------------------------" )
    print( "...having a pair of a prim. and a sec. alignment" )
    print( "= SEQUENCE-table cannot reconstruct READ" )
    A1 = sam.make_prim( "A1", 0, REF, REF_ALIAS, 17000, 20, "60M" )
    A2 = sam.make_sec( "A2", 0, REF, REF_ALIAS, 12500, 20, "50M", A1 )
    return load_sort_print( [ A1, A2 ] )

# the resulting X.CSRA produces errors in seq_restore_read_impl2
# but S.CSRA sefaults in vdb-dump!
def test3() :
    print( "test #3 --------------------------------------------------" )
    A1 = sam.make_prim( "A1", 0, REF, REF_ALIAS, 1000, 20, "53M" )
    A2 = sam.make_sec( "A2", 0, REF, REF_ALIAS, 3500, 20, "50M", A1 )
    A3 = sam.make_sec( "A3", 0, REF, REF_ALIAS, 6800, 20, "55M" )
    A3.flags |= sam.FLAG_NEXT_UNMAPPED
    return load_sort_print( [ A3, A1, A2 ] )


if test1() :
    if test2() :
        if test3() :
            sys.exit( 0 )
sys.exit( 3 )
