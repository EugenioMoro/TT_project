simple Queue
{
    parameters:
        volatile double serviceTime;
        @display("i=block/queue;q=queue");
        
        @signal[qlen](type="long");
        @signal[busy](type="bool");
        @signal[queueingTime](type="simtime_t");
        @signal[responseTime](type="simtime_t");
        //this signal sends the waiting time of each packet in order to compute the avg at runtime
        @signal[seviceCompletedSignal](type="cCompletedServiceNotif");
        @signal[interarrivalTimes](type="simtime_t");
        
        @statistic[qlen](title="queue length";record=vector,timeavg,max;interpolationmode=sample-hold);
        @statistic[busy](title="server busy state";record=vector?,timeavg;interpolationmode=sample-hold);
        @statistic[queueingTime](title="queueing time at dequeue";unit=s;record=vector,mean,max;interpolationmode=none);
        @statistic[responseTime](title="response time at end of service";unit=s;record=vector,mean,max;interpolationmode=none);
        @statistic[interarrivalTimes](title="interarrival time of the queue";unit=s;record=mean);
    	
    	int queueId = default(1);
    	
    	//the number of ports for the queue, as many ports as input connections
    	int n_ports = default(1);
    gates:
        input in[n_ports];
        input arRequestGateIn;
        output arRequestGateOut;
        output out;
}