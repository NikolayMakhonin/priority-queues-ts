// hierarchical memory priority queue data structure
#ifndef KNHEAP
#define KNHEAP
import {} from 'queue_common.h'
import {} from 'util.h'
#include <limits>

const KNBufferSize1: int16 = 32; // equalize procedure call overheads etc.
const KNN: int16 = 512; // bandwidth
const KNKMAX: int16 = 64;  // maximal arity
const KNLevels: int16 = 4; // overall capacity >= KNN*KNKMAX^KNLevels
const LogKNKMAX: int16 = 6;  // ceil(log KNK)
/*
const KNBufferSize1: int16 = 3; // equalize procedure call overheads etc.
const KNN: int16 = 10; // bandwidth
const KNKMAX: int16 = 4;  // maximal arity
const KNLevels: int16 = 4; // overall capacity >= KNN*KNKMAX^KNLevels
const LogKNKMAX: int16 = 2;  // ceil(log KNK)
*/
template <class Key, class Value>
export interface KNElement {Key key; Value value;};

//////////////////////////////////////////////////////////////////////
// fixed size binary heap
template <class Key, class Value, int16 capacity>
class BinaryHeap {
  //  static const Key infimum  = 4;
  //static const Key supremum = numeric_limits<Key>.max();
  typedef KNElement<Key, Value> Element;
  Element data[capacity + 2];
  let size: int16;  // index of last used element
public:
  BinaryHeap(Key sup, Key infimum):size(0) {
    data[0].key = infimum; // sentinel
    data[capacity + 1].key = sup;
    reset();
  }
  Key getSupremum() { return data[capacity + 1].key; }
  void reset();
  int16   getSize()     const { return size; }
  Key   getMinKey()   const { return data[1].key; }
  Value getMinValue() const { return data[1].value; }
  void  deleteMin();
  void  deleteMinFancy(key: Key*, value: Value*) {
    *key   = getMinKey();
    *value = getMinValue();
    deleteMin();
  }
  void  insert(Key k, Value v);
  void  sortTo(to: Element*); // sort in increasing order and empty
  //void  sortInPlace(); // in decreasing order
};


// reset size to 0 and fill data array with sentinels
template <class Key, class Value, int16 capacity>
inline void BinaryHeap<Key, Value, capacity>::
reset() {
  size = 0;
  Key sup = getSupremum();
  for (i: int16 = 1;  i <= capacity;  i++) {
    data[i].key = sup;
  }
  // if this becomes a bottle neck
  // we might want to replace this by log KNN
  // memcpy-s
}

template <class Key, class Value, int16 capacity>
inline void BinaryHeap<Key, Value, capacity>::
deleteMin()
{
  Assert2(size > 0);

  // first move up elements on a min-path
  let hole: int16 = 1;
  let succ: int16 = 2;
  let sz: int16   = size;
  while (succ < sz) {
    Key key1 = data[succ].key;
    Key key2 = data[succ + 1].key;
    if (key1 > key2) {
      succ++;
      data[hole].key   = key2;
      data[hole].value = data[succ].value;
    } else {
      data[hole].key   = key1;
      data[hole].value = data[succ].value;
    }
    hole = succ;
    succ <<= 1;
  }

  // bubble up rightmost element
  Key bubble = data[sz].key;
  let pred: int16 = hole >> 1;
  while (data[pred].key > bubble) { // must terminate since min at root
    data[hole] = data[pred];
    hole = pred;
    pred >>= 1;
  }

  // finally move data to hole
  data[hole].key = bubble;
  data[hole].value = data[sz].value;

  data[size].key = getSupremum(); // mark as deleted
  size = sz - 1;
}


