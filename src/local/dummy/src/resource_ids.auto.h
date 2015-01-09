/*
Because of some reasons (perhaps SDL?) changing the entry point creates crashes,
but because pebble.h includes this file and all your pebble apps include pebble.h
I change the name of your "main" function to "pbl_main"
*/
#ifdef main
#undef main
#endif

#define main pbl_main
