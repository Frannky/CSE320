#include "const.h"
#include "transplant.h"
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

int stringCompare(char *first, char *second);
int stringCompare1(char first, char *second);
int stringCompare2(unsigned char *first, char *second);
int stringCompare3(unsigned char *first, unsigned char *second);
int length(char * name);
char *stringCopy(char * first , char * second);
char *stringNCopy(char * first, char * second, size_t n);
int stringNCompare(char * first, char* second, size_t n);
int pop_dir();
int push_dir(char *name);
void name_clear();
/*
 * You may modify this file and/or move the functions contained here
 * to other source files (except for main.c) as you wish.
 *
 * IMPORTANT: You MAY NOT use any array brackets (i.e. [ and ]) and
 * you MAY NOT declare any arrays or allocate any storage with malloc().
 * The purpose of this restriction is to force you to use pointers.
 * Variables to hold the pathname of the current file or directory
 * as well as other data have been pre-declared for you in const.h.
 * You must use those variables, rather than declaring your own.
 * IF YOU VIOLATE THIS RESTRICTION, YOU WILL GET A ZERO!
 *
 * IMPORTANT: You MAY NOT use floating point arithmetic or declare
 * any "float" or "double" variables.  IF YOU VIOLATE THIS RESTRICTION,
 * YOU WILL GET A ZERO!
 */

/*
 * A function that returns printable names for the record types, for use in
 * generating debugging printout.
 */
static char *record_type_name(int i) {
    switch(i) {
        case START_OF_TRANSMISSION:
            return "START_OF_TRANSMISSION";
        case END_OF_TRANSMISSION:
            return "END_OF_TRANSMISSION";
        case START_OF_DIRECTORY:
            return "START_OF_DIRECTORY";
        case END_OF_DIRECTORY:
            return "END_OF_DIRECTORY";
        case DIRECTORY_ENTRY:
            return "DIRECTORY_ENTRY";
        case FILE_DATA:
            return "FILE_DATA";
        default:
            return "UNKNOWN";
    }
}

/*
 * @brief  Initialize path_buf to a specified base path.
 * @details  This function copies its null-terminated argument string into
 * path_buf, including its terminating null byte.
 * The function fails if the argument string, including the terminating
 * null byte, is longer than the size of path_buf.  The path_length variable
 * is set to the length of the string in path_buf, not including the terminating
 * null byte.
 *
 * @param  Pathname to be copied into path_buf.
 * @return 0 on success, -1 in case of error
 */
int path_init(char *name) {
    // To be implemented.
    int i =0;
    int len = length(name);
    if(len+1 > PATH_MAX){

        return -1;
    }

    while(*(name+i) != '\0'){

        *(path_buf + i) = *(name + i);
        i++;
    }
//    *(path_buf + i++) = '/';
    *(path_buf + i) = '\0';
    path_length = len;
    return 0;
}

/*
 * @brief  Append an additional component to the end of the pathname in path_buf.
 * @details  This function assumes that path_buf has been initialized to a valid
 * string.  It appends to the existing string the path separator character '/',
 * followed by the string given as argument, including its terminating null byte.
 * The length of the new string, including the terminating null byte, must be
 * no more than the size of path_buf.  The variable path_length is updated to
 * remain consistent with the length of the string in path_buf.
 *
 * @param  The string to be appended to the path in path_buf.  The string must
 * not contain any occurrences of the path separator character '/'.
 * @return 0 in case of success, -1 otherwise.
 */
int path_push(char *name) {
    // To be implemented.
    int i = 0;

    while(*(path_buf+i) != '\0'){

        i++;
    }
    int j = 0;

    if(*(path_buf+ i-1) == '/'){
        while(*(name+ j) != '\0'){

            *(path_buf + i++) = *(name + j++);
        }
    }else{
        *(path_buf + i++) = '/';
        while(*(name+ j) != '\0'){

            *(path_buf + i++) = *(name + j++);
        }
    }

    *(path_buf + i) = '\0';
    path_length = i-1;

    if(path_length > PATH_MAX){

        return -1;
    }
    return 0;
}

/*
 * @brief  Remove the last component from the end of the pathname.
 * @details  This function assumes that path_buf contains a non-empty string.
 * It removes the suffix of this string that starts at the last occurrence
 * of the path separator character '/'.  If there is no such occurrence,
 * then the entire string is removed, leaving an empty string in path_buf.
 * The variable path_length is updated to remain consistent with the length
 * of the string in path_buf.  The function fails if path_buf is originally
 * empty, so that there is no path component to be removed.
 *
 * @return 0 in case of success, -1 otherwise.
 */
int path_pop() {
    // To be implemented.
    int i = 0;
    int index =0;
    while(*(path_buf + i) != '\0'){

        if(*(path_buf + i) == '/'){

            index = i;
        }

        i++;
    }
//    fprintf(stderr, "index = %d\n", index);

    i = 0;
    while(*(path_buf + i) != '\0'){

        if(i == index){

            *(path_buf + i) = '\0';
        }

        if(i > index){

            *(path_buf + i) = '0';
        }

        i++;
    }

    path_length = index;

    if(path_length > PATH_MAX) return -1;

    return 0;
}

