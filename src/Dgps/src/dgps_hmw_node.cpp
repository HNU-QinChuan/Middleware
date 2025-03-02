//
// Created by aoao on 25-3-2.
//

#include <thread>
#include <string>
#include "ntrip.h"
#include <Std/dgps.pb.h>
#include <hmw/Node.hpp>
#include <hmw/Publisher.hpp>
#include <spdlog/spdlog.h>
#include <memory>
#include <yaml-cpp/yaml.h>

// #define STATUS_NO_FIX -1
// #define STATUS_FIX 0
// #define STATUS_GBAS_FIX 2
// #define SERVICE_GPS 1
// #define COVARIANCE_TYPE_DIAGONAL_KNOWN  2

namespace Dgps_hmw_node{


class DGpsHmw : public Hnu::Middleware::Node
{
public:
    explicit DGpsHmw(const std::string &name)
        : Hnu::Middleware::Node(name)
    {

        YAML::Node config = YAML::LoadFile("../yaml/dgps.yaml");
        serverName = config[serverName].as<std::string>();
        serverPort = config[serverPort].as<std::string>();
        userName = config[userName].as<std::string>();
        password = config[password].as<std::string>();
        serialPort = config[serialPort].as<std::string>();
        topic = config[topic].as<std::string>();
        frame_id = config[frame_id].as<std::string>();

        args.server = serverName.c_str();
        args.port = serverPort.c_str();
        args.user = userName.c_str();
        args.password = password.c_str();

        // args.nmea = "$GNGGA,034458.00,2810.79928,N,11256.54467,E,2,12,0.64,36.0,M,-12.7,M,1.0,0773*7D";
        args.nmea = "$GPGGA,074745.00,2810.7222,N,11256.4403,E,1,11,63.4,54.76,M,-16.80,M,,*46";
        args.data = "RTCM33_GRCE";
        args.bitrate = 0;
        args.proxyhost = 0;
        args.proxyport = "2101";
        args.mode = NTRIP1;
        args.initudp = 0;
        args.udpport = 0;
        args.protocol = SPAPROTOCOL_NONE;
        args.parity = SPAPARITY_NONE;
        args.stopbits = SPASTOPBITS_1;
        args.databits = SPADATABITS_8;
        args.baud = SPABAUD_115200;
        // args.baud = SPABAUD_460800;
        if( serialPort.empty())
            args.serdevice = 0;//"/dev/ttyUSB0";
        else
            args.serdevice = serialPort.c_str();
        args.serlogfile = 0;
        args.stop = false;

        // RCLCPP_ERROR(get_logger(), "Username= %s",args.user);
        // RCLCPP_ERROR(get_logger(), "password= %s",args.password);
        // RCLCPP_ERROR(get_logger(), "port= %s",args.port);
        // RCLCPP_ERROR(get_logger(), "dev= %s",args.serdevice);


        pub = this->createPublisher<Std::NavSatFix>(topic);

        pub_gpfpd = this->createPublisher<Std::Gpfpd>("/dgps_gpfpd");
        pub_gtimu = this->createPublisher<Std::Gtimu>("/dgps_gtimu");
        pub_gnvtg = this->createPublisher<Std::Gnvtg>("/dgps_gnvtg");

    }


private:
    std::string topic;
    std::shared_ptr<Hnu::Middleware::Publisher<Std::Gpfpd>> pub_gpfpd;
    std::shared_ptr<Hnu::Middleware::Publisher<Std::Gtimu>> pub_gtimu;
    std::shared_ptr<Hnu::Middleware::Publisher<Std::Gnvtg>> pub_gnvtg;
    std::shared_ptr<Hnu::Middleware::Publisher<Std::NavSatFix>> pub;

    std::shared_ptr<std::thread> ntrip_thread;
    struct Args args;
    std::string serverName,userName,password,serverPort,serialPort,frame_id;

};


}