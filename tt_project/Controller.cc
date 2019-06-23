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
    nextSource=(int)par("sources") - 1;
    EV<<"Controller Initialized:"<<endl;
    EV<<"Controller will wait "<<(int)par("waitBeforeNext")<<"seconds between each turn"<<endl;
    scheduleAt(simTime(), nextTurnEvent);
}



void Controller::handleMessage(cMessage *msg)
{
    ASSERT(msg == nextTurnEvent);
    EV<<"Controller: Turn of source "<<nextSource<<endl;

    //message to source

    //next source here
    nextSource = (nextSource++) % (int)par("sources");
    scheduleAt(simTime()+(int)par("waitBeforeNext"), nextTurnEvent);
}
