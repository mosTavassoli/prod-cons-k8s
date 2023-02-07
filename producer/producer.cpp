#include "etcdAPIs.hpp"

using namespace std;

/* CRUD: Create, Read, Update, Delete operations test */

int main(int argc, char *argv[])
{
  std::string tokens[10];
  std::string arg1 = argv[1];
  unsigned int size = 0;
  bool strong_consistency = true;
  KV get_kv;
  
  //std::string cmd = defineCmd(0);
  std::cout << "THIS is the IP " << arg1 << std::endl;
  // Put key test
  KV new_kv;
  new_kv.key = "hello";
  new_kv.value = "Hello_MOSI_JOON";
	
  std::string cmd;
  cmd = "etcdctl --endpoints="+arg1+":2379";
  
  //std::cout << cmd << std::endl;
   for (int i = 1; i <= 5; ++i)
  {
    new_kv.key = "hello" + std::to_string(i);
    new_kv.value = "Hello_MOSI_JOON" + std::to_string(i);
    etcdPut(cmd, new_kv);
  }
  //etcdPut(cmd, new_kv);
  //sleep(50);
  return 0;
}
