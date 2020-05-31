#include <stdlib.h>
#include <nfc/nfc.h>

void infoToHex(uint8_t* data, size_t size){
    for(size_t pos = 0; pos < size; pos++){
        printf("%02x", data[pos]);
    }
}

void infoToText(uint8_t* data, size_t size){
    for(size_t pos = 0; pos < size; pos++){
        printf("%s", data[pos]);
    }
}

int main(int argc, char* argv[]){
    nfc_device* dev;
    nfc_context* context;
    nfc_target nfcTarget;
    nfc_init(&context);
    if(context == NULL){
        perror("Failed to open nfc reader!\r\n");
        exit(EXIT_FAILURE);
    }
    dev = nfc_open(context, NULL);
    if(dev == NULL){
        perror("Failed to connect to nfc reader!\r\n");
        exit(EXIT_FAILURE);
    }
    if(nfc_initiator_init(dev) < 0){
        perror("Failed to setup nfc reader!\r\n");
        exit(EXIT_FAILURE);
    }
    const nfc_modulation nmMifare = {
    .nmt = NMT_ISO14443A,
    .nbr = NBR_106,
    };
    printf("Please scan your NFC tag...\r\n");
    if(nfc_initiator_select_passive_target(dev, nmMifare, NULL, 0, &nfcTarget) > 0){
        if(argc < 2){
            printf("UID: ");
            infoToHex(nfcTarget.nti.nai.abtUid, nfcTarget.nti.nai.szUidLen);
            printf("\r\n");
        }
        else if(argv[1][1] == 'd'){
            printf("Fetching Card Data!\r\n");
            infoToText(nfcTarget.nti.nai.abtAts, nfcTarget.nti.nai.szAtsLen);
        }
        else{
            printf(argv[1]);
        }
    }
    nfc_close(dev);
    nfc_exit(context);
    exit(EXIT_SUCCESS);
}