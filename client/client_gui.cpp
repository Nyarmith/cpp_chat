//-ok take break then study up the specific window functions!, we're still implementing draw()
#include "client_gui.hpp"
#include <ctype.h>

chat_client_gui::chat_client_gui(boost::asio::io_service& io_service,
                                 tcp::resolver::iterator endpoint_iterator,
                                 std::string& host, 
                                 std::string& port, 
                                 std::string& username)
    : io_service_(io_service), socket_(io_service),
    prompt_text_(""), read_msg_(""), exiting_(false), uname_(username)
{
    //tcp::resolver  resolver(io_service_);
    //tcp::resolver::query  query(host, port);
    //tcp::resolver::iterator iterator = resolver.resolve(query);
    ////socket_ = tcp::socket(io_service); //apparently this makes a socket
    //socket_(io_service); //apparently this makes a socket

    host_ = host+port;

    //make a handler for when we connect
    boost::asio::async_connect(socket_, endpoint_iterator,
            boost::bind(&chat_client_gui::handle_connect, this, boost::asio::placeholders::error));
}

std::string chat_client_gui::get_line(){
    return prompt_text_;
}

void chat_client_gui::run(){
    boost::thread t(boost::bind(&boost::asio::io_service::run, &io_service_));

    int y, x, c;
    //init ncurses
    initscr();
    keypad(stdscr, TRUE);
    getmaxyx(stdscr, y, x);
    noecho();
    //create the windows

    int rhs  =  height_ - prompt_height_ - border_size_; //right-horizontal-split
    int rss  =  rooms_width_ + border_size_; //right-side-start
    int rssz =  width_ - rss; //right-side-size

    logo_window_   = util::create_newwin(logo_height_, rooms_width_, 0, 0  );  //top left
    chat_window_   = util::create_newwin(rhs         , width_ - rss, 0, rss);  //top right
    rooms_window_  = util::create_newwin(height_ - logo_height_ - border_size_,  rooms_width_,
                                         logo_height_ + border_size_, 0);   //bottom left

    prompt_window_ = util::create_newwin(prompt_height_, width_ - rss,
                                         rhs + border_size_, rss);   //bottom right
    
    //draw gui
    draw();

    //wait in eternal loop for new messages
    while(!exiting_){
        c = getch(); //this is blocking, but the other threads should update the screen when we get new messages
        switch(c){
            case KEY_ENTER:
                enter_msg();
                break;
            case KEY_BACKSPACE:
                backspace_msg();
                break;
            default:
                if (isprint(c)){
                    //prompt_text_.append(c); //please work implicitly
                    prompt_text_ += static_cast<char>(c);
                    draw();
                }
                break;
        }
    }

    //quit safely
    util::destroy_win(logo_window_  );
    util::destroy_win(chat_window_  );
    util::destroy_win(rooms_window_ );
    util::destroy_win(prompt_window_);
    endwin();
}

void chat_client_gui::draw(){

    clear();

    //draw border between windows
    int rhs  =  height_ - prompt_height_ - border_size_; //right-horizontal-split
    int rss  =  rooms_width_ + border_size_; //right-side-start

    //left horizontal border
    for (int x=0; x<rhs; x++){
        for (int y=0; y < border_size_; y++)
            mvaddch(logo_height_ + y, x, '#');
    }
    
    //right horizontal border
    for (int x=rss + border_size_; x<width_; x++){
        for (int y=0; y < border_size_; y++)
            mvaddch(rhs + y, x, '#');
    }

    //middle vertical border
    for (int y=0; y < height_; y++){
        for (int x = 0; x < border_size_; x++)
            mvaddch(y, x + rss, '#');
    }
    
    draw_logo_window();
    draw_chat_window();
    draw_rooms_window();
    draw_prompt_window();

    refresh();
}

void chat_client_gui::enter_msg(){
    if(prompt_text_.length() != 0){
        //prepend username
        prompt_text_ = uname_ + prompt_text_;
        send_message(prompt_text_);
        prompt_text_ = "";
        draw();
    }
}

void chat_client_gui::backspace_msg(){
    if(prompt_text_.length() != 0){
        //prompt_text_.pop_back();    //c++11  !!!
        prompt_text_ = prompt_text_.substr(0, prompt_text_.length() - 1);
        draw();
    }
}

void chat_client_gui::send_message(const std::string& msg){
    io_service_.post(boost::bind(&chat_client_gui::do_write, this, msg));
}

void chat_client_gui::draw_logo_window(){
    //for now just a simple set of asterisks
    for (int x=0; x<rooms_width_; x++)
        for (int y=0; y<logo_height_; y++)
            mvwaddch(logo_window_, y, x, '~');
}

void chat_client_gui::draw_chat_window(){
    int chat_height = (height_ - border_size_ - prompt_height_);
    for (int i=0; i < chat_messages_.size() && i < chat_height; i++){
        mvwprintw(chat_window_, chat_height - i, 0, "%s", chat_messages_[i].c_str());
    }
}

void chat_client_gui::draw_rooms_window(){
    wprintw(rooms_window_, "%s", host_.c_str());
}

void chat_client_gui::draw_prompt_window(){
    wprintw(prompt_window_, "%s", prompt_text_.c_str());
}

//handlers for asio
void chat_client_gui::do_write(std::string msg){
    bool write_in_progress = !write_messages_.empty();
    write_messages_.push_back(msg);
    if (!write_in_progress)
    {
        boost::asio::async_write(socket_,
                boost::asio::buffer(write_messages_.front().data(),
                    write_messages_.front().length()),
                boost::bind(&chat_client_gui::handle_write, this,
                    boost::asio::placeholders::error));
    }

}

void chat_client_gui::handle_write(const boost::system::error_code& error){
    if (!error)
    {
        write_messages_.pop_front();
        if (!write_messages_.empty())
        {
            boost::asio::async_write(socket_,
                    boost::asio::buffer(write_messages_.front().data(),
                        write_messages_.front().length()),
                    boost::bind(&chat_client_gui::handle_write, this,
                        boost::asio::placeholders::error));
        }
    }
    else
    {
        do_close();
    }
}

void chat_client_gui::handle_connect(const boost::system::error_code& error){
    if (!error)
    {
        boost::asio::async_read(socket_,
                boost::asio::buffer( read_msg_.data(), msg_max_size_),
                boost::bind(&chat_client_gui::handle_read, this,
                    boost::asio::placeholders::error));

    }
    else
    {
        do_close();
    }
}

void chat_client_gui::handle_read(const boost::system::error_code& error){
    if(!error)
    {
        //like push_back but creates a new object that isn't deconstructed
        //when we exit this scope
        chat_messages_.emplace_back(read_msg_.data());
        draw();

        //set up next handler
        boost::asio::async_read(socket_,
                boost::asio::buffer(read_msg_.data(), msg_max_size_),
                boost::bind(&chat_client_gui::handle_read, this,
                    boost::asio::placeholders::error));
    }
    else
    {
        do_close();
    }
}

void chat_client_gui::do_close(){
    socket_.close();
}
