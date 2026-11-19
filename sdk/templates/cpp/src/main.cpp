#include <iostream>
#include <iterator>
#include <string>

int main()
{
    const std::string image(std::istreambuf_iterator<char>(std::cin), {});
    std::cout << "{\"protocol_version\":1,\"status\":\"success\","
                 "\"result\":{\"type\":\"notification\","
                 "\"title\":\"@PLUGIN_NAME@\",\"text\":\"Received "
              << image.size() << " PNG bytes\"}}\n";
}
