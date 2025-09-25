#include <iostream>
#include <chrono>
#include <mutex>
#include <cstdint>
#include <thread>
#include <stdexcept>

class DistributedIDGenerator {
public:
    // 静态接口，传入node_id，只能第一次调用时设置，后续调用忽略
    static int next_id(uint8_t node_id = 0) {
        static DistributedIDGenerator instance(node_id);
        return instance.generate_id();
    }

private:
    explicit DistributedIDGenerator(uint8_t node_id) {
        if (node_id >= 16) {
            throw std::invalid_argument("Node ID must be in [0, 15]");
        }
        this->node_id = node_id;
    }

    //分布式ID，Snowflake压缩为32位
    int generate_id() {
        std::lock_guard<std::mutex> lock(mtx);

        uint32_t current_time = get_current_timestamp();
        if (current_time == last_timestamp) {
            if (sequence < max_sequence) {
                sequence++;
            } else {
                // 等待下一秒
                while (get_current_timestamp() == last_timestamp) {
                    std::this_thread::sleep_for(std::chrono::milliseconds(1));
                }
                current_time = get_current_timestamp();
                last_timestamp = current_time;
                sequence = 0;
            }
        } else {
            last_timestamp = current_time;
            sequence = 0;
        }

        uint32_t id = ((current_time & time_mask) << 10) | (node_id << 6) | sequence;
        return id;
    }

private:
    static constexpr uint32_t epoch_start = 1735689600; // 2025-01-01 00:00:00 UTC
    static constexpr uint8_t max_sequence = 63;
    static constexpr uint32_t time_mask = (1 << 22) - 1; // 22-bit mask

    uint8_t node_id;
    uint8_t sequence = 0;
    uint32_t last_timestamp = 0;
    std::mutex mtx;

    uint32_t get_current_timestamp() {
        using namespace std::chrono;
        uint32_t now = static_cast<uint32_t>(
            duration_cast<seconds>(system_clock::now().time_since_epoch()).count()
        );
        return now - epoch_start;
    }
};