/*
 * @brief Deserialize directory contents into an existing directory.
 * @details  This function assumes that path_buf contains the name of an existing
 * directory.  It reads (from the standard input) a sequence of DIRECTORY_ENTRY
 * records bracketed by a START_OF_DIRECTORY and END_OF_DIRECTORY record at the
 * same depth and it recreates the entries, leaving the deserialized files and
 * directories within the directory named by path_buf.
 *
 * @param depth  The value of the depth field that is expected to be found in
 * each of the records processed.
 * @return 0 in case of success, -1 in case of an error.  A variety of errors
 * can occur, including depth fields in the records read that do not match the
 * expected value, the records to be processed to not being with START_OF_DIRECTORY
 * or end with END_OF_DIRECTORY, or an I/O error occurs either while reading
 * the records from the standard input or in creating deserialized files and
 * directories.
 */
int deserialize_directory(int depth) {
    // To be implemented.
    // deserialize the SOD
    int magic1 = 12;
    int magic2 = 13;
    int magic3 = 237;

    unsigned int size = 0;
    unsigned int dep = 0;
    unsigned int n = 0;
    n = getchar();
    if(n != magic1) {
        return -1;
    }
    n = getchar();
    if(n != magic2) return -1;
    n = getchar();
    if(n != magic3) return -1;
    n = getchar();
    if(n != START_OF_DIRECTORY) return -1;

    dep = 0;
    for(int i = 0 ; i< 4; i++){

        n = getchar();
        dep = dep + (n << (24 - 8*i));

    }
    //  depth ????????????????????????????
    if(dep != depth) return -1;
//    fprintf(stderr,"dep = %d ", dep);
    size = 0;
    for(int i = 0; i<8; i++){

        n = getchar();
        size = size + (n << (56-8*i));
    }

//    fprintf(stderr,"size = %d\n", size);

    while((n = getchar()) != EOF){
//        fprintf(stderr,"test\n");
//        fprintf(stderr, "number n = %d\n" , n);
        if(n != magic1) return -1;


        n = getchar();
        if(n != magic2) return -1;
        n = getchar();
        if(n != magic3) return -1;

        n = getchar();

        if(n == END_OF_DIRECTORY) break;
        else if(n == DIRECTORY_ENTRY){
            dep = 0;
            for(int i = 0 ; i< 4; i++){

                n = getchar();
                dep = dep + (n << (24 - 8*i));

            }

//            fprintf(stderr,"dep = %d, depth = %d ", dep,depth);

            if(dep != depth) return -1;
            size = 0;
            for(int i = 0; i<8; i++){

                n = getchar();
                size = size + (n << (56-8*i));
            }
//            fprintf(stderr,"size = %d\n" , size);


//            fprintf(stderr,"size = %d\n", size);

            unsigned int name_size = size - HEADER_SIZE - 12;
//            fprintf(stderr,"content_size = %d\n", name_size);

            unsigned int stat_mode = 0;//(unsigned char *) &stat_buf.st_mode
//    unsigned char *size = (unsigned char *) &stat_buf.st_size;

            //stat.mode
            for(int i = 0; i < 4 ; i++){
                n = getchar();
                stat_mode = stat_mode + (n << (24 - 8*i));
            }
//            fprintf(stderr,"stat_mode = %d\n", stat_mode);

            unsigned int stat_size = 0;
            //stat.size
            for(int i = 0; i<8 ; i++){
                n = getchar();
                stat_size = stat_size + (n << (56 -8*i));
            }
//            fprintf(stderr,"stat_size = %d\n", stat_size);

            //??????????????????????????????
            //student file size have problem
//    char  entry_name[name_size];
            name_clear(name_buf);
            int name_len;
            for(name_len = 0 ; name_len < name_size ; name_len++){
                n = getchar();
                *(name_buf + name_len) = (unsigned char) n;
            }
            *(name_buf + name_len) = '\0';
//            fprintf(stderr,"name_buf = %s\n", name_buf);

            struct stat stat_buf;

            if(S_ISDIR(stat_mode)){
                path_push(name_buf);

                if(stat(path_buf, &stat_buf) != -1 && global_options != 0x0c){
                    return -1;
                }
//        if(stat(path_buf,&st) == -1 || global_options == 0xc)
//                fprintf(stderr,"dir_path = %s\n", path_buf);
                mkdir(path_buf, 0700);
                deserialize_directory(depth+1);
                chmod(path_buf, stat_mode & 0777);
                name_clear(name_buf);
                path_pop();
            }else if(S_ISREG(stat_mode)){
                path_push(name_buf);
                stat(path_buf,&stat_buf);
                if(stat(path_buf, &stat_buf) != -1 && global_options != 0x0c){
                    return -1;
                }
//                fprintf(stderr,"dir_path = %s\n", path_buf);
                deserialize_file(depth+1);
                name_clear(name_buf);
                path_pop();
//                deserialize_directory(depth+1);
            }

        }else return -1;
    }


    // deserialize the DE

//    n = 0;
//    n = getchar();
//    if(n != magic1) return -1;
//    n = getchar();
//    if(n != magic2) return -1;
//    n = getchar();
//    if(n != magic3) return -1;
//    n = getchar();
//    if(n != DIRECTORY_ENTRY) return -1;




    //end of the directory
//    n = getchar();
//    if(n != magic1) {
//        return -1;
//    }
//    n = getchar();
//    if(n != magic2) return -1;
//    n = getchar();
//    if(n != magic3) return -1;
//    n = getchar();
    if(n != END_OF_DIRECTORY) return -1;
    dep = 0;
    for(int i = 0 ; i< 4; i++){

        n = getchar();
        dep = dep + (n << (24 - 8*i));

    }
    if(dep != depth) return -1;
//    fprintf(stderr,"dep = %d ", dep);
    size = 0;
    for(int i = 0; i<8; i++){

        n = getchar();
        size = size + (n << (56-8*i));
    }

    return 0;
}

