/*****************************************************************************
 * Licensed to Qualys, Inc. (QUALYS) under one or more
 * contributor license agreements. See the NOTICE file distributed with
 * this work for additional information regarding copyright ownership.
 * QUALYS licenses this file to You under the Apache License, Version 2.0
 * (the "License"); you may not use this file except in compliance with
 * the License. You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 * *****************************************************************************/

/*****
 * Author: Sergey Shekyan sshekyan@qualys.com
 *
 * Slow HTTP attack  vulnerability test tool
 *  http://code.google.com/p/slowhttptest/
 *****/
#include "config.h"
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <memory>

#include "slowlog.h"
#include "slowhttptest.h"

static void usage() {
  printf(
      "%s v.%s, a tool to test for slow HTTP "
      "DoS vulnerabilities.\n"
      "Usage:\n"
      "slowtest [-v <verbosity level>] [-<h|b>] [-u <URL>] "
      "[-c <number of connections>] [-i <interval in seconds>] "
      "[-r <connections per second>] [-l <test duration in seconds>]\n"
      "Options:\n\t"
      "-c,          target number of connections\n\t"
      "-h or -p,    specifies test mode(either slow headers or POST)\n\t"
      "-i,          interval between followup data in seconds\n\t"
      "-l,          target test length in seconds\n\t"
      "-r,          connections rate(connections per seconds)\n\t"
      "-u,          absolute URL to target, e.g http(s)://foo/bar\n\t"
      "-v,          verbosity level 0-4: Fatal, Info, Error, Warning, Debug\n"
      , PACKAGE
      , VERSION
      );
}


using slowhttptest::slowlog_init;
using slowhttptest::slowlog;
using slowhttptest::SlowHTTPTest;
using slowhttptest::SlowTestType;

int main(int argc, char **argv) {

  if (argc < 3) {
    usage();
    return -1;
  }

  char url[1024] = { 0 };
  int conn_cnt = 100;
  int delay = 100;
  int duration = 300;
  int interval = 10;
  long tmp;
  int debug_level = LOG_INFO;
  SlowTestType type = slowhttptest::eHeader;
  char o;
  while((o = getopt(argc, argv, ":hpc:i:l:r:u:v:")) != -1) {
    switch (o) {
    case 'c':
      tmp = strtol(optarg, 0, 10);
      if(tmp && tmp <= INT_MAX) {
        conn_cnt = static_cast<int>(tmp);
      }
      else {
        usage();
        return -1;
      }
      break;
    case 'h':
      type = slowhttptest::eHeader;
      break;
    case 'i':
      tmp = strtol(optarg, 0, 10);
      if(tmp && tmp <= INT_MAX) {
        interval = static_cast<int>(tmp);
      } else {
        usage();
        return -1;
      }
      break;
    case 'l':
      tmp = strtol(optarg, 0, 10);
      if(tmp && tmp <= INT_MAX) {
        duration = static_cast<int>(tmp);
      } else {
        usage();
        return -1;
      }
      break;
    case 'p':
      type = slowhttptest::ePost;
      break;
    case 'r':
      tmp = strtol(optarg, 0, 10);
      if(tmp && tmp <= INT_MAX) {
        delay = static_cast<int>(tmp);
      } else {
        usage();
        return -1;
      }
      break;
    case 'u':
      strncpy(url, optarg, 1024);
      break;
    case 'v':
      tmp = strtol(optarg, 0, 10);
      if(0 <= tmp && tmp <= 4) {
        debug_level = static_cast<int>(tmp);
      }
      else {
        debug_level = LOG_FATAL;
      }
      break;
    case '?':
      printf("Illegal option\n");
      usage();
      return -1;
      break;
    default:
      usage();
      return -1;
    }
  }
  slowlog_init(debug_level, NULL);
  std::auto_ptr<SlowHTTPTest> slow_test(new SlowHTTPTest(delay, duration, interval, conn_cnt, type));
  if(!slow_test->init(url)) {
    slowlog(LOG_FATAL, "%s: error setting up slow HTTP test\n", __FUNCTION__);
    return -1;
  } else if(!slow_test->run_test()) {
    slowlog(LOG_FATAL, "%s: error running slow HTTP test\n", __FUNCTION__);
    return -1;
  }
  return 0;
}