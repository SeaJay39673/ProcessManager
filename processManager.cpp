/*
    Author: Christopher Edmunds
    Date: 9/16/2024
    Last Updated: 9/25/2024
    Description: Process Manager. The class object handles all of the logic for the process manager.
        The main function spawns the object, reads in input from commander (assuming input is already validated),
        and sends it off to the class object to digest and handle the command.
*/
//====| TEMPLATE |====//

//====| STL Includes |====//
#include <iostream>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sstream>
#include <vector>
#include <map>
#include <iomanip>
#include <sys/wait.h>

//====| Local Includes |====//
#include "queue_array.h"
#include "PCB.h"

//====| Namespace |====//
using namespace std;

//====| Process Manager Class |====//
class Process_Manager
{
private:
    int Time;                                                    // Global Time
    PCB PCB_Table[100];                                          // Array of possible PCBs (Processes)
    QueueArray<int> *ReadyState;                                 // (size 4 for everything)
    QueueArray<int> *BlockedState[3];                            // 3 Resources to Block processes for
    int RunningState[3] = {-1, -1, -1};                          // Holds PID, Time elapsed, and Quantum for current running process
    map<int, int> quantumMap = {{0, 1}, {1, 2}, {2, 4}, {3, 8}}; // Map Priority to Quantum
    double turnaroundTimeSum;                                    // Keeps track of the total time completed processes took
    double processesCompleted;                                   // Keeps track of the processes completed
    double totalProcesses;                                       // Keeps track of the total processes read

    //----| Commands |----//
    int S(int, int, int); // Creates and starts a new process
    int B(int);           // Blocks currently running process
    int U(int);           // Unblocks currently blocked process
    int Q();              // Increments Time
    int C(string, int);   // Perform operation on value of process
    int P();              // Report current state of the process manager
    int T();              // Report on the final information of the process manager (turnaround time, etc).

    //----| Helpers |-----//
    void swap(bool);              // Swap processes in and out of RunningState
    void updateRunningState(int); // Updates the current running process by PID

public:
    //----|Constructor(s)|----//
    Process_Manager();
    ~Process_Manager();

    //----| Helpers |----//
    int digestInput(string); // Reads in string and processes it to call respective commands
};

//====| Main Program |====//

int main(int argc, char *argv[])
{
    int mcpipe2[2];
    char chr;
    Process_Manager pm = Process_Manager();
    int result;

    mcpipe2[0] = atoi(argv[1]);
    mcpipe2[1] = atoi(argv[2]);

    close(mcpipe2[1]); // Don't need this. . .

    string str = "";
    while (read(mcpipe2[0], &chr, 1)) // Build string from one char at a time, send string off to process manager object
    {
        if (chr != 0)
        {
            str.push_back(chr);
        }
        else
        {
            result = pm.digestInput(str);
            if (str[0] == 'T')
            {
                break;
            }
            str = "";
        }
    }
    close(mcpipe2[0]);

    return 1;
}

//====| Class Definitions |====//

//====| Constructors |====//

Process_Manager::Process_Manager() : Time(0), turnaroundTimeSum(0), processesCompleted(0), totalProcesses(0)
{
    ReadyState = new QueueArray<int>(4);
    for (int i = 0; i < 3; i++)
    {
        BlockedState[i] = new QueueArray<int>(4);
    }
}

Process_Manager::~Process_Manager()
{
    delete ReadyState;
}

//====| Commands |====//

/*
    Creates and starts new process. If process already running, add to ReadyState (queue process to run)
*/
int Process_Manager::S(int pid, int value, int run_time)
{
    totalProcesses++;
    PCB_Table[pid] = PCB(pid, value, run_time, Time);
    if (RunningState[0] == -1) // If RunningState has no process
    {
        updateRunningState(pid);
    }
    else
    {
        ReadyState->Enqueue(pid, PCB_Table[pid].getPriority());
    }
    return 0;
}

/*
    Decrement priority and block currently running process (queue onto block with resource rid).
    Next process from ReadyState now runs
*/
int Process_Manager::B(int rid)
{
    int pid = RunningState[0];
    PCB_Table[pid].decrementPriority();
    BlockedState[rid]->Enqueue(pid, PCB_Table[pid].getPriority());
    pid = ReadyState->Dequeue();
    updateRunningState(pid);
    return 0;
}

/*
    Unblock first process from the BlockedState (dequeue off of block with resource rid).
*/
int Process_Manager::U(int rid)
{
    int pid = BlockedState[rid]->Dequeue();
    if (RunningState[0] < 1)
    {
        updateRunningState(pid);
    }
    else
    {
        ReadyState->Enqueue(pid, PCB_Table[pid].getPriority());
    }
    return 0;
}

/*
    Increment Time.
    Check if process has finished and swap if needed.
    Check if process has met quantum and swap if needed
*/
int Process_Manager::Q()
{
    if (RunningState[0] < 1) // FLAG
    {
        if (ReadyState->QAsize() > 0)
        {
            int pid = ReadyState->Dequeue();
            updateRunningState(pid);
        }
        else
        {
            return 1;
        }
    }
    Time++;
    RunningState[1]++; // Increment current time elapsed for the running process

    if (PCB_Table[RunningState[0]].incrementTime()) // Is Process Finished?
    {
        processesCompleted++;
        turnaroundTimeSum += Time - (PCB_Table[RunningState[0]].getStart());
        swap(true); // Done with this process = true
    }

    if (RunningState[1] >= RunningState[2]) // If time elapsed >= quantum, change process
    {
        swap(false); // Not done with this process
    }

    return 0;
}

