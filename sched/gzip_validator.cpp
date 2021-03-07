// This file is part of BOINC.
// http://boinc.berkeley.edu
// Copyright (C) 2008 University of California
//
// BOINC is free software; you can redistribute it and/or modify it
// under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation,
// either version 3 of the License, or (at your option) any later version.
//
// BOINC is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
// See the GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with BOINC.  If not, see <http://www.gnu.org/licenses/>.

// A validator that accepts gzip file as result

#include <string>
#include <cstring>
#include <sys/stat.h>
#include <sys/types.h>
#include "validate_util2.h"
#include "sched_config.h"
#include "sched_util.h"
#include "error_numbers.h"

#define ERR_INVALID_RESULT ~( ERR_OPENDIR | VAL_RESULT_SUSPICIOUS )

using std::string;

#define MAX_INPUTPATH_LENGTH 512

// get path to result file
int getOutputFilePath(RESULT const& result, string& path_str, int file_num) {
   char buf[256], path[256];
   string str = result.xml_doc_in;
   size_t pos = 0;
   int file_count = 0;
   while ((pos = str.find("<file_info>", pos)) != string::npos) {
      pos++;
      file_count++;
      if (file_count == file_num) break;
   }

   if (file_count < file_num) {
      return ERR_XML_PARSE;
   }

   // we only care about the next file reference
   str.erase(0, pos);

   if (!parse_str(str.c_str(), "<name>", buf, sizeof (buf))) {
      return ERR_XML_PARSE;
   }
   dir_hier_path(buf, config.upload_dir, config.uldl_dir_fanout, path);
   path_str = path;

   return BOINC_SUCCESS;
}

bool checkFile(RESULT& result, string& file) {
   char cmd[1024];
   struct stat info;
   int retval;
   char *token;
   char *localJobId;
   char *instance;
   char inputPath[MAX_INPUTPATH_LENGTH+1];
   char inputFile[MAX_INPUTPATH_LENGTH+1];

   sprintf(cmd, "gzip -tf %s > /dev/null", file.c_str());
   retval = system(cmd);
   if (retval) {
      log_messages.printf(MSG_CRITICAL, "[RESULT#%d %s] gzip -t failed on the file %s\n", result.id, result.name, file.c_str());

      retval = stat(config.upload_dir, &info);
      if (retval) {
         log_messages.printf(MSG_CRITICAL, "[RESULT#%d %s] Upload filesystem not present.  Exiting.\n", result.id, result.name);
         exit(-1);
      }

      return false;
   }
   if(strlen((char *)file.c_str()) <= MAX_INPUTPATH_LENGTH) {
      std::strcpy(inputPath,(char *)file.c_str());
// Skip directory segments of path
      token = std::strtok(inputPath, "/");
      while (token != NULL) {
         std::strcpy(inputFile,token);
         token = std::strtok(NULL, "/");
      }

      localJobId = std::strtok(inputFile, "_");
      if(localJobId != NULL) {
         instance = std::strtok(NULL, "_");
         if(instance != NULL) {
            sprintf(cmd,"tar tvf %s ./.__timestamp_start.%s_%s > /dev/null",file.c_str(),localJobId,instance);
            retval = system(cmd);
            if (retval) {
               log_messages.printf(MSG_CRITICAL,"[RESULT#%d %s] tar tvf failed on the file %s\n",result.id,result.name,file.c_str());
               return false;
            } else {
               return true;
            }
         }
      }
   }

   return false;
}

int validate_handler_init(int, char**) {
   return BOINC_SUCCESS;
}

void validate_handler_usage() {
   // describe the project specific arguments here
   //fprintf(stderr,
   //    "    Custom options:\n"
   //    "    [--project_option X]  a project specific option\n"
   //);
}

int init_result(RESULT& result, void*&) {
// results may contain multiple files
   int file_num = 1;
   string path_str = "";
   int errCode = getOutputFilePath(result,path_str,file_num);
   if(errCode) {
      log_messages.printf(MSG_CRITICAL,"[RESULT#%d %s] Could not find result path\n",result.id,result.name);
      return ERR_INVALID_RESULT;
   }
   if(!checkFile(result,path_str)) {
      return ERR_INVALID_RESULT;
   }

   return BOINC_SUCCESS;
}

int compare_results(RESULT&, void*, RESULT const&, void*, bool& match) {
   match = true;

   return BOINC_SUCCESS;
}

int cleanup_result(RESULT const&, void*) {
   return BOINC_SUCCESS;
}

const char *BOINC_RCSID_f3a7a34795 = "$Id$";
