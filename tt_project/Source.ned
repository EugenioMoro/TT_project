simple Source
{
    parameters:
        volatile double interArrivalTime @unit(s);
        @display("i=block/source");
        
        int sourceId;
        int associatedQueueId;
    gates:
        output out;
}