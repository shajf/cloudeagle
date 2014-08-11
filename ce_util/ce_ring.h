/*
 * =====================================================================================
 *
 *       Filename:  ce_ring.h
 *
 *    Description:  
 *
 *        Version:  
 *        Created:  07/01/2013 13:56:34 AM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  shajianfeng
 *   Organization:  ,2013,CE TEAM
 *
 * =====================================================================================
 */

#ifndef CE_RING_H
#define CE_RING_H

/**
 * @file ce_ring.h
 * @brief CE Rings
 */

/*
 * for offsetof()
 */
#include "ce_basicdefs.h"

/**
 * @defgroup ce_ring Ring Macro Implemecetions
 * @ingroup CE 
 * A ring is a kind of doubly-linked list that can be manipulated
 * without knowing where its head is.
 * @{
 */

/**
 * The Ring Element
 *
 * A ring element struct is linked to the other elements in the ring
 * through its ring entry field, e.g.
 * <pre>
 *      struct my_element_t {
 *          CE_RING_ENTRY(my_element_t) link;
 *          int foo;
 *          char *bar;
 *      };
 * </pre>
 *
 * An element struct may be put on more than one ring if it has more
 * than one CE_RING_ENTRY field. Each CE_RING_ENTRY has a corresponding
 * CE_RING_HEAD declaration.
 *
 * @warning For strict C standards compliance you should put the CE_RING_ENTRY
 * first in the element struct unless the head is always part of a larger
 * object with enough earlier fields to accommodate the offsetof() used
 * to compute the ring sentinel below. You can usually ignore this caveat.
 */
#define CE_RING_ENTRY(elem)						\
    struct {								\
	struct elem *  next;					\
	struct elem *  prev;					\
    }

/**
 * The Ring Head
 *
 * Each ring is managed via its head, which is a struct declared like this:
 * <pre>
 *      CE_RING_HEAD(my_ring_t, my_element_t);
 *      struct my_ring_t ring, *ringp;
 * </pre>
 *
 * This struct looce just like the element link struct so that we can
 * be sure that the typecasting games will work as expected.
 *
 * The first element in the ring is next after the head, and the last
 * element is just before the head.
 */
#define CE_RING_HEAD(head, elem)					\
    struct head {							\
	struct elem *  next;					\
	struct elem *  prev;					\
    }

/**
 * The Ring Sentinel
 *
 * This is the magic pointer value that occurs before the first and
 * after the last elements in the ring, computed from the address of
 * the ring's head.  The head itself isn't an element, but in order to
 * get rid of all the special cases when dealing with the ends of the
 * ring, we play typecasting games to make it look like one.
 *
 * Here is a diagram to illustrate the arrangements of the next and
 * prev pointers of each element in a single ring. Note that they point
 * to the start of each element, not to the CE_RING_ENTRY structure.
 *
 * <pre>
 *     +->+------+<-+  +->+------+<-+  +->+------+<-+
 *     |  |struct|  |  |  |struct|  |  |  |struct|  |
 *    /   | elem |   \/   | elem |   \/   | elem |  \
 * ...    |      |   /\   |      |   /\   |      |   ...
 *        +------+  |  |  +------+  |  |  +------+
 *   ...--|prev  |  |  +--|ring  |  |  +--|prev  |
 *        |  next|--+     | entry|--+     |  next|--...
 *        +------+        +------+        +------+
 *        | etc. |        | etc. |        | etc. |
 *        :      :        :      :        :      :
 * </pre>
 *
 * The CE_RING_HEAD is nothing but a bare CE_RING_ENTRY. The prev
 * and next pointers in the first and last elements don't actually
 * point to the head, they point to a phantom place called the
 * sentinel. Its value is such that last->next->next == first because
 * the offset from the sentinel to the head's next pointer is the same
 * as the offset from the start of an element to its next pointer.
 * This also worce in the opposite direction.
 *
 * <pre>
 *        last                            first
 *     +->+------+<-+  +->sentinel<-+  +->+------+<-+
 *     |  |struct|  |  |            |  |  |struct|  |
 *    /   | elem |   \/              \/   | elem |  \
 * ...    |      |   /\              /\   |      |   ...
 *        +------+  |  |  +------+  |  |  +------+
 *   ...--|prev  |  |  +--|ring  |  |  +--|prev  |
 *        |  next|--+     |  head|--+     |  next|--...
 *        +------+        +------+        +------+
 *        | etc. |                        | etc. |
 *        :      :                        :      :
 * </pre>
 *
 * Note that the offset mentioned above is different for each kind of
 * ring that the element may be on, and each kind of ring has a unique
 * name for its CE_RING_ENTRY in each element, and has its own type
 * for its CE_RING_HEAD.
 *
 * Note also that if the offset is non-zero (which is required if an
 * element has more than one CE_RING_ENTRY), the unreality of the
 * sentinel may have bad implications on very perverse implemecetions
 * of C -- see the warning in CE_RING_ENTRY.
 *
 * @param hp   The head of the ring
 * @param elem The name of the element struct
 * @param link The name of the CE_RING_ENTRY in the element struct
 */
