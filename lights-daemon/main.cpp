
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
#include <sqlite3.h>

using namespace std; 

static int callback(void *NotUsed, int argc, char **argv, char **szColName)
{
 
 for(int i = 0; i < argc; i++)
  {
    cout << szColName[i] << " = " << argv[i] << endl;
  }

  std::cout << "\n";
 
  return 0;
}

const int LIGHTS_ID = 0;
const int LIGHTS_STATE = 1;
const int LIGHTS_LAST_UPDATED = 2;
const int LIGHTS_PIN = 3;

const char* GPIO_EXPORT = "echo \"%s\" > /sys/class/gpio/export";
const char* GPIO_DIRECTION = "echo \"out\" > /sys/class/gpio/gpio%s/direction";
const char* GPIO_OUTPUT = "echo \"%s\" > /sys/class/gpio/gpio%s/value";
const char* DB_PATH = "/var/www/pi.s3db";

void init()
{
    
     sqlite3 *db; // sqlite3 db struct

 char *szErrMsg = 0;
   int  rc   = sqlite3_open(DB_PATH, &db);
     
   const char *sqlSelect = "SELECT id, state, lastupdated, pin FROM lights;";
  char **results = NULL;
  int rows, columns;
  sqlite3_get_table(db, sqlSelect, &results, &rows, &columns, &szErrMsg);

    for (int i = 0; i <= rows; ++i)
    {
        char* pin = results[i * columns + LIGHTS_PIN];
        
        char buffer [50];
        //int n, a=5, b=3;
        int n;
        n=sprintf (buffer, GPIO_EXPORT, pin);
        cout <<"Exporting GPIO for Pin " << pin << ":" << buffer << endl;
        system(buffer);
        
        n=sprintf (buffer, GPIO_DIRECTION, pin);
        cout <<"Setting Direction for Pin " << pin << ":" << buffer << endl;
        system(buffer);
        
         n=sprintf (buffer, GPIO_OUTPUT, "1",  pin);
        cout <<"Setting Value for Pin " << pin << ":" << buffer << endl;
        system(buffer);
             
     }
  
    for (int j = 0; j < 3; j++)
    {
        sleep(1);
        for (int i = 0; i <= rows; ++i)
        {
            char* pin = results[i * columns + LIGHTS_PIN];
        
            char buffer [50];
            //int n, a=5, b=3;
            int n;
            char value[1];
            n = sprintf (value, "%d", j % 2);
            n=sprintf (buffer, GPIO_OUTPUT, value,  pin);
            cout <<"Setting Value for Pin " << pin << ":" << buffer << endl;
            system(buffer);
        }
    }   
    sqlite3_free_table(results);
    sqlite3_close(db);
    
}


int main()
{
 string green = "\033[32m";
 string red = "\033[31m";
 string cyan = "\033[36m";
 string yellow = "\033[33m";
 init();
 sqlite3 *db; // sqlite3 db struct
 char *szErrMsg = 0;
 int rc;
 // open database
 rc   = sqlite3_open(DB_PATH, &db);
 if(rc) {
   std::cout << "Can't open database\n";
 }
 else {
   std::cout << "Open database successfully\n";
 } 

 
  int lastTime = 0;
  while(true)
  {
     sleep(1);
      rc   = sqlite3_open(DB_PATH, &db);
    //this needs to track time of last query, then only query changed values since then!
    
    char queryBuffer[100];
    int m;
    const char *sqlSelect = "SELECT id, state, lastupdated, pin FROM lights WHERE lastupdated > %d;";
    m=sprintf (queryBuffer, sqlSelect, lastTime);
    
    
    
    char **results = NULL;
    int rows, columns;
    sqlite3_get_table(db, queryBuffer, &results, &rows, &columns, &szErrMsg);
    cout << "Result Rows = " << rows << endl;
    if (rows > 0)
    {
        
        lastTime = time(NULL);
        
    }
    
    if (rc)
    {
    cerr << "Error executing SQLite3 query: " << sqlite3_errmsg(db) << endl << endl;
    sqlite3_free(szErrMsg);
    }
    else
    {
      for (int i = 1; i <= rows; ++i)
      {
        char* id = results[i * columns + LIGHTS_ID] ;
        char* pin = results[i * columns + LIGHTS_PIN];
        char* state = results[i * columns + LIGHTS_STATE];
        char buffer [50];
        int n;
        n=sprintf (buffer, GPIO_OUTPUT, state,  pin);
        cout <<"Time:" << lastTime << " Pin" << pin << ":" << buffer << endl;
        system(buffer);
      }
    }
    sqlite3_free_table(results);
    sqlite3_close(db);
  }
  return 0;
}


