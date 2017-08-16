//asynchronous TCP server
#include <ctime>
#include <iostream>
#include <string>
#include <boost/bind.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/asio.hpp>

#define CHAT_PORT 8421

using boost::asio::ip::tcp;

//OBJECT ORIENTED DESIGN WOOO
std::string return_some_response()
{
  std::string lval = "wow, you're contacting this chat server, get ready for chatting";
  return lval;
}

class tcp_connection
  : public boost::enable_shared_from_this<tcp_connection>
{
public:
  //use shared ptr and enable_shared_ptr to keep the object alive as long as there is an operation that refers to it
  //and seeing whether something refers to it is easy with smart pointers
  typedef boost::shared_ptr<tcp_connection> pointer;

  static pointer create(boost::asio::io_service& io_service)
  {
    return pointer(new tcp_connection(io_service));
  }

  tcp::socket& socket()
  {
    return socket_;
  }

  void start()
  {
    //the data to be sent is stored int he class memeber message_, so we keep that valid until the operation is complete
    message_ = return_some_response();

    //when initiating the asynchronous operation, and if using boost::bind(), you must specify only the arugments that match the handler's parameters list. In this program, both of the arg placeholders could potentially have been removed(since thjey're not used in handle_write).
    boost::asio::async_write(socket_, boost::asio::buffer(message_),
        boost::bind(&tcp_connection::handle_write, shared_from_this(),
          boost::asio::placeholders::error,
          boost::asio::placeholders::bytes_transferred));
  }

private:
  tcp_connection(boost::asio::io_service& io_service)
    : socket_(io_service)
  {
  }

  void handle_write(const boost::system::error_code& /*error*/,
      size_t /*bytes_transferred*/)
  {
  }

  tcp::socket socket_;
  std::string message_;
};

class tcp_server
{
public:
  tcp_server(boost::asio::io_service& io_service)
    : acceptor_(io_service, tcp::endpoint(tcp::v4(), CHAT_PORT ))
  {
    start_accept();
  }

private:
  //creates a socket and initializes async accept to wait for a new connection
  void start_accept()
  {
    tcp_connection::pointer new_connection =
      tcp_connection::create(acceptor_.get_io_service());

    acceptor_.async_accept(new_connection->socket(),
        boost::bind(&tcp_server::handle_accept, this, new_connection,
          boost::asio::placeholders::error));
  }

  //we bind this as a handler for new connections. It's what's called when accept operation initiated by start_accept() finishes
  void handle_accept(tcp_connection::pointer new_connection,
      const boost::system::error_code& error)
  {
    if (!error)
    {
      new_connection->start();
    }

    start_accept();
  }

  tcp::acceptor acceptor_;
};

int main()
{
  try
  {
    boost::asio::io_service io_service;
    tcp_server server(io_service);
    io_service.run();
  }
  catch (std::exception& e)
  {
    std::cerr << e.what() << std::endl;
  }

  return 0;
}