#define CE_RING_SENTINEL(hp, elem, link)				\
    (struct elem *)((char *)(&(hp)->next) - CE_OFFSETOF(struct elem, link))

/**
 * The first element of the ring
 * @param hp   The head of the ring
 */
#define CE_RING_FIRST(hp)	(hp)->next
/**
 * The last element of the ring
 * @param hp   The head of the ring
 */
#define CE_RING_LAST(hp)	(hp)->prev
/**
 * The next element in the ring
 * @param ep   The current element
 * @param link The name of the CE_RING_ENTRY in the element struct
 */
#define CE_RING_NEXT(ep, link)	(ep)->link.next
/**
 * The previous element in the ring
 * @param ep   The current element
 * @param link The name of the CE_RING_ENTRY in the element struct
 */
#define CE_RING_PREV(ep, link)	(ep)->link.prev


/**
 * Initialize a ring
 * @param hp   The head of the ring
 * @param elem The name of the element struct
 * @param link The name of the CE_RING_ENTRY in the element struct
 */
#define CE_RING_INIT(hp, elem, link) do {				\
	CE_RING_FIRST((hp)) = CE_RING_SENTINEL((hp), elem, link);	\
	CE_RING_LAST((hp))  = CE_RING_SENTINEL((hp), elem, link);	\
    } while (0)

/**
 * Determine if a ring is empty
 * @param hp   The head of the ring
 * @param elem The name of the element struct
 * @param link The name of the CE_RING_ENTRY in the element struct
 * @return true or false
 */
#define CE_RING_EMPTY(hp, elem, link)					\
    (CE_RING_FIRST((hp)) == CE_RING_SENTINEL((hp), elem, link))

/**
 * Initialize a singleton element
 * @param ep   The element
 * @param link The name of the CE_RING_ENTRY in the element struct
 */
#define CE_RING_ELEM_INIT(ep, link) do {				\
	CE_RING_NEXT((ep), link) = (ep);				\
	CE_RING_PREV((ep), link) = (ep);				\
    } while (0)


/**
 * Splice the sequence ep1..epN into the ring before element lep
 *   (..lep.. becomes ..ep1..epN..lep..)
 * @warning This doesn't work for splicing before the first element or on
 *   empty rings... see CE_RING_SPLICE_HEAD for one that does
 * @param lep  Element in the ring to splice before
 * @param ep1  First element in the sequence to splice in
 * @param epN  Last element in the sequence to splice in
 * @param link The name of the CE_RING_ENTRY in the element struct
 */
#define CE_RING_SPLICE_BEFORE(lep, ep1, epN, link) do {		\
	CE_RING_NEXT((epN), link) = (lep);				\
	CE_RING_PREV((ep1), link) = CE_RING_PREV((lep), link);	\
	CE_RING_NEXT(CE_RING_PREV((lep), link), link) = (ep1);	\
	CE_RING_PREV((lep), link) = (epN);				\
    } while (0)

