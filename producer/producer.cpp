#include "etcdAPIs.hpp"

using namespace std;

/* CRUD: Create, Read, Update, Delete operations test */

int main(int argc, char *argv[])
{
  unsigned int size = 0;
  unsigned int keys = 0;
  bool strong_consistency = true;
  int pod_idx = std::stoi(argv[1]);
  KV get_kv;
  unsigned int i=0;
  unsigned int max = std::numeric_limits<unsigned int>::max();
  std::string cmd = defineCmd(pod_idx);
  KV new_kv;
  new_kv.key = "key";

  auto start = std::chrono::high_resolution_clock::now();
  auto end = start + std::chrono::seconds(1);

  while (true)
  {
    new_kv.value = "key-" + std::to_string(i);
    etcdPut(cmd, new_kv);
    i++;
    if ( std::chrono::high_resolution_clock::now() >= end )
    {
      keys = i;
      break;
    }
  }

  std::cout << "keys/sec: " << keys << std::endl;

  while(true)
  {
    if (i == max)
      i = 0;
    new_kv.value = "key-" + std::to_string(i);
    etcdPut(cmd, new_kv);
    i++;
  }
  return 0;
}
