network Net
{
    parameters:
        int n_queues = default(3);
        int n_sources = default(2);

        @display("bgb=549,310");
    submodules:

        isps[n_sources]: Source {
            parameters:
        	    //set sourceid as index of the vector
                sourceId = index;
                nQueues = n_queues;
                //choose random queue for this source to be initially connected to
                associatedQueueId = intuniform(0,n_queues-1);

                @display("p=100,50,c,70");
                interArrivalTime = 0.5;
        }

        queues[n_queues]: Queue {
            parameters:
                //set queue id as index of the vector
                queueId = index;
                //set as many ports as the sources (worst case)
                n_ports = n_sources;
                //serviceTime = 0.4;
                @display("p=250,50,c,70");
        }

        sink: Sink {
            parameters:

                n_ports = n_queues;

                @display("p=448,41");
        }
        controller: Controller {
            parameters:
            	waitBeforeNext=10s;
		    	sources=n_sources;
		    	@display("p=458,217;i=block/cogwheel");
            
        }
    connections allowunconnected: //we need to allow queues not to have all the ports connected

        //sources connections
        for i=0..n_sources-1 {
            //connect the isp to a port of the associated initial queue
            isps[i].out --> queues[isps[i].associatedQueueId].in[i];
        }

        //queues connection
        for i=0..n_queues-1 {
            queues[i].out --> sink.in[i];
        }
}
