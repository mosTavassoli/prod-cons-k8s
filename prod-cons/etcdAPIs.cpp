#include "etcdAPIs.hpp"
#define K8S 1

using namespace std;

std::string callToSystem(std::string cmd)
{
  std::ostringstream output;
  FILE *fpipe;
  char c = 0;

  if (0 == (fpipe = (FILE *)popen(cmd.c_str(), "r")))
  {
    perror("popen() failed.");
    exit(EXIT_FAILURE);
  }

  while (fread(&c, sizeof c, 1, fpipe))
  {
    output << c;
  }

  pclose(fpipe);

  return output.str();
} // leveraging pipes to redirect output to string

std::string defineCmd(int pod_idx /*=-1*/, bool rbac /*=false*/)
{
  std::string cmd;

  if (K8S == 1)
  {
    std::string cmd_tmp1 = "etcdctl --endpoints=";
    std::string cmd_tmp2 = ":2379";

    if (pod_idx == -1)
    {
      std::string clusterIP = callToSystem("kubectl get svc my-etcd --template={{.spec.clusterIP}}");
      cmd = cmd_tmp1 + clusterIP + cmd_tmp2;
      std::cout << cmd << std::endl;
    }
    else
    {
      std::cout << "I am here " << std::endl;
      std::string podIP_cmd = "kubectl get pod my-etcd-" + std::to_string(pod_idx) + " --template={{.status.podIP}}";
      std::string podIP = callToSystem(podIP_cmd);
      cmd = cmd_tmp1 + podIP + cmd_tmp2;
    }

    if (rbac)
    {
      std::string rootPwd = callToSystem("kubectl get secret --namespace default my-etcd -o jsonpath=\"{.data.etcd-root-password}\" | base64 -d");
      std::string cmd_tmp3 = " --user root:";
      cmd = cmd + cmd_tmp3 + rootPwd;
    }
  }

  else
    cmd = "etcdctl";
  // cmd = "etcdctl --endpoints=10.254.2.87:2379";

  return cmd;
}

unsigned int parseString(std::string s, std::string *output, std::string delimiter /*="\n"*/, bool stdoutEn /*=false*/)
{
  size_t pos = 0;
  unsigned int rows = 0;
  std::string token;
  while ((pos = s.find(delimiter)) != std::string::npos)
  {
    token = s.substr(0, pos);
    output[rows++] = token;
    if (stdoutEn)
      std::cout << token << std::endl;
    s.erase(0, pos + delimiter.length());
  }
  output[rows++] = s;
  if (stdoutEn)
    std::cout << s << std::endl;

  return rows;
}

std::vector<std::string> parseStringVector(std::string s, std::string delimiter /*="\n"*/, bool stdoutEn /*=false*/)
{
  size_t pos = 0;
  std::string token;
  std::vector<string> output;
  while ((pos = s.find(delimiter)) != std::string::npos)
  {
    token = s.substr(0, pos);
    output.push_back(token);
    if (stdoutEn)
      std::cout << token << std::endl;
    s.erase(0, pos + delimiter.length());
  }
  output.push_back(s);
  if (stdoutEn)
    std::cout << s << std::endl;

  return output;
}

std::vector<std::string> getPodIPs()
{
  std::string podIPs_output = callToSystem("kubectl get pods --selector app.kubernetes.io/instance=my-etcd -o json | egrep '\"podIP\"' | grep -E -o \"([0-9]{1,3}[\\.]){3}[0-9]{1,3}\"");
  std::vector<string> podIPs = parseStringVector(podIPs_output);
  podIPs.pop_back();
  return podIPs;
}

int findLeader()
{
  std::string member_id;
  std::string leader_id;
  int leader_idx = -2;
  std::vector<std::string> podIPs = getPodIPs();

  if (podIPs.size() == 1)
    return -1; // Only one etcd instance

  for (std::vector<std::string>::iterator it = podIPs.begin(); it != podIPs.end(); ++it)
  {
    member_id = callToSystem("etcdctl --endpoints=" + *it + ":2379 endpoint status -w fields | grep MemberID | grep -Eo '[0-9]{1,30}'");
    leader_id = callToSystem("etcdctl --endpoints=" + *it + ":2379 endpoint status -w fields | grep Leader | grep -Eo '[0-9]{1,30}'");

    if (strcmp(member_id.c_str(), leader_id.c_str()) == 0)
    {
      leader_idx = stoi(callToSystem("kubectl get pods --selector app.kubernetes.io/instance=my-etcd -o wide | grep " + *it + " | awk '{print $1}' | grep -Eo '[0-9]{1,2}'"));
      // std::cout << "Leader is: " << callToSystem("kubectl get pods --selector app.kubernetes.io/instance=my-etcd -o wide | grep "+ *it +" | awk '{print $1}'");
    }
  }

  return leader_idx;
}

