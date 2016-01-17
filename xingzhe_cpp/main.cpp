#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <syslog.h>

#include <iostream>

#include "pub.h"
#include "common.h"

#include "xingzhe.h"

using namespace std;

int main(int argc, char *argv[])
{
    xingzhe *wukong;
    
    cout << "Hello!" << endl;

    wukong = new xingzhe("dasheng");

    delete wukong;

    return 0;
}


