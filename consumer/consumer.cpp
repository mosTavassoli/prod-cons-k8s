#include "etcdAPIs.hpp"

using namespace std;

/* CRUD: Create, Read, Update, Delete operations test */

int main(int argc, char *argv[])
{
  unsigned int i = 0;
  int get_s = 0;
  int get_l = 0;
  std::string pod_ip = argv[1];
  KV get_kv;

  // std::string cmd = defineCmd();
  std::string cmd;

  auto start = std::chrono::high_resolution_clock::now();
  auto end = start + std::chrono::seconds(1);
  // Reading test linearizable
  while (true)
  {
    get_kv = etcdGet(cmd, "key");
    if (std::chrono::high_resolution_clock::now() >= end)
    {
      get_l = i;
      break;
    }
    i++;
  }
  std::cout << "Get_L keys/sec: " << get_l << std::endl;
  i = 0;
  start = std::chrono::high_resolution_clock::now();
  end = start + std::chrono::seconds(1);
  // Reading test serializable
  while (true)
  {
    get_kv = etcdGet(cmd, "key", true);
    if (std::chrono::high_resolution_clock::now() >= end)
    {
      get_s = i;
      break;
    }
    i++;
  }
  std::cout << "Get_S keys/sec: " << get_s << std::endl;
  return 0;
}
