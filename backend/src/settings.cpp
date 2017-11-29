#include "settings.hpp"
#include "validate.hpp"

#include <cstdio>
#include <algorithm>
#include <iostream>

using namespace std;
using namespace Ats;

Initial::Initial(int argc, char** argv) {
    /* Options:
     * -h help
     * -m msgtype
     * -o output uri
     */
    
    if (char* lvl = getenv("LOGLEVEL")) {
        switch (lvl[0]) {
        case 'E': {
            log_level = Logger::Error;
            break;
        }
        case 'I': {
            log_level = Logger::Info;
            break;
        }
        case 'D': {
            log_level = Logger::Debug;
            break;
        }
        default:
            log_level = Logger::None;
            break;
        }
    } else {
        log_level = Logger::Error;
    }
    
    if (argc < 2) throw Wrong_option("Too few arguments");
    for (int i = 1; i < argc; i++) {
        if (argv[i][0] != '-') {
            string uri = argv[i];
            if (! validate_uri(uri)) throw Wrong_option("Bad uri: " + uri);
            uris.push_back(uri);
        } else {
            switch (argv[i][1]) {
            case 'h': {
                throw Wrong_option();
                break;
            }
            case 'o': {
                if (i == (argc - 1)) throw Wrong_option("-o requires argument");
                string uri = argv[++i];
                if (! validate_uri(uri)) throw Wrong_option("Bad output uri: " + uri);
                multicast_address = uri;
                break;
            }
            case 'm': {
                if (i == (argc - 1)) throw Wrong_option("-m requires argument");
                string msgt = argv[++i];
                if (msgt == "debug") {
                    msg_type = Msg_type::Debug;
                } else if (msgt == "json") {
                    msg_type = Msg_type::Json;
                } else if (msgt == "msgpack") {
                    msg_type = Msg_type::Msgpack;
                } else {
                    throw Wrong_option("Bad message type: " + msgt);
                }
                break;
            }
            default:
                throw Wrong_option("Unknown option: " + string(argv[i]));
                break;
            }
        }
    }
}

string
Initial::usage (string prog_name) {
    string rval = "Usage:\n";
    rval += prog_name;
    rval += " [-opt arg] uri1 [uri2 uri3]\n"
        "Options:\n"
        "\t-o\toutput uri\n"
        "\t-m\tipc message type [json | msgpack | debug]\n"
        "\t-h\thelp\n"
        "Additional:\n"
        "\turi format: udp://[ip] or udp://[ip]:[port]\n";
    return rval;
}

/* ---------- Settings -------------------- */

void
Settings_facade::init(Initial& i) {
    if(i.multicast_address) {
        return;
    }
}
