simple Controller{
    	//@class(Controller);
        //this signal sends the waiting time of each packet in order to compute the avg at runtime
        @signal[seviceCompletedSignal](type="cCompletedServiceNotif");
    	@signal[responseTime](type="simtime_t");
}