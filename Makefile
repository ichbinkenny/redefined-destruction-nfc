LIBS = -lnfc -lfreefare
main:
	gcc -o readNFCTag ./scanNFCTag.c $(LIBS)

run:
	gcc -o readNFCTag ./scanNFCTag.c $(LIBS)
	./readNFCTag

uidRun:
	./readNFCTag

devInfo:
	./readNFCTag -i info