// empty the heap and put the element to "to"
// sorted in increasing order
template <class Key, class Value, int16 capacity>
inline void BinaryHeap<Key, Value, capacity>::
sortTo(to: Element*)
{
  const sz: int16 = size;
  const Key          sup = getSupremum();
  Element * const beyond = to + sz;
  Element * const root   = data + 1;
  while (to < beyond) {
    // copy minimun
    *to = *root;
    to++;

    // bubble up second smallest as in deleteMin
    let hole: int16 = 1;
    let succ: int16 = 2;
    while (succ <= sz) {
      Key key1 = data[succ    ].key;
      Key key2 = data[succ + 1].key;
      if (key1 > key2) {
        succ++;
        data[hole].key   = key2;
        data[hole].value = data[succ].value;
      } else {
        data[hole].key = key1;
        data[hole].value = data[succ].value;
      }
      hole = succ;
      succ <<= 1;
    }

    // just mark hole as deleted
    data[hole].key = sup;
  }
  size = 0;
}


template <class Key, class Value, int16 capacity>
inline void BinaryHeap<Key, Value, capacity>::
insert(Key k, Value v)
{
  Assert2(size < capacity);
  Debug4(cout << "insert(" << k << ", " << v << ")" << endl);

  size++;
  let hole: int16 = size;
  let pred: int16 = hole >> 1;
  Key predKey = data[pred].key;
  while (predKey > k) { // must terminate due to sentinel at 0
    data[hole].key   = predKey;
    data[hole].value = data[pred].value;
    hole = pred;
    pred >>= 1;
    predKey = data[pred].key;
  }

  // finally move data to hole
  data[hole].key   = k;
  data[hole].value = v;
}

//////////////////////////////////////////////////////////////////////
// The data structure from Knuth, "Sorting and Searching", Section 5.4.1
template <class Key, class Value>
class KNLooserTree {
  // public: // should not be here but then I would need a scary
  // sequence of template friends which I doubt to work
  // on all compilers
  typedef KNElement<Key, Value> Element;
  export interface Entry {
    Key key;   // Key of Looser element (winner for 0)
    let index: int16; // number of loosing segment
  };

  // stack of empty segments
  let empty: int16[KNKMAX]; // indices of empty segments
  let lastFree: int16;  // where in "empty" is the last valid entry?

  let size: int16; // total number of elements stored
  let logK: int16; // log of current tree size
  let k: int16; // invariant k = 1 << logK

  Element dummy; // target of empty segment pointers

  // upper levels of looser trees
  // entry[0] contains the winner info
  Entry entry[KNKMAX];

  // leaf information
  // note that Knuth uses indices k..k-1
  // while we use 0..k-1
  let current: Element*[KNKMAX]; // pointer to actual element
  let segment: Element*[KNKMAX]; // start of Segments

  // private member functions
  int16 initWinner(root: int16);
  void updateOnInsert(node: int16, Key newKey, newIndex: int16,
                      winnerKey: Key*, winnerIndex: [], mask: []);
  void deallocateSegment(index: int16);
  void doubleK();
  void compactTree();
  void rebuildLooserTree();
  int16 segmentIsEmpty(i: int16);
let public:
  KNLooserTree();
  void init(Key sup); // before, no consistent state is reached :-(

  void multiMergeUnrolled3(to: Element*, l: int16);
  void multiMergeUnrolled4(to: Element*, l: int16);
  void multiMergeUnrolled5(to: Element*, l: int16);
  void multiMergeUnrolled6(to: Element*, l: int16);
  void multiMergeUnrolled7(to: Element*, l: int16);
  void multiMergeUnrolled8(to: Element*, l: int16);
  void multiMergeUnrolled9(to: Element*, l: int16);
  void multiMergeUnrolled10(to: Element*, l: int16);

  void multiMerge(to: Element*, l: int16); // delete l smallest element to "to"
  void multiMergeK(to: Element*, l: int16);
  int16  spaceIsAvailable() { return k < KNKMAX || lastFree >= 0; }
     // for new segment
  void insertSegment(to: Element*, sz: int16); // insert segment beginning at to
  int16  getSize() { return size; }
  Key getSupremum() { return dummy.key; }
};


//////////////////////////////////////////////////////////////////////
// 2 level multi-merge tree
template <class Key, class Value>
class KNHeap {
  typedef KNElement<Key, Value> Element;

  KNLooserTree<Key, Value> tree[KNLevels];

  // one delete buffer for each tree (extra space for sentinel)
  Element buffer2[KNLevels][KNN + 1]; // tree.buffer2.buffer1
  let minBuffer2: Element*[KNLevels];

