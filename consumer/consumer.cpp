#include "etcdAPIs.hpp"

using namespace std;

/* CRUD: Create, Read, Update, Delete operations test */

int main(int argc, char *argv[])
{
  unsigned int i = 0;
  //bool strong_consistency = true;
  int get_s = 0;
  int get_l = 0;
  //std::string pod_ip = argv[1];
  KV get_kv;
  int pod_idx = std::stoi(argv[1]);
  std::string cmd = defineCmd(pod_idx);
   //std::string cmd;
   //cmd = "etcdctl --endpoints="+pod_ip+":2379";
   // Get key test

  auto start = std::chrono::high_resolution_clock::now();
  auto end = start + std::chrono::seconds(1);
  //Reading test linearizable
  while(true)
  {
    get_kv = etcdGet(cmd, "key");
    i++;
    if ( std::chrono::high_resolution_clock::now() >= end )
    {
     get_l = i;
     break;
    }
    std::cout << "key: " << get_kv.key << " value: " << get_kv.value << endl;
  }
  std::cout << "Get_L keys/sec: " << get_l << std::endl;

  i=0;
  start = std::chrono::high_resolution_clock::now();
  end = start + std::chrono::seconds(1);
  //Reading test serializable
  while(true)
  {
    get_kv = etcdGet(cmd, "key", true);
    i++;
    if ( std::chrono::high_resolution_clock::now() >= end )
    {
     get_s = i;
     break;
    }
    std::cout << "key: " << get_kv.key << " value: " << get_kv.value << endl;
  }
  std::cout << "Get_S keys/sec: " << get_s << std::endl;

  while(true){
   get_kv = etcdGet(cmd, "key");
  }
  return 0;
}
