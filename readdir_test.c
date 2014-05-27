#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "fb_file.h"
#include "fb_utils.h"

/*
**this test for fb_*_readdir;debug in host system.
*/
int test_callback(const char *json_info, void *arg){
    fprintf (stderr, "%s\n", json_info);
    printf ("----------------------------------------------\n");
    printf ("arg is %d\n", *(int*)arg);
    printf ("----------------------------------------------\n");
}

int g_reset;

int main(int argc, char **argv)
{
    if (argc < 2)
        return -1;

    ReaddirParams rp_test ;

    strcpy (rp_test.path, argv[1]);
    if (argc > 2)
        rp_test.maxcount = atoi(argv[2]);

    rp_test.cb = test_callback;
    rp_test.cb_arg = malloc (sizeof(int));
    if (argc > 3)
        *(int *)rp_test.cb_arg = atoi(argv[3]);
    else
        *(int *)rp_test.cb_arg = 5050;

    while (1){
		fb_file_readdir(&rp_test);
		sleep (1);
	}
}