/*
    Operate on value of currently running process
*/
int Process_Manager::C(string cmd, int val)
{
    int pcbVal = PCB_Table[RunningState[0]].getValue();
    switch (cmd[0])
    {
    case 'A':
        PCB_Table[RunningState[0]].setValue(pcbVal + val);
        break;
    case 'S':
        PCB_Table[RunningState[0]].setValue(pcbVal - val);
        break;
    case 'M':
        PCB_Table[RunningState[0]].setValue(pcbVal * val);
        break;
    case 'D':
        PCB_Table[RunningState[0]].setValue(pcbVal / val);
        break;
    default:
        return 1;
    }
    return Q();
}

/*
    Spawn reporter (fork) and print the current state of processManager. Master waits for child to terminate.
*/
int Process_Manager::P()
{
    int c1, status;
    if ((c1 = fork()) == -1)
    {
        perror("unable to fork child for printing");
        exit(1);
    }
    else if (c1 == 0)
    {
        string header = "PID  Priority Value  Start Time  Total CPU time";

        cout << "*****************************************************\nThe current system state is as follows : \n*****************************************************\n " << endl;
        cout
            << "CURRENT TIME: "
            << Time
            << endl
            << endl
            << "RUNNING PROCESS:"
            << endl;

        cout << header << endl;
        cout << PCB_Table[RunningState[0]] << endl;

        cout << "BLOCKED PROCESS:" << endl;
        int *blocked;
        for (int i = 0; i < 3; i++)
        {
            int rsize = BlockedState[i]->QAsize();
            cout << "Queue of processes Blocked for resource " << i << (rsize == 0 ? " is empty" : ":") << endl;
            if (rsize > 0)
            {
                cout << header << endl;
                for (int j = 0; j < 4; j++)
                {
                    int size = BlockedState[i]->Qsize(j);
                    if (size > 0)
                    {
                        blocked = BlockedState[i]->Qstate(j);
                        for (int k = 0; k < size; k++)
                        {
                            cout << PCB_Table[blocked[k]];
                        }
                        delete[] blocked;
                    }
                }
            }
        }

        cout << endl;

        cout << "PROCESSES READY TO EXECUTE:" << endl;
        for (int i = 0; i < 4; i++)
        {
            int size = ReadyState->Qsize(i);
            cout << "Queue of processes with priority " << i << ((size == 0) ? " is empty" : ":") << endl;
            if (size > 0)
            {
                cout << header << endl;
                int *ready = ReadyState->Qstate(i);
                for (int j = 0; j < size; j++)
                {
                    cout << PCB_Table[ready[j]];
                }
            }
        }
        cout << "*****************************************************" << endl
             << endl;
        exit(0);
    }
    else
    { // parent
    }
    wait(&status);
    return 0;
}

/*
    Spawn reporter (fork) and print turnaround time of process manager.
*/
int Process_Manager::T()
{
    int c1, status;
    if ((c1 = fork()) == -1)
    {
        perror("unable to fork child for printing");
        exit(1);
    }
    else if (c1 == 0)
    {
        cout << "The average Turnaround Time: " << (processesCompleted ? turnaroundTimeSum / processesCompleted : processesCompleted) << endl
             << endl;
        cout << "Extra information you might want to know:" << endl;
        cout << processesCompleted << " processes finished in a total of " << turnaroundTimeSum << " seconds" << endl;
        exit(0);
    }
    wait(&status);
    return 0;
}
//====| Helpers |====//

/*
    Swaps ReadyState and RunningState.
    done indicates if a process is finished. If process is finished (done), process does not go back to ReadyState.
    If process is not finsihed (not done), process swaps with ReadyState and RunningState.
*/
void Process_Manager::swap(bool done)
{
    if (!done) // If process has not completed (Has met quantum), enqueue it
    {
        PCB_Table[RunningState[0]].incrementPriority(); // Increment priority since it's met it's quantum
        ReadyState->Enqueue(RunningState[0], PCB_Table[RunningState[0]].getPriority());
    }

    int pid = ReadyState->Dequeue();
    updateRunningState(pid);
}

/*
    Tokenizes string input into arugments and runs varying commands based on the args.
*/
int Process_Manager::digestInput(string input)
{
    istringstream iss(input);
    string s;
    vector<string> args = vector<string>();
    while (getline(iss, s, ' '))
    {
        args.push_back(s);
    }
    switch (input[0])
    {
    case 'S':
        return S(atoi(args[1].c_str()), atoi(args[2].c_str()), atoi(args[3].c_str()));
    case 'B':
        return B(atoi(args[1].c_str()));
    case 'U':
        return U(atoi(args[1].c_str()));
    case 'Q':
        return Q();
    case 'C':
        return C(args[1], atoi(args[2].c_str()));
    case 'P':
        return P();
    case 'T':
        return T();
    default:
        perror("Incorrect Command");
        exit(1);
        break;
    }
    return 0;
};

/*
    Updates the running state.
    Time elapsed is for comparing if process has met quantum
*/
void Process_Manager::updateRunningState(int pid)
{
    RunningState[0] = pid;                                      // Pointer to process is PID
    RunningState[1] = 0;                                        // Set current time elapsed for process on CPU back to 0
    RunningState[2] = quantumMap[PCB_Table[pid].getPriority()]; // Set quantum based on priority
}