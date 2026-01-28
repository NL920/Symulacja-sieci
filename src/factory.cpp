#include "factory.hxx"
#include <sstream>
#include <stdexcept>
#include <cctype>
#include <map>
#include <string>
#include <algorithm>

enum class ElementType { NONE, RAMP, WORKER, STOREHOUSE, LINK };

struct ParsedLineData {
    ElementType type = ElementType::NONE;
    std::map<std::string, std::string> params;
};

static ElementType parse_element_type(const std::string& token) {
    if (token == "LOADING_RAMP") return ElementType::RAMP;
    if (token == "WORKER") return ElementType::WORKER;
    if (token == "STOREHOUSE") return ElementType::STOREHOUSE;
    if (token == "LINK") return ElementType::LINK;
    throw std::runtime_error("Unknown element type: " + token);
}

static ParsedLineData parse_line(const std::string& line) {
    ParsedLineData parsed;
    if (line.empty() || line[0] == '#' || line[0] == ';') return parsed;

    std::istringstream iss(line);
    std::string token;
    iss >> token;
    parsed.type = parse_element_type(token);

    while (iss >> token) {
        auto pos = token.find('=');
        if (pos == std::string::npos)
            throw std::runtime_error("Invalid token: " + token);
        parsed.params[token.substr(0, pos)] = token.substr(pos + 1);
    }

    return parsed;
}


void Factory::load_factory_structure(std::istream& is) {
    std::string line;

    while (std::getline(is, line)) {
        std::string trimmed = line;
        trimmed.erase(0, trimmed.find_first_not_of(" \t\r\n"));
        trimmed.erase(trimmed.find_last_not_of(" \t\r\n") + 1);

        if (trimmed.empty() || trimmed[0] == '#' || trimmed[0] == ';')
            continue;

        ParsedLineData parsed = parse_line(trimmed);

        switch (parsed.type) {
            case ElementType::RAMP: {
                add_ramp(Ramp(
                    std::stoi(parsed.params.at("id")),
                    std::stoi(parsed.params.at("delivery-interval"))
                ));
                break;
            }
            case ElementType::WORKER: {
                PackageQueueType qt =
                    parsed.params.at("queue-type") == "LIFO"
                        ? PackageQueueType::LIFO
                        : PackageQueueType::FIFO;

                add_worker(Worker(
                    std::stoi(parsed.params.at("id")),
                    std::stoi(parsed.params.at("processing-time")),
                    std::make_unique<PackageQueue>(qt)
                ));
                break;
            }
            case ElementType::STOREHOUSE: {
                add_storehouse(Storehouse(
                    std::stoi(parsed.params.at("id"))
                ));
                break;
            }
            case ElementType::LINK: {
                auto split = [](const std::string& s) {
                    auto p = s.find('-');
                    if (p == std::string::npos)
                        throw std::runtime_error("Invalid link format");
                    return std::make_pair(s.substr(0, p), std::stoi(s.substr(p + 1)));
                };

                auto [src_type, src_id] = split(parsed.params.at("src"));
                auto [dst_type, dst_id] = split(parsed.params.at("dest"));

                IPackageReceiver* receiver = nullptr;

                if (dst_type == "worker") {
                    auto it = find_worker_by_id(dst_id);
                    if (it == container_worker_.end())
                        throw std::runtime_error("Worker not found");
                    receiver = &(*it);
                } else {
                    auto it = find_storehouse_by_id(dst_id);
                    if (it == container_storehouse_.end())
                        throw std::runtime_error("Storehouse not found");
                    receiver = &(*it);
                }

                if (src_type == "ramp") {
                    find_ramp_by_id(src_id)->receiver_preferences_.add_receiver(receiver);
                } else {
                    find_worker_by_id(src_id)->receiver_preferences_.add_receiver(receiver);
                }
                break;
            }
            default:
                break;
        }
    }
}


void Factory::save_factory_structure(std::ostream& os) const {
    for (const auto& ramp : container_ramp_) {
        os << "LOADING_RAMP id=" << ramp.get_id()
           << " delivery-interval=" << ramp.get_delivery_interval() << "\n";
    }

    for (const auto& worker : container_worker_) {
        os << "WORKER id=" << worker.get_id()
           << " processing-time=" << worker.get_processing_duration()
           << " queue-type="
           << (worker.get_queue()->get_queue_type() == PackageQueueType::FIFO ? "FIFO" : "LIFO")
           << "\n";
    }

    for (const auto& storehouse : container_storehouse_) {
        os << "STOREHOUSE id=" << storehouse.get_id() << "\n";
    }

    for (const auto& ramp : container_ramp_) {
        for (auto& [receiver, _] : ramp.receiver_preferences_.get_preferences()) {
            os << "LINK src=ramp-" << ramp.get_id()
               << " dest="
               << (dynamic_cast<const Worker*>(receiver) ? "worker-" : "store-")
               << receiver->get_id() << "\n";
        }
    }

    for (const auto& worker : container_worker_) {
        for (auto& [receiver, _] : worker.receiver_preferences_.get_preferences()) {
            os << "LINK src=worker-" << worker.get_id()
               << " dest="
               << (dynamic_cast<const Worker*>(receiver) ? "worker-" : "store-")
               << receiver->get_id() << "\n";
        }
    }
}


Factory load_factory_structure(std::istream& is) {
    Factory f;
    f.load_factory_structure(is);
    return f;
}

void save_factory_structure(const Factory& factory, std::ostream& os) {
    factory.save_factory_structure(os);
}
