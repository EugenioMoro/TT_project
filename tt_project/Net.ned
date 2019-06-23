network Net
{
    parameters:
        int n_queues = default(1);
        int n_sources = default(1);

    submodules:

        isps[n_sources]: Source {
            parameters:
        	    //set sourceid as index of the vector
                sourceId = index;

                //choose random queue for this source to be initially connected to
                associatedQueueId = intuniform(0,n_queues-1);

                @display("p=100,50,c,70");
                interArrivalTime=exponential(0.5s);
                //@display("p=448,131");
        }

        queues[n_queues]: Queue {
            parameters:
                //set queue id as index of the vector
                queueId = index;
                //set as many posrts are the sources (worst case)
                n_ports = n_sources;
				serviceTime=exponential(0.4s);
                @display("p=250,50,c,70");
        }

        sink: Sink {
            parameters:

                n_ports = n_queues;

                @display("p=448,41");
        }
        Controller: Controller {
            @display("p=392,102;i=block/cogwheel");
            waitBeforeNext=50;
            sources=n_sources;
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
