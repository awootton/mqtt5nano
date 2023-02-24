

#include "commonCommands.h"

namespace mqtt5nano {

    EepromItem adminPublicKeyStash(44, "admin key", "0000000000000000000000000000000000000000aaa");
    EepromItem devicePublicKeyStash(44, "token", "0000000000000000000000000000000000000000aaa");
    EepromItem devicePrivateKeyStash(44, "token", "0000000000000000000000000000000000000000aaa");

    adminKeyGet adminKeyGetInstance;
    pubKeyGet pubKeyGetInstance;
    adminKeySet adminKeySetInstance;
    deviceKeySet deviceKeySetInstance;

    mqttStatus mqttStatusCmdInstance;

    void setAdminPassword(slice arg) {
        char adminPublicKey[32];
        char adminPrivateKey[32];
        char tmp[44];
        char *passphrase = arg.getCstr(tmp, sizeof(tmp));

        nanocrypto::getBoxKeyPairFromPassphrase(passphrase, adminPublicKey, adminPrivateKey);

        globalSerial->print("admin pub ");
        globalSerial->print(0xFF & (unsigned int)adminPublicKey[0]);
        globalSerial->print(",");
        globalSerial->print(0xFF & (unsigned int)adminPublicKey[1]);
        globalSerial->print(",");
        globalSerial->print(0xFF & (unsigned int)adminPublicKey[2]);
        globalSerial->print(",");
        globalSerial->print(0xFF & (unsigned int)adminPublicKey[3]);
        globalSerial->println();

        globalSerial->print("admin priv ");
        globalSerial->print(0xFF & (unsigned int)adminPrivateKey[0]);
        globalSerial->print(",");
        globalSerial->print(0xFF & (unsigned int)adminPrivateKey[1]);
        globalSerial->print(",");
        globalSerial->print(0xFF & (unsigned int)adminPrivateKey[2]);
        globalSerial->print(",");
        globalSerial->print(0xFF & (unsigned int)adminPrivateKey[3]);
        globalSerial->println();

        int len = base64::encode(adminPublicKey, 32, tmp, sizeof(tmp));
        adminPublicKeyStash.write(slice(tmp, 0, len));
    }

    void setDevicePassword(slice arg) {
        char thingPublicKey[32];
        char thingPrivateKey[32];
        char tmp[44];
        char *passphrase = arg.getCstr(tmp, sizeof(tmp));

        nanocrypto::getBoxKeyPairFromPassphrase(passphrase, thingPublicKey, thingPrivateKey);

        globalSerial->print("device pub ");
        globalSerial->print(0xFF & (unsigned int)thingPublicKey[0]);
        globalSerial->print(",");
        globalSerial->print(0xFF & (unsigned int)thingPublicKey[1]);
        globalSerial->print(",");
        globalSerial->print(0xFF & (unsigned int)thingPublicKey[2]);
        globalSerial->print(",");
        globalSerial->print(0xFF & (unsigned int)thingPublicKey[3]);
        globalSerial->println();

        globalSerial->print("device priv ");
        globalSerial->print(0xFF & (unsigned int)thingPrivateKey[0]);
        globalSerial->print(",");
        globalSerial->print(0xFF & (unsigned int)thingPrivateKey[1]);
        globalSerial->print(",");
        globalSerial->print(0xFF & (unsigned int)thingPrivateKey[2]);
        globalSerial->print(",");
        globalSerial->print(0xFF & (unsigned int)thingPrivateKey[3]);
        globalSerial->println();

        int len = base64::encode(thingPublicKey, 32, tmp, sizeof(tmp));
        devicePublicKeyStash.write(slice(tmp, 0, len));

        len = base64::encode(thingPrivateKey, 32, tmp, sizeof(tmp));
        devicePrivateKeyStash.write(slice(tmp, 0, len));
    }

    void setShortName(const char *name) {
        hostStash.write(name);
    }

}
