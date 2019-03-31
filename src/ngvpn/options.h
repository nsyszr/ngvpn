#ifndef OPTIONS_H
#define OPTIONS_H

/* Command line options */
struct options
{
    /* option: config */
    const char *config;

#define MODE_SERVER 0
#define MODE_BROKER 1
    /* option: mode */
    int mode;

    /* option: realtime-messages-server-addr */
    const char *realtime_messages_server_addr;
    /* option: management-commands-server-addr */
    const char *management_commands_server_addr;
    /* option: realtime-logs-server-addr */
    const char *realtime_logs_server_addr;

    /* Mode management options */

    /* option: management-socket-file */
    const char *management_socket_file;

    /* Mode logs options */

    /* option: management-socket-file */
    const char *log_file;

    /* Mode broker options */

    /* option: realtime-messages-broker-addr */
    const char *realtime_messages_broker_addr;
    /* option: management-commands-broker-addr */
    const char *management_commands_broker_addr;
    /* option: realtime-logs-broker-addr */
    const char *realtime_logs_broker_addr;
};

void init_options(struct options *o);
void parse_options(struct options *o, int argc, char *argv[]);

#endif /* ifndef OPTIONS_H */
