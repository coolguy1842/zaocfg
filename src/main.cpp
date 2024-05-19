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
        printf("Battery: %d%%\n", mouse->getBatteryPercentage());    
    }

    int profile;
    cmdl({ "-p", "--set-profile" }, -1) >> profile;
    if(profile != -1) {
        int profileCount = mouse->getProfileCount();

        if(profile > profileCount - 1) {
            fprintf(stderr, "ERROR: Profile not in range of 0-%d.", profileCount - 1);
            return -1;
        }
        
        if(mouse->setDPIProfile(profile) == -1) {
            wprintf(L"%S\n", mouse->getError());
            return -1;
        }

        printf("Set profile to %u\n", profile);
    }

    int profiles;
    cmdl({ "-pc", "--set-profile-count" }, -1) >> profiles;
    if(profiles != -1) {
        if(profiles < 1 || profiles > 6) {
            fprintf(stderr, "ERROR: Profile count not in range of 1-6.");
            return -1;
        }

        if(mouse->setDPIProfilesCount(profiles) == -1) {
            wprintf(L"%S\n", mouse->getError());
            return -1;
        }

        printf("Set profile count to %u\n", profiles);
    }
    

    delete mouse;
    return 0;
}
