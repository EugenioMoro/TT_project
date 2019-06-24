#include <omnetpp.h>

using namespace omnetpp;
class Controller : public cSimpleModule, cListener
{
private:
    int nextSource;
    cMessage *nextTurnEvent;
protected:
    virtual void initialize() override;
    virtual void handleMessage(cMessage *msg) override;
};

Define_Module(Controller);

void Controller::initialize()
{
    nextTurnEvent = new cMessage("nextTurnEvent");
    nextSource=0;//(int)par("sources") - 1;
    EV<<"Controller Initialized:"<<endl;
    EV<<"Controller will wait "<<(int)par("waitBeforeNext")<<"seconds between each turn"<<endl;
    EV<<"Controller: first source will be "<<nextSource<<endl;
    scheduleAt(simTime()+(int)par("waitBeforeNext"), nextTurnEvent);
}



void Controller::handleMessage(cMessage *msg)
{
    ASSERT(msg == nextTurnEvent);
    EV<<"Controller: Turn of source "<<nextSource<<endl;

    //connect to next source
    cModule * nextSourceModule = getParentModule()->getSubmodule("isps", nextSource);
    if(nextSourceModule==NULL){
        EV<<"ERROR NULL POINTER MODULE (MODULE NOT FOUND) "<<nextSource<<endl;
    } else {
        cGate * nextSourceGate = nextSourceModule->gate("in");
        cGate * outGate = gate("out");
        outGate->disconnect();//just in case...
        outGate->connectTo(nextSourceGate);
        EV<<"Controller: connected to source "<<nextSource<<" sending ACT message"<<endl;
        //message
        cMessage* actMessage = new cMessage("ACT");
        send(actMessage, "out");
        outGate->disconnect();
    }

    //next source here
    nextSource++;
    if(nextSource >= (int)par("sources")){
        nextSource=0;
    }

    EV<<"Controller: Next Source will be "<<nextSource<<endl;
    scheduleAt(simTime()+(int)par("waitBeforeNext"), nextTurnEvent);
}
