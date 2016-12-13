#include <CppUTest/TestHarness.h>
#include <CppUTestExt/MockSupport.h>

#include <reusables/linked_list.hpp>

extern "C"
{
    #include <event_loop/main_loop.h>
    #include <event_loop/queue.h>
    #include <event_loop/signal_slot_queue.h>
}

TEST_GROUP(MainLoop)
{
    main_loop_t cut;

    void setup ()
    {
    }

    void teardown ()
    {
        mock().clear();
    }
};

TEST(MainLoop, queue_order)
{
    queue_t queue0;
    queue_t queue1;
    queue_t queue2;
    queue_t queue3;
    queue_t queue4;
    queue_t queue5;

    main_loop_init(&cut, main_loop_strategy_fare);
    cut.looping = false;

    queue_init(&queue0, 2);
    queue_init(&queue1, 2);
    queue_init(&queue2, 2);
    queue_init(&queue3, 2);
    queue_init(&queue4, 2);
    queue_init(&queue5, 2);

    main_loop_add_queue(&cut, &queue4, 5);
    main_loop_add_queue(&cut, &queue0, 0);
    main_loop_add_queue(&cut, &queue1, 1);
    main_loop_add_queue(&cut, &queue3, 2);
    main_loop_add_queue(&cut, &queue2, 2);
    main_loop_add_queue(&cut, &queue5, 121);

    CHECK_EQUAL(&queue0, cut.root);
    CHECK_EQUAL(&queue1, linked_list_next(cut.root, ll));
    CHECK_EQUAL(&queue2, linked_list_next(&queue1, ll));
    CHECK_EQUAL(&queue3, linked_list_next(&queue2, ll));
    CHECK_EQUAL(&queue4, linked_list_next(&queue3, ll));
    CHECK_EQUAL(&queue5, linked_list_last(cut.root, ll));

    main_loop_deinit(&cut);
}

int slot_handler_sleep (slot_t * s)
{
    mock().actualCall("slot_handler_sleep")
        .withPointerParameter("slot", s);

    return mock().returnValue().getIntValue();
}

TEST(MainLoop, sleep)
{
    queue_t queue;
    slot_t sleep;
    int b[2];

    main_loop_init(&cut, main_loop_strategy_priority_queue);
    cut.looping = false;

    slot_init(&sleep, slot_handler_sleep);
    slot_connect(&sleep, main_loop_sleep_signal(&cut));

    queue_init(&queue, 2);
    main_loop_add_queue(&cut, &queue, 0);
    queue_push(&queue, b, 10);
    CHECK_EQUAL(10, b[0]); /* This check is to deal with warning variable unused */

    CHECK_FALSE(main_loop_ready_to_sleep(&cut));
    main_loop_run(&cut);
    mock().checkExpectations();

    mock().expectOneCall("slot_handler_sleep")
            .withParameter("slot", &sleep);

    CHECK_TRUE(main_loop_ready_to_sleep(&cut));
    main_loop_run(&cut);
    mock().checkExpectations();

    main_loop_deinit(&cut);
}

TEST(MainLoop, queue_deinit)
{
    queue_t queue1;
    queue_t queue2;

    main_loop_init(&cut, main_loop_strategy_fare);
    cut.looping = false;

    queue_init(&queue1, 2);
    queue_init(&queue2, 2);

    main_loop_add_queue(&cut, &queue1, 0);
    main_loop_add_queue(&cut, &queue2, 0);

    main_loop_deinit(&cut);
    CHECK_EQUAL(1, linked_list_count(&queue1, ll))
    CHECK_EQUAL(1, linked_list_count(&queue2, ll))
    CHECK_EQUAL(0, queue1.loop);
    CHECK_EQUAL(0, queue2.loop);
}

TEST(MainLoop, bad_disconnect)
{
    mock().expectOneCall("_log_file_line")
            .withParameter("msg", "Error: Provided queue hasn't been added to the main_loop, cannot remove it.")
            .ignoreOtherParameters();

    queue_t queue;

    main_loop_init(&cut, main_loop_strategy_fare);
    cut.looping = false;

    queue_init(&queue, 2);
    main_loop_remove_queue(&cut, &queue);
}

static int event_buf[4];

int slot_handler (queue_t * queue)
{
    int d;
    queue_peek(queue, event_buf, &d);

    mock().actualCall("slot_handler")
        .withPointerParameter("queue", queue)
        .withIntParameter("event", d);

    return mock().returnValue().getIntValue();
}

