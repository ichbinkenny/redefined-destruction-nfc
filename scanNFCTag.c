#include <stdlib.h>
#include <nfc/nfc.h>
#include <freefare.h>

uint8_t default_key[] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff};

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

void dumpTagInfo(MifareTag tag){
    if(mifare_classic_connect(tag) != 0){
        perror("Failed to connect to tag!\r\n");
    }
    else{
        Mad mad;
        if((mad = mad_read(tag))){
            printf("Found device MAD\r\n");
            uint8_t dataBuff[4096]; //max size is for 4K card.
            int len;
            if((len = mifare_application_read(tag, mad, mad_nfcforum_aid, dataBuff, sizeof(dataBuff), mifare_classic_nfcforum_public_key_a, MFC_KEY_A)) != -1){
                printf("Read application! Length is: %d\r\n", len);
                uint8_t type;
                uint16_t dataLen;
                uint8_t* buffer = dataBuff;
                bool readData = true;
                uint8_t* data;
                while(readData){
                data = tlv_decode(buffer, &type, &dataLen);
                switch(type){
                    case 0x00: //null tlv
                        buffer += tlv_record_length(buffer, NULL, NULL);
                        if(buffer >= dataBuff + sizeof(dataBuff)){
                            readData = false;
                        }
                        break;
                    case 0x03:
                        printf("NDEF tlv found!\r\n");
                        break;
                    case 0xFD:
                        //Proprietary TLV
                        buffer += tlv_record_length(buffer, NULL, NULL);
                        if(buffer >= dataBuff + sizeof(dataBuff)){
                            readData = false;
                        }
                        break;
                    case 0xFE:
                        readData = false;
                        break;
                    default:
                        printf("Cant handle data :(\r\n");
                        readData = false;
                        break;
                }
                if(readData && type == 0x03){
                FILE* ndef_out = fopen("battleBotTagInfo.mfd", "wb");
                if(ndef_out){
                    printf("Writing to file!\r\n");
                    if(fwrite(data, 1, dataLen, ndef_out) != dataLen){
                        perror("Failed to write to file :(\r\n");
                    }
                }
                readData = false;
                }
                }
                free(data); 
            }
            else{
                perror("Failed to read application!\r\n");
            }
            free(mad);
        }
        else{
            perror("Unable to read MAD of tag.\r\n");
        }
    }
}

void printTagInfo(MifareTag* tags){
    for(int i = 0; tags[i]; i++){
        switch(freefare_get_tag_type(tags[i])){
            case CLASSIC_1K:
                printf("MIFARE Classic 1K Tag\r\n");
                break;
            case CLASSIC_4K:
                printf("MIFARE Classic 4K Tag\r\n");
                break;
            default:
                break;
        }
        char* tagUID = freefare_get_tag_uid(tags[i]);
        printf("Tag has uid: %s\r\n", tagUID);
        dumpTagInfo(tags[i]);
    }
}

int main(int argc, char* argv[]){
    nfc_device* dev;
    nfc_context* context;
    nfc_target nfcTarget;
    MifareTag *deviceTags = NULL;
    Mad deviceMad;
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
        deviceTags = freefare_get_tags(dev);
        if(!deviceTags){
            printf("Unable to get device tags...\r\n");
        }
        else{
            printf("Loaded device tags.\r\n");
            printTagInfo(deviceTags);
        }
    }

    nfc_close(dev);
    nfc_exit(context);
    exit(EXIT_SUCCESS);
}