#include "hpMqttPub.hpp"
#include "mqtt/connect_options.h"
#include "mqtt/message.h"
#include <iostream>
#include <sstream>
#include <iomanip>
#include <stdexcept>




//constructor for mqtt client with broker address and client id
HpMqttPub::HpMqttPub(const std::string& brokerAddress, const std::string& clientId,const std::string& site, const std::string& deviceId)
    : brokerAddress_(brokerAddress),
      clientId_(clientId),
      site_(site),
      deviceId_(deviceId),
      hasCredentials_(false),
      hasWill_(false) {
    // try to initialize mqtt client if not intialized catch exceptions thrown exception catched
    try {
        client_ = std::make_unique<mqtt::async_client>(brokerAddress, clientId);
        std::cout << "MQTT client created for broker: " << brokerAddress << std::endl;
    } catch (const std::exception &e) {
        std::cerr << "Error creating MQTT client: " << e.what() << std::endl;
        throw;
    }
}

HpMqttPub::~HpMqttPub(){
    // try to disconnect from broker if connected and catch all execptions if failed
    if (client_ && client_->is_connected()) {
       try{
          disconnect();
       }catch(...){
       }
    }
}

std::string HpMqttPub::buildBaseTopic() const{
    return site_ + "/" + deviceId_;
}

std::string HpMqttPub::buildTopic(const std::string& measurement) const{
    return buildBaseTopic() + "/" + measurement;
}

std::string HpMqttPub::getBaseTopic() const{
    return buildBaseTopic();
}

bool HpMqttPub::connect(){
    if(!client_) {
        std::cerr << "Mqtt client not initialized" << std::endl;
        return false;
    }
    try {
        //set connection options
        mqtt::connect_options connOpts;
        connOpts.set_keep_alive_interval(60);
        connOpts.set_clean_session(true);
        connOpts.set_automatic_reconnect(true);
        connOpts.set_automatic_reconnect(true);
        // if it needs password and username to connect set them.
        if(hasCredentials_){
            connOpts.set_user_name(username_);
            connOpts.set_password(password_);
        }
        //if last will is provided set it
        if(hasWill_){
            mqtt::message willMsg(willTopic_, willMessage_, 1, true);
            connOpts.set_will(willMsg);

        }

        std::cout << "Connecting to MQTT broker: " << brokerAddress_ << std::endl;
        auto tok = client_->connect(connOpts);
        tok->wait();

        std::cout << "Success fully connected to mqtt broker" << std::endl;
        return true;

    } catch (const mqtt::exception& e) {
       std::cerr << "Error connecting to mqtt broker: " << e.what() << std::endl;
       return false;
    }
}
//disconnect from broker
void HpMqttPub::disconnect(){
    if(client_ && client_->is_connected()){
      try {
        std::cout << "Disconnecting from MQTT broker" << std::endl;
        auto tok = client_->disconnect();
        tok->wait();
        std::cout << "Disconnected from MQTT broker" << std::endl;
      } catch (const mqtt::exception& e) {
        std::cerr << "Error disconneting from mqtt broker: " << e.what() << std::endl;
      }
   }
}



// to check if connected to broker
bool HpMqttPub::isConnected() const {
    return client_ && client_->is_connected();
}
// to publis message to broker
bool HpMqttPub::publish(const std::string& topic, const std::string& payload, int qos) {
    if(!isConnected()) {
        std::cerr << "Not connected to mqtt broker" << std::endl;
        return false;
    }
    try {
        auto msg = mqtt::make_message(topic, payload);
        msg->set_qos(qos);

        auto tok = client_->publish(msg);
        tok->wait();

        std::cout << "Published to '" << topic << "': " << payload << std::endl;
        return true;
    } catch (const mqtt::exception& e) {
        std::cerr << "Error publishing message: " << e.what() << std::endl;
        return false;
    }
}

bool HpMqttPub::publishFloat(const std::string& topic, float value) {
    std::ostringstream oss;
    oss << std::fixed << std::setprecision(2) << value;
    return publish(topic, oss.str(), 1);
}

