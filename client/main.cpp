#include "client_gui.cpp"

int main(int argc, char* argv[])
{
  //try
  //{
    if (argc != 4)
    {
      std::cerr << "Usage: chat_client <host> <port> <uname>\n";
      return 1;
    }

    std::string host      =  argv[1];
    std::string portno    =  argv[2];
    std::string username  =  argv[3];

    boost::asio::io_service io_service;

    //form a query
    tcp::resolver resolver(io_service);
    tcp::resolver::query query(host, portno); //may need to be c_strings rather than std::strings
    tcp::resolver::iterator iterator = resolver.resolve(query);

    chat_client_gui c(io_service, iterator, host, portno, username);
    //chat_client_gui c(host, portno, username);
    c.run();

    /*
    boost::asio::io_service io_service;

    //form a query
    tcp::resolver resolver(io_service);
    tcp::resolver::query query(host, portno); //may need to be c_strings rather than std::strings
    tcp::resolver::iterator iterator = resolver.resolve(query);

    //initiallize a chat client with given endpoint(these just
    //populate the io_service_ and socket_ members of chat_client)
    chat_client c(io_service, iterator, username);
    chat_client_gui g(); //init a 400x200 char chat client gui

    //bind a thread to our io_service ?
    boost::thread t(boost::bind(&boost::asio::io_service::run, &io_service));

    //an individual line of the message
    char line[chat_message::max_body_length + 1];

    //handle input
    std::string user_input = g.get_user_line();
    while (user_input != ":quit"){
        c.queue_message(user_input);  //TODO make queue_message, to handle encode_header and copying to message object(or several)
        user_input = g.get_user_line(); //TODO make get_user_line() an interactive wait function
    }

//    while (std::cin.getline(line, chat_message::max_body_length + 1))
//    {
//      chat_message msg;
//      msg.body_length(std::strlen(line));
//      std::memcpy(msg.body(), line, msg.body_length());
//      msg.encode_header();
//      c.write(msg);
//    }
//
//    c.close();
//    t.join();

  }
  catch (std::exception& e)
  {
    std::cerr << "Exception: " << e.what() << "\n";
  }
  */

  return 0;
}

