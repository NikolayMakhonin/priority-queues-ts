import {mem_map} from './memory_management_dumb'
import {uint32} from './typedefs'

export const PQ_MEM_WIDTH = 32;

/**
 * Dummy API for node allocation.  Just makes simple calls to associated system
 * functions.
 */

export interface mem_map {
    //! number of different node types
    types: uint32;
    //! sizes of single nodes
    sizes: uint32[];
}

/**
 * Creates a new memory map for the specified node sizes
 *
 * @param types         The number of different types of nodes to manage
 * @param sizes         Sizes of a single node of each type
 * @param capacities    The number of nodes of each type to allocate
 * @return              Pointer to the new memory map
 */
export function mm_create(types: uint32, sizes: uint32[] ): mem_map {
    return {
        types: types,
        sizes: new Array<uint32>(types),
    };
}

/**
 * Releases all allocated memory associated with the map.
 *
 * @param map   Map to deallocate
 */
export function mm_destroy( map: mem_map ): void {
    // TODO remove this method
}

/**
 * Resets map to initial state.  Does not deallocate memory.
 *
 * @param map   Map to reset
 */
export function mm_clear( map: mem_map ): void {
    return;
}

/**
 * Allocates a single node from the memory pool.  First attempts to recycle old
 * data from the free list.  If there is nothing to recycle, then it takes a new
 * node off the allocated list.  Zeroes the memory of the allocated node.
 *
 * @param map   Map from which to allocate
 * @param type  Type of node to allocate
 * @return      Pointer to allocated node
 */
export function pq_alloc_node( map: mem_map, type: uint32 ): any {
    // TODO

    let node: any = new Array(1);

    return node;
}

/**
 * Takes a previously allocated node and adds it to the free list to be
 * recycled with further allocation requests.
 *
 * @param map   Map to which the node belongs
 * @param type  Type of node to free
 * @param node  Node to free
 */
export function pq_free_node( map: mem_map, type: uint32, node: any ): void {

}
