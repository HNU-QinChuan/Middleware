//
// Created by aoao on 25-3-2.
//

#include "dgps_hmw_node.cpp"
#include <chrono>
#include <thread>


int main(){
    // 创建 DGpsHmw 节点实例，名称可根据需要修改
    auto node = std::make_shared<Dgps_hmw_node::DGpsHmw>("dgps_hmw_node");
    // 主线程进入循环，确保节点一直运行
    while (true)
    {
        std::this_thread::sleep_for(std::chrono::seconds(1));
        // 如果有需要的话，可以在这里添加退出条件或其他逻辑
    }
    return 0;
}
