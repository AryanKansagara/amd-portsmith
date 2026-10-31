#include "portsmith.hpp"
#include <iostream>
#include <functional>
int main(){int groups=0,checks=0;auto check=[&](bool b){++checks;if(!b)throw std::runtime_error("assertion failed");};auto test=[&](const char*name,const std::function<void()>&f){f();++groups;std::cout<<"PASS "<<name<<'\n';};auto rejects=[&](const std::string&s){try{portsmith::scan(s);}catch(const std::runtime_error&){return true;}return false;};
try{
test("five explicit hazard rules",[&]{auto f=portsmith::scan("x/32; x%32; x&31; x>>5; __CUDA_ARCH__; asm(\"ptx\");");check(f.size()==6);check(f[0].rule=="hardcoded-warp-division");check(f[2].rule=="hardcoded-warp-mask");check(f[3].rule=="hardcoded-warp-shift");});
test("comments strings chars excluded",[&]{check(portsmith::scan("// __CUDA_ARCH__ /32\n/* asm( */ const char*s=\"x&31 asm(\"; char c='5';").empty());});
test("raw string prefixes excluded",[&]{for(auto prefix:{"R","u8R","uR","UR","LR"})check(portsmith::scan(std::string(prefix)+"\"tag(__CUDA_ARCH__ x/32 \\\" )tag\";").empty());});
test("escaped quotes and comment markers",[&]{check(portsmith::scan("\"a\\\" __CUDA_ARCH__ // /*\" x/32").size()==1);check(portsmith::scan("'\\\'' x&31").size()==1);});
test("exact byte line column CRLF",[&]{std::string s="// ignored\r\n\tint x = a / 32;\n__CUDA_ARCH__";auto f=portsmith::scan(s);check(f.size()==2);check(f[0].offset==s.find("32"));check(f[0].line==2&&f[0].column==14);check(f[1].line==3&&f[1].column==1);});
test("lookalike identifiers numbers operators",[&]{check(portsmith::scan("my__CUDA_ARCH__ x/320 x&310 x>>50; int asm_name; int a=32;").empty());});
test("comments between tokens",[&]{auto f=portsmith::scan("x / /* why */ 32; asm /*x*/ volatile(\"ptx\");");check(f.size()==2);});
test("ordinary prefixed literals",[&]{for(auto prefix:{"u8","u","U","L"})check(portsmith::scan(std::string(prefix)+"\"__CUDA_ARCH__\"").empty());});
test("unterminated constructs fail closed",[&]{for(auto s:{"/*","\"x","'x","R\"tag(x","\"x\n\"","\"x\\"})check(rejects(s));});
test("invalid raw delimiter rejected",[&]{check(rejects("R\"bad delim(x)bad delim\""));check(rejects("R\"12345678901234567(x)12345678901234567\""));});
test("deterministic offsets across whitespace",[&]{for(int n=0;n<80;++n){auto s=std::string(n,' ') +"x/32";auto f=portsmith::scan(s);check(f.size()==1&&f[0].offset==static_cast<std::size_t>(n+2)&&f[0].column==static_cast<std::size_t>(n+3));}});
test("bounds empty and portable warp variable",[&]{check(portsmith::scan("").empty());check(portsmith::scan("x / warpSize; x % warpSize;").empty());check(rejects(std::string(1024*1024+1,' ')));check(rejects("// hidden \\\n x/32"));});
std::cout<<"SUMMARY groups="<<groups<<" assertions="<<checks<<'\n';return 0;}catch(const std::exception&e){std::cerr<<"FAIL "<<e.what()<<'\n';return 1;}}
