#ifndef EVENT_LOOP_MAIN_LOOP_H_
#define EVENT_LOOP_MAIN_LOOP_H_

#include <stddef.h>
#include <reusables/signal_slot.h>
#include "event_loop_types.h"

/* First queues get prioritized */
extern const main_loop_strategy main_loop_strategy_priority_queue;
/* Queues get approximately the same importance */
extern const main_loop_strategy main_loop_strategy_fare;

void main_loop_init(main_loop_t * obj, main_loop_strategy strategy);
void main_loop_deinit(main_loop_t * obj);

void main_loop_add_queue(main_loop_t * obj, queue_t * queue, int position);
int main_loop_remove_queue(main_loop_t * obj, queue_t * queue);

void main_loop_run(main_loop_t * obj);
void main_loop_quit(main_loop_t * obj);

bool main_loop_ready_to_sleep (main_loop_t * obj);

signal_t * main_loop_sleep_signal (main_loop_t * obj);

#endif /* EVENT_LOOP_MAIN_LOOP_H_ */
