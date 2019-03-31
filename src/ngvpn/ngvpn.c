#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "broker.h"
#include "options.h"
#include "server.h"

int main(int argc, char *argv[])
{
    struct options o;

    init_options(&o);
    parse_options(&o, argc, argv);

    switch (o.mode)
    {
    case MODE_SERVER:
        return server_main(&o);
    case MODE_BROKER:
        return server_main(&o);
    }
}
