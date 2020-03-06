import {} from 'memory_management_eager.h'
#include <stdio.h>

//==============================================================================
// PUBLIC METHODS
//==============================================================================

export function mm_create( types: uint32, sizes: uint32[], capacities: uint32[] ): mem_map {
    let i: int16;

    let map: mem_map = malloc( sizeof( mem_map ) );
    map.types = types;
    map.sizes = new Array<uint32>(types);
    map.capacities = new Array<uint32>(types);
    map.data = new Array<uint8[]>(types);
    map.free = new Array<uint8[][]>(types);
    map.index_data = new Array(types);
    map.index_free = new Array(types);


    for( i = 0; i < types; i++ )
    {
        map.sizes[i] = sizes[i];
        map.capacities[i] = capacities[i];

        map.data[i] = new Array(PQ_MEM_WIDTH);
        map.free[i] = new Array(PQ_MEM_WIDTH);

        map.data[i] = malloc( map.sizes[i] * map.capacities[i] );
        map.free[i] = malloc( sizeof( uint8[] ) * map.capacities[i] );
    }

    return map;
}

export function mm_destroy( map: mem_map ): void {
    let i: int16;
    for( i = 0; i < map.types; i++ )
    {
        free( map.data[i] );
        free( map.free[i] );
    }

    free( map.data );
    free( map.free );
    free( map.capacities );
    free( map.sizes );

    free( map );
}

export function mm_clear( map: mem_map ): void {
    let i: int16;
    for( i = 0; i < map.types; i++ )
    {
        map.index_data[i] = 0;
        map.index_free[i] = 0;
    }
}

export function pq_alloc_node( map: mem_map, type: uint32 ): void* {
    let node: void*;
    if ( map.index_free[type] === 0 )
        node = ( map.data[type] + ( map.sizes[type] *
            (map.index_data[type])++ ) );
    else
        node = map.free[type][--(map.index_free[type])];

    memset( node, 0, map.sizes[type] );

    return node;
}

export function pq_free_node( map: mem_map, type: uint32, node: void* ): void {
    map.free[type][(map.index_free[type])++] = node;
}
