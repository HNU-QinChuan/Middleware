#include "shm/UdsRouter.hpp"
#include "MiddlewareManager.hpp"
#include "InterfaceManager.hpp"
#include "shm/Node.hpp"
#include "shm/Publish.hpp"
#include "shm/Subscribe.hpp"
#include <spdlog/spdlog.h>
#include <algorithm>
#include <jsoncpp/json/reader.h>
#include <jsoncpp/json/json.h>

namespace Hnu::Middleware {

class NodeStopController {
public:
    void handlePost(Request& req, Response& res) {
        //std::string node_name = req["node"].to_string();
        const std::string body = req.body();
        Json::Reader reader;
        Json::Value root;
        if (!reader.parse(body, root)) {
            spdlog::debug("Failed to parse JSON body: {}", body);
            res.result(http::status::bad_request);
            return;
        }
        std::string node_name = root.get("node", "").asString();
        spdlog::debug("node_name is {}", node_name);
        auto itNode = MiddlewareManager::middlewareManager.m_nodes.find(node_name);
        if (itNode == MiddlewareManager::middlewareManager.m_nodes.end()) {
            res.result(http::status::bad_request);
            return;
        }

        auto node = itNode->second;

        // 清理订阅
        std::vector<std::shared_ptr<Subscribe>> subs = node->removeAllSubscribe();
        for (auto& sub : subs) {
            sub->cancle();
            auto& vec = MiddlewareManager::middlewareManager.m_subscribes[sub->getName()];
            vec.erase(std::remove(vec.begin(), vec.end(), sub), vec.end());
            if (vec.empty()) {
                MiddlewareManager::middlewareManager.m_subscribes.erase(sub->getName());
            }
        }

        // 清理发布
        std::vector<std::shared_ptr<Publish>> pubs = node->removeAllPublish();
        for (auto& pub : pubs) {
            pub->cancel();
            auto& vec = MiddlewareManager::middlewareManager.m_publishes[pub->getName()];
            vec.erase(std::remove(vec.begin(), vec.end(), pub), vec.end());
            if (vec.empty()) {
                MiddlewareManager::middlewareManager.m_publishes.erase(pub->getName());
            }
        }

        // 从服务器端删除节点
        MiddlewareManager::middlewareManager.m_nodes.erase(itNode);

        // 通知 InterfaceManager
        Interface::InterfaceManager::deleteNode(node_name);

        res.result(http::status::ok);
    }
};

CONTROLLER_REGISTER(NodeStopController, "/node/stop", http::verb::post, &NodeStopController::handlePost);

} // namespace Hnu::Middleware