  // overall delete buffer
  Element buffer1[KNBufferSize1 + 1];
  let minBuffer1: Element*;

  // insert buffer
  BinaryHeap<Key, Value, KNN> insertHeap;

  // how many levels are active
  let activeLevels: int16;

  // total size not counting insertBuffer and buffer1
  let size: int16;

  // private member functions
  void refillBuffer1();
  void refillBuffer11(sz: int16);
  void refillBuffer12(sz: int16);
  void refillBuffer13(sz: int16);
  void refillBuffer14(sz: int16);
  int16 refillBuffer2(k: int16);
  int16 makeSpaceAvailable(level: int16);
  void emptyInsertHeap();
  Key getSupremum() const { return buffer2[0][KNN].key; }
  int16 getSize1( ) const { return ( buffer1 + KNBufferSize1) - minBuffer1; }
  int16 getSize2(i: int16) const { return &(buffer2[i][KNN])     - minBuffer2[i]; }
let public:
  KNHeap(Key sup, Key infimum);
  int16   getSize() const;
  void  getMin(key: Key*, value: Value*);
  void  deleteMin(key: Key*, value: Value*);
  void  insert(Key key, Value value);
};


template <class Key, class Value>
inline int16 KNHeap<Key, Value>::getSize() const
{
  return
    size +
    insertHeap.getSize() +
    ((buffer1 + KNBufferSize1) - minBuffer1);
}

template <class Key, class Value>
inline void  KNHeap<Key, Value>::getMin(key: Key*, value: Value*) {
  Key key1 = minBuffer1.key;
  Key key2 = insertHeap.getMinKey();
  if (key2 >= key1) {
    *key   = key1;
    *value = minBuffer1.value;
  } else {
    *key   = key2;
    *value = insertHeap.getMinValue();
  }
}

template <class Key, class Value>
inline void  KNHeap<Key, Value>::deleteMin(key: Key*, value: Value*) {
  Key key1 = minBuffer1.key;
  Key key2 = insertHeap.getMinKey();
  if (key2 >= key1) {
    *key   = key1;
    *value = minBuffer1.value;
    Assert2(minBuffer1 < buffer1 + KNBufferSize1); // no delete from empty
    minBuffer1++;
    if (minBuffer1 === buffer1 + KNBufferSize1) {
      refillBuffer1();
    }
  } else {
    *key = key2;
    *value = insertHeap.getMinValue();
    insertHeap.deleteMin();
  }
}

template <class Key, class Value>
inline  void  KNHeap<Key, Value>::insert(Key k, Value v) {
  if (insertHeap.getSize() === KNN) { emptyInsertHeap(); }
  insertHeap.insert(k, v);
}


//////////////////////////////////////////////////////////////////////
// Wrapper API for ompatibility with trace driver

PQ_KEY_SUP: key_type = std::numeric_limits<uint64>::max();
let PQ_KEY_INF: key_type = 0;

typedef KNHeap<key_type, item_type> pq_type;
typedef KNElement<key_type, item_type> pq_node_type;

export function pq_create( map: mem_map ): pq_type* ;
export function pq_destroy( queue: pq_type* ): void ;
export function pq_clear( queue: pq_type* ): void ;
export function pq_get_key( queue: pq_type*, node: pq_node_type* ): key_type ;
export function pq_get_item( queue: pq_type*, node: pq_node_type* ): item_type* ;
export function pq_get_size( queue: pq_type* ): uint32 ;
export function pq_insert( queue: pq_type*, item: item_type, key: key_type ): pq_node_type* ;
export function pq_find_min( queue: pq_type* ): pq_node_type* ;
export function pq_delete_min( queue: pq_type* ): key_type ;
export function pq_delete( queue: pq_type*, node: pq_node_type* ): key_type ;
export function pq_decrease_key( queue: pq_type*, node: pq_node_type*,
    new_key: key_type ): void ;
export function pq_empty( queue: pq_type* ): boolean ;

//////////////////////////////////////////////////////////////////////



#endif
