#include "hpMqttPub.hpp"
#include "mqtt/connect_options.h"
#include "mqtt/message.h"
#include <iostream>
#include <sstream>
#include <iomanip>
#include <stdexcept>




//constructor for mqtt client with broker address and client id
hpMqttPub::hpMqttPub(const std::string& brokerAddress, const std::string& clientId)
    : BrokerAddress_(brokerAddress),
      clientId_(clientId),
      site_(""),
      deviceId(""),
      hasCredentials_(false),
      hasWill_(false) {
    // try to initialize mqtt client if not intialized catch exceptions thrown exception catched
    try {
        client_ = std_make_unique<mqtt::async_client>(brokerAddress, clientId);
        std::cout << "MQTT client created for broker: " << brokerAddress << std::endl;
    } catch (const std::exception &e) {
        std::cerr << "Error creating MQTT client: " << e.what() << std::endl;
        throw;
    }
}

hpMqttPub::~hpMqttPub(){
    // try to disconnect from broker if connected and catch all execptions if failed
    if (client_ && client_->is_connected()) {
       try{
          disconnect();
       }catch(...){
       }
    }
}
//set the site of the client
void hpMqttPub::setSite(const std::string& site){
    site_ = site;
}
void hpMqttPub::setDeviceId(const std::string& deviceId){
    deviceId = deviceId;
}

std::string hpMqttPub::buildBaseTopic() const{

}

