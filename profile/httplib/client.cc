#include <iostream>
#include "profile/httphttplib.h"

int main() {
    // 创建 httplib 客户端
    httplib::Client client("http://www.example.com");

    // 发送 GET 请求
    auto res = client.Get("/");

    // 检查响应
    if (res && res->status == 200) {
        std::cout << res->body << std::endl;
    } else {
        std::cout << "Error: " << res.error() << std::endl;
    }

    return 0;
}
