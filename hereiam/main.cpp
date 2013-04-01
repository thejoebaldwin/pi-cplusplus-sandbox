#include <string.h>
#include <sys/wait.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>
#include <cstdlib>
#include <iostream>
#include <iostream>
#include <fstream>

using namespace std;

int main()
{
	
 
//ifconfig eth0 | grep 'inet addr:' | cut -d: -f2



FILE *fp;
  int status;
  char path[1035];

  /* Open the command for reading. */
  fp = popen("ifconfig eth0 | grep 'inet addr:' | cut -d: -f2", "r");
  if (fp == NULL) {
    printf("Failed to run command\n" );
    exit;
  }

  /* Read the output a line at a time - output it. */
  while (fgets(path, sizeof(path)-1, fp) != NULL) {

   string output = path;
   int spaceIndex = 0;
   for (int i = 0; i < output.length(); i++)
   {
    if (output[i] == ' ')
      {
         spaceIndex = i;
         output = output.substr(0, spaceIndex);
         break;
      }
 
   }
   cout << output << endl;
string request = "curl http://humboldttechgroup.com/mailzorz.php?ip=" + output;
   system(request.c_str());
   cout << path;

   // printf("%s", path);
  }

  /* close */
  pclose(fp);

  return 0;


}




