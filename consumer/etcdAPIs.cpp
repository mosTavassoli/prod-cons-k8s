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
    //cmd = "etcdctl --endpoints=10.254.2.87:2379";

  return cmd;
}

unsigned int parseString (std::string s, std::string* output, std::string delimiter/*="\n"*/, bool stdoutEn/*=false*/) {
  size_t pos = 0;
  unsigned int rows = 0;
  std::string token;
  while ((pos = s.find(delimiter)) != std::string::npos) {
      token = s.substr(0, pos);
      output[rows++] = token;
      if(stdoutEn) std::cout << token << std::endl;
      s.erase(0, pos + delimiter.length());
  }
  output[rows++] = s;
  if(stdoutEn) std::cout << s << std::endl;

  return rows;
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
    //std::cout << "Get error! Output: " << output << " Size: " << rows << std::endl;
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
  std::cout << "Command is " <<  output << std::endl; 
  if (strcmp(output.c_str(), "OK\n") == 0)
     return true;
   else
   {
     std::cout << "Error putting key! Output: " << output << std::endl;
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