/*
 * @brief Deserialize the contents of a single file.
 * @details  This function assumes that path_buf contains the name of a file
 * to be deserialized.  The file must not already exist, unless the ``clobber''
 * bit is set in the global_options variable.  It reads (from the standard input)
 * a single FILE_DATA record containing the file content and it recreates the file
 * from the content.
 *
 * @param depth  The value of the depth field that is expected to be found in
 * the FILE_DATA record.
 * @return 0 in case of success, -1 in case of an error.  A variety of errors
 * can occur, including a depth field in the FILE_DATA record that does not match
 * the expected value, the record read is not a FILE_DATA record, the file to
 * be created already exists, or an I/O error occurs either while reading
 * the FILE_DATA record from the standard input or while re-creating the
 * deserialized file.
 */
int deserialize_file(int depth){

//    unsigned char * dep = (unsigned char *) &depth;

    int magic1 = 12;
    int magic2 = 13;
    int magic3 = 237;

    // deserialize FD
    unsigned int n = 0;
    n = getchar();
    if(n != magic1) return -1;
    n = getchar();
    if(n != magic2) return -1;
    n = getchar();
    if(n != magic3) return -1;
    n = getchar();
    if(n != FILE_DATA) return -1;

    unsigned int dep = 0;
    for(int i = 0 ; i< 4; i++){

        n = getchar();
        dep = dep + (n << (24 - 8*i));

    }
    if(dep != depth) return -1;
//    fprintf(stderr,"dep = %d ", dep);
    unsigned int size = 0;
    for(int i = 0; i<8; i++){

        n = getchar();
        size = size + (n << (56-8*i));
    }

//    fprintf(stderr,"size = %d\n", size);
    unsigned int content_size = size - HEADER_SIZE;
//    fprintf(stderr,"content_size = %d\n", content_size);
    //mkdir(path_buf,0777); ????????????????????

    // clobber !!!!!!????????????????????
//    FILE *tes = fopen(path_buf, "r");
//    if(tes != NULL && global_options != 0x0c) return -1;
//    fclose(tes);
    FILE *f = fopen(path_buf,"w");
//    chmod(path_buf, 0777);
//    fprintf(stderr, "path_buf %s\n", path_buf);

    if(f == NULL) {
//        fprintf(stderr, "path_buf %s\n", "abc");
        return -1;
//        f = fopen(path_buf,"w");
    }

    //file size  has a method to directly get content size
//    int ch;
    while(content_size != 0) {
        int ch = 0;
//        fprintf(stder
//        r,"position is = %p\n", f);
//        fprintf(stderr,"content is = %d\n", ch = getchar());
        ch = getchar();
        if((fputc(ch,f) == EOF)) return -1;
        content_size--;
//        fprintf(stderr,"content is = %d\n", ch);

    }
    fflush(f);

//    fprintf(stderr, "pointer = %p\n", f);

    int re;
    if((re = fclose(f)) == EOF) return -1;
    return 0;
};

/*
 * @brief  Serialize the contents of a directory as a sequence of records written
 * to the standard output.
 * @details  This function assumes that path_buf contains the name of an existing
 * directory to be serialized.  It serializes the contents of that directory as a
 * sequence of records that begins with a START_OF_DIRECTORY record, ends with an
 * END_OF_DIRECTORY record, and with the intervening records all of type DIRECTORY_ENTRY.
 *
 * @param depth  The value of the depth field that is expected to occur in the
 * START_OF_DIRECTORY, DIRECTORY_ENTRY, and END_OF_DIRECTORY records processed.
 * Note that this depth pertains only to the "top-level" records in the sequence:
 * DIRECTORY_ENTRY records may be recursively followed by similar sequence of
 * records describing sub-directories at a greater depth.
 * @return 0 in case of success, -1 otherwise.  A variety of errors can occur,
 * including failure to open files, failure to traverse directories, and I/O errors
 * that occur while reading file content and writing to standard output.
 */
