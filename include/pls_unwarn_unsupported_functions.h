/* This file unwarns all functions that are unsupported by the pebble sdk 
 * In addition to that, we all hope that the clib did not defined these functions as macros :(
 */

#undef printf
#undef fopen
#undef fclose
#undef fread
#undef fwrite
#undef fseek
#undef ftell
#undef fsetpos
#undef fscanf
#undef fgetc
#undef fgets
#undef fputc
#undef fputs
#undef fprintf
#undef sprintf
#undef vfprintf
#undef vsprintf
#undef vsnprintf
#undef open
#undef close
#undef creat
#undef read
#undef write
#undef stat
#undef alloca
#undef mmap
#undef brk
#undef sbrk

