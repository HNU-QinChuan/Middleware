#pragma once
#include <chrono>
#include <thread>
#include <atomic>
#include <functional>
#include <mutex>

namespace Hnu::Uwb {

class SimpleTimeSlotManager {
public:
    static constexpr uint32_t SLOT_DURATION_MS = 500;      // 500ms时隙总长度
    static constexpr uint32_t TRANSMISSION_WINDOW_MS = 400; // 400ms传输窗口
    static constexpr uint32_t GUARD_INTERVAL_MS = 100;     // 100ms保护间隔
    static constexpr uint8_t MAX_NODES = 3;                // 3个节点
    
    SimpleTimeSlotManager(uint8_t node_id);
    ~SimpleTimeSlotManager();
    
    void start();
    void stop();
    
    // 获取时隙信息
    uint32_t get_current_slot() const;
    bool is_my_slot() const;
    uint8_t get_slot_owner() const;
    uint32_t get_time_in_slot_ms() const;
    
    // 新增：传输窗口相关功能
    bool is_in_transmission_window() const;
    bool can_transmit() const;
    uint32_t get_transmission_time_remaining() const;
    
    // 事件回调
    void set_my_slot_callback(std::function<void()> callback);
    
    uint8_t get_node_id() const { return m_node_id; }
    
private:
    // 全局时隙系统起始时间（所有节点共用的固定基准时间）
    static constexpr uint64_t GLOBAL_EPOCH_MS = 1704067200000;  // 2024-01-01 00:00:00 UTC
    
    uint8_t m_node_id;
    std::atomic<bool> m_running{false};
    std::thread m_monitor_thread;
    
    uint32_t m_last_slot = 0xFFFFFFFF;
    
    std::function<void()> m_my_slot_callback;
    std::mutex m_callback_mutex;
    
    void monitor_thread();
};

} // namespace Hnu::Uwb