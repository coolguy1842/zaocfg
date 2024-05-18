#include <stdio.h>

#include "Z2.hpp"
#include "argh.h"

static const char USAGE[] =
R"(ZaoCFG

    Usage:
      zaocfg --battery
      zaocfg -p=2

    Options:
      -h --help     Show this screen.
      -b --battery     Show the battery percentage.
      -p --set-profile  Sets the profile to the specified profile. (0-5)
)";


int main(int argc, char** argv) {
    Z2* mouse = findZ2();
    if(mouse == nullptr) {
        fprintf(stderr, "ERROR: Z2 not found.");
        return -1;
    }

    argh::parser cmdl(argv);
    if(cmdl[{ "-h", "--help" }]) {
        printf("%s\n", USAGE);
        return 0;
    }

    if(cmdl[{ "-b", "--battery" }]) {
        printf("Battery: %d% \n", mouse->getBatteryPercentage());    
    }

    unsigned int profile;
    cmdl({ "-p", "--set-profile" }, -1u) >> profile;
    if(profile != -1u) {
        if(profile > 5) {
            fprintf(stderr, "ERROR: Profile not in range of 0-5.");
            return -1;
        }

        mouse->setDPIProfile(profile);
    }
    

    delete mouse;
    return 0;
}
