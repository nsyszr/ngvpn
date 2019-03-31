#include <errno.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/select.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "zhelpers.h"

#define MAX_BUF_SIZE 1024
#define UDS_FILE "/home/tlx3m3j/src/github.com/nsyszr/ngvpn/ovpn_demo/mgmt.sock"

/* 
 * Drain reads from socket untils it's empty. It allocates the given buf pointer
 * and returns the size into given bufsz arg. The usec arg is used for timeout 
 * of the select command. In case of an error drain returns -1 and errno 
 * contains the error code.
 */
static int drain(int fd, uint8_t **bufp, size_t *bufszp, long usec)
{
    fd_set fds_master, fds_tmp;
    struct timeval tv;
    int ready = 0;
    uint8_t buf[MAX_BUF_SIZE];
    uint8_t *tmpbuf = NULL;
    size_t bufsz = MAX_BUF_SIZE;
    ssize_t rv = 0;

    /* Initialize file descriptor set */
    FD_ZERO(&fds_master);
    FD_SET(fd, &fds_master);

    tv.tv_sec = usec / 1000;
    tv.tv_usec = usec % 1000;

    /* Initialize pointer args in case we read nothing we return also nothing */
    (*bufp) = NULL;
    (*bufszp) = 0;

    /* We read until the socket is empty. This happens when select returns 0. */
    for (;;)
    {
        /* TODO: Ask Michl why he's doing that */
        FD_ZERO(&fds_tmp);
        fds_tmp = fds_master;

        /* Let's see if there's something in socket to read */
        ready = select(fd + 1, &fds_tmp, NULL, NULL, &tv);

        if (ready > 0)
        {
            /* Clear buffer with zeros */
            memset(buf, 0, bufsz);
            rv = read(fd, buf, bufsz);

            if (rv > 0)
            {
                /* Allocate tmpbuf with size of existing buf size and reeded 
                 * content. In case we're out of memory we return an error. */
                if ((tmpbuf = malloc((*bufszp) + rv)) == NULL)
                {
                    errno = ENOMEM;
                    return -1;
                }

                /* Check if return buffer contains content */
                if ((*bufp) != NULL)
                {
                    /* Copy existing return buffer content into tmpbuf */
                    bcopy((*bufp), tmpbuf, (*bufszp));
                }

                /* Copy the readed content into tmpbuf at the right position */
                bcopy(buf, tmpbuf + (*bufszp), rv);

                /* Set the return buf size to the new buffer size */
                (*bufszp) = (*bufszp) + rv;

                /* Free the old return buffer */
                free((*bufp));

                /* Point to the new return buffer */
                (*bufp) = tmpbuf;
            }

            /* read returned an error, we exit with error too */
            if (rv == -1)
            {
                return -1;
            }
        }
        else
        {
            /* select returned 0 (no ready fds => nothing to read) or error */
            return ready;
        }
    }
}

static char *mc_rtmsg(const char *prefix, const char *msg)
{
    size_t needed = snprintf(NULL, 0, "%s: %s", prefix, msg + 1) + 1;
    char *buf = malloc(needed);
    sprintf(buf, "%s: %s", prefix, msg + 1);

    return buf;
}

static char *build_command(const char *cmd)
{
    size_t needed = snprintf(NULL, 0, "%s\n", cmd) + 1;
    char *buf = malloc(needed);
    sprintf(buf, "%s\n", cmd);
    return buf;
}

static int publish_rt_messages(void *publ, const uint8_t *buf, size_t bufsz)
{
    char delim[] = "\r\n";
    char rtmsg_prefix[] = ">";
    char *msgs = NULL;
    char *ptr = NULL;
    int rc = 0;

    /* Duplicate given buf into proper char pointer for the strtok operation. */
    if ((msgs = strndup(buf, bufsz)) == NULL)
    {
        return ENOMEM;
    }

    /* Split the msgs by CR/LF */
    ptr = strtok(msgs, delim);

    /* We received a single line event with no newline */
    if (ptr == NULL && strstr(ptr, rtmsg_prefix) == ptr)
    {
        char *msg = mc_rtmsg("1194", ptr);
        int rc = s_send(publ, msg);
        printf("Published: %d bytes [%s]\n", rc, msg);
        free(msg);
    }

    /* Iterate thru all lines */
    while (ptr != NULL)
    {
        if (strstr(ptr, rtmsg_prefix) == ptr)
        {
            /* Publish RT messages */
            /* TODO: Copy messages into a buffer and process the messages after the loop */
            /* printf("RTMSG: %s\n", ptr); */
            char *msg = mc_rtmsg("1194", ptr);
            rc = s_send(publ, msg);
            printf("Published: %d bytes [%s]\n", rc, msg);
            free(msg);
        }

        ptr = strtok(NULL, delim);
    }

    /* Free the msgs pointer allocated by the strdup operation. */
    free(msgs);

    return 0;
}

