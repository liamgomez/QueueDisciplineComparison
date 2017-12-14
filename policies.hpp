class SJF
{
    public:
        bool operator() (Arrival* a, Arrival *b)
        {
            return a->serviceTime < b->serviceTime;
        }
};

class FCFS
{
    public:
        bool operator() (Arrival* a, Arrival *b)
        {
            return a->arrivalTime < b->arrivalTime;
        }
};

class LCFS
{
    public:
        bool operator() (Arrival* a, Arrival *b)
        {
            return a->arrivalTime > b->arrivalTime;
        }
};