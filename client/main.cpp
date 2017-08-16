#include <iostream>
#include <string>
#include <boost/array.hpp>
#include <boost/asio.hpp>

//need to put this guy in common config file or header
#define CHAT_PORT "8421"


using boost::asio::ip::tcp;

void send_message(boost::asio::io_service& io_service, std::string hostname, std::string message)
{
    //resolve hostname and construct a query
    tcp::resolver resolver(io_service);
    tcp::resolver::query q( hostname.c_str(), CHAT_PORT );

    //get endpoints
    tcp::resolver::iterator endpoint_iterator = resolver.resolve(q);

    //create socket and connect it to the endpoint
    tcp::socket sock(io_service);
    boost::asio::connect(sock, endpoint_iterator);

    for (;;)
    {
        boost::array<char, 128> buf; //we could also use a char[] or std::vector
        boost::system::error_code error;

        size_t len = sock.read_some(boost::asio::buffer(buf),error);

        if (error == boost::asio::error::eof)
            break;
        else if (error)
            throw boost::system::system_error(error);
        std::cout.write(buf.data(), len);
    }
}

int main(int argc, char* argv[])
{
    try
    {
        if (argc != 3)
        {
            std::cerr << "Usage: cpp_client <host> <message>" << std::endl;
            return 0;
        }

        boost::asio::io_service io;

        send_message(io, argv[1], argv[2]);
    }
    catch (std::exception& e)
    {
        std::cerr << e.what() << std::endl;
    }
    return 0;
}
