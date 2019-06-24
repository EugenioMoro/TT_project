#include <omnetpp.h>
#include "arrivalRateMsg_m.h"
using namespace omnetpp;

class cCompletedServiceNotif: public omnetpp::cObject, public omnetpp::noncopyable {
public:
    int queueId;
    simtime_t time;
};

class Queue : public cSimpleModule
{
protected:
    cMessage *msgServiced;
    cMessage *endServiceMsg;

    arrivalRateMsg *arMessage;

    cQueue queue;
    simsignal_t qlenSignal;
    simsignal_t busySignal;
    simsignal_t queueingTimeSignal;
    simsignal_t responseTimeSignal;
    simsignal_t seviceCompletedSignal;
    simsignal_t interarrivalTimesSignal;
    cCompletedServiceNotif tmp;
    simtime_t lastArrivalTimestamp;

    double arrivalCounts;
    simtime_t avgWindowStart;
    simtime_t avgWindowEnd;

    int queueId;

public:
    Queue();
    virtual ~Queue();

protected:
    virtual void initialize() override;
    virtual void handleMessage(cMessage *msg) override;
    double getLambda();
};

Define_Module(Queue);


Queue::Queue()
{
    msgServiced = endServiceMsg = nullptr;
}

Queue::~Queue()
{
    delete msgServiced;
    cancelAndDelete(endServiceMsg);
}

void Queue::initialize()
{
    queueId=par("queueId");
    endServiceMsg = new cMessage("end-service");
    queue.setName("queue" + queueId);

    qlenSignal = registerSignal("qlen");
    busySignal = registerSignal("busy");
    queueingTimeSignal = registerSignal("queueingTime");
    responseTimeSignal = registerSignal("responseTime");
    interarrivalTimesSignal = registerSignal("interarrivalTimes");

    //register the signal of service completion
    seviceCompletedSignal = registerSignal("seviceCompletedSignal");

    emit(qlenSignal, queue.getLength());
    emit(busySignal, false);

    lastArrivalTimestamp = simTime();
    arrivalCounts=0;
    avgWindowStart=simTime();
}

void Queue::handleMessage(cMessage *msg)
{
    if (msg == endServiceMsg) { // Self-message arrived
        EV << "Completed service of " << msgServiced->getName() << endl;
        send(msgServiced, "out");

        //Response time: time from msg arrival timestamp to time msg ends service (now)
        emit(responseTimeSignal, simTime() - msgServiced->getTimestamp());

        //build and emit the service completion signal
        //tmp.queueId = this->queueId;
        //tmp.time = simTime() - msgServiced->getTimestamp();
        //EV<<"emitting the DIO signal"<<endl;
        //if (hasListeners(seviceCompletedSignal)){
        //    EV<<"listener c'è"<<endl;
        //}
        //emit(seviceCompletedSignal, &tmp);

        if (queue.isEmpty()) { // Empty queue, server goes in IDLE

            EV << "Empty queue, server goes IDLE" <<endl;
            msgServiced = nullptr;
            emit(busySignal, false);

        }
        else{// Queue contains users

            msgServiced = (cMessage *)queue.pop();
            emit(qlenSignal, queue.getLength()); //Queue length changed, emit new length!

            //Waiting time: time from msg arrival to time msg enters the server (now)
            emit(queueingTimeSignal, simTime() - msgServiced->getTimestamp());

            EV << "Starting service of " << msgServiced->getName() << endl;
            simtime_t serviceTime = par("serviceTime");
            scheduleAt(simTime()+serviceTime, endServiceMsg);
        }
    }
    else { // Data msg has arrived

        if(strcmp(msg->getName(),"arMessage") == 0) {
            arMessage = check_and_cast<arrivalRateMsg *>(msg);
            EV<<"Queue "<<queueId<<": received arrival rate request from source: "<<arMessage->getSourceID()<<endl;
            int sourceId=arMessage->getSourceID();
            //delete arMessage;
            arrivalRateMsg* response = new arrivalRateMsg("arMessage");
            //put ask bool to false
            response->setAsk(false);
            //add lambda and mu to message
            response->setLambda(getLambda());
            response->setMu(1/(double)par("serviceTime"));
            //connect to source and send
            cModule * answerTo = getParentModule()->getSubmodule("isps", arMessage->getSourceID());
                       if(answerTo==NULL){
                           EV<<"ERROR NULL POINTER MODULE (MODULE NOT FOUND) "<<arMessage->getSourceID()<<endl;
                       } else {
                       cGate * answerGate = answerTo->gate("in");
                       cGate * outGate = gate("arRequestGateOut");
                       outGate->disconnect();
                       outGate->connectTo(answerGate);
                       EV<<"Queue :"<<queueId<<": answering to source "<<arMessage->getSourceID()<<" with lambda="<<getLambda()<<endl;
                       send(response, outGate);
                       outGate->disconnect();
                       }
            //ready for next message
            return;
        }

        if(strcmp(msg->getName(),"DISCONNECT") == 0 || strcmp(msg->getName(),"CONNECT") == 0){
            //if connection variation, reset average window and arrival counts
            EV<<"Queue "<<queueId<<": received connect/disconnect message, resetting avg window"<<endl;
            avgWindowStart=simTime();
            arrivalCounts = 0;
            return;
        }

        //for debug pourposes
        //EV<<getLambda()<<endl;

        //Setting arrival timestamp as msg field
        msg->setTimestamp();
        //compute the interarrival time between this and the previous msg and the signal it
        emit(interarrivalTimesSignal, msg->getTimestamp()-lastArrivalTimestamp);
        //now update the new last arrival time
        lastArrivalTimestamp = msg->getTimestamp();

        //update the counter of the arrivals
        arrivalCounts++;


        if (!msgServiced) { //No message in service (server IDLE) ==> No queue ==> Direct service

            ASSERT(queue.getLength() == 0);

            msgServiced = msg;
            emit(queueingTimeSignal, SIMTIME_ZERO);

            EV << "Starting service of " << msgServiced->getName() << endl;
            simtime_t serviceTime = exponential((double)par("serviceTime"), 0);
            scheduleAt(simTime()+serviceTime, endServiceMsg);
            emit(busySignal, true);
        }
        else {  //Message in service (server BUSY) ==> Queuing
            EV << msg->getName() << " enters queue"<< endl;
            queue.insert(msg);
            emit(qlenSignal, queue.getLength()); //Queue length changed, emit new length!

        }
    }
}

double Queue::getLambda(){
    //compute the lambda value by dividing the arrivals by the length of the avg window
    simtime_t windowLen = simTime() - avgWindowStart;
    return arrivalCounts/windowLen.inUnit(SIMTIME_S);
}