int serialize_directory(int depth) {
    // To be implemented.
    // what is all the situations to return -1 ???????????????????????????????????????????????????
    struct stat stat_buf;
    //it is allowed to use local variable to store the output to standard in

    //convert depth int into char bytes

    unsigned int header_size = HEADER_SIZE;
    unsigned char * pp = (unsigned char *) &header_size;
    unsigned char * dep = (unsigned char *) &depth;
    // start of directory
    putchar(MAGIC0);
    putchar(MAGIC1);
    putchar(MAGIC2);
    putchar(START_OF_DIRECTORY);
    for (int i = 3; i >=0 ; i--) putchar(*(dep + i));
    // SOD the size is just the header
    //depth
    for(int i = 0; i< 4; i++) putchar(0x00);

    //size
    for (int i = 3; i >= 0; i--)
    {
        putchar(*(pp + i));
    }

//    mkdir(path_buf,0700);

    DIR *dir = opendir(path_buf);
//    fprintf(stderr, "path_buf = %s\n", path_buf);

    if(dir == NULL){
//        fprintf(stderr, "path_buf %s\n", "aa");
        return -1;
    }

    // NULL means dir is not a directory path, it is a file
    struct dirent *de;
    //de can read both directory and file
    while((de = readdir(dir)) != NULL){


        if(stringCompare(de->d_name,"..") ==1 || stringCompare(de->d_name,".") ==1) continue;
        name_clear(name_buf);
        stringCopy(name_buf,de->d_name);
        path_push(de->d_name);

//        path_push(de->d_name);
//        fprintf(stderr, "d_name = %s\n", de->d_name);
//        fprintf(stderr, "path_buf = %s\n", path_buf);

        //path_buf
        if(stat(path_buf,&stat_buf) >= 0 && S_ISDIR(stat_buf.st_mode)) {
//            printf("aa");
            path_pop(de->d_name);
            push_dir(de->d_name);
//            fprintf(stderr, "d_name_dir = %s\n", de->d_name);
//            fprintf(stderr, "st_mode_dir = %d\n", stat_buf.st_mode);
//            fprintf(stderr, "st_size_dir = %ld\n", stat_buf.st_size);
//            fprintf(stderr, "dir_path_buf = %s\n", path_buf);

            int x = 0;
            while(*(de->d_name+x) != '\0') {

//                putchar(*((de->d_name)+x));
//                fprintf(stderr, "name = %c ", *((de->d_name)+x));
                x++;
            }
//            fprintf(stderr, "x_dir = %d\n", x);

            unsigned char *mode = (unsigned char *) &stat_buf.st_mode;
            unsigned char *size = (unsigned char *) &stat_buf.st_size;
            unsigned int total_size = HEADER_SIZE + 12 + x;
            unsigned char * p = (unsigned char *)&total_size;
            // directory entry
            putchar(MAGIC0);
            putchar(MAGIC1);
            putchar(MAGIC2);
            putchar(DIRECTORY_ENTRY);
            for (int i = 3; i >=0 ; i--) putchar(*(dep + i));
            // SOD the size is just the header
            //depth
            for(int i = 0; i< 4; i++) putchar(0x00);

            //size
            for (int i = 3; i >= 0; i--)
            {
                putchar(*(p + i));
            }
            // metadata mode
            for (int i = 3; i >= 0; i--)
            {
                putchar(*(mode + i));
            }
            // metadata size
            for (int i = 7; i >= 0; i--)
            {
                putchar(*(size + i));
            }
            x = 0;
            while(*(de->d_name+x) != '\0') {

                putchar(*((de->d_name)+x));
//                fprintf(stderr, "name = %c ", *((de->d_name)+x));
                x++;
            }


//            fprintf(stderr, "name_length = %d\n", x);

            int result = serialize_directory(depth+1);
            if(result == -1) return -1;
            name_clear(name_buf);
            pop_dir();

        }
        else if(stat(path_buf,&stat_buf) >= 0 && S_ISREG(stat_buf.st_mode)){
//            fprintf(stderr, "test = %s\n", "bb");

//            fprintf(stderr, "d_name_file = %s\n", de->d_name);
//            fprintf(stderr, "st_mode_file = %d\n", stat_buf.st_mode);
//            fprintf(stderr, "st_size_file = %ld\n", stat_buf.st_size);
//            fprintf(stderr, "file_path_buf = %s\n", path_buf);

            int x = 0;
            while(*(de->d_name+x) != '\0') {

//                putchar(*((de->d_name)+x));
//                fprintf(stderr, "name = %c ", *((de->d_name)+x));
                x++;
            }
//            fprintf(stderr, "x_file = %d\n", x);

            unsigned char *mode = (unsigned char *) &stat_buf.st_mode;
            unsigned char *size = (unsigned char *) &stat_buf.st_size;
            unsigned int total_size = HEADER_SIZE + 12 + x;
            unsigned char * p = (unsigned char *)&total_size;
            // directory entry
            putchar(MAGIC0);
            putchar(MAGIC1);
            putchar(MAGIC2);
            putchar(DIRECTORY_ENTRY);
            for (int i = 3; i >=0 ; i--) putchar(*(dep + i));
            // SOD the size is just the header
            //depth
            for(int i = 0; i< 4; i++) putchar(0x00);

            //size
            for (int i = 3; i >= 0; i--)
            {
                putchar(*(p + i));
            }

//            // metadata mode
            for (int i = 3; i >= 0; i--)
            {
                putchar(*(mode + i));
            }
            // metadata size
            for (int i = 7; i >= 0; i--)
            {
                putchar(*(size + i));
            }
            x = 0;
            while(*(de->d_name+x) != '\0') {

                putchar(*((de->d_name)+x));
//                fprintf(stderr, "name = %c ", *((de->d_name)+x));
                x++;
            }

            FILE *f = fopen(path_buf,"r");
//            fprintf(stderr, "path_buf %s\n", path_buf);

            if(f == NULL) {

                return -1;
            }

            //file size  has a method to directly get content size
            off_t count = 0;
            while(1) {

                char c;
                if((c = fgetc(f)) == EOF) break;
                count++;
            }
            count++;
//            fprintf(stderr, "count = %ld\n", count);
            int re;

            if((re = fclose(f)) == EOF) return -1;
            unsigned long max = 1UL;
            if(count < 0 || count >= (max<<63)) return -1;

            int result  = serialize_file(depth+1, stat_buf.st_size);
            if(result == -1) return -1;
            name_clear(name_buf);
            path_pop();
//            fprintf(stderr, "path_buf_after = %s\n", path_buf);

        }
    }

    // end of directory
    putchar(MAGIC0);
    putchar(MAGIC1);
    putchar(MAGIC2);
    putchar(END_OF_DIRECTORY);
    for (int i = 3; i >=0 ; i--) putchar(*(dep + i));
    // SOD the size is just the header
    //depth
    for(int i = 0; i< 4; i++) putchar(0x00);

    //size
    for (int i = 3; i >= 0; i--)
    {
        putchar(*(pp + i));
    }

    closedir(dir);
    return 0;
}

