#include <sys/types.h>
#include <sys/shm.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>


typedef struct {
    int version;
    int dataOffset;
    int dataSize;
    int offsetsOffset;
    int offsetsSize;
    int freeItemsOffset;
    int freeItemsSize;
    int maxItems;
    int items;
    int freeItemCount;
    int cacheFindHits;
    int cacheFindMisses;
    int cacheFindStrcmps;
    int cacheInsertHits;
    int cacheInsertMisses;
    int cacheInsertStrcmps;
} QSharedMemoryData;


int main(int argc, char *argv[])
{
    char const *types[] = {
	"       Global",
	"       Pixmap",
	"GlyphRowIndex",
	"     GlyphRow",
	"      Unknown"
    };
    char pipeData[1024] = "/tmp/qtembedded-";
    char *pipe = pipeData;
    char *display = 0;
    const char *logname = getenv("LOGNAME");
    int shmId, i;
    QSharedMemoryData *shm;
    int totalReferenced = 0;
    int totalSize = 0, count = 0;
    int version = 0, verbose = 0, showAll = 1, showFree = 0, help = 0, showUnrefed = 0, showProfile = 0;
    int showGlyphRowCache = 1;
    char *data = 0;
    int *offsets = 0, *freeItemOffsets = 0;

    printf("ShmDiag - A diagnostic tool for QWS shared memory.\n");

    for ( i = 0; i < argc; i++ ) {
	if ( argv[i][0] == '-') {
	    if ( !strcmp(argv[i], "--version") )
		version = 1;
	    else if ( !strcmp(argv[i], "--verbose") )
		verbose = 1;
	    else if ( !strcmp(argv[i], "--hide-unreferenced") )
		showAll = 0;
	    else if ( !strcmp(argv[i], "--only-show-unreferenced") )
		showUnrefed = 1;
	    else if ( !strcmp(argv[i], "--show-cleanup-list") )
		showFree = 1;
	    else if ( !strcmp(argv[i], "--only-show-profiling") )
		showProfile = 1;
	    else if ( !strcmp(argv[i], "--pipe") && (i+1 <= argc) )
		i++, pipe = argv[i];
	    else if ( !strcmp(argv[i], "--qwsdisplay") && (i+1 <= argc) )
		i++, display = argv[i];
	    else 
		help = 1;
	}
    }

    if ( pipe == pipeData )
        strcat(strcat(strcat(pipe, logname ? logname : "unknown"), "/QtEmbedded-"), display ? display : "0");

    if ( help ) {
	printf("\n  Usage: %s [options]\n\n", argv[0]);
	printf("    Options:\n");
	printf("	--version		    version information\n");
	printf("	--verbose		    verbose output\n");
	printf("	--hide-unreferenced	    don't show zero ref'd items\n");
	printf("	--only-show-unreferenced    only show zero ref'd items\n");
	printf("	--show-cleanup-list	    show the free item list\n");
	printf("	--only-show-profiling       only show profiling data\n");
	printf("	--qwsdisplay DispNo         show cache for display DispNo\n");
	printf("	--pipe QteLockFile	    use pipe file for finding shared\n");
	printf("                                    memory associated with display\n");
	printf("    DispNo      - eg 0\n");
	printf("    QteLockFile - eg %s\n\n", pipe);
	return 0;
    }

    if ( version ) {
	printf("\nShmDiag v1.00	    compiled: %s, %s\n", __TIME__, __DATE__);
	return 0;
    }

    shmId = shmget(ftok(pipe, 0xEEDDCCC2), 0, 0);

    if ( shmId == -1 ) {
	fprintf(stderr, "QWS shared memory not found.\n");
	return 0;
    }

    shm = (QSharedMemoryData*)shmat(shmId, 0, 0);

    if ( shmId == -1 || (int)shm == -1 ) {
	fprintf(stderr, "Failed attaching to QWS shared memory.\n");
	return 0;
    }

    printf("Connected to QWS shared memory found at %p\n", shm);

    if ( !showProfile ) {
	printf("--------------------------------------------\n");
	printf("Items held in the cache\n");
	printf("--------------------------------------------\n");

	data = (char*)shm + shm->dataOffset;
	offsets = (int*)((char*)shm + shm->offsetsOffset);
	freeItemOffsets = (int*)((char*)shm + shm->freeItemsOffset);

	for ( i = 0; i < shm->offsetsSize; i++ ) {
	    int index = offsets[i];
	    if ( index != -1 && index != -2 ) {
		int *d = (int*)&data[index];
		if ( d && (int)d != -1 ) {
		    unsigned int type = d[2];
		    type = type > 4 ? 4 : type;
		    totalReferenced += d[0];
		    if ( showUnrefed ) {
			if ( d[0] == 0 ) {
			    count++;
			    totalSize += d[-2] - index;
			    printf("hash_index: %4i, referenced: %3i, type: %s, size: %5i key: %s\n", i, d[0],
						types[type], d[-2] - index, d[1] != -1 ? &data[d[1]] : 0 );
			}
		    } else if ( showAll || d[0] != 0 ) {
			count++;
			totalSize += d[-2] - index;
			printf("hash_index: %4i, referenced: %3i, type: %s, size: %5i key: %s\n", i, d[0],
					    types[type], d[-2] - index, d[1] != -1 ? &data[d[1]] : 0 );
		    }
		    if ( type == 2 && showGlyphRowCache ) {
			int row;
			int *indexList = (int*)&d[3];
			for ( row = 0; row < 4096; row++ ) {
			    if ( indexList[row] != -1 )
				printf("%4i -> %5i (0x%X)  (absolute memory location: %i 0x%X, offset from shm %i 0x%X)\n",
					row, indexList[row], indexList[row], (int)&(indexList[row]), (int)&(indexList[row]),
					(int)&(indexList[row]) - (int)shm, (int)&(indexList[row]) - (int)shm );
			}
			printf("\n");
		    }
		}
	    }
	}

	printf("--------------------------------------------\n");
	printf("Cache totals: references: %3i, count: %3i, size: %5i\n", totalReferenced, count, totalSize);
	printf("--------------------------------------------\n");
    }

    if ( !showProfile && showFree ) {
	count = 0;
	totalSize = 0;

	printf("------------------------------------------------------------------------------\n");
	printf("Items marked for deletion held in the cleanup list (they can be re-referenced)\n");
	printf("------------------------------------------------------------------------------\n");

	for ( i = 0; i < shm->freeItemCount; i++ ) {
	    int index = freeItemOffsets[i];
	    if ( index != -1 && index != -2 ) {
		int *d = (int*)&data[index];
		if ( d && (int)d != -1 ) {
		    unsigned int type = d[2];
		    type = type > 4 ? 4 : type;
		    count++;
		    totalSize += d[-2] - index;
		    printf("free item index: %3i, references: %3i, type: %s, size: %5i key: %s\n", i, d[0],
			types[type], d[-2] - index, d[1] != -1 ? &data[d[1]] : 0 );
		}
	    }
	}

	printf("--------------------------------------------\n");
	printf("Free item totals: count: %i, size: %5i\n", count, totalSize);
	printf("--------------------------------------------\n");
    }

    printf("------------------------------------------------------------\n");
    printf("Cache profiling data\n");
    printf("------------------------------------------------------------\n");
    printf("finding:          hits: %5i, misses: %5i, strcmps: %5i\n", shm->cacheFindHits, shm->cacheFindMisses, shm->cacheFindStrcmps);
    printf("inserting:        hits: %5i, misses: %5i, strcmps: %5i\n", shm->cacheInsertHits, shm->cacheInsertMisses, shm->cacheInsertStrcmps);
    printf("------------------------------------------------------------\n");
    printf("Profiling totals: hits: %5i, misses: %5i, strcmps: %5i\n", shm->cacheFindHits+shm->cacheInsertHits,
						    shm->cacheFindMisses+shm->cacheInsertMisses, shm->cacheFindStrcmps+shm->cacheInsertStrcmps);
    printf("------------------------------------------------------------\n");

    shmdt((char*)shm);
    printf("Disconnected from QWS shared memory found at %p.\n", shm);

    return -1;
}

