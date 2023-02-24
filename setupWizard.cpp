
#include "setupWizard.h"

namespace mqtt5nano {

    WizardScreen *current[8];
    int currentLevel = 0;

    WizardScreen *makeGetWifi() {
        return new getWifi();
    }

    WizardScreen *makeGetPass() {
        return new getPass();
    }

    WizardScreen *makeGetShortName() {
        return new getShortName();
    }

    void pushWizard(WizardScreen *a) {
        current[currentLevel++] = a;
        if (currentLevel == 8) {
            for (int i = 0; i < 7; i++) {
                current[i] = current[i + 1];
            }
            currentLevel--;
        }
    }

    void clearScreen(Destination &out) {
        for (int i = 0; i < 10; i++) {

            out.write(" \n");
            out.write("\n");
        }
    }

    void makeMessage(Destination &out) {
        if (currentLevel > 0) {
            current[currentLevel - 1]->makeMessage(out);
        } else {
            // running = false;
            return;
        }
    }

    void execute(int picked, const char *typed) {

        if (currentLevel > 0) {
            current[currentLevel - 1]->execute(picked, typed);
        }
    }

    int wizardStackSize() {

        return currentLevel;
    }

}
