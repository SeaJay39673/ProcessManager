#ifndef PCB_H
#define PCB_H
/*

*/
//====| TEMPLATE |====//

//====| STL Includes |====//
#include <stdio.h>

//====| Namespaces |====//
using namespace std;

//====| Class Declaration |====//
class PCB
{
private:
    int pid;
    int priority;
    int value; // Value the process has
    int start_time;
    int run_time; // Time process needs to complete
    int cpu_time; // Time process has spent on CPU

public:
    //----| Constructor(s) |----//
    PCB() {}
    PCB(int pd, int val, int run, int Time) : pid(pd), value(val), run_time(run)
    {
        priority = 0;
        start_time = Time;
        cpu_time = 0;
    }
    //----| Getters |----//
    int getPID() { return pid; }
    int getPriority() { return priority; }
    int getValue() { return value; }
    int getStart() { return start_time; }
    int getRun() { return run_time; }
    int getCPU() { return cpu_time; }

    //----| Setters |----//
    void setValue(int val)
    {
        value = val;
    }
    void incrementPriority()
    {
        if (priority < 3)
        {
            priority++;
        }
    }
    void decrementPriority()
    {
        if (priority > 0)
        {
            priority--;
        }
    }
    bool incrementTime()
    {
        cpu_time++;
        if (cpu_time >= run_time)
        {
            return true;
        }
        return false;
    };

    //----| Helpers |----//
    friend ostream &operator<<(ostream &out, const PCB &pcb);
};

ostream &operator<<(ostream &out, const PCB &pcb)
{
    out << setw(2) << pcb.pid << "  "
        << setw(4) << pcb.priority << "  "
        << setw(7) << pcb.value << "  "
        << setw(11) << pcb.start_time << "  "
        << setw(8) << pcb.cpu_time << endl;
    return out;
}

#endif