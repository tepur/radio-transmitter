#include <iostream>
#include <unistd.h>

#include "transmitter.h"

int main(int argc, char* argv[]) {
    Transmitter transmitter;

    int i;
    while((i = getopt(argc, argv, "a:P:C:p:f:R:n:")) != -1) {
        switch(i) {
            case 'a':
                transmitter.set_mcast_addr(optarg);
                break;
            case 'P':
                transmitter.set_data_port(optarg);
                break;
            case 'C':
                transmitter.set_ctrl_port(optarg);
                break;
            case 'p':
                transmitter.set_psize(optarg);
                break;
            case 'f':
                transmitter.set_fsize(optarg);
                break;
            case 'R':
                transmitter.set_rtime(optarg);
                break;
            case 'n':
                transmitter.set_name(optarg);
                break;
            default:
                std::cerr << "Usage: " << argv[0] << " -a mcast_addr [-P data_port] [-C ctrl_port] [-p psize] "
                        "[-f fsize] [-R rtime] [-n name]" << std::endl;
                return 1;
        }
    }
    if(transmitter.mcast_addr_not_set()) {
        std::cerr << "MCAST_ADDR has not been set" << std::endl;
        return 1;
    }

    transmitter.run();

    return 0;
}