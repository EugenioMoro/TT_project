#include <omnetpp.h>
#include "arrivalRateMsg_m.h"

using namespace omnetpp;


class Source : public cSimpleModule
{
private:
    cMessage *sendMessageEvent;
    int nbGenMessages;

    int sourceId; //the id of this source
    int associatedQueueId;  //the id of the queue at which this source is connected
    double associatedQueueWT;

    int nextQueueToAsk;
    double candidateWT;
    int candidateQueueId;

    int n_queues;
public:
    Source();
    virtual ~Source();

protected:
    virtual void initialize() override;
    virtual void handleMessage(cMessage *msg) override;
    void askNextQueue(int nextQueueToAsk);
    void connectToQueue(int queueId);
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
    associatedQueueId = (int) par("associatedQueueId");
    n_queues = (int) par("nQueues");
    nextQueueToAsk=0;
    associatedQueueWT=1e8;
}

void Source::handleMessage(cMessage *msg)
{
    if(strcmp(msg->getName(),"ACT") == 0){
        EV<<"Source "<<sourceId<<":ACT message received from controller"<<endl;

        //round robin asking for lambda
        //reset relevant variables
        //  the candidate is always the one I'm already connected to
        candidateWT=1e8;
        candidateQueueId=associatedQueueId;
        nextQueueToAsk=0;
        askNextQueue(nextQueueToAsk);
        delete msg;
        return;
    }

    if(strcmp(msg->getName(),"arMessage") == 0) {
        arrivalRateMsg* arMessage = check_and_cast<arrivalRateMsg *>(msg);
        EV<<"Source "<<sourceId<<": answer received from queue "<<nextQueueToAsk<<" with lambda="<<arMessage->getLambda()<<" and mu "<<arMessage->getMu()<<endl;
        //compute expected waiting time
        double lambda = arMessage->getLambda();
        double myLambda =(1/(double)par("interArrivalTime"));
        EV<<"My lambda: "<<myLambda<<endl;
        if(nextQueueToAsk==associatedQueueId){
            //this is the queue I am connected to, I need to subtract my lambda
            lambda=lambda-myLambda;
        }
        double expWT = 1/(arMessage->getMu()-(lambda+myLambda));
        if(expWT<=0){
            EV<<"Source "<<sourceId<<": queue "<<nextQueueToAsk<<" would be overloaded, discarding"<<endl;
            if(nextQueueToAsk==associatedQueueId){
                associatedQueueWT=10^8;
            }
        } else{
            EV<<"Source "<<sourceId<<": waiting time of this queue is "<<expWT<<" seconds, the best one so far is "<<candidateWT<<endl;

            //update candidate queue if needed
            if(nextQueueToAsk==associatedQueueId){
                associatedQueueWT=expWT;
            }

            if(expWT<candidateWT){
                EV<<"Since expwt "<<expWT<<" is smaller than the candidate "<<candidateWT<<" the candidate is "<<nextQueueToAsk<<" instead of "<<candidateQueueId<<endl;
                candidateWT=expWT;
                candidateQueueId=nextQueueToAsk;
            }
        }
        //continue with next queue
        nextQueueToAsk++;
        //if this was last queue, select the best
        if(nextQueueToAsk>=n_queues){
            EV<<"Source "<<sourceId<<": all the queues analyzed"<<endl;
            //if the candidate is different than the current then switch
            if(candidateQueueId==associatedQueueId){
                EV<<"Source "<<sourceId<<": no gain from switching, I'll stay connected to the same source"<<endl;
                return;
            } else {
                connectToQueue(candidateQueueId); //-1 is needed because we have just incremented it
            }
            return;
        } //thiw was not the last queue, go on with the next
        EV<<"Source "<<sourceId<<": turn of queue no "<<nextQueueToAsk+1<<" of "<<n_queues<<endl;
        askNextQueue(nextQueueToAsk);
        return;
    }

    ASSERT(msg == sendMessageEvent);


    char msgname[20];
    sprintf(msgname, "message-%d", ++nbGenMessages);
    cMessage *message = new cMessage(msgname);
    send(message, "out");

    //char queueStringBuffer[20];
    //sprintf(queueStringBuffer,"isps[%d]", sourceId);

    //cModule * nextQueue = getModuleByPath(queueStringBuffer);
    //cModule * nextQueue = getModuleByPath("Sink");

    scheduleAt(simTime()+exponential((double)par("interArrivalTime"), 0), sendMessageEvent);
}

void Source::askNextQueue(int nextQueueToAsk){
    //connect to next queue to ask and send lambda request
    cModule * nextQueueModule = getParentModule()->getSubmodule("queues", nextQueueToAsk);
    if(nextQueueModule==NULL){
        EV<<"ERROR NULL POINTER MODULE (MODULE NOT FOUND) "<<nextQueueToAsk<<endl;
    } else {
        cGate * nextQueueGate = nextQueueModule->gate("arRequestGateIn");
        cGate * outGate = gate("responseTimeOutGateway");
        outGate->disconnect();
        outGate->connectTo(nextQueueGate);
        EV<<"Source :"<<sourceId<<": connecting to queue "<<nextQueueToAsk<<" to ask for lambda"<<endl;

        arrivalRateMsg* arMessage= new arrivalRateMsg("arMessage");
        arMessage->setAsk(true);
        arMessage->setSourceID(sourceId);
        send(arMessage, outGate);
        outGate->disconnect();
    }
}

void Source::connectToQueue(int queueId){
    //send disconnect message to current queue
    cMessage* disconnectMsg = new cMessage("DISCONNECT");
    send(disconnectMsg,"out");

    //connect to the new queue
    cModule * nextQueueModule = getParentModule()->getSubmodule("queues", queueId);
    if(nextQueueModule==NULL){
        EV<<"ERROR NULL POINTER MODULE (MODULE NOT FOUND) "<<queueId<<endl;
    } else {
        cGate * nextQueueGate = nextQueueModule->gate("in", sourceId);
        cGate * outGate = gate("out");
        outGate->disconnect();
        outGate->connectTo(nextQueueGate);
        EV<<"Source :"<<sourceId<<": connecting to queue "<<queueId<<" for service"<<endl;

        //send connect message
        cMessage* connectMsg = new cMessage("CONNECT");
        send(connectMsg,"out");
    }

    //update internal queue reference
    associatedQueueId=queueId;
    associatedQueueWT=candidateWT;
}
