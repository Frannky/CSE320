#include <stdio.h>
#include <stdlib.h>

#include "const.h"
#include "debug.h"

#ifdef _STRING_H
#error "Do not #include <string.h>. You will get a ZERO."
#endif

#ifdef _STRINGS_H
#error "Do not #include <strings.h>. You will get a ZERO."
#endif

#ifdef _CTYPE_H
#error "Do not #include <ctype.h>. You will get a ZERO."
#endif

int main(int argc, char **argv)
{
    // file name and dir sequence should be in the same level not parent and child
//    freopen("", "w", stdout);
//    char* name = "./ada";
//    path_init(name);
//    int res = 0;
//    res = serialize(0);
//    fprintf(stderr,"result = %d", res);

    int ret;
    if(validargs(argc, argv) == 0){
//        fprintf(stderr,"path_buf = %s\n", path_buf);
        if(global_options == 0x00){
            USAGE(*argv, EXIT_FAILURE);
            return EXIT_FAILURE;
        }
        else if(global_options == 0x1){
            USAGE(*argv, EXIT_SUCCESS);
            return EXIT_SUCCESS;
        }
        else if(global_options == 0x2){
            ret = serialize(0);
//            fprintf(stderr,"s_result = %d", ret);

            if(ret == -1){
                USAGE(*argv, EXIT_FAILURE);
                return EXIT_FAILURE;
            }

            else{
                return EXIT_SUCCESS;
            }

        }
        else if(global_options == 0x4 || global_options == 0xc){

            ret = deserialize(0);
//            fprintf(stderr,"d_result = %d", ret);
            if(ret == -1){
                USAGE(*argv, EXIT_FAILURE);
                return EXIT_FAILURE;
            }
            else{
                return EXIT_SUCCESS;
            }
        }

    }else{
        USAGE(*argv, EXIT_FAILURE);
        return EXIT_FAILURE;
    }

//    ret = validargs(argc,argv);
//    fprintf(stderr,"path_buf = %s\n", path_buf);
//    fprintf(stderr,"name_buf = %s\n", name_buf);
//    fprintf(stderr,"result = %d\n", ret);
//    fprintf(stderr,"global_options = %d\n", global_options);


//    debug("Options: 0x%x", global_options);
//    if(global_options & 1)
//        USAGE(*argv, EXIT_SUCCESS);


    return EXIT_SUCCESS;
}

/*
 * Just a reminder: All non-main functions should
 * be in another file not named main.c
 */
