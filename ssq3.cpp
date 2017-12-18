#include <cstdio>
#include <cmath>
#include <iostream>
#include <fstream>
#include <queue>
#include <list>
#include <cstdlib>
#include <algorithm>
#include <random>

#include "rngs.h"
#include "arrival.hpp"

using namespace std;

#define START         0.0
#define STOP      30000.0
#define LARGENUM   (100.0 * STOP)

// Constants ///////////////////////////////////////////////////////////////////
    // modify this to increase utilization, increase to decrease arrival rate
    const double ARRIVAL_RATE_DENOM = 1.66;

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

bool fcfs(const Arrival *a, const Arrival *b)
{
    return a->arrivalTime < b->arrivalTime;
}

bool lcfs(const Arrival *a, const Arrival *b)
{
    return a->arrivalTime > b->arrivalTime;
}

bool sjf(const Arrival *a, const Arrival *b)
{
    return a->serviceTime < b->serviceTime;
}

bool randomOrder(const Arrival *a, const Arrival *b)
{
    SelectStream(2);
    double randomNum = Uniform(0, 1.0);
    return randomNum < 0.5;
}

// Function Implementation /////////////////////////////////////////////////////
void writeStatsFile(vector<double> delayTimes, vector<double> waitTimes, string schedule);

int main(int argc, char const *argv[])
{
    string scheduleInput = "FCFS";
    cout << "Please enter the schedule you wish to simulate (FCFS, LCFS, SJF, RO) : ";
    cin >> scheduleInput;

    if (scheduleInput != "FCFS" &&
        scheduleInput != "LCFS" &&
        scheduleInput != "SJF" &&
        scheduleInput != "RO")
    {
        cout << "Error: invalid scheduling policy input options are (FCFS, LCFS, SJF, RO)";
        return 0;
    }

    // print info for simulation
    cout << "Performing simulation with arrival rate of 1 / "
         << ARRIVAL_RATE_DENOM << " = "
         << 1/ARRIVAL_RATE_DENOM
         << endl;

    cout << "With the scheduling policy " << scheduleInput << endl;

    list<Arrival*> eventList;

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

                eventList.push_back(event);
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

            if (eventList.empty() && number == 0)
            {
                double delay = 0;
                double wait = notBusyST;

                delayTimes.push_back(delay);
                waitTimes.push_back(wait);
            }

            if (! eventList.empty() && number > 0) {
                if ( scheduleInput == "FCFS") {
                    eventList.sort(fcfs);
                }
                if ( scheduleInput == "LCFS") {
                    eventList.sort(lcfs);
                }
                if ( scheduleInput == "SJF") {
                    eventList.sort(sjf);
                }
                if ( scheduleInput == "RO") {
                    eventList.sort(randomOrder);
                }

                event = eventList.front();

                double delay = t.current - event->arrivalTime;
                double wait = delay + event->serviceTime;

                t.completion = t.current + event->serviceTime;

                delayTimes.push_back(delay);
                waitTimes.push_back(wait);

                delete event;
                event = nullptr;
                eventList.pop_front();
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

    writeStatsFile(delayTimes, waitTimes, scheduleInput);
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

void writeStatsFile(vector<double> delayTimes, vector<double> waitTimes, string schedule)
{
    ofstream fout;
    fout.open("delays" + schedule + ".csv", ios_base::trunc);
    fout << "delays" << endl;
    for (vector<double>::iterator i = delayTimes.begin(); i != delayTimes.end(); ++i)
    {
        fout << *i;
        if (delayTimes.begin() != delayTimes.end())
        {
            fout << endl;
        }
    }
    fout.close();

    fout.open("response_times" + schedule + ".csv", ios_base::trunc);
    fout << "response_times" << endl;

    for (std::vector<double>::iterator i = waitTimes.begin(); i != waitTimes.end(); ++i)
    {
        fout << *i;

        if (waitTimes.begin() != waitTimes.end())
        {
            fout << endl;
        }
    }
    fout.close();
}