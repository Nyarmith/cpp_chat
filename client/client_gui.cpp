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

    host_ = host+ " " + port;

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
    
    int chat_height = height_ - border_size_ - prompt_height_;
    int rooms_height= height_ - border_size_ - logo_height_;
    int right_width = width_ - border_size_ - rooms_width_;

    logo_window_   = util::create_newwin(0, 0, logo_height_, rooms_width_);  //top left
    chat_window_   = util::create_newwin(0, rooms_width_ + border_size_,
                                         chat_height, right_width);  //top right
    rooms_window_  = util::create_newwin(logo_height_ + border_size_, 0,
                                         rooms_height, rooms_width_); //bottom left
                                        
    prompt_window_ = util::create_newwin(chat_height + border_size_, rooms_width_ + border_size_,
                                         prompt_height_, right_width);
    keypad(prompt_window_, TRUE);
    //draw gui
    draw();

    //wait in eternal loop for new messages
    while(!exiting_){
        c = wgetch(prompt_window_); //this is blocking, but the other threads should update the screen when we get new messages
        switch(c){
            case 10: //I used KEY_ENTER but it didn't work
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


    /*
    //draw border between windows
    int rhs  =  height_ - prompt_height_ - border_size_; //right-horizontal-split
    int rss  =  rooms_width_ + border_size_; //right-side-start

    //left horizontal border
    for (int x=0; x<=rooms_width_; x++){
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
    refresh();
    */
    
    draw_logo_window();
    draw_chat_window();
    draw_rooms_window();
    draw_prompt_window();

    //refresh();
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
        prompt_text_.pop_back();    //c++11  !!!
        //prompt_text_ = prompt_text_.substr(0, prompt_text_.length() - 1);
        draw();
    }
}

void chat_client_gui::send_message(const std::string& msg){
    io_service_.post(boost::bind(&chat_client_gui::do_write, this, msg));
}

void chat_client_gui::draw_logo_window(){
    wclear(logo_window_);
    //for now just a simple set of asterisks
    for (int x=1; x<rooms_width_; x++)
        for (int y=1; y<logo_height_; y++)
            mvwaddch(logo_window_, y, x, '~');

    box(logo_window_, 0 , 0);
    wrefresh(logo_window_);
}

void chat_client_gui::draw_chat_window(){
    wclear(chat_window_);
    int chat_height = (height_ - border_size_ - prompt_height_);
    for (int i=0; i < chat_messages_.size() && i < chat_height; i++){
        mvwprintw(chat_window_, i, 1, "%s", chat_messages_[i].c_str());
    }
    box(chat_window_, 0 , 0);
    wrefresh(chat_window_);
}

void chat_client_gui::draw_rooms_window(){
    wclear(rooms_window_);
    // TODO, update when multi-chat support is added
    mvwprintw(rooms_window_, 1, 1, "%s", host_.c_str());
    box(rooms_window_, 0 , 0);
    wrefresh(rooms_window_);
}

void chat_client_gui::draw_prompt_window(){
    wclear(prompt_window_);
    if (prompt_text_.length() != 0){
        mvwprintw(prompt_window_,1,1, "> %s", prompt_text_.c_str());
    }
    else{
        mvwprintw(prompt_window_,1,1, "> ");
    }
    box(prompt_window_, 0 , 0);
    wrefresh(prompt_window_);
}

//handlers for asio
void chat_client_gui::do_write(std::string msg){
    bool write_in_progress = !write_messages_.empty();
    write_messages_.push_back(msg);
    if (!write_in_progress)
    {
        boost::asio::async_write(socket_,
                boost::asio::buffer(util::give_cstr(write_messages_.front()),
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
                    boost::asio::buffer(util::give_cstr(write_messages_.front()),
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
                boost::asio::buffer( util::give_cstr(read_msg_), msg_max_size_),
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
                boost::asio::buffer(util::give_cstr(read_msg_), msg_max_size_),
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

char*   util::give_cstr(std::string& ourstr){
    char *cstr = new char[ourstr.length() + 1]; //memory leak
    strcpy(cstr, ourstr.c_str());
    return cstr;
}

WINDOW* util::create_newwin(int y, int x, int hgt, int wdt){
    WINDOW *local_win;

	local_win = newwin(hgt, wdt, y, x);
	//box(local_win, 0 , 0);		/* 0, 0 gives default characters 
					 //* for the vertical and horizontal
					 //* lines			*/
	wrefresh(local_win);		/* Show that box 		*/

	return local_win;
}
void    util::destroy_win  (WINDOW * local_win){
	delwin(local_win);
}
