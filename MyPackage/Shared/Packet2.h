#pragma once

#include <vector>
#include <iostream>
#include <SFML/Graphics.hpp>

namespace Packet {
    enum class Type : uint32_t {
        STRING,
    };
    inline std::string_view TypeString(Type type)
    {
        switch (type)
        {
        case Type::STRING:
            return "STRING\n";
        default:
            return std::to_string(int(type));
            return "UNKNOWN";
        }
    }

    template<typename T>
    std::enable_if_t<std::is_arithmetic_v<T>, int> // ambiguous pt ca nu e exact overload fmm (nu pot sa fac unul general)
        getSize(const T& data) {
        return sizeof(T);
    }
    template<typename T>
    std::enable_if_t<std::is_arithmetic_v<T>, void>
        writeData(char*& data, const T& val) {
        memcpy(data, &val, getSize(val));
        data += getSize(val);
    }
    template<typename T>
    std::enable_if_t<std::is_arithmetic_v<T>, void>
        readData(const char*& data, T& val) {
        memcpy(&val, data, getSize(val));
        data += getSize(val);
    }

    inline int getSize(const std::string& data) {
        return sizeof(data.size()) + data.size() * sizeof(char);
    }
    inline void writeData(char*& data, const std::string& val) {
        size_t s = val.size();
        memcpy(data, &s, getSize(s));
        data += getSize(s);
        memcpy(data, val.data(), val.size());
        data += val.size();
    }
    inline void readData(const char*& data, std::string& val) {
        size_t s = 0;
        memcpy(&s, data, getSize(s));
        data += getSize(s);
        val.resize(s);
        memcpy(val.data(), data, s);
        data += s;
    }

    template<typename T>
    int getSize(const sf::Vector2<T>& data) {
        return getSize(data.x) * 2;
    }
    template<typename T>
    void writeData(char*& data, const sf::Vector2<T>& val) {
        writeData(data, val.x);
        writeData(data, val.y);
    }
    template<typename T>
    void readData(const char*& data, sf::Vector2<T>& val) {
        readData(data, val.x);
        readData(data, val.y);
    }


    template<typename T>
    int getSize(const std::vector<T>& data) {
        int s = sizeof(data.size());
        for (auto& i : data) s += getSize(i);
        return s;
    }
    template<typename T>
    void writeData(char*& data, const std::vector<T>& val) {
        size_t s = val.size();
        memcpy(data, &s, getSize(s));
        data += getSize(s);
        for (auto& i : val) {
            writeData(data, i);
        }
    }
    template<typename T>
    void readData(const char*& data, std::vector<T>& val) {
        size_t s = 0;
        memcpy(&s, data, sizeof(s));
        data += sizeof(s);
        for (int i = 0; i < s; i++) {
            T a;
            readData(data, a);
            val.push_back(a);
        }
    }


    template<Type _Type, typename... T>
    struct Packet {
    public:
        std::vector<char> data;

    public:
        Packet(Type type, T... args) {
            serialize(type, args...);
        }
        Packet(T... args) {
            serialize(_Type, args...);
        }
        void serialize(Type type, T... args) {
            this->data.clear();

            int headerSize = sizeof(uint32_t) + sizeof(int);
            //      type       payload size
            int payloadSize = 0;
            ((payloadSize += getSize(args)), ...);

            this->data.resize(headerSize + payloadSize);
            // o eroare de tipul 'int getSize(const std::string &)': cannot convert argument 1 from 'CEVA_AICI' to 'const std::string &'
            // inseamna ca trb sa dai overload la getSize
            //std::cout << "TotaolSize: " << totalSize << '\n';
            char* data = this->data.data();
            uint32_t type_u = static_cast<uint32_t>(type);

            // header
            writeData(data, type_u);
            writeData(data, payloadSize);

            // payload
            (writeData(data, args), ...);
        }
        void deserialize(T&... args) {
            const char* data = this->data.data();
            (readData(data, args), ...);
        }
        static Type getType(const char* data) {
            uint32_t type = 0;
            memcpy(&type, data, sizeof(uint32_t));
            return static_cast<Type>(type);
        }
        static int getCount(const char* data) {
            int count = 0;
            memcpy(&count, data + sizeof(uint32_t), sizeof(int));
            return count;
        }
        static void deserialize(const char* data, T&... args) {
            //const char* data = this->data.data();
            (readData(data, args), ...);
        }
        Packet() {
            ;
        }
    };
    //using PacketCUSTOM1 = Packet<std::vector<somedata>, std::string, double>;


}
using Packet_STRING = Packet::Packet<Packet::Type::STRING, std::string>;