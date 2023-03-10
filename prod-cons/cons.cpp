#include "etcdAPIs.hpp"
#include <unistd.h> //getpid()
#include <math.h>   //pow()
#include <chrono>   //time measurement
#include <fstream>  //file management

using namespace std;

/* CRUD: Create, Read, Update, Delete operations test */

int main(int argc, char *argv[])
{
  std::vector<std::string> results; // vector to store results
  unsigned int i = 0;
  unsigned int counter = 0;
  int get_s = 0;
  int get_l = 0;
  KV get_kv;
  std::string cmd = defineCmd(-1);

  int leader_idx = -2;             // index of the replica acting as leader
  //bool reqToLeader = false;         // make request directly to the leader
  
  std::ofstream outfile;
  std::ostringstream filename;
  // filename << "/app/logs/throughput_put_"+std::to_string(num_keys)+"rep.csv";
  //  Write data to file, the file is located inside the container
  filename << "/app/logs/throughput_get_50rep.csv";

  // Find and change Raft leader
  leader_idx = findLeader();
  //int dst_leader_idx = -1;
  //double role_change_duration = -1;

//  if (leader_idx == -2)
 // {
 //   printf("Error finding leader!\n");
 //   exit(-5);
 // }

  // Change leader to make requests to follower/leader
 // if (leader_idx != -1)
  //{ // if only one replica, then no leader change
  //  leader_idx = findLeader();
  //  if (reqToLeader)
  //    moveEtcdLeader(leader_idx, 0); // since requests are toward my-etcd-0, idx 0 must be leader
  //  else
   //   moveEtcdLeader(leader_idx, 1); // since requests are toward my-etcd-0, idx 0 must be follower
 // }

  // Write data to file
  outfile.open(filename.str().c_str(), std::ios::out | std::ios::app);
  if (!outfile.is_open())
  {
    std::cerr << "Unable to open file" << std::endl;
    return 1;
  }
  cmd = defineCmd(leader_idx);
  auto start = std::chrono::high_resolution_clock::now();

  for (int i = 0; i < 50; i++)
  {
    start = std::chrono::high_resolution_clock::now();
    while (std::chrono::duration<double>(std::chrono::high_resolution_clock::now() - start).count() < 1.0)
    {
      get_kv = etcdGet(cmd, "key");
      counter++;
    }
    get_l = counter;
    counter = 0;
    start = std::chrono::high_resolution_clock::now();
    while (std::chrono::duration<double>(std::chrono::high_resolution_clock::now() - start).count() < 1.0)
    {
      get_kv = etcdGet(cmd, "key", true); // enable serializable mode
      counter++;
    }
    get_s = counter;
    counter = 0;
    results.push_back(std::to_string((unsigned)time(NULL)) + "," + std::to_string(get_l) + "," + std::to_string(get_s));
  }

  outfile << "id,get_l,get_s\n";
  for (i = 0; i < results.size(); i++)
  {
    outfile << results[i] << std::endl;
  }

  outfile.close();
  // check if the application is finished
  std::cout << "cons-done" << std::endl;

  while (true){}

  return 0;
}
