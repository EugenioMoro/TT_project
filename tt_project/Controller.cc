#include <omnetpp.h>

using namespace omnetpp;
class Controller : public cSimpleModule, cListener
{
  protected:
    virtual void initialize() override;
    virtual void handleMessage(cMessage *msg) override;
    virtual void receiveSignal(cComponent *source, simsignal_t signalID, cObject *obj, cObject *details) override{
        EV<<"orcodio ho ricevuto"<<endl;
    }

    simsignal_t seviceCompletedSignal;
    simsignal_t responseTimeSignal;
};

// The module class needs to be registered with OMNeT++
Define_Module(Controller);
//Register_Class(Controller);

void Controller::initialize()
{
    EV<<"Controller Initialized"<<endl;
    //seviceCompletedSignal = registerSignal("seviceCompletedSignal");
    responseTimeSignal = registerSignal("responseTime");
    subscribe(responseTimeSignal, this);
    if (hasListeners(responseTimeSignal)){
               EV<<"listener c'è"<<endl;
           }
}



void Controller::handleMessage(cMessage *msg)
{
    // The handleMessage() method is called whenever a message arrives
    // at the module. Here, we just send it to the other module, through
    // gate `out'. Because both `tic' and `toc' does the same, the message
    // will bounce between the two.
    //send(msg, "out"); // send out the message
}
