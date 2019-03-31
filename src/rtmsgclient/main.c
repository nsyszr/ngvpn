#include "zhelpers.h"

int main(int argc, char *argv[])
{
    //  Socket to talk to server
    printf("Monitor OpenVPN real-time messages\n");
    void *context = zmq_ctx_new();
    void *subscriber = zmq_socket(context, ZMQ_SUB);
    int rc = zmq_connect(subscriber, "tcp://localhost:6001");
    assert(rc == 0);

    //  Subscribe to zipcode, default is NYC, 10001
    char *filter = (argc > 1) ? argv[1] : "";
    rc = zmq_setsockopt(subscriber, ZMQ_SUBSCRIBE,
                        filter, strlen(filter));
    assert(rc == 0);

    for (;;)
    {
        char *rt_message = s_recv(subscriber);
        printf("%s\n", rt_message);
        free(rt_message);
    }

    zmq_close(subscriber);
    zmq_ctx_destroy(context);
    return 0;
}
