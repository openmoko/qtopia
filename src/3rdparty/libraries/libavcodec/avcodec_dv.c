/*
  This is a hack to rename the outputted object file to avcodec_dv.o instead of
  dv.o which will overwrite the dv.o compiled from the libav directory.

  The reason this happens is because Qtopia's make system compiles all the files
  to make a given component in to the one .obj directory.

  Because this is 3rd party code, it is not so easy to rename the file and keep in
  sync with the 3rdparty code, but with this hack, it simply bypasses the problem
  and the .o files no longer overlap and the orginal .c file can be updated.
*/

#include "dv.c"