/*
 * @brief  Serialize the contents of a file as a single record written to the
 * standard output.
 * @details  This function assumes that path_buf contains the name of an existing
 * file to be serialized.  It serializes the contents of that file as a single
 * FILE_DATA record emitted to the standard output.
 *
 * @param depth  The value to be used in the depth field of the FILE_DATA record.
 * @param size  The number of bytes of data in the file to be serialized.
 * @return 0 in case of success, -1 otherwise.  A variety of errors can occur,
 * including failure to open the file, too many or not enough data bytes read
 * from the file, and I/O errors reading the file data or writing to standard output.
 */
int serialize_file(int depth, off_t size) {
    //off_t size is a 8 bytes size of the file
    // what is all the situations to return -1?????????????????????????????????????????????????
    // To be implemented.
//    printf("%d",1);
//    putchar('a');

    unsigned char * dep = (unsigned char *) &depth;
    off_t total_size = size + HEADER_SIZE;
    unsigned char * siz =(unsigned char *) &total_size;
//    fprintf(stderr, "total_size =  %ld\n", total_size);
//    fprintf(stderr, "total_size =  %d\n", *dep);
//    fprintf(stderr, "total_size =  %d\n", *siz);

    // file_data
    // total size = 16 + size of file contents

    //FILE DATA
    putchar(MAGIC0);
    putchar(MAGIC1);
    putchar(MAGIC2);
    putchar(FILE_DATA);

    for(int i =3 ; i>=0 ; i--) putchar(*(dep + i));
    for(int i =7 ; i>=0 ; i--) putchar(*(siz + i));

    FILE *f = fopen(path_buf,"r");
//    fprintf(stderr, "jinlai %s\n", path_buf);

    if(f == NULL) {
//        fprintf(stderr, "sssss %s\n", "sssss");

        return -1;
    }

    //file size  has a method to directly get content size
    off_t count = 0;
    while(1) {

        char c;
        if((c = fgetc(f)) == EOF) break;
        putchar(c);
//        fprintf(stderr, "count = %c ", c);

        count++;
    }
//    fprintf(stderr,"size = %ld\n", size);
//    fprintf(stderr,"count = %ld\n",count);
    if(size != count) return -1;
//    fprintf(stderr, "%s\n", "");

//    fprintf(stderr, "count = %ld\n", count);
    int re;

    if((re = fclose(f)) == EOF) return -1;

    return 0;
}

/**
 * @brief Serializes a tree of files and directories, writes
 * serialized data to standard output.
 * @details This function assumes path_buf has been initialized with the pathname
 * of a directory whose contents are to be serialized.  It traverses the tree of
 * files and directories contained in this directory (not including the directory
 * itself) and it emits on the standard output a sequence of bytes from which the
 * tree can be reconstructed.  Options that modify the behavior are obtained from
 * the global_options variable.
 *
 * @return 0 if serialization completes without error, -1 if an error occurs.
 */
int serialize() {
    // To be implemented.
    // what is all the situations to return -1 ???????????????????????????????????????????????????????
    // start of transmission
    putchar(MAGIC0);
    putchar(MAGIC1);
    putchar(MAGIC2);
    putchar(START_OF_TRANSMISSION);

    // SOT and EOT the depth is zero
    // SOT and EOT the size is just the header

    //depth
    for(int i = 0; i< 4; i++) putchar(0x00);

    unsigned int header_size = HEADER_SIZE;
    unsigned char * p = (unsigned char *)&header_size;

    //size
    for(int i = 0; i< 4; i++) putchar(0x00);

    //size
    for (int i = 3; i >= 0; i--)
    {
        putchar(*(p + i));
    }
    if(serialize_directory(1) == -1) {

        // end of transmission
        putchar(MAGIC0);
        putchar(MAGIC1);
        putchar(MAGIC2);
        putchar(END_OF_TRANSMISSION);

        // SOT and EOT the depth is zero
        // SOT and EOT the size is just the header

        //depth
        for(int i = 0; i< 4; i++) putchar(0x00);

        //size
        for(int i = 0; i< 4; i++) putchar(0x00);

        //size
        for (int i = 3; i >= 0; i--)
        {
            putchar(*(p + i));
        }


        return -1;
    }
    // end of transmission
    putchar(MAGIC0);
    putchar(MAGIC1);
    putchar(MAGIC2);
    putchar(END_OF_TRANSMISSION);

    // SOT and EOT the depth is zero
    // SOT and EOT the size is just the header

    //depth
    for(int i = 0; i< 4; i++) putchar(0x00);

    for(int i = 0; i< 4; i++) putchar(0x00);

    //size
    for (int i = 3; i >= 0; i--)
    {
        putchar(*(p + i));
    }
    fflush(stdout);
    return 0;
}

