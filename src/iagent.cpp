#include "iagent.hpp"

IAgent::IAgent() {
    running = false;
    sleeper.stop();
}

IAgent::~IAgent() {
    this->stop();
}

void IAgent::start(std::vector<Message::node> mNodes) {
    sleeper.start();
    running = true;
}

void IAgent::stop() {
    running = false;
    sleeper.stop();
}

void IAgent::setParent(Node * node) {
    this->node = node;
}