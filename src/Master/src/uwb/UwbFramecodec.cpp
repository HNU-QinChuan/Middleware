#include "uwb/UwbFramecodec.hpp"
#include <sstream>
#include <iomanip>
#include <stdexcept>

namespace Hnu::Uwb {

TimeSlotFrame::TimeSlotFrame(uint8_t src, uint8_t dst, uint8_t seq, FrameType t, const std::vector<uint8_t>& data)
    : src_id(src), dst_id(dst), seq_no(seq), type(t), payload(data), checksum(0) {
    checksum = FrameCodec::calculate_checksum(*this);
}

TimeSlotFrame::TimeSlotFrame(uint8_t src, uint8_t dst, uint8_t seq, FrameType t, const std::string& data)
    : src_id(src), dst_id(dst), seq_no(seq), type(t), checksum(0) {
    payload.assign(data.begin(), data.end());
    checksum = FrameCodec::calculate_checksum(*this);
}

std::string FrameCodec::encode(const TimeSlotFrame& frame) {
    std::ostringstream oss;
    
    // Header
    oss << FRAME_HEADER;
    
    // SrcID, DstID, SeqNo
    oss << to_hex(frame.src_id) << to_hex(frame.dst_id) << to_hex(frame.seq_no); //修改： 此处将dst_id从http报文中读出然后赋值即可
    
    // Type
    oss << frame_type_to_string(frame.type);
    
    // Length and Payload (only for data frames)
    if (frame.type == FrameType::DATA || frame.type == FrameType::HEARTBEAT) {
        oss << to_hex(static_cast<uint16_t>(frame.payload.size()));
        for (uint8_t byte : frame.payload) {
            oss << static_cast<char>(byte);
        }
    }
    
    // Checksum
    oss << to_hex(frame.checksum);
    
    return oss.str();
}

bool FrameCodec::decode(const std::string& data, TimeSlotFrame& frame) {
    if (data.length() < MIN_FRAME_SIZE || data.substr(0, HEADER_SIZE) != FRAME_HEADER) {
        return false;
    }
    
    try {
        size_t pos = HEADER_SIZE;
        
        // Parse SrcID, DstID, SeqNo
        frame.src_id = from_hex_byte(data.substr(pos, 2)); pos += 2;
        frame.dst_id = from_hex_byte(data.substr(pos, 2)); pos += 2;
        frame.seq_no = from_hex_byte(data.substr(pos, 2)); pos += 2;
        
        // Parse Type
        std::string type_str = data.substr(pos, 3); pos += 3;
        frame.type = string_to_frame_type(type_str);
        
        // Parse Length and Payload
        frame.payload.clear();
        if (frame.type == FrameType::DATA || frame.type == FrameType::HEARTBEAT) {
            if (pos + 4 > data.length()) return false;
            
            uint16_t length = from_hex_word(data.substr(pos, 4)); pos += 4;
            if (pos + length + 2 > data.length()) return false;
            
            frame.payload.assign(data.begin() + pos, data.begin() + pos + length);
            pos += length;
        }
        
        // Parse Checksum
        if (pos + 2 > data.length()) return false;
        frame.checksum = from_hex_byte(data.substr(pos, 2));
        
        return verify_checksum(frame);
        
    } catch (const std::exception&) {
        return false;
    }
}

uint8_t FrameCodec::calculate_checksum(const TimeSlotFrame& frame) {
    uint8_t checksum = 0;
    checksum ^= frame.src_id;
    checksum ^= frame.dst_id;
    checksum ^= frame.seq_no;
    checksum ^= static_cast<uint8_t>(frame.type);
    
    for (uint8_t byte : frame.payload) {
        checksum ^= byte;
    }
    return checksum;
}

bool FrameCodec::verify_checksum(const TimeSlotFrame& frame) {
    return frame.checksum == calculate_checksum(frame);
}

std::string FrameCodec::frame_type_to_string(FrameType type) {
    switch (type) {
        case FrameType::DATA: return "DAT";
        case FrameType::ACK: return "ACK";
        case FrameType::HEARTBEAT: return "HBT";
        default: return "UNK";
    }
}

FrameType FrameCodec::string_to_frame_type(const std::string& type_str) {
    if (type_str == "DAT") return FrameType::DATA;
    if (type_str == "ACK") return FrameType::ACK;
    if (type_str == "HBT") return FrameType::HEARTBEAT;
    throw std::invalid_argument("Unknown frame type: " + type_str);
}

std::string FrameCodec::to_hex(uint8_t value) {
    std::ostringstream oss;
    oss << std::hex << std::uppercase << std::setfill('0') << std::setw(2) << static_cast<int>(value);
    return oss.str();
}

std::string FrameCodec::to_hex(uint16_t value) {
    std::ostringstream oss;
    oss << std::hex << std::uppercase << std::setfill('0') << std::setw(4) << value;
    return oss.str();
}

uint8_t FrameCodec::from_hex_byte(const std::string& hex_str) {
    return static_cast<uint8_t>(std::stoul(hex_str, nullptr, 16));
}

uint16_t FrameCodec::from_hex_word(const std::string& hex_str) {
    return static_cast<uint16_t>(std::stoul(hex_str, nullptr, 16));
}

} // namespace Hnu::Uwb