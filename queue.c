/*
 * Developed by R. E. Bryant, 2017
 * Extended to store strings, 2018
 */

/*
 * This program implements a queue supporting both FIFO and LIFO
 * operations.
 *
 * It uses a singly-linked list to represent the set of queue elements
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "harness.h"
#include "queue.h"

/*
  Create empty queue.
  Return NULL if could not allocate space.
*/
queue_t *q_new()
{
    queue_t *q =  malloc(sizeof(queue_t));
    if(!q){ // if it returned null
      return NULL;
    }

    q->head = NULL;
    q->tail = NULL;
    q->size = 0;
    return q;
}

/* Free all storage used by queue */
void q_free(queue_t *q)
{
  if ( !q )
    return;
  list_ele_t * curr = q->head;

  while ( curr != NULL ){
    if ( curr->value != NULL )
      free(curr->value);
    list_ele_t * temp_next = curr->next;
    free ( curr );

    curr = temp_next;
  }

  free(q);
}

/*
  Attempt to insert element at head of queue.
  Return true if successful.
  Return false if q is NULL or could not allocate space.
  Argument s points to the string to be stored.
  The function must explicitly allocate space and copy the string into it.
 */
bool q_insert_head(queue_t *q, char *s)
{
    if (!q)
      return false;

    list_ele_t *newh;
    newh = malloc(sizeof(list_ele_t));
    if(!newh)
      return false;


    newh->next = q->head;
    newh->value = malloc( strlen(s) +1 );

    // copies value
    if ( !(newh->value) ){ // if it's null return false
      free(newh);
      return false;
    }
    strcpy(newh->value, s);

    // links head -> newNode
    q->head = newh;


    if ( !q->tail )
      q->tail = newh;

    q->size += 1;
    return true;
}


/*
  Attempt to insert element at tail of queue.
  Return true if successful.
  Return false if q is NULL or could not allocate space.
  Argument s points to the string to be stored.
  The function must explicitly allocate space and copy the string into it.
 */
bool q_insert_tail(queue_t *q, char *s)
{
    if ( !q )
      return false;


    list_ele_t * node =  malloc( sizeof(list_ele_t) );
    if ( !node )
      return false;


    if ( !q->head ){ // if its empty
      free( node );
      return q_insert_head(q,s);
    }

    node->next = NULL; // because its at the end


    node->value =  malloc( strlen(s) +1 );
    if ( !node->value ){
      free(node);
      return false;
    }
    strcpy(node->value, s);

    // linking previous last to new last
    list_ele_t * prev_last = q->tail;
    prev_last->next = node;


    q->tail = node; // setting new tail
    q->size +=1;
    return true;
}

/*
  Attempt to remove element from head of queue.
  Return true if successful.
  Return false if queue is NULL or empty.
  If sp is non-NULL and an element is removed, copy the removed string to *sp
  (up to a maximum of bufsize-1 characters, plus a null terminator.)
  The space used by the list element and the string should be freed.
*/
bool q_remove_head(queue_t *q, char *sp, size_t bufsize)
{
  if (!q) // queue is false
    return false;
  if ( !q->head ) // empty
    return false;

  // storing head nodes value if sp is not null
  if ( sp ){
    strncpy( sp, q->head->value, (bufsize-1) ); // storing up to that many chacarterst into sp
    sp[bufsize-1] = '\0'; // plus a null terminator
  }

  free ( q->head->value );


  // rearranging the pointers
  list_ele_t * temp = q->head->next;
  free(q->head);
  q->head = temp;

  if(!q->head)
    q->tail = NULL;

  q->size -= 1;
  return true;
}

/*
  Return number of elements in queue.
  Return 0 if q is NULL or empty
 */
int q_size(queue_t *q)
{
  if (!q)
    return 0;
  return q->size;
}

/*
  Reverse elements in queue
  No effect if q is NULL or empty
  This function should not allocate or free any list elements
  (e.g., by calling q_insert_head, q_insert_tail, or q_remove_head).
  It should rearrange the existing ones.
 */
void q_reverse(queue_t *q)
{
  if ( !q )
    return;
  if ( q->head == NULL)
    return;

  list_ele_t * prev_head_ptr = q->head;
  list_ele_t * curr = q->head->next;

  list_ele_t * temp_h = q->head;

  q->head = q->tail;
  q->tail = temp_h;
  q->tail->next = NULL;

  while ( curr ){

    list_ele_t * temp = curr->next;
    curr->next = prev_head_ptr; // bring back node to front
    prev_head_ptr = curr; // setting the current node to the back

    //tail_ptr->next = temp;


    curr = temp;
  }


    /* You need to write the code for this function */
}
