#include "cvcore.h"
#include "cvanalyzer.h"
#include <fstream>
#include <iostream>
#include <iterator>
using namespace cvanalyzer;
int main(int argc,char**argv){
  std::ifstream f(argv[1],std::ios::binary);
  cvcore::Bytes b((std::istreambuf_iterator<char>(f)),std::istreambuf_iterator<char>());
  std::string t,e;
  if(!cvcore::extraireTexteCv(b,t,e)){std::cout<<"ERREUR: "<<e<<"\n";return 1;}
  CvInfo i=analyserTexte(t,2026,11);
  std::cout<<"nom="<<i.nom<<" | prenom="<<i.prenom<<" incertain="<<i.nomIncertain<<"\nemail="<<i.email<<" tel="<<i.telephone
   <<"\nformation="<<i.formation<<"\nexp="<<i.experienceAns<<" ("<<i.experienceSource<<")\nspec="<<i.specialite<<"\ncomp=";
  for(auto&s:i.competences)std::cout<<s<<"; ";std::cout<<"\ncert=";for(auto&s:i.certifications)std::cout<<s<<"; ";std::cout<<"\n";
  std::vector<Critere> cr={{TypeCritere::Formation,"Informatique",true,true},{TypeCritere::ExperienceMin,"3",true,true},
   {TypeCritere::Specialite,"Maintenance",true,true},{TypeCritere::Competence,"C++, SQL, Oracle",false,true},{TypeCritere::Certification,"Cisco",false,true}};
  Evaluation ev=evaluer(i,t,cr);
  for(auto&r:ev.resultats)std::cout<<"  ["<<libelleVerdict(r.verdict)<<"] "<<libelleType(r.critere.type)<<" "<<r.critere.valeur<<" -> "<<r.detail<<"\n";
  std::cout<<"pct="<<ev.pourcentage<<" conforme="<<ev.conforme<<" : "<<ev.raison<<"\n";
}