/**
 * @brief Reads serialized data from the standard input and reconstructs from it
 * a tree of files and directories.
 * @details  This function assumes path_buf has been initialized with the pathname
 * of a directory into which a tree of files and directories is to be placed.
 * If the directory does not already exist, it is created.  The function then reads
 * from from the standard input a sequence of bytes that represent a serialized tree
 * of files and directories in the format written by serialize() and it reconstructs
 * the tree within the specified directory.  Options that modify the behavior are
 * obtained from the global_options variable.
 *
 * @return 0 if deserialization completes without error, -1 if an error occurs.
 */
int deserialize() {
    // To be implemented.
    // start of transmission
    int magic1 = 12;
    int magic2 = 13;
    int magic3 = 237;

    unsigned int n = 0;
    n = getchar();
    if(n != magic1) return -1;
    n = getchar();
    if(n != magic2) return -1;
    n = getchar();
    if(n != magic3) return -1;
    n = getchar();
    if(n != START_OF_TRANSMISSION) return -1;

    unsigned int dep = 0;
    for(int i = 0 ; i< 4; i++){

        n = getchar();
        dep = dep + (n << (24 - 8*i));
//        fprintf(stderr, "input = %d ", n);//getchar() == (int)0x0C

    }
    if(dep != 0) return -1;
//    fprintf(stderr,"dep = %d ", dep);
    unsigned int size = 0;
    for(int i = 0; i<8; i++){

        n = getchar();
        size = size + (n << (56-8*i));
//        fprintf(stderr,"input = %d\n" , n);
    }

//    fprintf(stderr,"size = %d\n", size);

    if(deserialize_directory(1) == -1){

        // deserialize the EOT

        n = getchar();
        if(n != magic1) return -1;
        n = getchar();
        if(n != magic2) return -1;
        n = getchar();
        if(n != magic3) return -1;
        n = getchar();
        if(n != END_OF_TRANSMISSION) return -1;

        dep = 0;
        for(int i = 0 ; i< 4; i++){

            n = getchar();
            dep = dep + (n << (24 - 8*i));
//            fprintf(stderr, "input = %d ", n);//getchar() == (int)0x0C

        }
        if(dep != 0) return -1;
//        fprintf(stderr,"dep = %d ", dep);
        size = 0;
        for(int i = 0; i<8; i++){

            n = getchar();
            size = size + (n << (56-8*i));
//            fprintf(stderr,"input = %d\n" , n);
        }

//        fprintf(stderr,"size = %d\n", size);

        return -1;
    }

    //deserialize the EOT
    n = getchar();
    if(n != magic1) return -1;
    n = getchar();
    if(n != magic2) return -1;
    n = getchar();
    if(n != magic3) return -1;
    n = getchar();
    if(n != END_OF_TRANSMISSION) return -1;

    dep = 0;
    for(int i = 0 ; i< 4; i++){

        n = getchar();
        dep = dep + (n << (24 - 8*i));
//        fprintf(stderr, "input = %d ", n);//getchar() == (int)0x0C

    }
    if(dep != 0) return -1;
//    fprintf(stderr,"dep = %d ", dep);
    size = 0;
    for(int i = 0; i<8; i++){

        n = getchar();
        size = size + (n << (56-8*i));
//        fprintf(stderr,"input = %d\n" , n);
    }

//    fprintf(stderr,"size = %d\n", size);

    return 0;
}

/**
 * @brief Validates command line arguments passed to the program.
 * @details This function will validate all the arguments passed to the
 * program, returning 0 if validation succeeds and -1 if validation fails.
 * Upon successful return, the selected program options will be set in the
 * global variable "global_options", where they will be accessible
 * elsewhere in the program.
 *
 * @param argc The number of arguments passed to the program from the CLI.
 * @param argv The argument strings passed to the program from the CLI.
 * @return 0 if validation succeeds and -1 if validation fails.
 * Refer to the homework document for the effects of this function on
 * global variables.
 * @modifies global variable "global_options" to contain a bitmap representing
 * the selected options.
 */