static char *strip_rt_messages(const uint8_t *buf, size_t bufsz)
{
    char delim[] = "\r\n", rtmsg_prefix[] = ">";
    char *msg = NULL, *ptr = NULL, *result = NULL, *tmpresult = NULL;
    int rc = 0;
    size_t resultsz = 0;

    /* Duplicate given buf into proper char pointer for the strtok operation. */
    if ((msg = strndup(buf, bufsz)) == NULL)
    {
        errno = ENOMEM;
        return NULL;
    }

    /* Split the msgs by CR/LF */
    ptr = strtok(msg, delim);

    /* Check if we received a single line message without CR/LF thats not RT. */
    if (ptr == NULL && strstr(ptr, rtmsg_prefix) != ptr)
    {
        /* We can savely return msgs because return value has to be freed by 
           the caller. */
        // printf("Stripped single line: [%s]\n", msg);
        return msg;
    }

    /* Iterate thru all lines */
    while (ptr != NULL)
    {
        if (strstr(ptr, rtmsg_prefix) != ptr)
        {
            // printf("Process stripped result: [%s]\n", ptr);
            tmpresult = calloc(sizeof(char), strlen(ptr) + resultsz + 2);
            if (result != NULL)
            {
                bcopy(result, tmpresult, resultsz);
                // printf("Previous result: [%s]\n", result);
                free(result);
            }
            bcopy(ptr, tmpresult + resultsz, strlen(ptr));
            // printf("Current tmpresult: [%s]\n", tmpresult);
            resultsz = strlen(ptr) + resultsz + 1;
            // printf("New resultsz: [%ld]\n", resultsz);
            tmpresult[resultsz - 1] = '\n';
            result = tmpresult;
            // printf("Current stripped result: [%s]\n", result);
        }

        ptr = strtok(NULL, delim);
    }

    /* Free the msgs pointer allocated by the strdup operation. */
    free(msg);

    // printf("Stripped result: [%s]\n", result);
    return result;
}

int main()
{
    void *ctx = NULL, *publ = NULL, *resp = NULL;
    int sock;
    struct sockaddr_un address;
    uint8_t *buf = NULL;
    size_t bufsz = 0;
    int rc;
    bool await_cmd_resp = false;
    char *cmd_resp = NULL;

    /*  Prepare our ZMQ context */
    ctx = zmq_ctx_new();

    /* Prepare our publisher for real time messages */
    publ = zmq_socket(ctx, ZMQ_PUB);
    rc = zmq_bind(publ, "tcp://*:6001");
    assert(rc == 0);

    /*  Socket to reply commands send by clients */
    resp = zmq_socket(ctx, ZMQ_REP);
    rc = zmq_bind(resp, "tcp://*:6003");
    assert(rc == 0);

    /* Wait a little bit to ensure the publ socket is up */
    sleep(1);

    /* Create our connection to the OpenVPN management socket */
    if ((sock = socket(PF_LOCAL, SOCK_STREAM, 0)) == -1)
    {
        fprintf(stderr, "error creating socket: %d", errno);
        return EXIT_FAILURE;
    }

    address.sun_family = AF_LOCAL;
    strcpy(address.sun_path, UDS_FILE);
    if (connect(sock, (struct sockaddr *)&address, sizeof(address)) == -1)
    {
        fprintf(stderr, "error connecting socket: %d", errno);
        return EXIT_FAILURE;
    }

    /* Write command bytecount to socket */
    write(sock, "bytecount 1\n", 12);

    /* Enter our main loop */
    for (;;)
    {
        zmq_pollitem_t pollitems[] = {
            {resp, 0, ZMQ_POLLIN, 0}};

        /* Read management socket until it's empty */
        rc = drain(sock, &buf, &bufsz, 500);
        if (rc == -1)
        {
            fprintf(stderr, "error draining the socket: %d", errno);
            return EXIT_FAILURE;
        }

        /* Process messages from management socket */
        if (bufsz > 0)
        {
            publish_rt_messages(publ, buf, bufsz);

            if (await_cmd_resp)
            {
                cmd_resp = strip_rt_messages(buf, bufsz);
                printf("cmd_resp:\n\n%s\n\n", cmd_resp);

                char delim[] = "\n";
                // char *s = strdup(cmd_resp);
                char *ptr = strtok(cmd_resp, delim);
                while (ptr != NULL)
                {
                    rc = s_sendmore(resp, ptr);
                    printf("Replied: %d bytes [%s]\n", rc, ptr);

                    ptr = strtok(NULL, delim);
                }
                // free(s);

                rc = s_send(resp, "");
                printf("Replied end frame: %d bytes\n", rc);

                await_cmd_resp = false;
                free(cmd_resp);
            }

            /* drain allocates the buffer, free the reserved memory after 
               processing it! */
            free(buf);
            buf = NULL;
            bufsz = 0;
        }

        /* Poll for a command if previous command is not awaiting a response */
        zmq_poll(pollitems, 1, 0);
        if ((pollitems[0].revents & ZMQ_POLLIN) && !await_cmd_resp)
        {
            char *s = s_recv(resp);
            char *cmd = build_command(s);

            printf("Received: [%s]\n", s);
            write(sock, cmd, strlen(cmd) + 1);
            await_cmd_resp = true;
            /* zmq_send(resp, "OK", 2, 0); */

            free(cmd);
            free(s);
        }
    }

    zmq_close(resp);
    zmq_close(publ);
    zmq_ctx_destroy(ctx);

    return EXIT_SUCCESS;
}
