#ifndef HPMQTT_PUB_HPP
#define HPMQTT_PUB_HPP
#include <string>
#include <memory>
#include <mqtt/async_client.h>
#include <hpData.hpp>


class HpMqttPub{
    public:
        HpMqttPub(const std::string& brokerAddress, const std::string& clientId);
        //connect to broker
        bool connect();
        //Disconnect from the broker
        void disconnect();
        //Check if connected
        bool isConnected();
        //set the site name as root topic
        bool setSite(const std::string& site);
        //set the device id as subtopic
        bool setDeviceId(const std::string& deviceId);
        //publish all hpData fields to indivdual topics
        //Each field gets its own topic: {site}/{deviceId}/{field_name}
        bool publishAllData(const hpData& data);
        //publish all hpData fields with custom site and device id
        bool publishAllData(const hpData& data, std::string& site, std::string& deviceId);
        //publish a single measurement to a topic
        bool publishMeasurement(const std::string& measurementName, float value);
        //publish a raw message to any topic
        bool publish(const std::string& topic, const std::string& payload, int qos = 1);
        // Set username and password for authetication (optional)
        void setCrendetials(const std::string& username, const std::string& password);
        // Set last will and testament if connecting client connection is broken
        void setLastWill(const std::string& topic,const std::string& message);

        //Get the current base topic path
        std::string getBaseTopic() const;
        ~HpMqttPub();
    private:
        //smart pointer to mqtt_client and attributes to mqtt client
        std::unique_ptr<mqtt::async_client> client_;
        std::string brokerAddress_;
        std::string clientId_;
        std::string username_;
        std::string password_;
        std::string willTopic_;
        std::string willMessage_;
        std::string site_;
        std::string deviceId_;
        bool hasCredentials_;
        bool hasWill_;
        //build base topic path: {site}/{deviceId}
        std::string buildBaseTopic() const;
        //build full topic path: {site}/{deviceId}/{measurement}
        std::string buildTopic(const std::string& measurment) const;
        // Helper to publish a single float value
        bool publishFloat(const std::string& topic, float value);
        // Publish all individual fields from hpData
        bool publishIndividualFields(const hpData& data, const std::string& baseTopic);
};

#endif