simple Queue
{
    parameters:
        volatile double serviceTime @unit(s);
        @display("i=block/queue;q=queue");
        
        @signal[qlen](type="long");
        @signal[busy](type="bool");
        @signal[queueingTime](type="simtime_t");
        @signal[responseTime](type="simtime_t");
        
        @statistic[qlen](title="queue length";record=vector,timeavg,max;interpolationmode=sample-hold);
        @statistic[busy](title="server busy state";record=vector?,timeavg;interpolationmode=sample-hold);
        @statistic[queueingTime](title="queueing time at dequeue";unit=s;record=vector,mean,max;interpolationmode=none);
        @statistic[responseTime](title="response time at end of service";unit=s;record=vector,mean,max;interpolationmode=none);
    	
    	//the number of ports for the queue, as many ports as input connections
    	int n_ports = default(1);
    gates:
        input in[n_ports];
        output out;
}