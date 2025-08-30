# C++-Simple-Network-Package
Simple Network Package is designed to provide a simple interface for developers to send tcp packets without learning the boost/asio framework

# Prerequisites
1. [Boost/Asio 1.81.0](https://www.boost.org/releases/1.81.0/) libraries (I used static because it's easier to link)
- make sure you run bootstrap and b2 such that you have the `stage\lib` folder filled with the libraries

Alternative if building doesn't work: use the [pre-built binary](https://boost.teeks99.com/)

2. openssl libraries (for boost/asio/ssl)
- Simple installation guide in powershell
```
git clone https://github.com/microsoft/vcpkg.git
cd vcpkg
.\bootstrap-vcpkg.bat
.\vcpkg install openssl:x64-windows
.\vcpkg integrate install
```

# Using in your project
1. Create new console app
2. Copy MyPackage to the same folder where the main.cpp is 
3. In Visual Studio, show all files in your solution explorer
4. Select all files in the MyPackage folder + Right click + Include all files in project
5. in proprieties
- General | select C++20
- C/C++ | General | Additional include directories: `boost_1.81.0\boost_1.81.0\`
- Linker | General | Additional library directories `boost_1.81.0\boost_1.81.0\stage\lib`
6. Generate the `server.crt` and `server.key` with openssl
- `openssl req -x509 -nodes -days 365 -newkey rsa:2048 -keyout server.key -out server.crt`
and put it in the same folder as `main.cpp`

It should automatically add the dlls for the openssl. Inside `Packet2.h`, I also used SFML, you can just erase the include and the read/write functions if you don't use it. You can easily add custom structures to any Package by adding the appropriate read\write functions inside the `Packet2.h` file, along with custom packets (see the bottom of `Packet2.h`). 

# Simple example
The `main.cpp` files of the client and the server:
```c++
#include <iostream>
#include <chrono>
#include <../../MyPackage/Client/Client.h>
#include <../../MyPackage/Server/Server.h>
void readCallback(std::string ip, Packet::Type type, const char* ptr, int count) {
    switch (type) {
    case Packet::Type::STRING:
    {
        std::string s;
        Packet_STRING::deserialize(ptr, s);
        std::cout << ip << ": >" << s << "<\n";
        break;
    }
    }
}
int main() {
    Client::initHTTPS(readCallback);
    Client::initHTTP(readCallback);

    while (true) {
        std::string a;
        std::cin >> a;
        Client::writeHttpsPacket(Packet_STRING(a));
        Client::writeHttpPacket(Packet_STRING(a));
    }

    return 0;
}
```
```c++
#include <iostream>
#include <chrono>
#include <../../MyPackage/Client/Client.h>
#include <../../MyPackage/Server/Server.h>
void serverReadCallback(std::string ip, Packet::Type type, const char* ptr, int count) {
    switch (type) {
    case Packet::Type::STRING:
    {
        std::string s;
        Packet_STRING::deserialize(ptr, s);
        Server::getServerHttps()->send_message_all_except(ip, Packet_STRING(s));
        break;
    }
    }
}
int main() {
    Server::initHTTP(serverReadCallback);
    Server::initHTTPS(serverReadCallback);
    while (true);
}
```
Now you can say you made a horizontally-scalable, event-driven, low-latency communication platform supporting concurrent multi-user interactions in real time in your resume. 
