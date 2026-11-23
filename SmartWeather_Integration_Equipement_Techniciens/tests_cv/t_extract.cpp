#include "cvcore.h"
#include <fstream>
#include <iostream>
#include <iterator>
int main(int argc,char**argv){
  std::ifstream f(argv[1],std::ios::binary);
  cvcore::Bytes b((std::istreambuf_iterator<char>(f)),std::istreambuf_iterator<char>());
  std::string t,e;
  bool ok=cvcore::extraireTexteCv(b,t,e);
  if(!ok){std::cout<<"ERREUR: "<<e<<"\n";return 1;}
  std::cout<<t;
}
