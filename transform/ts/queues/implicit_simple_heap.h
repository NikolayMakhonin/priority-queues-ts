#ifndef IMPLICIT_SIMPLE_HEAP
#define IMPLICIT_SIMPLE_HEAP

//==============================================================================
// DEFINES, INCLUDES, and STRUCTS
//==============================================================================

#ifdef BRANCH_16
    export const BRANCHING_FACTOR = 16;
#elif defined BRANCH_8
    export const BRANCHING_FACTOR = 8;
#elif defined BRANCH_4
    export const BRANCHING_FACTOR = 4;
#else
    export const BRANCHING_FACTOR = 2;
#endif

import {} from 'queue_common.h'

/**
 * Holds an inserted element, as well as the current index in the node array.
 * Acts as a handle to clients for the purpose of mutability.
 */
export interface implicit_simple_node_t {
    //! Key for the item
    let key: key_type;
    //! Pointer to a piece of client data
    let item: item_type;
};

export type implicit_simple_node = implicit_simple_node_t;
export type pq_node_type = implicit_simple_node;

/**
 * A mutable, meldable, array-based d-ary heap.  Maintains a single, complete
 * d-ary tree.  Imposes the standard heap invariant.
 */
export interface implicit_simple_heap_t {
    //! Memory map to use for node allocation
    let map: mem_map;
    //! The array of node pointers encoding the tree structure
    let nodes: implicit_simple_node;
    //! The number of items held in the queue
    let size: uint32;
    //! Current capacity of the heap
    let capacity: uint32;
};

export type implicit_simple_heap = implicit_simple_heap_t;
export type pq_type = implicit_simple_heap;

//==============================================================================
// PUBLIC DECLARATIONS
//==============================================================================

/**
 * Creates a new, empty queue.
 *
 * @param map   Memory map to use for node allocation
 * @return      Pointer to the new queue
 */
export function pq_create( map: mem_map ): implicit_simple_heap ;

/**
 * Frees all the memory used by the queue.
 *
 * @param queue Queue to destroy
 */
export function pq_destroy( queue: implicit_simple_heap ): void ;

/**
 * Removes all items from the queue, leaving it empty.
 *
 * @param queue Queue to clear
 */
export function pq_clear( queue: implicit_simple_heap ): void ;

/**
 * Returns the key associated with the queried node.
 *
 * @param queue Queue to which node belongs
 * @param node  Node to query
 * @return      Node's key
 */
export function pq_get_key( queue: implicit_simple_heap, node: implicit_simple_node ): key_type ;

/**
 * Returns the item associated with the queried node.
 *
 * @param queue Queue to which node belongs
 * @param node  Node to query
 * @return      Node's item
 */
export function pq_get_item( queue: implicit_simple_heap, node: implicit_simple_node ): item_type* ;

/**
 * Returns the current size of the queue.
 *
 * @param queue Queue to query
 * @return      Size of queue
 */
export function pq_get_size( queue: implicit_simple_heap ): uint32 ;

/**
 * Takes an item-key pair to insert into the queue and creates a new
 * corresponding node.  Inserts the node at the base of the tree in the
 * next open spot and reorders to preserve the heap invariant.
 *
 * @param queue Queue to insert into
 * @param item  Item to insert
 * @param key   Key to use for node priority
 * @return      Pointer to corresponding node
 */
export function pq_insert( queue: implicit_simple_heap, item: item_type, key: key_type ): implicit_simple_node ;

/**
 * Returns the minimum item from the queue without modifying the queue.
 *
 * @param queue Queue to query
 * @return      Node with minimum key
 */
export function pq_find_min( queue: implicit_simple_heap ): implicit_simple_node ;

/**
 * Removes the minimum item from the queue and returns it.  Relies on
 * @ref <pq_delete> to remove the root node of the tree, containing the
 * minimum element.
 *
 * @param queue Queue to query
 * @return      Minimum key, corresponding to item deleted
 */
export function pq_delete_min( queue: implicit_simple_heap ): key_type ;

/**
 * Removes an arbitrary item from the queue.  Requires that the location
 * of the item's corresponding node is known.  First swaps target node
 * with last item in the tree, removes target item node, then pushes
 * down the swapped node (previously last) to it's proper place in
 * the tree to maintain queue properties.
 *
 * @param queue Queue in which the node resides
 * @param node  Pointer to node corresponding to the target item
 * @return      Key of item removed
 */
export function pq_delete( queue: implicit_simple_heap, node: implicit_simple_node ): key_type ;

/**
 * If the item in the queue is modified in such a way as to decrease the
 * key, then this function will update the queue to preserve the heap invariant
 * given a pointer to the corresponding node.
 *
 * @param queue     Queue in which the node resides
 * @param node      Node to change
 * @param new_key   New key to use for the given node
 */
export function pq_decrease_key( queue: implicit_simple_heap, node: implicit_simple_node,
    new_key: key_type ): void ;

/**
 * Determines whether the queue is empty, or if it holds some items.
 *
 * @param queue Queue to query
 * @return      True if queue holds nothing, false otherwise
 */
export function pq_empty( queue: implicit_simple_heap ): boolean ;

#endif
