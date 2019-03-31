#include "zhelpers.h"

int main(int argc, char *argv[])
{
    void *context = zmq_ctx_new();
    char *cmd = NULL;

    cmd = calloc(sizeof(char), 1024);

    if (argc <= 1)
    {
        printf("No command given. Quit.\n");
        return 1;
    }

    for (int i = 1; i < argc; i++)
    {
        cmd = strcat(cmd, argv[i]);
        cmd = strcat(cmd, " ");
    }

    printf("Command: [%s]\n", cmd);

    //  Socket to talk to server
    void *requester = zmq_socket(context, ZMQ_REQ);
    zmq_connect(requester, "tcp://localhost:6002");

    s_send(requester, cmd);

    /*char *string = s_recv(requester);
    printf("Received reply:\n%s\n\n", string);
    free(string);*/

    while (1)
    {
        zmq_msg_t message;
        zmq_msg_init(&message);
        zmq_msg_recv(&message, requester, 0);

        //  Process the message frame
        char *msg = strndup(zmq_msg_data(&message), zmq_msg_size(&message));
        printf("%s\n", msg);
        free(msg);

        zmq_msg_close(&message);
        if (!zmq_msg_more(&message))
            break; //  Last message frame
    }

    zmq_close(requester);
    zmq_ctx_destroy(context);
    return 0;
}