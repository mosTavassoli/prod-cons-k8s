#ifndef MYETCDAPIS_H
#define MYETCDAPIS_H

#include <iostream>  //stdin stdout
#include <stdlib.h>  //standard types and macros
#include <string.h>  //strings
#include <sstream>   //string streams
#include <algorithm> //all_of isnum check
#include <vector>    //IPs for defrag
#include <limits>
#include <chrono>
#include <fstream>

typedef struct
{
  std::string key;
  std::string value;
} KV;

std::string callToSystem(std::string cmd);

std::string defineCmd(int pod_idx = -1, bool rbac = false);

unsigned int parseString (std::string s, std::string* output, std::string delimiter="\n", bool stdoutEn=false);

KV etcdGet(std::string cmd, std::string key, bool serializable = false);

bool etcdPut(std::string cmd, KV kv_t);

std::string randStr(const int len);

#endif
