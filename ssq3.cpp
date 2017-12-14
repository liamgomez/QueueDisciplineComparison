#include <cstdio>
#include <cmath>
#include <iostream>
#include <fstream>
#include <queue>

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
void generateArrivals(vector<Arrival*> *arrivals);

int main(int argc, char const *argv[])
{
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
            t.arrival = GetArrival();

            if (t.arrival > STOP) {
                t.last      = t.current;
                t.arrival   = LARGENUM;
            }

            if (number == 1) {
                t.completion = t.current + GetService();
            }
            else {
                event = new Arrival();
                event->arrivalTime = t.arrival;
                event->serviceTime = GetService();
                pQueue.push(event);
                event = nullptr;
            }
        }
        else {
            index++;
            number--;
            if (number > 0) {
                event = pQueue.top();
                pQueue.pop();

                t.completion = t.current + event->serviceTime;
                delete event;
                event = nullptr;
            }
            else {
                t.completion = LARGENUM;
            }
        }
    }

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