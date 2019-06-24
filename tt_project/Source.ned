simple Source
{
    parameters:
        volatile double interArrivalTime;// @unit(s);
        @display("i=block/source");
        
        int sourceId;
        int associatedQueueId;
        int nQueues;
    gates:
        input in;
        output out;
        output responseTimeOutGateway;
}