bool HpMqttPub::publishIndividualFields(const hpData& data, const std::string& baseTopic) {
    bool allSuccess = true;

    // Timestamp
    allSuccess &= publishFloat(baseTopic + "/timestamp", data.time_stamp);

    // Energy measurements
    allSuccess &= publishFloat(baseTopic + "/energy/active_import_total", data.active_energy_import_total);
    allSuccess &= publishFloat(baseTopic + "/energy/active_export_total", data.active_energy_export_total);
    allSuccess &= publishFloat(baseTopic + "/energy/reactive_import_total", data.reactive_energy_import_total);
    allSuccess &= publishFloat(baseTopic + "/energy/reactive_export_total", data.reactive_energy_export_total);

    // Power measurements
    allSuccess &= publishFloat(baseTopic + "/power/active_import", data.active_power_import);
    allSuccess &= publishFloat(baseTopic + "/power/active_export", data.active_power_export);
    allSuccess &= publishFloat(baseTopic + "/power/reactive_import", data.reactive_power_import);
    allSuccess &= publishFloat(baseTopic + "/power/reactive_export", data.reactive_power_export);

    // L1 Phase measurements
    allSuccess &= publishFloat(baseTopic + "/l1/active_power_import", data.l1_active_power_import);
    allSuccess &= publishFloat(baseTopic + "/l1/active_power_export", data.l1_active_power_export);
    allSuccess &= publishFloat(baseTopic + "/l1/reactive_power_import", data.l1_reactive_power_import);
    allSuccess &= publishFloat(baseTopic + "/l1/reactive_power_export", data.l1_reactive_power_export);
    allSuccess &= publishFloat(baseTopic + "/l1/voltage_rms", data.l1_voltage_rms);
    allSuccess &= publishFloat(baseTopic + "/l1/current_rms", data.l1_current_rms);

    // L2 Phase measurements
    allSuccess &= publishFloat(baseTopic + "/l2/active_power_import", data.l2_active_power_import);
    allSuccess &= publishFloat(baseTopic + "/l2/active_power_export", data.l2_active_power_export);
    allSuccess &= publishFloat(baseTopic + "/l2/reactive_power_import", data.l2_reactive_power_import);
    allSuccess &= publishFloat(baseTopic + "/l2/reactive_power_export", data.l2_reactive_power_export);
    allSuccess &= publishFloat(baseTopic + "/l2/voltage_rms", data.l2_voltage_rms);
    allSuccess &= publishFloat(baseTopic + "/l2/current_rms", data.l2_current_rms);

    // L3 Phase measurements
    allSuccess &= publishFloat(baseTopic + "/l3/active_power_import", data.l3_active_power_import);
    allSuccess &= publishFloat(baseTopic + "/l3/active_power_export", data.l3_active_power_export);
    allSuccess &= publishFloat(baseTopic + "/l3/reactive_power_import", data.l3_reactive_power_import);
    allSuccess &= publishFloat(baseTopic + "/l3/reactive_power_export", data.l3_reactive_power_export);
    allSuccess &= publishFloat(baseTopic + "/l3/voltage_rms", data.l3_voltage_rms);
    allSuccess &= publishFloat(baseTopic + "/l3/current_rms", data.l3_current_rms);

    return allSuccess;
}

bool HpMqttPub::publishAllData(const hpData& data) {
    std::string baseTopic = buildBaseTopic();
    return publishIndividualFields(data, baseTopic);
}

bool HpMqttPub::publishAllData(const hpData& data, const std::string& site, const std::string& deviceId) {
    std::string baseTopic = site + "/" + deviceId;
    return publishIndividualFields(data, baseTopic);
}

bool HpMqttPub::publishMeasurement(const std::string& measurementName, float value) {
    std::string topic = buildTopic(measurementName);
    return publishFloat(topic, value);
}

void HpMqttPub::setCrendetials(const std::string& username, const std::string& password) {
    username_= username;
    password_ = password;
    hasCredentials_ = true;
}

void HpMqttPub::setLastWill(const std::string& topic, const std::string& message) {
    willTopic_ = topic;
    willMessage_= message,
    hasWill_ = true;
}


