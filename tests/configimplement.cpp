std::unique_ptr<HpMqttPub> HpMqttPub::fromConfigFile(const std::string &path, const std::string &site, std::string &err) {
    Config cfg;
    if (!cfg.loadFromFile(path, err)) return nullptr;
    if (!cfg.hasSection(site)) { err = "Config has no section '" + site + "'"; return nullptr; }
    auto broker = cfg.get(site, "broker", "");
    if (broker.empty()) { err = "broker not set in config for site '" + site + "'"; return nullptr; }
    auto clientId = cfg.get(site, "clientId", "");
    if (clientId.empty()) {
        // default clientId: hanport-<site>
        clientId = std::string("hanport-") + site;
    }
    auto deviceId = cfg.get(site, "deviceId", "");
    if (deviceId.empty()) { err = "deviceId not set in config for site '" + site + "'"; return nullptr; }

    auto p = std::unique_ptr<HpMqttPub>(new HpMqttPub(broker, clientId, site, deviceId));

    // optional creds
    auto username = cfg.get(site, "username", "");
    auto password = cfg.get(site, "password", "");
    if (!username.empty() || !password.empty()) p->setCrendetials(username, password);

    // optional last will
    auto will_topic = cfg.get(site, "will_topic", "");
    auto will_msg = cfg.get(site, "will_msg", "");
    if (!will_topic.empty()) p->setLastWill(will_topic, will_msg);

    return p;
}