/**
 * Splice the sequence ep1..epN into the ring after element lep
 *   (..lep.. becomes ..lep..ep1..epN..)
 * @warning This doesn't work for splicing after the last element or on
 *   empty rings... see CE_RING_SPLICE_TAIL for one that does
 * @param lep  Element in the ring to splice after
 * @param ep1  First element in the sequence to splice in
 * @param epN  Last element in the sequence to splice in
 * @param link The name of the CE_RING_ENTRY in the element struct
 */
#define CE_RING_SPLICE_AFTER(lep, ep1, epN, link) do {			\
	CE_RING_PREV((ep1), link) = (lep);				\
	CE_RING_NEXT((epN), link) = CE_RING_NEXT((lep), link);	\
	CE_RING_PREV(CE_RING_NEXT((lep), link), link) = (epN);	\
	CE_RING_NEXT((lep), link) = (ep1);				\
    } while (0)

/**
 * Insert the element nep into the ring before element lep
 *   (..lep.. becomes ..nep..lep..)
 * @warning This doesn't work for inserting before the first element or on
 *   empty rings... see CE_RING_INSERT_HEAD for one that does
 * @param lep  Element in the ring to insert before
 * @param nep  Element to insert
 * @param link The name of the CE_RING_ENTRY in the element struct
 */
#define CE_RING_INSERT_BEFORE(lep, nep, link)				\
	CE_RING_SPLICE_BEFORE((lep), (nep), (nep), link)

/**
 * Insert the element nep into the ring after element lep
 *   (..lep.. becomes ..lep..nep..)
 * @warning This doesn't work for inserting after the last element or on
 *   empty rings... see CE_RING_INSERT_TAIL for one that does
 * @param lep  Element in the ring to insert after
 * @param nep  Element to insert
 * @param link The name of the CE_RING_ENTRY in the element struct
 */
#define CE_RING_INSERT_AFTER(lep, nep, link)				\
	CE_RING_SPLICE_AFTER((lep), (nep), (nep), link)


/**
 * Splice the sequence ep1..epN into the ring before the first element
 *   (..hp.. becomes ..hp..ep1..epN..)
 * @param hp   Head of the ring
 * @param ep1  First element in the sequence to splice in
 * @param epN  Last element in the sequence to splice in
 * @param elem The name of the element struct
 * @param link The name of the CE_RING_ENTRY in the element struct
 */
#define CE_RING_SPLICE_HEAD(hp, ep1, epN, elem, link)			\
	CE_RING_SPLICE_AFTER(CE_RING_SENTINEL((hp), elem, link),	\
			     (ep1), (epN), link)

/**
 * Splice the sequence ep1..epN into the ring after the last element
 *   (..hp.. becomes ..ep1..epN..hp..)
 * @param hp   Head of the ring
 * @param ep1  First element in the sequence to splice in
 * @param epN  Last element in the sequence to splice in
 * @param elem The name of the element struct
 * @param link The name of the CE_RING_ENTRY in the element struct
 */
#define CE_RING_SPLICE_TAIL(hp, ep1, epN, elem, link)			\
	CE_RING_SPLICE_BEFORE(CE_RING_SENTINEL((hp), elem, link),	\
			     (ep1), (epN), link)

/**
 * Insert the element nep into the ring before the first element
 *   (..hp.. becomes ..hp..nep..)
 * @param hp   Head of the ring
 * @param nep  Element to insert
 * @param elem The name of the element struct
 * @param link The name of the CE_RING_ENTRY in the element struct
 */
#define CE_RING_INSERT_HEAD(hp, nep, elem, link)			\
	CE_RING_SPLICE_HEAD((hp), (nep), (nep), elem, link)

/**
 * Insert the element nep into the ring after the last element
 *   (..hp.. becomes ..nep..hp..)
 * @param hp   Head of the ring
 * @param nep  Element to insert
 * @param elem The name of the element struct
 * @param link The name of the CE_RING_ENTRY in the element struct
 */
#define CE_RING_INSERT_TAIL(hp, nep, elem, link)			\
	CE_RING_SPLICE_TAIL((hp), (nep), (nep), elem, link)

/**
 * Concatenate ring h2 onto the end of ring h1, leaving h2 empty.
 * @param h1   Head of the ring to concatenate onto
 * @param h2   Head of the ring to concatenate
 * @param elem The name of the element struct
 * @param link The name of the CE_RING_ENTRY in the element struct
 */
