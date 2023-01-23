#include <iostream>

#include <string.h>

#include "nanobase64.h"
#include "slices.h"

#include "nanoCrypto.h"

using namespace std;
using namespace nanocrypto;

int main()
{
    std::cout << "Hello World" << std::endl;

    // functest();

    char hashdest[32];
    const char *source = "testString123";

    Sha256FromString(hashdest, source);

    char base64[256];
    //    int encode(const unsigned char *src, int srcLen, char *dest, int destMax)
    int n = base64::encode((const unsigned char *)&hashdest, 32, base64, 256);
    base64[n] = 0;

    std::cout << source << " hashes to " << base64 << "\n";
    // testString123hashes to VY5e4pCAwDlr-HdfioX6TCiv41Xx_SsTtUcupKndFpQ
    // needs: in Go
    //	testString123 hashes to  VY5e4pCAwDlr-HdfioX6TCiv41Xx_SsTtUcupKndFpQ
    if (0 != strcmp(base64, "VY5e4pCAwDlr-HdfioX6TCiv41Xx_SsTtUcupKndFpQ"))
    {
        cout << "FAIL Sha256 is wrong";
    }

    char cdest[32];
    char privkey[32];
    char pubkey[32];

    pubkey[0] = 0x10;

    memcpy(privkey, hashdest, 32);
    Curve25519BaseMpy(pubkey, privkey);

    n = base64::encode((const unsigned char *)&privkey, 32, base64, 256);
    base64[n] = 0;

    std::cout << source << " privkey " << base64 << "\n";

    n = base64::encode((const unsigned char *)&pubkey, 32, base64, 256);
    base64[n] = 0;

    std::cout << source << " pubkey " << base64 << "\n";

    // got testString123pubkey bht-Ka3j7GKuMFOablMlQnABnBvBeugvSf4CdFV3LXs
    // need  bht-Ka3j7GKuMFOablMlQnABnBvBeugvSf4CdFV3LXs`
    if (0 != strcmp(base64, "bht-Ka3j7GKuMFOablMlQnABnBvBeugvSf4CdFV3LXs"))
    {
        cout << "FAIL Curve25519BaseMpy is wrong";
    }

    // then the pass to keys thing

    getBoxKeyPairFromPassphrase(source, pubkey, privkey);

    n = base64::encode((const unsigned char *)&pubkey, 32, base64, 256);
    base64[n] = 0;
    std::cout << source << " pubkey " << base64 << "\n";

    n = base64::encode((const unsigned char *)&privkey, 32, base64, 256);
    base64[n] = 0;
    std::cout << source << " privkey " << base64 << "\n";

    // then box

    const char *senderPass = "testString123";
    char sprivkey[32];
    char spubkey[32];

    getBoxKeyPairFromPassphrase(senderPass, spubkey, sprivkey);

    const char *receiverPass = "myFamousOldeSaying";
    char rprivkey[32];
    char rpubkey[32];

    getBoxKeyPairFromPassphrase(receiverPass, rpubkey, rprivkey);

    const char *message = "this is my test message";

    char cryptBuffer[256];
    char cryptBuffer2[256];
    char nonce[24];
    const char *tmp = "EhBJOkFN3CjwqBGzkSurniXj";
    for (int i = 0; i < 24; i++)
    {
        nonce[i] = tmp[i];
    }
    sink dest(cryptBuffer, 256);
    bool ok = box(&dest, slice(message), nonce, rpubkey, sprivkey);

    n = base64::encode((const unsigned char *)dest.base, dest.start, base64, 256);
    base64[n] = 0;
    std::cout << source << " encrypted " << base64 << "\n";
    // needs VIxCfsoyYNAmAT3uz4J_TCkxLyhrAOdojmICbUUawkXWQNKqIT_O
    if (0 != strcmp(base64, "VIxCfsoyYNAmAT3uz4J_TCkxLyhrAOdojmICbUUawkXWQNKqIT_O"))
    {
        cout << "FAIL base64::encode wrong";
    }
    slice encrypted(dest);
    sink decoded(cryptBuffer2, 256);

    // then unbox
    ok = unbox(&decoded, encrypted, nonce, spubkey, rprivkey);

    decoded.writeByte(0);

    std::cout << source << " decoded " << cryptBuffer2 << "\n";
    
}
