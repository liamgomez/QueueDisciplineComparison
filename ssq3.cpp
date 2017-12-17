#include <cstdio>
#include <cmath>
#include <iostream>
#include <fstream>
#include <queue>
#include <list>

#include "rngs.h"
#include "arrival.hpp"
#include "policies.hpp"

using namespace std;

#define START         0.0
#define STOP      20000.0
#define LARGENUM   (100.0 * STOP)

// Constants ///////////////////////////////////////////////////////////////////
    // modify this to increase utilization, increase to decrease arrival rate
    const double ARRIVAL_RATE_DENOM = 2.0;
    const string SCHEDULE = "FCFS";

// Functions ///////////////////////////////////////////////////////////////////
double Min(double a, double c)
{ 
    if (a < c)
        return (a);
    else
        return (c);
} 

double Exponential(double m)
{
    return (-m * log(1.0 - Random()));
}


double Uniform(double a, double b)
{
    return (a + (b - a) * Random());
}

double GetArrival()
{
    static double arrival = START;

    SelectStream(0); 
    arrival += Exponential(ARRIVAL_RATE_DENOM);
    return (arrival);
}

double GetService()
{
    SelectStream(1);
    return (Uniform(1.0, 2.0));
}

// Function Implementation /////////////////////////////////////////////////////
void writeStatsFile(vector<double> delayTimes, vector<double> waitTimes);

int main(int argc, char const *argv[])
{
    // print info for simulation
    cout << "Performing simulation with arrival rate of 1 / "
         << ARRIVAL_RATE_DENOM << " = "
         << 1/ARRIVAL_RATE_DENOM
         << endl;

    cout << "With the scheduling policy " << SCHEDULE << endl;

    // Change policy here based on classes in policies.hpp
    priority_queue<Arrival*, std::vector<Arrival*>, FCFS> pQueue;

    struct {
        double arrival;
        double completion;
        double current;
        double next;
        double last;
    } t;

    struct {
        double node;
        double queue;
        double service;
    } area      = {0.0, 0.0, 0.0};

    long index  = 0;
    long number = 0;

    PlantSeeds(123456789);
    t.current    = START;
    t.arrival    = GetArrival();
    t.completion = LARGENUM;

    Arrival* event = nullptr;

    vector<double> delayTimes;
    vector<double> waitTimes;
    cout << "First arrival : " << t.arrival << endl;

    double notBusyST = 0;

    while ((t.arrival < STOP) || (number > 0))
    {
        t.next = Min(t.arrival, t.completion);

        if (number > 0) {
            area.node    += (t.next - t.current) * number;
            area.queue   += (t.next - t.current) * (number - 1);
            area.service += (t.next - t.current);
        }

        t.current = t.next;

        if (t.current == t.arrival) {
            number++;

            double st = GetService();

            if (number > 1 && t.arrival < STOP)
            {
                event = new Arrival();
                event->arrivalTime = t.arrival;
                event->serviceTime = st;
                pQueue.push(event);
                event = nullptr;
            }

            if (number == 1) {
                notBusyST = st;
                t.completion = t.current + st;
            }

            t.arrival = GetArrival();

            if (t.arrival > STOP) {
                t.last      = t.current;
                t.arrival   = LARGENUM;
            }

        }
        else {
            index++;
            number--;

            if (pQueue.empty() && number == 0)
            {
                double delay = 0;
                double wait = notBusyST;

                delayTimes.push_back(delay);
                waitTimes.push_back(wait);
            }

            if (! pQueue.empty() && number > 0) {

                event = pQueue.top();
                pQueue.pop();

                double delay = t.current - event->arrivalTime;
                double wait = delay + event->serviceTime;

                t.completion = t.current + event->serviceTime;

                delayTimes.push_back(delay);
                waitTimes.push_back(wait);

                delete event;
                event = nullptr;
            }
            else {
                t.completion = LARGENUM;
            }
        }
    }

    double delaySum = 0.0;

    for (vector<double>::iterator i = delayTimes.begin(); i != delayTimes.end(); ++i)
    {
        delaySum += *i;
    }

    double waitSum = 0.0;

    for (std::vector<double>::iterator i = waitTimes.begin(); i != waitTimes.end(); ++i)
    {
        waitSum += *i;
    }

    writeStatsFile(delayTimes, waitTimes);
    double avgDelay = delaySum / index;
    double avgWait = waitSum /index;
    cout << "Average wait (non-time avg) : " << avgWait << endl;
    cout << "Average delay (non-time avg) : " << avgDelay << endl;

    printf("\nfor %ld jobs\n", index);
    printf("   average interarrival time = %6.2f\n", t.last / index);
    printf("   average wait ............ = %6.2f\n", area.node / index);
    printf("   average delay ........... = %6.2f\n", area.queue / index);
    printf("   average service time .... = %6.2f\n", area.service / index);
    printf("   average # in the node ... = %6.2f\n", area.node / t.current);
    printf("   average # in the queue .. = %6.2f\n", area.queue / t.current);
    printf("   utilization ............. = %6.2f\n", area.service / t.current);

    return 0;
}

void writeStatsFile(vector<double> delayTimes, vector<double> waitTimes)
{
    ofstream fout;
    fout.open("delays.csv", ios_base::trunc);
    for (vector<double>::iterator i = delayTimes.begin(); i != delayTimes.end(); ++i)
    {
        fout << *i << ',';
    }
    fout.close();

    fout.open("response_times.csv", ios_base::trunc);

    for (std::vector<double>::iterator i = waitTimes.begin(); i != waitTimes.end(); ++i)
    {
        fout << *i << ',';
    }
    fout.close();
}