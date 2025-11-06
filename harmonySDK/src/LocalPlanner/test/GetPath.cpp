#include <unistd.h>
#include <limits.h>
#include <iostream>
#include <string>
#include <libgen.h>

int main(){
  char result[1024];
  ssize_t count = readlink("/proc/self/exe", result, 1024);
  if (count != -1) {
      result[count] = '\0';
      std::string yamlPath = std::string(dirname(result));
      std::cout << yamlPath << std::endl;
  }
}