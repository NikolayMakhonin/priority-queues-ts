#ifndef BINARY_ARRAY_HEAP
#define BINARY_ARRAY_HEAP

#include "heap_common.h"

/**
 * Holds an inserted element, as well as pointers to maintain tree
 * structure.  Acts as a handle to clients for the purpose of
 * mutability.  Each node has a pointer to its parent and its left and
 * right siblings.
 */
typedef struct binary_array_node_t {
    //! Pointer to a piece of client data
    void* item;
    //! Key for the item
    uint32_t key;
    //! Index for the item in the "tree" array
    uint32_t index;
} binary_array_node;

/**
 * A mutable, meldable, node-based binary heap.  Maintains a single, complete
 * binary tree.  Imposes the standard heap invariant.
 */
typedef struct binary_array_heap_t {
    //! The root of the binary tree representing the heap
    binary_array_node **nodes;
    //! The number of items held in the heap
    uint32_t size;
    //! The maximum number of items the heap can currently hold
    uint32_t capacity;
    //! A collection of operation counters
    heap_stats *stats;
} binary_array_heap;

/**
 * Creates a new, empty heap.
 *
 * @return  Pointer to the new heap
 */
binary_array_heap* create_heap();

/**
 * Frees all the memory used by the heap.
 *
 * @param heap  Heap to destroy
 */
void destroy_heap( binary_array_heap *heap );

/**
 * Repeatedly deletes nodes associated with the heap until it is empty.
 *
 * @param heap  Heap to clear
 */
void clear_heap( binary_array_heap *heap );

/**
 * Returns the key associated with the queried node.
 *
 * @param heap  Node to query
 * @return      Node's key
 */
uint32_t get_key( binary_array_node *node );

/**
 * Returns the item associated with the queried node.
 *
 * @param heap  Node to query
 * @return      Node's item
 */
void* get_item( binary_array_node *node );

/**
 * Returns the current size of the heap.
 *
 * @param heap  Heap to query
 * @return      Size of heap
 */
uint32_t get_size( binary_array_heap *heap );

/**
 * Takes a item-key pair to insert into the heap and creates a new
 * corresponding node.  Inserts the node at the base of the tree in the
 * next open spot and reorders to preserve the heap property.
 *
 * @param heap  Heap to insert into
 * @param item  Item to insert
 * @param key   Key to use for node priority
 * @return      Pointer to corresponding node
 */
binary_array_node* insert( binary_array_heap *heap, void *item, uint32_t key );

/**
 * Returns the minimum item from the heap without modifying the heap.
 *
 * @param heap  Heap to query
 * @return      Item with minimum key
 */
void* find_min( binary_array_heap *heap );

/**
 * Removes the minimum item from the heap and returns it.  Relies on
 * @ref <delete> to remove the root node of the tree, containing the
 * minimum element.
 *
 * @param heap  Heap to query
 * @return      Item with minimum key
 */
void* delete_min( binary_array_heap *heap ) ;

/**
 * Removes an arbitrary item from the heap.  Requires that the location
 * of the item's corresponding node is known.  First swaps target node
 * with last item in the tree, removes target item node, then pushes
 * down the swapped node (previously last) to it's proper place in
 * the tree to maintain heap properties.
 *
 * @param heap  Heap in which the node resides
 * @param node  Pointer to node corresponding to the target item
 * @return      Item removed
 */
void* delete( binary_array_heap *heap, binary_array_node* node );

/**
 * If the item in the heap is modified in such a way to decrease the
 * key, then this function will update the heap to preserve heap
 * properties given a pointer to the corresponding node.
 *
 * @param heap  Heap in which the node resides
 * @param node  Node to change
 * @param delta Amount by which to decrease the requested key
 */
void decrease_key( binary_array_heap *heap, binary_array_node *node, uint32_t delta );

/**
 * Moves all elements from the secondary heap into this one.  Leaves the
 * secondary heap empty.
 *
 * @param heap          Primary heap to be melded - target
 * @param other_heap    Secondary heap to be melded - source
 */
void meld( binary_array_heap *heap, binary_array_heap *other_heap );

/**
 * Determines whether the heap is empty, or if it holds some items.
 *
 * @param heap  Heap to query
 * @return      True if heap holds nothing, false otherwise
 */
bool empty( binary_array_heap *heap );

/**
 * Takes two nodes and switches their positions in the tree.  Does not
 * make any assumptions about null pointers or relative locations in
 * tree, and thus checks all edge cases to be safe.
 *
 * @param heap  Heap to which both nodes belong
 * @param a     First node to switch
 * @param b     Second node to switch
 */
void swap( binary_array_heap *heap, uint32_t a, uint32_t b );

/**
 * Takes a node that is potentially at a higher position in the tree
 * than it should be, and pushes it down to the correct location.
 *
 * @param heap  Heap to which node belongs
 * @param node  Potentially violating node
 */
void heapify_down( binary_array_heap *heap, binary_array_node *node );

/**
 * Takes a node that is potentially at a lower position in the tree
 * than it should be, and pulls it up to the correct location.
 *
 * @param heap  Heap to which node belongs
 * @param node  Potentially violating node
 */
void heapify_up( binary_array_heap *heap, binary_array_node *node );

/**
 * Resizes a heap to a given capacity.
 * 
 * @param heap          Heap to resize
 * @param new_capacity  New capacity
 */
void resize_heap( binary_array_heap *heap, uint32_t new_capacity );

#endif
