#pragma once
#include <string>
#include <vector>
#include <stdexcept>
#include <cctype>
namespace portsmith {
struct Token { std::string text; std::size_t offset,line,column; };
struct Finding { std::string rule; std::size_t offset,line,column; };
inline std::vector<Token> lex(const std::string& s) {
 if(s.size()>1024*1024) throw std::runtime_error("source exceeds 1 MiB");
 if(s.find("\\\n")!=std::string::npos||s.find("\\\r\n")!=std::string::npos) throw std::runtime_error("line splicing unsupported; normalize translation phase two first");
 std::vector<Token> out; std::size_t i=0,line=1,col=1;
 auto step=[&](){if(s[i++]=='\n'){++line;col=1;}else ++col;};
 auto advance=[&](std::size_t end){while(i<end)step();};
 while(i<s.size()) {
  if(std::isspace(static_cast<unsigned char>(s[i]))){step();continue;}
  if(s.compare(i,2,"//")==0){while(i<s.size()&&s[i]!='\n')step();continue;}
  if(s.compare(i,2,"/*")==0){auto end=s.find("*/",i+2);if(end==std::string::npos)throw std::runtime_error("unterminated comment");advance(end+2);continue;}
  // Recognize all standard string prefixes before ordinary identifier tokenization.
  std::size_t start=i,quote=i; bool raw=false,quoted=false;
  for(const auto& prefix:std::vector<std::string>{"u8R\"","uR\"","UR\"","LR\"","R\"","u8\"","u\"","U\"","L\"","\"","u8'","u'","U'","L'","'"}) {
   if(s.compare(i,prefix.size(),prefix)==0){quote=i+prefix.size()-1;raw=prefix.find('R')!=std::string::npos;quoted=true;break;}
  }
  if(quoted){
   if(raw){auto open=s.find('(',quote+1);if(open==std::string::npos||open-quote-1>16)throw std::runtime_error("invalid raw delimiter");
    auto delim=s.substr(quote+1,open-quote-1);for(unsigned char c:delim)if(c<=32||c=='\\'||c==')')throw std::runtime_error("invalid raw delimiter");
    auto close=")"+delim+"\"";auto end=s.find(close,open+1);if(end==std::string::npos)throw std::runtime_error("unterminated raw string");advance(end+close.size());
   }else{char q=s[quote];advance(quote+1);bool closed=false;while(i<s.size()){if(s[i]=='\\'){step();if(i==s.size())throw std::runtime_error("dangling escape");step();}else if(s[i]==q){step();closed=true;break;}else if(s[i]=='\n'||s[i]=='\r')throw std::runtime_error("newline in quoted literal");else step();}if(!closed)throw std::runtime_error("unterminated quoted literal");}
   continue;
  }
  auto atline=line,atcol=col;
  if(std::isalnum(static_cast<unsigned char>(s[i]))||s[i]=='_'){step();while(i<s.size()&&(std::isalnum(static_cast<unsigned char>(s[i]))||s[i]=='_'))step();}
  else {step();if(i<s.size()&&((s[start]=='>'&&s[i]=='>')||(s[start]=='<'&&s[i]=='<')))step();}
  out.push_back({s.substr(start,i-start),start,atline,atcol});
 }
 return out;
}
inline std::vector<Finding> scan(const std::string& s){
 auto t=lex(s);std::vector<Finding> f;
 for(std::size_t i=0;i<t.size();++i){std::string rule;
  if(t[i].text=="__CUDA_ARCH__")rule="nvidia-architecture-guard";
  else if((t[i].text=="asm"||t[i].text=="__asm__")&&i+1<t.size()&&(t[i+1].text=="("||t[i+1].text=="volatile"))rule="inline-assembly-review";
  else if(i>0&&t[i].text=="32"&&(t[i-1].text=="/"||t[i-1].text=="%"))rule="hardcoded-warp-division";
  else if(i>0&&t[i].text=="31"&&t[i-1].text=="&")rule="hardcoded-warp-mask";
  else if(i>0&&t[i].text=="5"&&t[i-1].text==">>")rule="hardcoded-warp-shift";
  if(!rule.empty())f.push_back({rule,t[i].offset,t[i].line,t[i].column});
 }return f;
}
inline std::string json(const std::vector<Finding>& f){std::string s="[";for(std::size_t i=0;i<f.size();++i){if(i)s+=",";const auto& x=f[i];s+="{\"rule\":\""+x.rule+"\",\"offset\":"+std::to_string(x.offset)+",\"line\":"+std::to_string(x.line)+",\"column\":"+std::to_string(x.column)+"}";}return s+"]";}
}
