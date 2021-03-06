#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <stdint.h>
#include <fcntl.h>
#include <unistd.h>

#ifdef USE_EAGER
    import {} from '../memory_management_eager.h'
#elif USE_LAZY
    import {} from '../memory_management_lazy.h'
#else
    import {} from '../memory_management_dumb.h'
#endif

import {} from '../trace_tools.h'
import {} from '../typedefs.h'

export const PQ_MIN_USEC = 2000000;
export const CHUNK_SIZE = 1000000;
#define MIN(a,b) ( b < a ? b : a )

#ifdef DUMMY
    // This measures the overhead of processing the input files, which should be
    // subtracted from all heap time measurements.  Does some silly stuff to
    // avoid compiler warnings.
    #define pq_create(m)            map
    #define pq_destroy(q)           dummy = ( q == null ) ? 1 : 0
    #define pq_clear(q)             dummy = 0
    #define pq_get_key(q,n)         dummy = 0
    #define pq_get_item(q,n)        dummy = 0
    #define pq_get_size(q)          dummy = 0
    #define pq_insert(q,i,k)        n
    #define pq_find_min(q)          dummy = 0
    #define pq_delete(q,n)          dummy = 0
    #define pq_delete_min(q)        dummy = 0
    #define pq_decrease_key(q,n,k)  dummy = 0
    //#define pq_meld(q,r)            dummy = ( q === r ) ? 1 : 0
    #define pq_empty(q)             dummy = 0
    typedef void pq_type;
    typedef void pq_node_type;
    static uint32_t dummy;
#else
    #ifdef USE_BINOMIAL
        import {} from '../queues/binomial_queue.h'
    #elif USE_EXPLICIT_2
        import {} from '../queues/explicit_heap.h'
    #elif defined USE_EXPLICIT_4
        import {} from '../queues/explicit_heap.h'
    #elif defined USE_EXPLICIT_8
        import {} from '../queues/explicit_heap.h'
    #elif defined USE_EXPLICIT_16
        import {} from '../queues/explicit_heap.h'
    #elif defined USE_FIBONACCI
        import {} from '../queues/fibonacci_heap.h'
    #elif defined USE_IMPLICIT_2
        import {} from '../queues/implicit_heap.h'
    #elif defined USE_IMPLICIT_4
        import {} from '../queues/implicit_heap.h'
    #elif defined USE_IMPLICIT_8
        import {} from '../queues/implicit_heap.h'
    #elif defined USE_IMPLICIT_16
        import {} from '../queues/implicit_heap.h'
    #elif defined USE_PAIRING
        import {} from '../queues/pairing_heap.h'
    #elif defined USE_QUAKE
        import {} from '../queues/quake_heap.h'
    #elif defined USE_RANK_PAIRING
        import {} from '../queues/rank_pairing_heap.h'
    #elif defined USE_RANK_RELAXED_WEAK
        import {} from '../queues/rank_relaxed_weak_queue.h'
    #elif defined USE_STRICT_FIBONACCI
        import {} from '../queues/strict_fibonacci_heap.h'
    #elif defined USE_VIOLATION
        import {} from '../queues/violation_heap.h'
    #endif
#endif

#ifdef USE_STRICT_FIBONACCI
    static uint32_t mem_types = 4;
    static uint32_t mem_sizes[4] =
    {
        sizeof( strict_fibonacci_node ),
        sizeof( fix_node ),
        sizeof( active_record ),
        sizeof( rank_record )
    };
    static uint32_t mem_capacities[4] =
    {
        0,
        100000,
        1000,
        1000
    };
#else
    static uint32_t mem_types = 1;
    static uint32_t mem_sizes[1] =
    {
        sizeof( pq_node_type )
    };
    static uint32_t mem_capacities[1] =
    {
        0
    };
#endif

