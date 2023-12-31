#include <iostream>
#include <thread>
#include <mutex>
#include <queue>
#include <random>
#include <string>
#include "my_socket.h"

struct Point {
    double x;
    double y;
    static Point generate();
};

Point Point::generate() {
    static std::default_random_engine rnd;
    static std::uniform_real_distribution<double> dist(-1.0, 1.0);
    double x = dist(rnd);
    double y = dist(rnd);
    Point point = {x, y};
    return point;
}

class PiEstimation {
public:
    PiEstimation();
    void addPoint(const Point& point);
    std::string& serialize(std::string& output);
private:
    long long totalCount;
    long long insideCount;
};

void PiEstimation::addPoint(const Point& point) {
    ++this->totalCount;
    if (point.x * point.x + point.y * point.y <= 1) {
        ++this->insideCount;
    }
    std::cout << "Odhad pi: " << 4 * (double)this->insideCount / (double)this->totalCount << std::endl;
}

PiEstimation::PiEstimation() :
        totalCount(0),
        insideCount(0) {
}

std::string &PiEstimation::serialize(std::string &output) {
    // TODO PiEstimation::serialize
    output += std::to_string(totalCount) + " " + std::to_string(insideCount) + ";";
    return output;
}


class ThreadData {
public:
    ThreadData(long long replicationsCount, int bufferCapacity, MySocket* serverSocket);
    void produce();
    void consume();
private:
    const long long replicationsCount;
    const int bufferCapacity;
    std::queue<Point> buffer;
    std::mutex mutex;
    std::condition_variable isFull;
    std::condition_variable isEmpty;
    MySocket* serverSocket; // pomocou tohto atributu komunikujem so serverom
};

ThreadData::ThreadData(long long replicationsCount, int bufferCapacity, MySocket* serverSocket) :
        replicationsCount(replicationsCount),
        bufferCapacity(bufferCapacity),
        buffer(),
        mutex(),
        isFull(),
        isEmpty(),
        serverSocket(serverSocket) {

}

void ThreadData::produce() {
    for (long long i = 1; i <= this->replicationsCount; ++i) {
        Point item = Point::generate();
        {
            std::unique_lock<std::mutex> lock(this->mutex); // tu sa mutex rovno uzamkne // premenna ktorá obauje mutex
            while (static_cast<long long>(this->buffer.size()) >= bufferCapacity) {
                this->isEmpty.wait(lock);
            }
            this->buffer.push(item);
            this->isFull.notify_one();
        } // tu sa mutex automaticky odomkne
    }
}

void ThreadData::consume() {
    PiEstimation piEstimaton;
    for (long long i = 1; i <= this->replicationsCount; ++i) {
        Point item;

        {
            std::unique_lock<std::mutex> lock(this->mutex);
            while (this->buffer.size() <= 0) {
                this->isFull.wait(lock);
            }
            item = this->buffer.front();
            this->buffer.pop();
            this->isEmpty.notify_one();
        }
        std::cout << i << ": ";
        // KOMUNIKÁCIA SO SERVEROM
        piEstimaton.addPoint(item);
        if (i % 1000 == 0 && this->serverSocket != nullptr) { // po každom 1000com bode a mam vytvorené spojenie
            // tam posle odhad
            // TODO ThreadData::consume 1
            std::string output;
            piEstimaton.serialize(output);
            //this->serverSocket->sendData(piEstimaton.serialize(output));
            this->serverSocket->sendData(output);
        }
    }
    if (this->serverSocket != nullptr) {
        // TODO ThreadData::consume 2
        // na ukončenie komunikácie po spracovní všetkých bodov
        std::string output;
        this->serverSocket->sendData(piEstimaton.serialize(output));
        this->serverSocket->sendEndMessage(); // pre ukončenie komunikácie so serverom
    }
}

void produce(ThreadData& data) {
    data.produce();
}

void consume(ThreadData& data) {
    data.consume();
}

int main() {
    // 1. vytvorenie socketu + pripojenie sa na frios2... na port 17589 (porty od 10 000 do 20 000 sú dispozicii)
    MySocket* mySocket = MySocket::createConnection("frios2.fri.uniza.sk", 12545);

    ThreadData data(3000, 10, mySocket);
    std::thread thProduce(produce, std::ref(data));

    consume(data);
    thProduce.join();

    delete mySocket; // nutné, aby nevznikal memory leak
    mySocket = nullptr;

    return 0;
}
