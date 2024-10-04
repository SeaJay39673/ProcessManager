/*
    Author: Christopher Edmunds
    Date: 9/16/2024
    Last Updated: 9/25/2024
    Description:
        This is the commander program that will take in input from the user and pass them to the process manager.
*/

//====| STL Includes | ====//
#include <iostream>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <vector>
#include <iomanip>
//====| Local Includes |====//

//====| Namespace |====//
using namespace std;

//====| Globals Variables |====//
#define READ_END 0
#define WRITE_END 1

//====| Function Declarations |====//
bool is_digit(string);
bool validateInput(string);

//====| Main Program|====//
int main(int argc, char *argv[])
{

    int c1, mcpipe1[2], status;
    char mc0[10], mc1[10];

    if (c1 = pipe(mcpipe1)) /* Create a pipe for master and a child process */
    {
        perror("unable to create the pipe");
        exit(1);
    }

    sprintf(mc0, "%d", mcpipe1[READ_END]);
    sprintf(mc1, "%d", mcpipe1[WRITE_END]);

    if ((c1 = fork()) == -1)
    {
        perror("unable to fork child");
        exit(1);
    }
    else if (c1 == 0)
    {
        execl("processManager", "processManager", mc0, mc1, NULL);
        exit(1);
    }
    else
    {
        // Parent Here
        close(mcpipe1[READ_END]); // don't need this. . .

        string line;
        while (getline(cin, line))
        {
            if (validateInput(line))
            {
                write(mcpipe1[WRITE_END], line.c_str(), line.size() + 1);
                if (line[0] == 'T')
                {
                    break;
                }
            }
            else
            {
                cout << "ERROR: " << line << endl;
                perror("Incorrect Command");
                return 1;
            }
            sleep(2);
        };
    }
    wait(&status);
    // cout << "Child status is " << WEXITSTATUS(status) << endl;
    return 0;
}

//====| Function Definitions |====//

bool is_digit(string input)
{
    for (char ch : input)
    {
        if (!isdigit(ch))
        {
            return false;
        }
    }
    return true;
}

bool validateInput(string input)
{
    istringstream iss(input);
    string s;
    vector<string> args = vector<string>();
    while (getline(iss, s, ' '))
    {
        args.push_back(s);
    }
    switch (args[0][0])
    {
    case 'S':
        return (is_digit(args[1]) && is_digit(args[2]) && is_digit(args[3]) && args.size() == 4);
    case 'B':
        return is_digit(args[1]) && args.size() == 2;
    case 'U':
        return is_digit(args[1]) && args.size() == 2;
    case 'C':
        return (args[1] == "A" || args[1] == "S" || args[1] == "M" || args[1] == "D") && is_digit(args[2]) && args.size() == 3;
    case 'Q':
    case 'P':
    case 'T':
        return args.size() == 1;
    default:
        return false;
    }
}