int validargs(int argc, char **argv)
{
    // To be implemented.
    if(argc <= 1) {
//        fprintf(stderr,"agrc = %d\n", argc);
//        fprintf(stderr,"agrv = %s\n", **argv);
        global_options = 0x0;
        return -1;
    }

    // argv first element must be bin/transplant
    if(stringCompare(*argv , "bin/transplant") == 0) {
//        fprintf(stderr,"agrc = %d\n", argc);
//        fprintf(stderr,"agrv = %p\n", *argv);
        global_options = 0x0;
        return -1;
    }

    int d_flag = 0;
    int h_flag = 0;
    int s_flag = 0;
    int p_flag = 0;
    int c_flag = 0;
    for(int i = 1; i< argc ; i++){

        if(stringCompare(*(argv + i), "-d")) d_flag = i;
        if(stringCompare(*(argv + i), "-h")) h_flag = i;
        if(stringCompare(*(argv + i), "-s")) s_flag = i;
        if(stringCompare(*(argv + i), "-p")) p_flag = i;
        if(stringCompare(*(argv + i), "-c")) c_flag = i;
    }
//    fprintf(stderr,"d_flag = %d\n" , d_flag);
//    fprintf(stderr,"h_flag = %d\n" , h_flag);
//    fprintf(stderr,"s_flag = %d\n" , s_flag);
//    fprintf(stderr,"p_flag = %d\n" , p_flag);
//    fprintf(stderr,"c_flag = %d\n" , c_flag);

    if(h_flag == 1) {
        global_options = 0x1;
//        printf("global_options = %x\n" , global_options);
        return 0;
    }

    if(d_flag && h_flag && s_flag && p_flag && c_flag){
        global_options = 0x0;
        return -1;
    }

    if(d_flag && c_flag && c_flag < argc && c_flag < d_flag){

        global_options = 0x0;
        return -1;
    }

    if(s_flag && c_flag && s_flag < argc && c_flag <argc){

        global_options = 0x0;
        return -1;
    }

    //-c cannot appear before -d

    //if -h is given global_options is set to be 0x1

    // cannot have -s and -c at the same time

    // if -p is followed by any other instructions except for the path should return -1
    //need to modify a little bit that if dir is not present that . was assumed in the current dir
    if(p_flag && p_flag < argc -1 && (stringCompare(*(argv + p_flag+1),"-s") == 1 || stringCompare(*(argv + p_flag+1),"-d") == 1
                                      || stringCompare(*(argv + p_flag+1),"-c") == 1 || stringCompare(*(argv + p_flag+1),"-p") == 1
                                      || stringCompare(*(argv + p_flag+1),"-h") == 1 || stringCompare(*(argv + p_flag+1),"\0") == 1)) {

        global_options = 0x0;
        return -1;
    }
    // if -p is not appear, which means it is in current directory

    if(p_flag == 0) {

        *(path_buf+0) = '.';
        *(path_buf+1) = '/';
        *(path_buf+2) = '\0';

    }else {

        int count = 0;

        // deal with absolute path
        if (**(argv + p_flag + 1) == '/') {
            int path_len = length(*(argv + p_flag + 1));
//        printf("%d", path_length);

            while (count < path_len) {
                *(path_buf + count) = *(*(argv + p_flag + 1) + count);

                count++;

            }
            *(path_buf + count) = '\0';

        }

            //deal with relative path
        else {

            int path_len = length(*(argv + p_flag + 1));

            while (count < path_len) {
                *(path_buf + count) = *(*(argv + p_flag + 1) + count);

                count++;

            }

            *(path_buf + count) = '\0';

        }
    }

    // -d is given but not current dirctory
    if(d_flag == 1 && p_flag && length(path_buf) > 0) {
        int i = 0;

        if (*(path_buf + 0) == '/') {
            i = 1;
            int j = 0;
            while (i < length(path_buf)) {
                while (*(path_buf + i) != '/' && i <= length(path_buf) - 1) {
                    *(name_buf + j) = *(path_buf + i);
                    i++;
                    j++;
                }
//                fprintf(stderr, "i = %d\n", i);
//                fprintf(stderr, "j = %d\n", j);
//                fprintf(stderr, "agrc = %d\n", argc);
//                fprintf(stderr, "agrv = %p\n", *argv);
//                fprintf(stderr, "name_buf = %s\n", name_buf);
//                fprintf(stderr, "path_buf = %s\n", path_buf);
//                fprintf(stderr, "length = %d\n", length(path_buf));

                FILE *f = fopen(name_buf, "r");
                if (f == NULL) {
                    mkdir(name_buf, 0777);
//                    fprintf(stderr, "agrc = %d\n", argc);

                } else {
                    int re;
                    if((re = fclose(f)) == EOF) return -1;
                }

                *(name_buf + j) = '/';
//                fprintf(stderr, "name_buf = %s\n", name_buf);

                i++;
                j++;
//                fprintf(stderr, "i = %d\n", i);
//                fprintf(stderr, "j = %d\n", j);
            }


        } else if (*(path_buf + 0) == '.') {
            i = 2;
            int j = 0;
            while (i < length(path_buf)) {
                while (*(path_buf + i) != '/' && i <= length(path_buf) - 1) {
                    *(name_buf + j) = *(path_buf + i);
                    i++;
                    j++;
                }
//                fprintf(stderr, "i = %d\n", i);
//                fprintf(stderr, "j = %d\n", j);
//                fprintf(stderr, "agrc = %d\n", argc);
//                fprintf(stderr, "agrv = %p\n", *argv);
//                fprintf(stderr, "name_buf = %s\n", name_buf);
//                fprintf(stderr, "path_buf = %s\n", path_buf);
//                fprintf(stderr, "length = %d\n", length(path_buf));

                FILE *f = fopen(name_buf, "r");
                if (f == NULL) {
                    mkdir(name_buf, 0777);
//                    fprintf(stderr, "agrc = %d\n", argc);

                } else {
                    int re;
                    if((re = fclose(f)) == EOF) return -1
                    ;
                }

                *(name_buf + j) = '/';
//                fprintf(stderr, "name_buf = %s\n", name_buf);

                i++;
                j++;
//                fprintf(stderr, "i = %d\n", i);
//                fprintf(stderr, "j = %d\n", j);
            }
        } else if (*(path_buf + 0) != '.' && *(path_buf + 0) != '/') {
            i = 0;
            int j = 0;
            while (i < length(path_buf)) {
                while (*(path_buf + i) != '/' && i <= length(path_buf) - 1) {
                    *(name_buf + j) = *(path_buf + i);
                    i++;
                    j++;
                }
//                fprintf(stderr, "i = %d\n", i);
//                fprintf(stderr, "j = %d\n", j);
//                fprintf(stderr, "agrc = %d\n", argc);
//                fprintf(stderr, "agrv = %p\n", *argv);
//                fprintf(stderr, "name_buf = %s\n", name_buf);
//                fprintf(stderr, "path_buf = %s\n", path_buf);
//                fprintf(stderr, "length = %d\n", length(path_buf));

                FILE *f = fopen(name_buf, "r");
                if (f == NULL) {
                    mkdir(name_buf, 0777);
//                    fprintf(stderr, "agrc = %d\n", argc);

                } else {
                    int re;
                    if((re = fclose(f)) == EOF) return -1;
                }

                *(name_buf + j) = '/';
//                fprintf(stderr, "name_buf = %s\n", name_buf);

                i++;
                j++;
//                fprintf(stderr, "i = %d\n", i);
//                fprintf(stderr, "j = %d\n", j);

            }
        }
    }

    if(s_flag == 1 && h_flag && h_flag != 1){
        global_options = 0x0;
        return -1;
    }

    if(d_flag == 1 && h_flag && h_flag != 1){
        global_options = 0x0;
        return -1;
    }

    if((d_flag == 1 && p_flag ==0 && argc > 2) || (s_flag == 1 && p_flag ==0 && argc > 2)){
        global_options = 0x0;
        return -1;
    }

    if(d_flag == 1 && c_flag && p_flag == 0 && argc >3){
        global_options = 0x0;
        return -1;
    }

    if(d_flag == 1 && p_flag && c_flag && p_flag < argc && c_flag < argc){

        global_options =0xc;
        return 0;
    }

    //if -s is given global_options is set to be 0x2
    if(s_flag == 1) {
        global_options = 0x2;
        return 0;
    }

    if(d_flag == 1) {
        global_options = 0x4;
        return 0;
    }

    return 0;
}


