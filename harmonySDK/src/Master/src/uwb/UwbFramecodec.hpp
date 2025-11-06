#pragma once
#include <vector>
#include <string>
#include <cstdint>

namespace Hnu::Uwb {

enum class FrameType : uint8_t {
    DATA = 0,      // 数据帧
    ACK = 1,       // 确认帧  
    HEARTBEAT = 2  // 心跳帧
};

struct TimeSlotFrame {
    uint8_t src_id;
    uint8_t dst_id;
    uint8_t seq_no;//标记每一条消息
    FrameType type;
    std::vector<uint8_t> payload;
    uint8_t checksum;
    
    TimeSlotFrame() = default;
    TimeSlotFrame(uint8_t src, uint8_t dst, uint8_t seq, FrameType t, const std::vector<uint8_t>& data = {});
    TimeSlotFrame(uint8_t src, uint8_t dst, uint8_t seq, FrameType t, const std::string& data);
};

class FrameCodec {
public:
    static constexpr char FRAME_HEADER[] = "[TS]";
    static constexpr size_t HEADER_SIZE = 4;
    static constexpr size_t MIN_FRAME_SIZE = 12;
    static constexpr uint8_t BROADCAST_ID = 0xFF;
    
    // 编码解码
    static std::string encode(const TimeSlotFrame& frame);
    static bool decode(const std::string& data, TimeSlotFrame& frame);
    
    // 校验和
    static uint8_t calculate_checksum(const TimeSlotFrame& frame);
    static bool verify_checksum(const TimeSlotFrame& frame);
    
private:
    static std::string frame_type_to_string(FrameType type);
    static FrameType string_to_frame_type(const std::string& type_str);
    static std::string to_hex(uint8_t value);
    static std::string to_hex(uint16_t value);
    static uint8_t from_hex_byte(const std::string& hex_str);
    static uint16_t from_hex_word(const std::string& hex_str);
};

} // namespace Hnu::Uwb