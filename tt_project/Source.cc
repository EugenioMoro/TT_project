 #include <omnetpp.h>

using namespace omnetpp;


class Source : public cSimpleModule
{
  private:
    cMessage *sendMessageEvent;
    int nbGenMessages;

    int sourceId; //the id of this source
    int associatedQueueId;  //the id of the queue at which this source is connected

  public:
    Source();
    virtual ~Source();

  protected:
    virtual void initialize() override;
    virtual void handleMessage(cMessage *msg) override;
};

Define_Module(Source);

Source::Source()
{
    sendMessageEvent = nullptr;
}

Source::~Source()
{
    cancelAndDelete(sendMessageEvent);
}

void Source::initialize()
{
    sendMessageEvent = new cMessage("sendMessageEvent");
    scheduleAt(simTime(), sendMessageEvent);
    nbGenMessages = 0;
    sourceId=par("sourceId");
}

void Source::handleMessage(cMessage *msg)
{
    ASSERT(msg == sendMessageEvent);


    char msgname[20];
    sprintf(msgname, "message-%d", ++nbGenMessages);
    cMessage *message = new cMessage(msgname);
    send(message, "out");

    //char queueStringBuffer[20];
    //sprintf(queueStringBuffer,"isps[%d]", sourceId);

    //cModule * nextQueue = getModuleByPath(queueStringBuffer);
    //cModule * nextQueue = getModuleByPath("Sink");
    int nextQueueId=intuniform(0,(int)getParentModule()->par("n_queues")-1);

    cModule * nextQueue = getParentModule()->getSubmodule("queues", nextQueueId);
    if(nextQueue==NULL){
        EV<<"ORCODIOOOOVOIDDDDDDDDDDDDDDDDDD"<<nextQueueId<<endl;
    } else {
    EV<<"ORCODIOOOO"<<nextQueue->getName()<<sourceId<<endl;
    EV<<nextQueue->getName();
    EV<<sourceId;
    cGate * nextQueueGate = nextQueue->gate("in", sourceId);
    cGate * outGate = gate("out");
    outGate->disconnect();
    outGate->connectTo(nextQueueGate);
    }

    scheduleAt(simTime()+par("interArrivalTime").doubleValue(), sendMessageEvent);
}
