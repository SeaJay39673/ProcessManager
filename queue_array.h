#ifndef QUEUE_ARRAY_H
#define QUEUE_ARRAY_H

/*
  Here is some example code for Program 1.  You don't have to use any of
  it, but if you are having trouble, it might get you started.
  The T used through this is the template class your need to create
  in this class, T means Example TYPE
*/
#include <cstdlib> //This is for the exit command.

#include <iostream>
#include <queue>

using namespace std;

/*
 *  In the class below, it declares a "queue<T>" array.
 *  you don't need to use the STL queue (but you can), choose a data structure
 * that you think works.   If you choose to write you own queue class, then
 * it has better follow all the requirements to a queue, to include the O time
 * minimum timing requirements of c++
 */

template <class T>
class QueueArray
{
public:
  QueueArray(int);
  ~QueueArray();
  int Asize();
  T Dequeue();
  int Enqueue(const T &item, const int index);
  int QAsize();
  int Qsize(int index);
  T *Qstate(int index);

private:
  int size;        // size of the array
  queue<T> *array; // the array of queues.  It must be an array (NO not a vector)
  int totalItems;  // total number of items stored in the queues
  bool inRange(int index);
};

//==== Public ====//

//---- Constructors ----//

// Constructor for the queue array.  It sets the default size
// to 10, initializes the private variables size and totalItems
template <class T>
QueueArray<T>::QueueArray(int sz) : size(sz), totalItems(0), array(new queue<T>[size])
{
  if (array == NULL)
  {
    cout << "Not enough memory to create the array" << endl;
    exit(-1);
  }
}

//---- Destructor ----//

/*
Deletes all pointers used in the QueueArray class
*/
template <class T>
QueueArray<T>::~QueueArray()
{
  delete[] array;
}

//---- Setters ----//

/*
Enqueues item in the queue of array index
index. Returns 1 if item is successfully enqueued; -1, if index is out of range; 0,
otherwise
*/
template <class T>
int QueueArray<T>::Enqueue(const T &item, const int index)
{
  if (!inRange(index))
  {
    return -1;
  }
  try
  {
    array[index].emplace(item); // Warning: May be better to use push() instead. . .
    totalItems++;
  }
  catch (exception &e)
  {
    return 0;
  }
  return 1;
}

//---- Getters ----//

/*
Returns the size of the array.
*/
template <class T>
int QueueArray<T>::Asize()
{
  return size;
}

/*
Returns the number of items in the queue at array index
index; -1, if index is out of range.
*/
template <class T>
int QueueArray<T>::Qsize(int index)
{
  if (!inRange(index))
  {
    return -1;
  }
  return array[index].size();
}

/*
Returns the total number of items stored in the array of queues.
*/
template <class T>
int QueueArray<T>::QAsize()
{
  return totalItems;
}

/*
Dequeues an item from the first non-empty queue in the array,
i.e., from the non-empty queue at the lowest numbered index in the array. Returns
the dequeued item, if there is at least one item in the queue array; 0 otherwise.
*/
template <class T>
T QueueArray<T>::Dequeue()
{
  for (int i = 0; i < Asize(); i++)
  {
    if (array[i].size() >= 1)
    {
      T val = array[i].front();
      array[i].pop();
      totalItems--;
      return val;
    }
  }
  return 0;
}

/*
Copies all items stored in the queue at array index index
in an array. Returns a pointer to this array; NULL, if index is out of range.
*/
template <class T>
T *QueueArray<T>::Qstate(int index)
{
  if (!inRange(index))
  {
    return NULL;
  }
  T *q = new T[array[index].size()];
  for (int i = 0; i < array[index].size(); i++)
  {
    T val = array[index].front();
    q[i] = val;
    array[index].pop();
    array[index].emplace(val);
  }
  return q;
}

//==== Private ====//

//---- Helpers ----//

/*
Takes an index and returns true if: 0 <= index < Asize (if index is in range).
*/
template <class T>
bool QueueArray<T>::inRange(int index)
{
  return index >= 0 && index < Asize();
}

#endif