#define CE_RING_CONCAT(h1, h2, elem, link) do {			\
	if (!CE_RING_EMPTY((h2), elem, link)) {			\
	    CE_RING_SPLICE_BEFORE(CE_RING_SENTINEL((h1), elem, link),	\
				  CE_RING_FIRST((h2)),			\
				  CE_RING_LAST((h2)), link);		\
	    CE_RING_INIT((h2), elem, link);				\
	}								\
    } while (0)

/**
 * Prepend ring h2 onto the beginning of ring h1, leaving h2 empty.
 * @param h1   Head of the ring to prepend onto
 * @param h2   Head of the ring to prepend
 * @param elem The name of the element struct
 * @param link The name of the CE_RING_ENTRY in the element struct
 */
#define CE_RING_PREPEND(h1, h2, elem, link) do {			\
	if (!CE_RING_EMPTY((h2), elem, link)) {			\
	    CE_RING_SPLICE_AFTER(CE_RING_SENTINEL((h1), elem, link),	\
				  CE_RING_FIRST((h2)),			\
				  CE_RING_LAST((h2)), link);		\
	    CE_RING_INIT((h2), elem, link);				\
	}								\
    } while (0)

/**
 * Unsplice a sequence of elements from a ring
 * @warning The unspliced sequence is left with dangling pointers at either end
 * @param ep1  First element in the sequence to unsplice
 * @param epN  Last element in the sequence to unsplice
 * @param link The name of the CE_RING_ENTRY in the element struct
 */
#define CE_RING_UNSPLICE(ep1, epN, link) do {				\
	CE_RING_NEXT(CE_RING_PREV((ep1), link), link) =		\
		     CE_RING_NEXT((epN), link);			\
	CE_RING_PREV(CE_RING_NEXT((epN), link), link) =		\
		     CE_RING_PREV((ep1), link);			\
    } while (0)

/**
 * Remove a single element from a ring
 * @warning The unspliced element is left with dangling pointers at either end
 * @param ep   Element to remove
 * @param link The name of the CE_RING_ENTRY in the element struct
 */
#define CE_RING_REMOVE(ep, link)					\
    CE_RING_UNSPLICE((ep), (ep), link)

/**
 * Iterate over a ring
 * @param ep The current element
 * @param head The head of the ring
 * @param elem The name of the element struct
 * @param link The name of the CE_RING_ENTRY in the element struct
 */
#define CE_RING_FOREACH(ep, head, elem, link)                          \
    for (ep = CE_RING_FIRST(head);                                     \
         ep != CE_RING_SENTINEL(head, elem, link);                     \
         ep = CE_RING_NEXT(ep, link))

/**
 * Iterate over a ring safe against removal of the current element
 * @param ep1 The current element
 * @param ep2 Iteration cursor
 * @param head The head of the ring
 * @param elem The name of the element struct
 * @param link The name of the CE_RING_ENTRY in the element struct
 */
#define CE_RING_FOREACH_SAFE(ep1, ep2, head, elem, link)               \
    for (ep1 = CE_RING_FIRST(head), ep2 = CE_RING_NEXT(ep1, link);    \
         ep1 != CE_RING_SENTINEL(head, elem, link);                    \
         ep1 = ep2, ep2 = CE_RING_NEXT(ep1, link))

/* Debugging tools: */

#ifdef CE_RING_DEBUG
#include <stdio.h>
#include <assert.h>

#define CE_RING_CHECK_ONE(msg, ptr)					\
	fprintf(stderr, "*** %s %p\n", msg, ptr)

#define CE_RING_CHECK(hp, elem, link, msg)				\
	CE_RING_CHECK_ELEM(CE_RING_SENTINEL(hp, elem, link), elem, link, msg)

