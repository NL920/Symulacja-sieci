#include "factory.hxx"
#include <stdexcept>
#include <sstream>
#include <algorithm>

#include "../include/types.hxx"

ElementType parse_element_type(const std::string& token){
    if(token =="LOADING_RAMP") return ElementType::RAMP;
    if(token =="WORKER") return ElementType::WORKER;
    if(token =="STOREHOUSE") return ElementType::STOREHOUSE;
    if(token =="LINK") return ElementType::LINK;
    throw std::runtime_error("Unknown element type"+token); //wyrzuca błąd

}
ParsedLineData parse_line(const std::string& line)
{
    ParsedLineData parsed;
    if (line.empty() || line[0]=='#') 
        return parsed; //ignoruje komentarze i puste linie
    std::istringstream iss(line);
    std::string token;
    iss>>token;
    parsed.type=parse_element_type(token);
//key=value, znaczniku pary klucz-wartość (w postaci “klucz=wartość”, rozdzielone spacjami) i zachowaj je w kontenerze umożliwiającym ich wygodną analizę (np. w postaci mapy);
    while (iss>>token)
    {
        auto pos =token.find('=');
        if (pos==std::string::npos)
            throw std::runtime_error("Invalid token: "+ token);
        std::string key = token.substr(0, pos);
        std::string value = token. substr(pos +1);
        parsed.params[key]= value;

    }
    return parsed;
}
void Factory::load_factory_structure(std::istream& is)
{
    std::string line;
    while (std::getline(is, line))
    {
        if (line.empty() || std::all_of(line.begin(), line.end(), isspace) || line[0] == '#')
        {
            continue;
        }
        ParsedLineData parsed = parse_line(line);


        switch(parsed.type)
        {
            case ElementType::RAMP: {
                ElementID id = std::stoi(parsed.params.at("id"));
                TimeOffset di = std::stoi(parsed.params.at("delivery-interval"));
                add_ramp(Ramp(id, di));
                break;
            }
            case ElementType::WORKER: {
                ElementID id = std::stoi(parsed.params.at("id"));
                TimeOffset pd = std::stoi(parsed.params.at("processing-time"));
                PackageQueueType qt = (parsed.params.at("queue-type") == "LIFO") ? PackageQueueType::LIFO : PackageQueueType::FIFO;
                add_worker(Worker(id, pd, std::make_unique<PackageQueue>(qt)));
                break;
            }
            case ElementType::STOREHOUSE: {
                ElementID id = std::stoi(parsed.params.at("id"));
                add_storehouse(Storehouse(id));
                break;
            }
            case ElementType::LINK: {
                // Parsowanie src=ID i dest=ID
                std::string src_raw = parsed.params.at("src");
                std::string dest_raw = parsed.params.at("dest");
                auto split = [](const std::string& s) {
                    auto p = s.find('-');
                    if (p == std::string::npos) throw std::runtime_error("Invalid link format");
                    return std::make_pair(s.substr(0, p), std::stoi(s.substr(p + 1)));
                };

                auto [src_type, src_id] = split(src_raw);
                auto [dest_type, dest_id] = split(dest_raw);

                IPackageReceiver* receiver = nullptr;

                if (dest_type == "worker") {
                    auto it = workers_.find(dest_id);
                    if (it == workers_.end()) {
                        throw std::runtime_error("Link error: worker-" + std::to_string(dest_id) + " not found!"); // Semantyka!
                    }
                    receiver = it->second.get();
                } 
                else if (dest_type == "store") {
                     auto it = storehouses_.find(dest_id);
                    if (it == storehouses_.end()) {
                        throw std::runtime_error("Link error: store-" + std::to_string(dest_id) + " not found!");
                    }
                    receiver = it->second.get();
                }

    // 2. SZUKANIE NADAWCY (SRC) I DODANIE ODBIORCY
                if (src_type == "ramp") {
                    auto it = ramps_.find(src_id);
                    if (it == ramps_.end()) {
                        throw std::runtime_error("Link error: ramp-" + std::to_string(src_id) + " not found!");
                    }
                    it->second->receiver_preferences_.add_receiver(receiver);
                } 
                else if (src_type == "worker") {
                    auto it = workers_.find(src_id);
                    if (it == workers_.end()) {
                        throw std::runtime_error("Link error: worker-" + std::to_string(src_id) + " not found!");
                    }
                    it->second->receiver_preferences_.add_receiver(receiver);
                }
                break;
            }  
        }
    }
}
void Factory::save_factory_structure(std::ostream& os) const 
{

    for (const auto& [id, ramp] : ramps_) {
        os << "LOADING_RAMP id=" << id << " delivery-interval=" << ramp->get_delivery_interval() << "\n";
    }
    for (const auto& [id, worker] : workers_) {
        os << "WORKER id=" << id 
           << " processing-time=" << worker->get_processing_duration() 
           << " queue-type=FIFO\n"; 
    }
    for (const auto& [id, storehouse] : storehouses_) {
        os << "STOREHOUSE id=" << id << "\n";
    }
    auto save_links = [&](ElementID src_id, const auto& sender, std::string src_type) {
        for (const auto& [receiver, prob] : sender.receiver_preferences_.get_preferences()) {
            std::string dest_type = (dynamic_cast<Worker*>(receiver)) ? "worker" : "store";
            os << "LINK src=" << src_type << "-" << src_id << " dest=" << dest_type << "-" << receiver->get_id() << "\n";
        }
    };
for (const auto& [id, ramp] : ramps_) {
        for (const auto& [receiver, prob] : ramp->receiver_preferences_.get_preferences()) {
            os << "LINK src=ramp-" << id << " dest=";
            if (dynamic_cast<Worker*>(receiver)) os << "worker-";
            else os << "store-";
            os << receiver->get_id() << "\n";
        }
    }

    for (const auto& [id, worker] : workers_) {
        for (const auto& [receiver, prob] : worker->receiver_preferences_.get_preferences()) {
            os << "LINK src=worker-" << id << " dest=";
            if (dynamic_cast<Worker*>(receiver)) os << "worker-";
            else os << "store-";
            os << receiver->get_id() << "\n";
        }
    }
}

Ramp* Factory::find_ramp_by_id(ElementID id) {
    auto it = ramps_.find(id);
    return (it == ramps_.end()) ? nullptr : it->second.get();
}

Worker* Factory::find_worker_by_id(ElementID id) {
    auto it = workers_.find(id);
    return (it == workers_.end()) ? nullptr : it->second.get();
}

Storehouse* Factory::find_storehouse_by_id(ElementID id) {
    auto it = storehouses_.find(id);
    return (it == storehouses_.end()) ? nullptr : it->second.get();
}


bool Factory::is_consistent() const {
    try {
        for (const auto& [id, ramp] : ramps_) {
            if (ramp->receiver_preferences_.get_preferences().empty()) return false;
        }
        for (const auto& [id, worker] : workers_) {
            if (worker->receiver_preferences_.get_preferences().empty()) return false;
        }
    } catch (...) {
        return false;
    }
    return true;
}


Factory load_factory_structure(std::istream& is) {
    Factory f;
    f.load_factory_structure(is);
    return f;
}
// na początku wiedziałam co sie dzieje, aktualnie nie mam pojecia, cieszmy sie ze to działa testy wszystkie mi przeszły!!!!