#include <fstream>
#include <cstdio>
#include <cstring>

namespace{
struct Args{
  int mode;
  char const* src_file_name;
  char const* out_file_name;
};
}

static Args
parse_args(int argc, char const** argv){
  if(argc < 2){
    fprintf(stderr, "Not enough arguments.");
    exit(1);
  }

  char const* src_file_name = argv[1];
  int mode = 0;
  char const* out_file_name = 0;
  
  for(int i = 2; i < argc; ++i)
    if(strcmp(argv[i], "--lex") == 0)
      mode |= 0x1;
    else if(strcmp(argv[i], "--parse") == 0) 
      mode |= (0x1 << 1);
    else if(strcmp(argv[i], "--codegen") == 0)
      mode |= (0x1 << 2);
    else if(strcmp(argv[i], "-o") == 0){
      if(i == argc - 1){
        fprintf(stderr, "No argument for -o.");
        exit(1);
      }
      out_file_name = argv[i + 1];
      ++i;
    }
    else{
      fprintf(stderr, "Unrecognized argument %s", argv[i]);
      exit(1);
    }
  
  return Args{mode, src_file_name, out_file_name};
}

int
main(int argc, char const** argv){
  Args args = parse_args(argc, argv);
}