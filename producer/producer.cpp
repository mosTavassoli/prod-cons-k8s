#include "etcdAPIs.hpp"

using namespace std;

/* CRUD: Create, Read, Update, Delete operations test */

int main(int argc, char *argv[])
{
  std::string arg1 = argv[1];
  unsigned int size = 0;
  unsigned int keys = 0;
  KV get_kv;
  unsigned int i = 0;
  unsigned int max = std::numeric_limits<unsigned int>::max();
  // std::string cmd = defineCmd(0);
  KV new_kv;
  new_kv.key = "key";
  std::string cmd;
  cmd = "etcdctl --endpoints=" + arg1 + ":2379";

  // std::cout << cmd << std::endl;

  auto start = std::chrono::high_resolution_clock::now();
  auto end = start + std::chrono::seconds(1);

  while (true)
  {
    new_kv.value = "key-" + std::to_string(i);
    etcdPut(cmd, new_kv);
    if (std::chrono::high_resolution_clock::now() >= end)
    {
      keys = i;
      break;
    }
    i++;
  }

  std::cout << "keys/sec: " << keys << std::endl;

  while (true)
  {
    if (i == max)
      i = 0;
    new_kv.value = "key-" + std::to_string(i);
    etcdPut(cmd, new_kv);
    i++;
  }

  return 0;
}
