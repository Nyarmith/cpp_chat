#include <curses.h>
#include <deque>
#include <vector>
#include <boost/bind.hpp>
#include <boost/asio.hpp>
#include <boost/thread/thread.hpp>

using boost::asio::ip::tcp;


namespace util {
    WINDOW* create_newwin(int height, int width, int starty, int startx);
    void    destroy_win  (WINDOW * local_win);
    char*   give_cstr(std::string& ourstr);
}

class chat_client_gui {
    public:
    chat_client_gui(boost::asio::io_service&,
                    tcp::resolver::iterator endpoint_iterator,
                    std::string& host, 
                    std::string& port, 
                    std::string& username); //TODO, replace with menu that chooses this

    //Entrypoint to chat client
    void run();

    private:

    //Helper methods
    std::string get_line();  //get user's typed prompt
    void enter_msg();
    void backspace_msg();
    void send_message(const std::string& msg);
    void draw();             //draw the current state of the gui and chat
    void draw_logo_window();
    void draw_chat_window();
    void draw_rooms_window();
    void draw_prompt_window();

    //Members
    std::string uname_;
    std::string host_;
    boost::asio::io_service&  io_service_;
    tcp::socket socket_;
    std::vector<std::string> chat_messages_;
    std::deque <std::string> write_messages_;
    std::string prompt_text_;  //user's current typed message
    std::string read_msg_;     //current message we're reading
    bool exiting_;             //sentinel value for our main run loop

    //Ncurses Bookkeeping
    WINDOW* logo_window_;   //top left
    WINDOW* chat_window_;   //top right
    WINDOW* rooms_window_;  //bottom left
    WINDOW* prompt_window_; //bottom right


    //Constants
    const int width_          =  150;
    const int height_         =  40;
    const int rooms_width_    =  30;
    const int logo_height_    =  3;
    const int prompt_height_  =  2;
    const int border_size_    =  1;
    const int msg_max_size_   =  512;


    //Handlers for asio
    void handle_connect(const boost::system::error_code& error);
    void handle_read  (const boost::system::error_code& error);
    void do_write      (std::string msg);
    void handle_write  (const boost::system::error_code& error);
    void do_close();
};


