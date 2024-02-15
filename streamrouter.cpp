#include <iostream>
#include <string>
#include <unordered_set>
#include <unordered_map>
#include <vector>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <cstring>
#include <cstdlib>

int main(int argc, char* argv[]) {
    if (argc < 4) {
        std::cout << "Usage: " << argv[0] << " <ip> <port> <blocksize>" << std::endl;
        return 1;
    }

    const std::string UDP_IP = argv[1];
    const int UDP_PORT = std::stoi(argv[2]);
    const int BLOCKSIZE = std::stoi(argv[3]);

    std::unordered_set<unsigned int> even_uids, odd_uids;
    std::unordered_map<unsigned int, sockaddr_in> uid_ip_port_mapping;
    std::unordered_map<unsigned int, unsigned int> uid_last_sequence;

    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    sockaddr_in servaddr{}, cliaddr{};
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = inet_addr(UDP_IP.c_str());
    servaddr.sin_port = htons(UDP_PORT);

    bind(sockfd, (const sockaddr*)&servaddr, sizeof(servaddr));
    std::cout << "Router is listening at " << UDP_IP << ":" << UDP_PORT << std::endl;

    auto uidstr = [](unsigned int uid) {
        return std::to_string(uid % 1000).insert(0, 3 - std::to_string(uid % 1000).length(), '0');
    };

    auto reset_everything = [&](unsigned int uid) {
        even_uids.clear();
        odd_uids.clear();
        uid_ip_port_mapping.clear();
        uid_last_sequence.clear();
        std::cout << "Resetting over a new host uid: " << uid << std::endl;
    };

    auto multicast_message = [&](unsigned int uid, const char* message, size_t message_len) {
        const auto& target_set = uid % 2 == 0 ? odd_uids : even_uids;
        for (auto target_uid : target_set) {
            if (uid_ip_port_mapping.find(target_uid) != uid_ip_port_mapping.end()) {
                sendto(sockfd, message, message_len, 0, (const sockaddr*)&uid_ip_port_mapping[target_uid], sizeof(sockaddr_in));
            }
        }
    };

    char buffer[1024];
    while (true) {
        socklen_t len = sizeof(cliaddr);
        int n = recvfrom(sockfd, buffer, 1024, MSG_WAITALL, (sockaddr*)&cliaddr, &len);
        if (n > 0) {
            unsigned int uid, sequence;
            std::memcpy(&uid, buffer, sizeof(unsigned int));
            std::memcpy(&sequence, buffer + sizeof(unsigned int), sizeof(unsigned int));

            if (sequence % BLOCKSIZE != 0) {
                std::cout << "** Received out of sequence message **" << std::endl;
            }

            bool is_even_user = uid % 2 == 0;
            bool is_new_user = even_uids.find(uid) == even_uids.end() && odd_uids.find(uid) == odd_uids.end();

            if (is_new_user) {
                if (is_even_user) {
                    reset_everything(uid);
                    even_uids.insert(uid);
                } else {
                    odd_uids.insert(uid);
                }
                uid_ip_port_mapping[uid] = cliaddr;
            }

            if (uid_last_sequence.find(uid) != uid_last_sequence.end() && uid_last_sequence[uid] != sequence - BLOCKSIZE) {
                std::cout << uidstr(uid) << " window: " << sequence - BLOCKSIZE << " was not received" << std::endl;
            }
            uid_last_sequence[uid] = sequence;

            if (!odd_uids.empty() && !even_uids.empty()) {
                multicast_message(uid, buffer, static_cast<size_t>(n));
            } else {
                std::cout << uidstr(uid) << ", " << sequence << std::endl;
            }
        }
    }
    return 0;
}

