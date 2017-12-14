class Arrival
{
    public:

        double arrivalTime;
        double serviceTime;
        double iat;

        Arrival()
        {
            arrivalTime = 0;
            serviceTime = 0;
            iat = 0.0;
        }

        ~Arrival()
        {

        }
};