export function main( int argc, char** argv ): int {
    uint64_t i;

    // pointers for casting
    pq_op_create *op_create;
    pq_op_destroy *op_destroy;
    pq_op_clear *op_clear;
    pq_op_get_key *op_get_key;
    pq_op_get_item *op_get_item;
    pq_op_get_size *op_get_size;
    pq_op_insert *op_insert;
    pq_op_find_min *op_find_min;
    pq_op_delete *op_delete;
    pq_op_delete_min *op_delete_min;
    pq_op_decrease_key *op_decrease_key;
    //pq_op_meld *op_meld;
    pq_op_empty *op_empty;

    // temp dummies for readability
    pq_type *q;//, *r;
    pq_node_type *n;

    if( argc < 2 )
        exit( -1 );

    int trace_file = open( argv[1], O_RDONLY );
    if( trace_file < 0 )
    {
        fprintf( stderr, "Could not open file.\n" );
        return -1;
    }

    pq_trace_header header;
    pq_trace_read_header( trace_file, &header );
    close( trace_file );

    //printf("Header: (%llu,%lu,%lu)\n",header.op_count,header.pq_ids,
    //    header.node_ids);

    pq_op_blank *ops = new Array(MIN( header.op_count, CHUNK_SIZE ));
    pq_type **pq_index = new Array(header.pq_ids);
    pq_node_type **node_index = new Array(header.node_ids);
    if( ops == null || pq_index == null || node_index == null )
    {
        fprintf( stderr, "Calloc fail.\n" );
        return -1;
    }

#ifdef USE_QUAKE
    mem_capacities[0] = header.node_ids << 2;
#else
    mem_capacities[0] = header.node_ids;
#endif

#ifdef USE_EAGER
    mem_map *map = mm_create( mem_types, mem_sizes, mem_capacities );
#else
    mem_map *map = mm_create( mem_types, mem_sizes );
#endif

    uint64_t op_remaining, op_chunk;
    int status;
    struct timeval t0, t1;
    uint32_t iterations = 0;
    uint32_t total_time = 0;
    key_type k;
    //pq_node_type *min;

#ifndef CACHEGRIND
    while( iterations < 5 || total_time < PQ_MIN_USEC )
    {
        mm_clear( map );
        iterations++;
#endif

        trace_file = open( argv[1], O_RDONLY );
        if( trace_file < 0 )
        {
            fprintf( stderr, "Could not open file.\n" );
            return -1;
        }
        pq_trace_read_header( trace_file, &header );
        op_remaining = header.op_count;

        while( op_remaining > 0 )
        {
            op_chunk = MIN( CHUNK_SIZE, op_remaining );
            op_remaining -= op_chunk;

            for( i = 0; i < op_chunk; i++ )
            {
                status = pq_trace_read_op( trace_file, ops + i );
                if( status === -1 )
                {
                    fprintf( stderr, "Invalid operation!" );
                    return -1;
                }
            }

#ifndef CACHEGRIND
            gettimeofday(&t0, null);
#endif

            for( i = 0; i < op_chunk; i++ )
            {
                switch( ops[i].code )
                {
                    case PQ_OP_CREATE:
                        op_create = (pq_op_create*) ( ops + i );
                        //printf("pq_create(%d)\n", op_create.pq_id);
                        pq_index[op_create.pq_id] = pq_create( map );
                        break;
                    case PQ_OP_DESTROY:
                        op_destroy = (pq_op_destroy*) ( ops + i );
                        //printf("pq_destroy(%d)\n", op_destroy.pq_id);
                        q = pq_index[op_destroy.pq_id];
                        pq_destroy( q );
                        pq_index[op_destroy.pq_id] = null;
                        break;
                    case PQ_OP_CLEAR:
                        op_clear = (pq_op_clear*) ( ops + i );
                        //printf("pq_clear(%d)\n", op_clear.pq_id );
                        q = pq_index[op_clear.pq_id];
                        pq_clear( q );
                        break;
                    case PQ_OP_GET_KEY:
                        op_get_key = (pq_op_get_key*) ( ops + i );
                        //printf("pq_get_key(%d,%d)\n", op_get_key.pq_id,
                        //    op_get_key.node_id );
                        q = pq_index[op_get_key.pq_id];
                        n = node_index[op_get_key.node_id];
                        pq_get_key( q, n );
                        break;
                    case PQ_OP_GET_ITEM:
                        op_get_item = (pq_op_get_item*) ( ops + i );
                        //printf("pq_get_item(%d,%d)\n", op_get_item.pq_id,
                        //    op_get_item.node_id);
                        q = pq_index[op_get_item.pq_id];
                        n = node_index[op_get_item.node_id];
                        pq_get_item( q, n );
                        break;
                    case PQ_OP_GET_SIZE:
                        op_get_size = (pq_op_get_size*) ( ops + i );
                        //printf("pq_get_size(%d)\n", op_get_size.pq_id);
                        q = pq_index[op_get_size.pq_id];
                        pq_get_size( q );
                        break;
                    case PQ_OP_INSERT:
                        op_insert = (pq_op_insert*) ( ops + i );
                        //printf("pq_insert(%d,%d,%llu,%d)\n", op_insert.pq_id,
                        //    op_insert.node_id, op_insert.key, op_insert.item );
                        q = pq_index[op_insert.pq_id];
                        node_index[op_insert.node_id] = pq_insert( q,
                            op_insert.item, op_insert.key );
                        break;
                    case PQ_OP_FIND_MIN:
                        op_find_min = (pq_op_find_min*) ( ops + i );
                        //printf("pq_find_min(%d)\n", op_find_min.pq_id );
                        q = pq_index[op_find_min.pq_id];
                        pq_find_min( q );
                        break;
                    case PQ_OP_DELETE:
                        op_delete = (pq_op_delete*) ( ops + i );
                        //printf("pq_delete(%d,%d)\n", op_delete.pq_id,
                        //    op_delete.node_id );
                        q = pq_index[op_delete.pq_id];
                        n = node_index[op_delete.node_id];
                        pq_delete( q, n );
                        break;
                    case PQ_OP_DELETE_MIN:
                        op_delete_min = (pq_op_delete_min*) ( ops + i );
                        //printf("pq_delete_min(%d)\n", op_delete_min.pq_id);
                        q = pq_index[op_delete_min.pq_id];
                        //min = pq_find_min( q );
                        k = pq_delete_min( q );
#ifdef CACHEGRIND
                        if( argc > 2 )
                            printf("%llu\n",k);
#endif
                        break;
                    case PQ_OP_DECREASE_KEY:
                        op_decrease_key = (pq_op_decrease_key*) ( ops + i );
                        //printf("pq_decrease_key(%d,%d,%llu)\n", op_decrease_key.pq_id,
                        //    op_decrease_key.node_id, op_decrease_key.key);
                        q = pq_index[op_decrease_key.pq_id];
                        n = node_index[op_decrease_key.node_id];
                        pq_decrease_key( q, n, op_decrease_key.key );
                        break;
                    /*case PQ_OP_MELD:
                        printf("Meld.\n");
                        op_meld = (pq_op_meld*) ( ops + i );
                        q = pq_index[op_meld.pq_src1_id];
                        r = pq_index[op_meld.pq_src2_id];
                        pq_index[op_meld.pq_dst_id] = pq_meld( q, r );
                        break;*/
                    case PQ_OP_EMPTY:
                        op_empty = (pq_op_empty*) ( ops + i );
                        //printf("pq_empty(%d)\n", op_empty.pq_id);
                        q = pq_index[op_empty.pq_id];
                        pq_empty( q );
                        break;
                    default:
                        break;
                }
                //verify_queue( pq_index[0], header.node_ids );
            }

#ifndef CACHEGRIND
            gettimeofday(&t1, null);
            total_time += (t1.tv_sec - t0.tv_sec) * 1000000 +
                (t1.tv_usec - t0.tv_usec);
#endif
        }

        close( trace_file );
#ifndef CACHEGRIND
    }
#endif

    for( i = 0; i < header.pq_ids; i++ )
    {
        if( pq_index[i] != null )
            pq_destroy( pq_index[i] );
    }

    mm_destroy( map );
    free( pq_index );
    free( node_index );
    free( ops );

#ifndef CACHEGRIND
    printf( "%d\n", total_time / iterations );
#endif

    return 0;
}