bool moveEtcdLeader(int src_idx, int dst_idx)
{
  std::string leader_ip = callToSystem("kubectl get pods --field-selector metadata.name=my-etcd-" + std::to_string(src_idx) + " -o json | egrep '\"podIP\"' | grep -E -o \"([0-9]{1,3}[\\.]){3}[0-9]{1,3}\""); // find the IP of the current leader
  leader_ip.pop_back();                                                                                                                                                                                        // removing carriage return

  std::string dst_ip = callToSystem("kubectl get pods --field-selector metadata.name=my-etcd-" + std::to_string(dst_idx) + " -o json | egrep '\"podIP\"' | grep -E -o \"([0-9]{1,3}[\\.]){3}[0-9]{1,3}\""); // find the IP of the new leader
  dst_ip.pop_back();                                                                                                                                                                                        // removing carriage return

  std::string src_member_id = callToSystem("etcdctl --endpoints=" + leader_ip + ":2379 endpoint status | awk '{print $2}' | sed -e $'s/,//g'"); // find the member id of the current leader
  src_member_id.pop_back();                                                                                                                     // removing carriage return

  std::string dst_member_id = callToSystem("etcdctl --endpoints=" + dst_ip + ":2379 endpoint status | awk '{print $2}' | sed -e $'s/,//g'"); // find the member id of the new leader
  dst_member_id.pop_back();                                                                                                                  // removing carriage return

  // Leadership change request must be sent to the current leader directly
  std::string output = callToSystem("etcdctl --endpoints=" + leader_ip + ":2379 move-leader " + dst_member_id);
  // std::cout << output << std::endl;

  std::string expected_output = "Leadership transferred from " + src_member_id + " to " + dst_member_id + "\n";
  if (strcmp(output.c_str(), expected_output.c_str()) == 0)
  {
    // std::cout << "OK!" << std::endl;
    return true;
  }

  return false;
}

KV etcdGet(std::string cmd, std::string key, bool serializable /*=false*/)
{
  // std::string cmd = defineCmd(); time consuming
  std::string get_cmd = "get";
  std::string consistency_opt;
  std::string limit_opt = "--limit=1";
  std::string output;
  std::string tokens[5];
  unsigned int rows = 0;
  KV kv_t;

  // setting consistency level linearizable vs serializable
  if (serializable)
    consistency_opt = "--consistency=\"s\"";
  else
    consistency_opt = "--consistency=\"l\"";

  // executing command, getting output as string and parsing it
  output = callToSystem(cmd + " " + get_cmd + " " + key + " " + consistency_opt + " " + limit_opt);
  std::cout << output << std::endl;
  rows = parseString(output, tokens);
  if (rows == 3)
  {
    kv_t.key = tokens[0];
    kv_t.value = tokens[1];
  }
  else
  {
    // std::cout << "Get error! Output: " << output << " Size: " << rows << std::endl;
    kv_t.key = "Error";
    kv_t.value = "Error";
  }

  return kv_t;
}

bool etcdPut(std::string cmd, KV kv_t)
{
  // std::string cmd = defineCmd(); time consuming
  std::string put_cmd = "put";
  std::string output;

  output = callToSystem(cmd + " " + put_cmd + " " + kv_t.key + " " + kv_t.value);
  std::cout << "Command is " << output << std::endl;
  if (strcmp(output.c_str(), "OK\n") == 0)
    return true;
  else
  {
    std::cout << "Error putting key! Output: " << output << std::endl;
    return false;
  }
}

bool etcdRst(std::string cmd)
{
  // std::string cmd = defineCmd(); time consuming
  std::string del_cmd = "del / --prefix";
  std::string output;

  output = callToSystem(cmd + " " + del_cmd);
  if (!output.empty() && std::all_of(output.begin(), output.end() - 1, ::isdigit))
  {
    return true;
  }
  else
  {
    std::cout << "Error removing all keys! Output: " << output << std::endl;
    return false;
  }
}

std::string randStr(const int len)
{
  static const char alphanum[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
  std::string tmp_s;
  tmp_s.reserve(len);

  for (int i = 0; i < len; ++i)
  {
    tmp_s += alphanum[rand() % (sizeof(alphanum) - 1)];
  }

  return tmp_s;
}
