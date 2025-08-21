#include "uwb/UwbTimeslotmanager.hpp"
#include <spdlog/spdlog.h>

namespace Hnu::Uwb {

SimpleTimeSlotManager::SimpleTimeSlotManager(uint8_t node_id) 
    : m_node_id(node_id) {
    spdlog::info("SimpleTimeSlotManager created for node {}", static_cast<int>(node_id));
}

SimpleTimeSlotManager::~SimpleTimeSlotManager() {
    stop();
}

void SimpleTimeSlotManager::start() {
    if (m_running.load()) return;
    
    m_running = true;
    m_monitor_thread = std::thread(&SimpleTimeSlotManager::monitor_thread, this);
    
    spdlog::info("SimpleTimeSlotManager started for node {}", static_cast<int>(m_node_id));
}

void SimpleTimeSlotManager::stop() {
    if (!m_running.load()) return;
    
    m_running = false;
    if (m_monitor_thread.joinable()) {
        m_monitor_thread.join();
    }
    
    spdlog::info("SimpleTimeSlotManager stopped for node {}", static_cast<int>(m_node_id));
}

uint32_t SimpleTimeSlotManager::get_current_slot() const {
    auto now = std::chrono::system_clock::now();
    auto ms_since_epoch = std::chrono::duration_cast<std::chrono::milliseconds>(
        now.time_since_epoch()).count();
    
    // 基于全局固定起始时间计算绝对时隙编号
    uint64_t elapsed_ms = ms_since_epoch - GLOBAL_EPOCH_MS;
    return elapsed_ms / SLOT_DURATION_MS;
}

bool SimpleTimeSlotManager::is_my_slot() const {
    uint32_t current_slot = get_current_slot();
    return (current_slot % MAX_NODES) == ((m_node_id - 1) % MAX_NODES);
}

uint8_t SimpleTimeSlotManager::get_slot_owner() const {
    uint32_t current_slot = get_current_slot();
    return (current_slot % MAX_NODES) + 1;
}

uint32_t SimpleTimeSlotManager::get_time_in_slot_ms() const {
    auto now = std::chrono::system_clock::now();
    auto ms_since_epoch = std::chrono::duration_cast<std::chrono::milliseconds>(
        now.time_since_epoch()).count();
    
    uint64_t elapsed_ms = ms_since_epoch - GLOBAL_EPOCH_MS;
    return elapsed_ms % SLOT_DURATION_MS;
}

bool SimpleTimeSlotManager::is_in_transmission_window() const {
    if (!is_my_slot()) return false;
    
    uint32_t time_in_slot = get_time_in_slot_ms();
    return time_in_slot < TRANSMISSION_WINDOW_MS;  // 前400ms可以传输
}

bool SimpleTimeSlotManager::can_transmit() const {
    return is_my_slot() && is_in_transmission_window();
}

uint32_t SimpleTimeSlotManager::get_transmission_time_remaining() const {
    if (!is_in_transmission_window()) return 0;
    
    uint32_t time_in_slot = get_time_in_slot_ms();
    return TRANSMISSION_WINDOW_MS - time_in_slot;
}

void SimpleTimeSlotManager::set_my_slot_callback(std::function<void()> callback) {
    std::lock_guard<std::mutex> lock(m_callback_mutex);
    m_my_slot_callback = callback;
}

void SimpleTimeSlotManager::monitor_thread() {
    while (m_running.load()) {
        uint32_t current_slot = get_current_slot();
        
        // 检查时隙变化
        if (current_slot != m_last_slot) {
            m_last_slot = current_slot;
            
            // 如果是我的时隙，检查是否在传输窗口内
            if (is_my_slot()) {
                uint32_t time_in_slot = get_time_in_slot_ms();
                
                if (time_in_slot < TRANSMISSION_WINDOW_MS) {
                    // 在传输窗口内，触发回调
                    uint32_t remaining_tx_time = TRANSMISSION_WINDOW_MS - time_in_slot;
                    
                    std::lock_guard<std::mutex> lock(m_callback_mutex);
                    if (m_my_slot_callback) {
                        m_my_slot_callback();
                    }
                    spdlog::debug("Node {} slot started, {}ms transmission time available", 
                                static_cast<int>(m_node_id), remaining_tx_time);
                } else {
                    // 在保护间隔内，不触发回调
                    spdlog::debug("Node {} slot started in guard interval ({}ms), skipping transmission", 
                                static_cast<int>(m_node_id), time_in_slot);
                }
            }
        }
        
        std::this_thread::sleep_for(std::chrono::milliseconds(10)); // 10ms检查间隔
    }
}

} // namespace Hnu::Uwb