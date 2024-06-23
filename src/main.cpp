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

struct BatteryData {
    unsigned char overflow;
    unsigned char charging;
    unsigned short voltage;
};

// for testing
int dummyGetPercentage(BatteryData data) {
    uint16_t combinedVoltage = data.voltage;
    
    // Voltage thresholds and corresponding percentages
    uint16_t voltageThresholds[] = { 
        0x0C7A, 0x0D53, 0x0D6C, 0x0D9A, 0x0DE1, 0x0E28, 0x0E39, 0x0E5F, 
        0x0EB6, 0x0F12, 0x0F1F, 0x0F34, 0x0F45, 0x0F5E, 0x0FD3, 0x0FD7, 
        0x0FDF, 0x0FE3, 0x0FE8, 0x0FEC, 0x1072
    };
    int percentageThresholds[] = { 
        0, 4, 6, 10, 16, 21, 23, 26, 
        35, 46, 48, 51, 53, 57, 87, 
        88, 90, 92, 93, 94, 100 
    };

    // Determine battery percentage using linear interpolation
    int batteryPercentage = 0;
    size_t i;

    // Find the appropriate range for interpolation
    for (i = 1; i < sizeof(voltageThresholds) / sizeof(voltageThresholds[0]); ++i) {
        if (combinedVoltage <= voltageThresholds[i]) {
            // Linear interpolation between percentageThresholds[i-1] and percentageThresholds[i]
            int lowerPercentage = percentageThresholds[i - 1];
            int upperPercentage = percentageThresholds[i];
            uint16_t lowerVoltage = voltageThresholds[i - 1];
            uint16_t upperVoltage = voltageThresholds[i];
            
            // Calculate interpolated battery percentage
            batteryPercentage = lowerPercentage + 
                                ((upperPercentage - lowerPercentage) * 
                                (combinedVoltage - lowerVoltage)) / 
                                (upperVoltage - lowerVoltage);
            break;
        }
    }
    
    // Edge case for very low voltages
    if (i == sizeof(voltageThresholds) / sizeof(voltageThresholds[0])) {
        batteryPercentage = percentageThresholds[sizeof(voltageThresholds) / sizeof(voltageThresholds[0]) - 1];
    }
    
    return batteryPercentage;
}

// for testing
void testPercentages() {
    std::pair<BatteryData, int> batteryTests[] = {
        { { 0x64, 0x00, 0x1072 }, 100 },
        { { 0x5A, 0x00, 0x0FEC }, 94 },
        { { 0x5A, 0x00, 0x0FE8 }, 93 },
        { { 0x5A, 0x00, 0x0FE3 }, 92 },
        { { 0x5A, 0x00, 0x0FDF }, 90 },
        { { 0x55, 0x00, 0x0FD7 }, 88 },
        { { 0x55, 0x00, 0x0FD3 }, 87 },
        { { 0x37, 0x00, 0x0F5E }, 57 },
        { { 0x37, 0x00, 0x0F45 }, 53 },
        { { 0x32, 0x00, 0x0F34 }, 51 },
        { { 0x32, 0x00, 0x0F1F }, 48 },
        { { 0x32, 0x00, 0x0F12 }, 46 },
        { { 0x23, 0x00, 0x0EB6 }, 35 },
        { { 0x1E, 0x00, 0x0E5F }, 26 },
        { { 0x19, 0x00, 0x0E39 }, 23 },
        { { 0x19, 0x00, 0x0E28 }, 21 },
        { { 0x0F, 0x00, 0x0DE1 }, 16 },
        { { 0x0A, 0x00, 0x0D9A }, 10 },
        { { 0x05, 0x00, 0x0D6C }, 6 },
        { { 0x05, 0x00, 0x0D53 }, 4 },
        { { 0x05, 0x00, 0x0C7A }, 0 },
        
        //  08 04 00 00 00 02 19 00 0E 35 00 00 00 00 00 00   .........5......
 // EB                                                ë

        // out of dataset
        { { 0x19, 0x00, 0x0E35 }, 22 },
    };

    for(auto pair : batteryTests) {
        int percentage = dummyGetPercentage(pair.first);
        if(percentage != pair.second) {
            printf("expected %d%% but got %d%% from voltage 0x%04X\n", pair.second, percentage, pair.first.voltage);
        }
        else {
            printf("voltage 0x%04X success at %d%%\n", pair.first.voltage, pair.second);
        }
    }
}


int main(int argc, char** argv) {
    // for testing
    // testPercentages();
    // return 0;

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
