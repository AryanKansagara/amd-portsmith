#include "portsmith.hpp"
#include <iostream>
#include <fstream>
#include <iterator>
int main(int argc,char**argv){try{if(argc==1||(argc==2&&std::string(argv[1])=="demo")){auto f=portsmith::scan("// x / 32\nint lane = x & 31;\n#if __CUDA_ARCH__ >= 800\nasm(\"mov.u32 %0, %laneid;\");\n#endif\n");std::cout<<"{\"kind\":\"native_lexical_fixture\",\"gpu_tested\":false,\"findings\":"<<portsmith::json(f)<<"}\n";return 0;}if(argc!=3||std::string(argv[1])!="scan")throw std::runtime_error("usage: portsmith [demo | scan FILE]");std::ifstream in(argv[2],std::ios::binary);if(!in)throw std::runtime_error("cannot open source");std::string s;char c;while(in.get(c)){if(s.size()==1024*1024)throw std::runtime_error("source exceeds 1 MiB");s+=c;}std::cout<<"{\"kind\":\"advisory_lexical_scan\",\"findings\":"<<portsmith::json(portsmith::scan(s))<<"}\n";return 0;}catch(const std::exception&e){std::cerr<<e.what()<<'\n';return 1;}}
