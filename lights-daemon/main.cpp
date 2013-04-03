
#include <string.h>
#include <sys/wait.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>
#include <cstdlib>
#include <iostream>
#include <fstream>
#include <sqlite3.h>

using namespace std; 

const int LIGHTS_ID = 0;
const int LIGHTS_STATE = 1;
const int LIGHTS_LAST_UPDATED = 2;
const int LIGHTS_PIN = 3;

const char* GPIO_EXPORT = "echo \"%s\" > /sys/class/gpio/export";
const char* GPIO_DIRECTION = "echo \"out\" > /sys/class/gpio/gpio%s/direction";
const char* GPIO_OUTPUT = "echo \"%s\" > /sys/class/gpio/gpio%s/value";
const char* GPIO_STATE = "cat /sys/class/gpio/gpio%s/value";
const char* DB_PATH = "/var/www/%s/pi.s3db";

bool* _blinking;
char** _pins;
int _lightsCount;

char _dbPath[50];

void setPinState(char* pin, char* state)
{
    char buffer [50];
    int n;
    n=sprintf (buffer, GPIO_OUTPUT, state,  pin);
    system(buffer);
    cout <<"Setting Value for Pin " << pin << ":" << buffer << endl;
}


void init()
{
   sqlite3 *db; // sqlite3 db struct
   char *szErrMsg = 0;
   int  rc   = sqlite3_open(_dbPath, &db);
   const char *sqlSelect = "SELECT id, state, lastupdated, pin FROM lights;";
   char **results = NULL;
   int rows, columns;
   sqlite3_get_table(db, sqlSelect, &results, &rows, &columns, &szErrMsg);
   _lightsCount= rows;
   _blinking = new bool[_lightsCount];
   _pins = new char*[_lightsCount];
   for (int i = 0; i <= rows; ++i)
    {
        char* pin = results[i * columns + LIGHTS_PIN];
        char buffer [50];
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
            _pins[i] = pin;
            char buffer [50];
            int n;
            char value[1];
            n = sprintf (value, "%d", j % 2);
            
            setPinState(pin, value);
            
         
        }
    }   
    sqlite3_free_table(results);
    sqlite3_close(db);
    
}


string getPinValue(char* pin)
{ 
   char cmd[50];
   int n;
   n=sprintf (cmd, GPIO_STATE, pin);
   FILE* pipe = popen(cmd, "r");
   if (!pipe) return "ERROR";
   char buffer[128];
   std::string result = "";
   while(!feof(pipe)) {
    	if(fgets(buffer, 128, pipe) != NULL) result += buffer;
   }
   pclose(pipe);
   return result;
}



int main(int argc, char* argv[])
{
 string green = "\033[32m";
 string red = "\033[31m";
 string cyan = "\033[36m";
 string yellow = "\033[33m";

 cout << "argc = " << argc << endl; 
 for(int i = 0; i < argc; i++) 
 {
     cout << "argv[" << i << "] = " << argv[i] << endl; 
     if (i == 1)
     {
       int n;
       n=sprintf (_dbPath,DB_PATH, argv[i]);
     }
  }
  cout << "Using Database: "<< _dbPath << endl;

  init();
  sqlite3 *db; // sqlite3 db struct
  char *szErrMsg = 0;
  int rc;
  //open database
  rc = sqlite3_open(_dbPath, &db);
  if(rc)
  {
   std::cout << "Can't open database\n";
  }
  else
  {
   std::cout << "Open database successfully\n";
  } 
 
  int lastTime = 0;
  while(true)
  {
    sleep(1);
    rc   = sqlite3_open(_dbPath, &db);
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
        //if state is 3 (blinking), then do opposite of what it currently is.
        if (state == "3")
        {
            _blinking[i] = true;
        }
        else
        {
            _blinking[i] = false;
    		setPinState(pin, state);
        }
      }
    }

    for (int j = 0; j <= _lightsCount; j++)
      {
        if (_blinking[j])
        {
           char* pin = _pins[j];
           char* state;
           string pinValue =  getPinValue(pin);
           cout << "GETTING PIN "<< pin << " State: " << pinValue  << endl;
        	if (pinValue == "1")
        	{
                state = (char*) "0";
        	}
            else
            {
                state = (char*) "1";
            }
            setPinState(pin, state);
        }
      }
    sqlite3_free_table(results);
    sqlite3_close(db);
  }
  return 0;
}


