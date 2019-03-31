#include "broker.h"
#include "zhelpers.h"

int broker_main(const struct options *o)
{
    void *context = zmq_ctx_new();

    /*  This is where the mgmt interface controller sits */
    void *frontend = zmq_socket(context, ZMQ_XSUB);
    zmq_connect(frontend, o->realtime_messages_server_addr);

    /* This is our public endpoint for real time message subscribers */
    void *backend = zmq_socket(context, ZMQ_XPUB);
    zmq_bind(backend, o->realtime_messages_broker_addr);

    /*  Run the proxy until the user interrupts us */
    zmq_proxy(frontend, backend, NULL);

    zmq_close(frontend);
    zmq_close(backend);
    zmq_ctx_destroy(context);

    return 0;
}