#define CE_RING_CHECK_ELEM(ep, elem, link, msg) do {			\
	struct elem *start = (ep);					\
	struct elem *here = start;					\
	fprintf(stderr, "*** ring check start -- %s\n", msg);		\
	do {								\
	    fprintf(stderr, "\telem %p\n", here);			\
	    fprintf(stderr, "\telem->next %p\n",			\
		    CE_RING_NEXT(here, link));				\
	    fprintf(stderr, "\telem->prev %p\n",			\
		    CE_RING_PREV(here, link));				\
	    fprintf(stderr, "\telem->next->prev %p\n",			\
		    CE_RING_PREV(CE_RING_NEXT(here, link), link));	\
	    fprintf(stderr, "\telem->prev->next %p\n",			\
		    CE_RING_NEXT(CE_RING_PREV(here, link), link));	\
	    if (CE_RING_PREV(CE_RING_NEXT(here, link), link) != here) { \
		fprintf(stderr, "\t*** elem->next->prev != elem\n");	\
		break;							\
	    }								\
	    if (CE_RING_NEXT(CE_RING_PREV(here, link), link) != here) { \
		fprintf(stderr, "\t*** elem->prev->next != elem\n");	\
		break;							\
	    }								\
	    here = CE_RING_NEXT(here, link);				\
	} while (here != start);					\
	fprintf(stderr, "*** ring check end\n");			\
    } while (0)

#define CE_RING_CHECK_CONSISTENCY(hp, elem, link)			\
	CE_RING_CHECK_ELEM_CONSISTENCY(CE_RING_SENTINEL(hp, elem, link),\
					elem, link)

#define CE_RING_CHECK_ELEM_CONSISTENCY(ep, elem, link) do {		\
	struct elem *start = (ep);					\
	struct elem *here = start;					\
	do {								\
	    assert(CE_RING_PREV(CE_RING_NEXT(here, link), link) == here); \
	    assert(CE_RING_NEXT(CE_RING_PREV(here, link), link) == here); \
	    here = CE_RING_NEXT(here, link);				\
	} while (here != start);					\
    } while (0)

#else
/**
 * Print a single pointer value to STDERR
 *   (This is a no-op unless CE_RING_DEBUG is defined.)
 * @param msg Descriptive message
 * @param ptr Pointer value to print
 */
#define CE_RING_CHECK_ONE(msg, ptr)
/**
 * Dump all ring pointers to STDERR, starting with the head and looping all
 * the way around the ring back to the head.  Aborts if an inconsistency
 * is found.
 *   (This is a no-op unless CE_RING_DEBUG is defined.)
 * @param hp   Head of the ring
 * @param elem The name of the element struct
 * @param link The name of the CE_RING_ENTRY in the element struct
 * @param msg  Descriptive message
 */
#define CE_RING_CHECK(hp, elem, link, msg)
/**
 * Loops around a ring and checce all the pointers for consistency.  Pops
 * an assertion if any inconsistency is found.  Same idea as CE_RING_CHECK()
 * except that it's silent if all is well.
 *   (This is a no-op unless CE_RING_DEBUG is defined.)
 * @param hp   Head of the ring
 * @param elem The name of the element struct
 * @param link The name of the CE_RING_ENTRY in the element struct
 */
#define CE_RING_CHECK_CONSISTENCY(hp, elem, link)
/**
 * Dump all ring pointers to STDERR, starting with the given element and
 * looping all the way around the ring back to that element.  Aborts if
 * an inconsistency is found.
 *   (This is a no-op unless CE_RING_DEBUG is defined.)
 * @param ep   The element
 * @param elem The name of the element struct
 * @param link The name of the CE_RING_ENTRY in the element struct
 * @param msg  Descriptive message
 */
#define CE_RING_CHECK_ELEM(ep, elem, link, msg)
/**
 * Loops around a ring, starting with the given element, and checce all
 * the pointers for consistency.  Pops an assertion if any inconsistency
 * is found.  Same idea as CE_RING_CHECK_ELEM() except that it's silent
 * if all is well.
 *   (This is a no-op unless CE_RING_DEBUG is defined.)
 * @param ep   The element
 * @param elem The name of the element struct
 * @param link The name of the CE_RING_ENTRY in the element struct
 */
#define CE_RING_CHECK_ELEM_CONSISTENCY(ep, elem, link)
#endif

/** @} */ 

#endif /* !CE_RING_H */