TEST(MainLoop, process_events_priority)
{
    slot_queue_t slot;
    queue_t queue;
    main_loop_t loop;

    slot_queue_init(&slot, slot_handler);
    queue_init(&queue, 4);
    main_loop_init(&loop, main_loop_strategy_priority_queue);
    loop.looping = false;

    main_loop_add_queue(&loop, &queue, 0);
    slot_queue_connect(&slot, queue_signal(&queue));

    queue_push(&queue, event_buf, 10);
    queue_push(&queue, event_buf, 24);
    queue_push(&queue, event_buf, 32);

    mock().strictOrder();

    mock().expectOneCall("slot_handler")
            .withParameter("queue", &queue)
            .withParameter("event", 10)
            .andReturnValue(0);

    main_loop_run(&loop);
    mock().checkExpectations();

    mock().expectOneCall("slot_handler")
            .withParameter("queue", &queue)
            .withParameter("event", 24)
            .andReturnValue(0);

    main_loop_run(&loop);
    mock().checkExpectations();

    mock().expectOneCall("slot_handler")
            .withParameter("queue", &queue)
            .withParameter("event", 32)
            .andReturnValue(0);

    main_loop_run(&loop);
    mock().checkExpectations();
}

TEST(MainLoop, process_events_strategy_fare)
{
    slot_queue_t slot;
    queue_t queue;
    main_loop_t loop;

    slot_queue_init(&slot, slot_handler);
    queue_init(&queue, 4);
    main_loop_init(&loop, main_loop_strategy_fare);
    loop.looping = false;

    main_loop_add_queue(&loop, &queue, 0);
    slot_queue_connect(&slot, queue_signal(&queue));

    queue_push(&queue, event_buf, 10);
    queue_push(&queue, event_buf, 24);
    queue_push(&queue, event_buf, 32);

    mock().strictOrder();

    mock().expectOneCall("slot_handler")
            .withParameter("queue", &queue)
            .withParameter("event", 10)
            .andReturnValue(0);

    mock().expectOneCall("slot_handler")
            .withParameter("queue", &queue)
            .withParameter("event", 24)
            .andReturnValue(0);

    mock().expectOneCall("slot_handler")
            .withParameter("queue", &queue)
            .withParameter("event", 32)
            .andReturnValue(0);

    main_loop_run(&loop);
}

static int event_buf1[4];
static int event_buf2[4];

int slot_handler_1 (queue_t * queue)
{
    int d;
    queue_peek(queue, event_buf1, &d);

    mock().actualCall("slot_handler_1")
        .withPointerParameter("queue", queue)
        .withIntParameter("event", d);

    return mock().returnValue().getIntValue();
}

int slot_handler_2 (queue_t * queue)
{
    int d;
    queue_peek(queue, event_buf2, &d);

    mock().actualCall("slot_handler_2")
        .withPointerParameter("queue", queue)
        .withIntParameter("event", d);

    return mock().returnValue().getIntValue();
}

TEST(MainLoop, process_order)
{
    slot_queue_t slot1, slot2;
    queue_t queue1, queue2;
    main_loop_t loop;

    slot_queue_init(&slot1, slot_handler_1);
    slot_queue_init(&slot2, slot_handler_2);
    queue_init(&queue1, 4);
    queue_init(&queue2, 4);
    main_loop_init(&loop, main_loop_strategy_fare);
    loop.looping = false;

    main_loop_add_queue(&loop, &queue1, 0);
    main_loop_add_queue(&loop, &queue2, 1);
    slot_queue_connect(&slot1, queue_signal(&queue1));
    slot_queue_connect(&slot2, queue_signal(&queue2));

    queue_push(&queue1, event_buf1, 10);
    queue_push(&queue2, event_buf2, 24);
    queue_push(&queue1, event_buf1, 32);
    queue_push(&queue2, event_buf2, 64);

    mock().strictOrder();

    mock().expectOneCall("slot_handler_1")
            .withParameter("queue", &queue1)
            .withParameter("event", 10)
            .andReturnValue(0);

    mock().expectOneCall("slot_handler_1")
            .withParameter("queue", &queue1)
            .withParameter("event", 32)
            .andReturnValue(0);

    mock().expectOneCall("slot_handler_2")
            .withParameter("queue", &queue2)
            .withParameter("event", 24)
            .andReturnValue(0);

    mock().expectOneCall("slot_handler_2")
            .withParameter("queue", &queue2)
            .withParameter("event", 64)
            .andReturnValue(0);

    main_loop_run(&loop);
}
