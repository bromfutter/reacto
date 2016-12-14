#ifndef QUEUE_H_
#define QUEUE_H_

#include <stddef.h>
#include "event_loop_types.h"

/*
 * Number of slots must be a power of two, otherwise it will be down sized
 * to its approximate power of two.
 * `event_queue_init` returns -1 if error or the accepted number_of_slots.
 */
int queue_init(queue_t * obj, size_t number_of_slots);
void queue_deinit(queue_t * obj);

/*
 * ISR safe, use this function to add data to the queue.
 */
#define queue_push(queue, buffer, data)   \
({ \
    fast_ring_fifo_t * _fifo = &((queue)->fifo); \
    int _success = -1; \
    if (!fast_ring_fifo_full (_fifo)) {\
        buffer[fast_ring_fifo_write_index(_fifo)] = data; \
        fast_ring_fifo_write_increment(_fifo); \
        _success = 0; \
    } \
    _success; \
})

/*
 * Use this function to get data from the queue in the slot handler.
 */
#define queue_peek(queue, buffer, destination)   \
({ \
    fast_ring_fifo_t * _fifo = &((queue)->fifo); \
    int _success = -1; \
    if (!fast_ring_fifo_empty (_fifo)) {\
        *(destination) = buffer[fast_ring_fifo_read_index(_fifo)]; \
        _success = 0; \
    } \
    _success; \
})

#define queue_full(queue) \
({ \
    fast_ring_fifo_t * _fifo = &((queue)->fifo); \
    bool _full = false; \
    if (fast_ring_fifo_full (_fifo)) {\
        _full = true; \
    } \
    _full; \
})

signal_queue_t * queue_signal(queue_t * obj);
queue_i * queue_interface (queue_t * obj);

#endif /* QUEUE_H_ */
