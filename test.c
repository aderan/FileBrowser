#include <stdio.h>
#include <stdlib.h>

#include "fb_file.h"
#include "fb_common.h"

int main(int argc, char **argv)
{
    if (argc != 2)
        return -1;

    FileItem *items;
    while (1){
		fb_file_readdir(argv[1],&items);
    	char *json = fb_convert(&items);
    	printf ("%s\n", json);
    	fb_free_items(items);
		free(json);
        break;
		sleep (1);
	}
}
