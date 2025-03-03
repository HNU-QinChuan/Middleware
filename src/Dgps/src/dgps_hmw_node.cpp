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
#include <Std/String.pb.h>
#include <yaml-cpp/yaml.h>
#include <chrono>
#include <regex>
#include <boost/array.hpp>
#include <ctime>

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
        ntrip_thread = std::make_shared<std::thread>(ntrip_client,&args);
        timer_ = this->createTimer(20, [this] { timer_callback(); });
        pub_status = this->createPublisher<Std::String>("/Dev_status");

        std::this_thread::sleep_for(std::chrono::milliseconds(300));
        Std::String mstr;
        mstr.set_data("DGPS_HMW_NODE START");
        pub_status->publish(mstr);
    }

    ~DGpsHmw() {
        args.stop = true;
        ntrip_thread->join();
        Std::String mstr;
        mstr.set_data("DGPS_HMW_NODE STOP");
        pub_status->publish(mstr);
    }



    void timer_callback()
    {
        Std::NavSatFix sat;
        Std::Gpfpd gpfpd;
        Std::Gtimu gtimu;
        Std::Gnvtg gnvtg;

        int ret = getGpsMsg(sat, gpfpd, gtimu, gnvtg);
        if (ret > 0)
        {
            switch (ret)
            {
                case 1:
                    pub->publish(sat);
                    break;
                case 2:
                    pub_gpfpd->publish(gpfpd);
                    break;
                case 3:
                    pub_gtimu->publish(gtimu);
                    break;
                case 4:
                    pub_gnvtg->publish(gnvtg);
                    break;
                default:
                    break;
            }
        }
    }

    void nmeaToFix(std::smatch m, Std::NavSatFix &sat)
    {
        sat.mutable_header()->set_frame_id("gps");
        auto timestamp = new google::protobuf::Timestamp();
        std::time_t now = std::time(nullptr);
        timestamp->set_seconds(now);
        timestamp->set_nanos(0);
        sat.mutable_header()->set_allocated_stamp(timestamp);

        int8_t status = Std::NavSatStatus::STATUS_NO_FIX;
        uint16_t service = Std::NavSatStatus::SERVICE_GPS;
        double epe_quality = 1000000;
        uint8_t covariance_type = Std::NavSatFix::COVARIANCE_TYPE_UNKNOWN;

        if(m[2] != "") {
            // cout<<"Lat:"<<m[2]<<"  Longi:"<<m[4]<<endl;

            if(m[6] == "-1" || m[6] == "0") {   //Invalid
                status = -1;
                epe_quality = 1000000;
                covariance_type = Std::NavSatFix::COVARIANCE_TYPE_UNKNOWN;
            }
            else if(m[6] == "1") {  //SPS
                status = 1;
                epe_quality = 4.0;
                covariance_type = Std::NavSatFix::COVARIANCE_TYPE_APPROXIMATED;
            }
            else if(m[6] == "2") {  //DGPS
                status = 2;
                epe_quality = 0.1;
                covariance_type = Std::NavSatFix::COVARIANCE_TYPE_APPROXIMATED;
            }
            else if(m[6] == "4") {  //RTK Fix
                status = 4;
                epe_quality = 0.02;
                covariance_type = Std::NavSatFix::COVARIANCE_TYPE_APPROXIMATED;
            }
            else if(m[6] == "5") {  //RTK Float
                status = 5;
                epe_quality = 4.0;
                covariance_type = Std::NavSatFix::COVARIANCE_TYPE_APPROXIMATED;
            }
            else if(m[6] == "9") {  //WAAS
                status = 9;
                epe_quality = 3.0;
                covariance_type = Std::NavSatFix::COVARIANCE_TYPE_APPROXIMATED;
            }
            else {
                status = stoi(m[6]);
                epe_quality = 1000000;
                covariance_type = Std::NavSatFix::COVARIANCE_TYPE_UNKNOWN;
            }

            sat.mutable_status()->set_status(static_cast<Std::NavSatStatus_Status>(status));
            sat.mutable_status()->set_service(service);
            sat.set_position_covariance_type(static_cast<Std::NavSatFix_CovarianceType>(covariance_type));

            sat.set_latitude(NMEA2float(m[2]));
            if(m[3] == "S") sat.set_latitude(-sat.latitude());

            if(m[4] != "") sat.set_longitude(NMEA2float(m[4]));
            if(m[5] == "W") sat.set_longitude(-sat.longitude());

            if(m[9] != "") sat.set_altitude(std::stod(m[9]));

            google::protobuf::RepeatedField<double> temp_covariance(covariance.begin(), covariance.end());
            sat.mutable_position_covariance()->Swap(&temp_covariance);
            if(m[8] != "") {
                double hdop = std::stod(m[8]);
                // 确保 position_covariance 有 9 个元素
                sat.mutable_position_covariance()->Resize(9, 0.0);
                // 设置协方差矩阵的值
                sat.mutable_position_covariance()->Set(0, std::pow((hdop * epe_quality), 2));
                sat.mutable_position_covariance()->Set(4, std::pow((hdop * epe_quality), 2));
                sat.mutable_position_covariance()->Set(8, std::pow((2 * hdop * epe_quality * 2), 2));
            }
        }

    }

    int getGpsMsg(Std::NavSatFix &sat, Std::Gpfpd &gpfpd, Std::Gtimu &gtimu, Std::Gnvtg &gnvtg)
    {
        std::string candidate = getNmea();

        std::smatch m;
        // $GPGGA,093310.80,2810.7292,N,11256.4415,E,1,13,52.7,61.31,M,-16.80,M,,*49
        std::regex e1("\\$.*?GGA,(.*?),(.*?),(.*?),(.*?),(.*?),(.*?),(.*?),(.*?),(.*?),");
        // $GPFPD,2168,370484.000,353.425,0.452,0.079,28.17868576,112.94064199,36.06,0.018,-0.023,-0.041,0.974,11,10,04*4E
        std::regex e2("\\$GPFPD,(.*?),(.*?),(.*?),(.*?),(.*?),(.*?),(.*?),(.*?),(.*?),(.*?),(.*?),(.*?),(.*?),(.*?),(.*?)\\*");
        // $GTIMU,2168,370484.000,-0.0993,0.2741,-0.0604,-0.0012,0.0078,0.9957,41.0*48
        std::regex e3("\\$GTIMU,(.*?),(.*?),(.*?),(.*?),(.*?),(.*?),(.*?),(.*?),(.*?)\\*");

        // string candidate = "$GPFPD,2168,370484.000,353.425,0.452,0.079,28.17868576,112.94064199,36.06,0.018,-0.023,-0.041,0.974,11,10,04*4E";
        // string candidate = "$GTIMU,2168,370484.000,-0.0993,0.2741,-0.0604,-0.0012,0.0078,0.9957,41.0*48";

        // $GNVTG,0.00,T,0.00,M,0.00,N,0.000,K,A*3D
        std::regex e4("\\$GPVTG,(.*?),(.*?),(.*?),(.*?),(.*?),(.*?),(.*?),(.*?),(.*?)\\*");

        // regex e5("\\$GNGGA,(.*?),(.*?),.*?,(.*?),.*?,(.*?),.*?,(.*?),(.*?),");
        // regex e6("\\$BDGGA,(.*?),(.*?),.*?,(.*?),.*?,(.*?),.*?,(.*?),(.*?),");

        try
        {
            if(!candidate.empty())
            {
                if( std::regex_search (candidate, m, e1))
                {
                    //cout<<candidate<<endl;
                    //cout<<"Lat:"<<m[2]<<"Longi:"<<m[3]<<endl;
                    nmeaToFix(m, sat);

                    return 1;
                }
                else if(std::regex_search (candidate, m, e2))
                {
                    gpfpd.mutable_header()->set_frame_id("gpfpd");
                    auto* timestamp = new google::protobuf::Timestamp();
                    std::time_t now = std::time(nullptr);
                    timestamp->set_seconds(now);
                    timestamp->set_nanos(0);
                    gpfpd.mutable_header()->set_allocated_stamp(timestamp);

                    gpfpd.set_gpfpd(candidate);

                    if(m[3] != "") gpfpd.set_heading(std::stod(m[3]));
                    if(m[4] != "") gpfpd.set_pitch(std::stod(m[4]));
                    if(m[5] != "") gpfpd.set_roll(std::stod(m[5]));
                    if(m[6] != "") gpfpd.set_lattitude(std::stod(m[6]));
                    if(m[7] != "") gpfpd.set_longitude(std::stod(m[7]));
                    if(m[8] != "") gpfpd.set_altitude(std::stod(m[8]));
                    if(m[9] != "") gpfpd.set_velocity_east(std::stod(m[9]));
                    if(m[10] != "") gpfpd.set_velocity_north(std::stod(m[10]));
                    if(m[11] != "") gpfpd.set_velocity_up(std::stod(m[11]));
                    if(m[12] != "") gpfpd.set_baseline(std::stod(m[12]));
                    if(m[13] != "") gpfpd.set_nsv1(std::stod(m[13]));
                    if(m[14] != "") gpfpd.set_nsv2(std::stod(m[14]));
                    std::string sts = m[15];
                    if(m[15] != "") gpfpd.set_status(std::strtol(sts.c_str(), NULL, 16));

                    return 2;
                }else if(std::regex_search (candidate, m, e3))
                {
                    gtimu.mutable_header()->set_frame_id("gtimu");
                    auto timestamp = new google::protobuf::Timestamp();
                    std::time_t now = std::time(nullptr);
                    timestamp->set_seconds(now);
                    timestamp->set_nanos(0);
                    gtimu.mutable_header()->set_allocated_stamp(timestamp);

                    gtimu.set_gtimu(candidate);
                    if(m[3] != "") gtimu.set_gyro_x(std::stod(m[3]));
                    if(m[4] != "") gtimu.set_gyro_y(std::stod(m[4]));
                    if(m[5] != "") gtimu.set_gyro_z(std::stod(m[5]));
                    if(m[6] != "") gtimu.set_acc_x(std::stod(m[6]));
                    if(m[7] != "") gtimu.set_acc_y(std::stod(m[7]));
                    if(m[8] != "") gtimu.set_acc_z(std::stod(m[8]));
                    if(m[9] != "") gtimu.set_tpr(std::stod(m[9]));

                    return 3;
                }else if(std::regex_search (candidate, m, e4))
                {
                    gnvtg.mutable_header()->set_frame_id("gnvtg");
                    auto* timestamp = new google::protobuf::Timestamp();
                    std::time_t now = std::time(nullptr);
                    timestamp->set_seconds(now);
                    timestamp->set_nanos(0);
                    gnvtg.mutable_header()->set_allocated_stamp(timestamp);

                    gnvtg.set_gnvtg(candidate);
                    if(m[1] != "") gnvtg.set_heading1(std::stod(m[1]));
                    if(m[3] != "") gnvtg.set_heading2(std::stod(m[3]));
                    if(m[5] != "") gnvtg.set_speed1(std::stod(m[5]));
                    if(m[7] != "") gnvtg.set_speed2(std::stod(m[7]));
                    std::string sts = m.str(8);
                    gnvtg.set_status(sts);

                    return 4;
                }
            }
        }catch (const std::exception& e)
        {
            std::cerr << e.what() << '\n';
        }

        return 0;

    }


private:
    static double NMEA2float(const std::string& s)
    {
        double d = std::stod(s)/100.0;
        d = floor(d) + (d - floor(d))*10/6;
        return d;
    }

    boost::array<double,9> covariance={1,0,0,
                                        0,1,0,
                                        0,0,1 };

private:
    std::string topic;
    std::shared_ptr<Hnu::Middleware::Timer> timer_;

    std::shared_ptr<Hnu::Middleware::Publisher<Std::Gpfpd>> pub_gpfpd;
    std::shared_ptr<Hnu::Middleware::Publisher<Std::Gtimu>> pub_gtimu;
    std::shared_ptr<Hnu::Middleware::Publisher<Std::Gnvtg>> pub_gnvtg;
    std::shared_ptr<Hnu::Middleware::Publisher<Std::NavSatFix>> pub;
    std::shared_ptr<Hnu::Middleware::Publisher<Std::String>> pub_status;

    std::shared_ptr<std::thread> ntrip_thread;
    struct Args args;
    std::string serverName,userName,password,serverPort,serialPort,frame_id;

};


}