#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "options.h"

const char title_string[] = "INSYS icom NG-VPN controller";

static const char usage_message[] =
    "%s\n"
    "\n"
    "General options:\n"
    "--config file   : Read configuration options from file.\n"
    "--help          : Show options.\n"
    "--version       : Show copyright and version information.\n"
    "--mode m        : Server mode, m = 'server' or 'broker'.\n"
    "\n"
    "Management server options (when --mode server is used):\n"
    "--management-socket-file file : Path to OpenVPN management interface unix\n"
    "                  domain socket file.\n"
    "--realtime-messages-server-addr [addr] : Server bind address for subscribing\n"
    "                  the realtime messages (ZeroMQ Publish-Subscribe)\n"
    "                  (default=%s).\n"
    "--management-commands-server-addr [addr] : Server bind address for executing\n"
    "                  management interface commands (ZeroMQ Request-Reply)\n"
    "                  (default=%s).\n"
    "--realtime-logs-server-addr [addr] : Server bind address for subscribing\n"
    "                  the realtime logs (ZeroMQ Publish-Subscribe)\n"
    "                  (default=%s).\n"
    "\n"
    "Management proxy options (when --mode broker is used):\n"
    "--realtime-messages-server-addr addr : Server address for subscribing the\n"
    "                  realtime messages (ZeroMQ Publish-Subscribe).\n"
    "--management-commands-server-addr addr : Server address for executing\n"
    "                  management interface commands (ZeroMQ Request-Reply).\n"
    "--realtime-logs-server-addr addr : Server address for subscribing the realtime\n"
    "                  logs (ZeroMQ Publish-Subscribe).\n"
    "--realtime-messages-broker-addr : Server proxy bind address for subscribing\n"
    "                  the realtime messages (ZeroMQ Publish-Subscribe)\n"
    "                  (default=%s).\n"
    "--management-commands-broker-addr [addr] : Server proxy bind ddress for\n"
    "                  executing management interface commands (ZeroMQ Request\n"
    "                  -Reply) (default=%s).\n"
    "--realtime-logs-broker-addr [addr] : Server proxy bind address for subscribing\n"
    "                  the realtime logs (ZeroMQ Publish-Subscribe)\n"
    "                  (default=%s).\n";

/*
 * Show CLI usage
 */
static void usage(void)
{
    struct options o;
    init_options(&o);

    printf(usage_message,
           title_string,
           o.realtime_messages_server_addr,
           o.management_commands_server_addr,
           o.realtime_logs_server_addr,
           o.realtime_messages_broker_addr,
           o.management_commands_broker_addr,
           o.realtime_logs_broker_addr);

    exit(1);
}

/*
 * Set the default options.
 */
void init_options(struct options *o)
{
    memset(o, 0, sizeof(struct options));

    o->realtime_messages_server_addr = "tcp://*:6001";
    o->management_commands_server_addr = "tcp://*:6002";
    o->realtime_logs_server_addr = "tcp://*:6003";

    o->realtime_messages_broker_addr = "tcp://*:6011";
    o->management_commands_broker_addr = "tcp://*:6012";
    o->realtime_logs_broker_addr = "tcp://*:6013";
}

void parse_options(struct options *o, int argc, char *argv[])
{
    int opt = 0, long_index = 0;

    static struct option long_options[] = {
        {"config", required_argument, 0, 'a'},
        {"mode", required_argument, 0, 'b'},
        {"realtime-messages-server-addr", required_argument, 0, 'c'},
        {"management-commands-server-addr", required_argument, 0, 'd'},
        {"realtime-logs-server-addr", required_argument, 0, 'e'},
        {"realtime-messages-broker-addr", required_argument, 0, 'f'},
        {"management-commands-broker-addr", required_argument, 0, 'g'},
        {"realtime-logs-broker-addr", required_argument, 0, 'h'},
        {"management-socket-file", required_argument, 0, 'i'},
        {"version", no_argument, 0, 'v'},
        {"help", no_argument, 0, 'w'},
        {0, 0, 0, 0}};

    while ((opt = getopt_long_only(argc, argv, "",
                                   long_options, &long_index)) != -1)
    {
        switch (opt)
        {
        case 'b':
        {
            // TODO: find a better way for parsing string options and converting
            //       to a number.
            if (strcmp(optarg, "server") != 0 && strcmp(optarg, "broker") != 0)
            {
                fprintf(stderr, "Unknown mode: %s\n", optarg);
                usage();
            }

            if (strcmp(optarg, "server") == 0)
            {
                o->mode = MODE_SERVER;
            }

            if (strcmp(optarg, "broker") == 0)
            {
                o->mode = MODE_BROKER;
            }
        }
        break;
        case 'c':
            printf("set o->realtime_messages_server_addr=%s\n", optarg);
            o->realtime_messages_server_addr = optarg;
            break;
        case 'd':
            o->management_commands_server_addr = optarg;
            break;
        case 'e':
            o->realtime_logs_server_addr = optarg;
            break;
        case 'f':
            o->realtime_messages_broker_addr = optarg;
            break;
        case 'g':
            o->management_commands_broker_addr = optarg;
            break;
        case 'h':
            o->realtime_logs_broker_addr = optarg;
            break;
        case 'i':
            o->management_socket_file = optarg;
            break;
        case 'v':
        case 'w':
            usage();
            break;
        default:
            usage();
        }
    }

    if (o->mode == MODE_SERVER && (o->management_socket_file == NULL || o->management_socket_file == ""))
    {
        fprintf(stderr, "Option management-socket-file is required in mode server.\n\n");
        usage();
    }

    printf("mode=%d\n", o->mode);
    printf("management-socket-file=%s\n", o->management_socket_file);
    printf("realtime-messages-server-addr=%s\n", o->realtime_messages_server_addr);
    printf("management-commands-server-addr=%s\n", o->management_commands_server_addr);
    printf("realtime-logs-server-addr=%s\n", o->realtime_logs_server_addr);
}
