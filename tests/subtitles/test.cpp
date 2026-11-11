#include "subtitles.h"
#include <cstdio>
#include <fstream>
#include <sstream>
#include <iostream>
using namespace pplay;
static std::string rd(const char*p){std::ifstream f(p,std::ios::binary);std::stringstream s;s<<f.rdbuf();return s.str();}
int main(int argc,char**argv){
  int fail=0;
  for(int i=1;i+1<argc;i+=3){
    std::string d=rd(argv[i]); auto e=Subtitles::detect(d);
    std::string conv; bool ok=Subtitles::toUtf8(d,e,conv);
    std::string want=rd(argv[i+1]);
    bool pass = std::string(Subtitles::name(e))==argv[i+2] && (e==Subtitles::Encoding::Utf8 ? true : (ok && conv==want));
    printf("%-22s detected=%-8s expect=%-8s %s\n",argv[i],Subtitles::name(e),argv[i+2],pass?"OK":"FAIL"); fail|=!pass;
  }
  return fail;
}