// check if two element are equal, if equal return 1 else return 0
int stringCompare(char *first, char *second){

    int i = 0;

    while(*(first+ i) != '\0' || *(second+ i) != '\0'){


        if(*(first+ i) != *(second + i)) return 0;
        i++;
    }
    return 1;
}

int stringCompare2(unsigned char *first, char *second){

    int i = 0;

    while(*(first+ i) != '\0' || *(second+ i) != '\0'){


        if(*(first+ i) != *(second + i)) return 0;
        i++;
    }
    return 1;
}

int stringCompare3(unsigned char *first, unsigned char *second){

    int i = 0;

    while(*(first+ i) != '\0' || *(second+ i) != '\0'){


        if(*(first+ i) != *(second + i)) return 0;
        i++;
    }
    return 1;
}

int stringCompare1(char first, char *second){

    int i = 0;

    while((first+ i) != '\0' || *(second+ i) != '\0'){

        if((first+ i) != *(second + i)) return 0;
        i++;
    }
    return 1;
}

int length(char * name){
    int len = 0;
    while(*(name + len)){

        len++;
    }

    return len;
}

//compare n elements from first string with second string
//if equal return 1 else return 0
int stringNCompare(char * first, char* second, size_t n){

    while(n--){

        if(first == NULL || *first != *second) return 0;

        first++;
        second++;
    }
    return 1;
}

//copy all the elements in the second string to the first one
char *stringCopy(char * first , char * second){

    char * result =first;
    while((*first++ = *second++) != '\0');
    return result;
}

//copy n elements from the second string to the first one
char *stringNCopy(char * first, char * second, size_t n){

    char * result = first;
    int i = 0;
    while(i++ < n && (*first++ = *second++) != '\0');

    if(*(first) != '\0') *first = '\0';
    return result;
}

int push_dir(char *name) {
    // To be implemented.
    int i = 0;

    while(*(path_buf+i) != '\0'){

        i++;
    }
    int j = 0;
    *(path_buf + i++) = '/';
    while(*(name+ j) != '\0'){

        *(path_buf + i++) = *(name + j++);
    }

//    *(path_buf + i++) = '/';
    *(path_buf + i) ='\0';
    path_length = i-1;

    if(path_length > PATH_MAX){

        return -1;
    }
    return 0;
}

int pop_dir() {
    // To be implemented.
    int i = 0;
    int index =0;
    while(*(path_buf + i) != '\0'){

        if(*(path_buf + i) == '/'){

            index = '/';
        }

        i++;
    }
    i = 0;
    while(*(path_buf + i) != '\0'){

        if(i == index){

            *(path_buf + i) = '\0';
        }

        if(i > index){

            *(path_buf + i) = '0';
        }

        i++;
    }

    i = 0;
    index =0;
    while(*(path_buf + i) != '\0'){

        if(*(path_buf + i) == '/'){

            index = i;
        }

        i++;
    }
    i = 0;
    while(*(path_buf + i) != '\0'){

        if(i == index){

            *(path_buf + i) = '\0';
        }

        if(i > index){

            *(path_buf + i) = '0';
        }

        i++;
    }

    path_length = index;

    if(path_length>PATH_MAX) return -1;
    return 0;
}

void name_clear(char *name){
    int i = 0;
    while(*(name+i) != '\0'){

        *(name + i) = '0';
        i++;
    }
    *(name+i) = '\0';
}