#include "Resolver.h"
#include "Logging.h"
#include <stdexcept>
#include <string.h>

namespace tornado
{

//c-ares 初始管理类
class CAresLibInit
{
public:
    CAresLibInit():init_(false){}

    ~CAresLibInit()
    {
        if(init_)
        {
            init_ = false;
            ares_library_cleanup();
        }
    }

    void init_lib()
    {
        if(init_)
        {
            return;
        }
        int result = ares_library_init(ARES_LIB_INIT_ALL);
        if (result != ARES_SUCCESS)
        {
            TORNADO_LOG_ERROR("ares_library_init: %s", ares_strerror(result));
            throw std::runtime_error( std::string("ares_library_init fail :") + ares_strerror(result) );
        }
        init_ = true;
    }

private:
    bool init_;
};

static CAresLibInit g_ObjCAresLib;

 

Resolver::Resolver():
    channel_(nullptr),
    state_(0), 
    io_loop_(tornado::IOLoop::instance()),
    timer_(0)
{
    TORNADO_LOG_DEBUG_STR("======================Resolver");
}

Resolver::~Resolver() 
{
    TORNADO_LOG_DEBUG_STR("---------------------------~Resolver");
    if(channel_)
    {
        ares_destroy(channel_);
        channel_ = nullptr;
    }
    cb_ = nullptr;
}

int Resolver::init(int flags/*=0*/, int tries/*=0*/, int ndots/*=0*/, int udp_port/*=0*/, int tcp_port/*=0*/)
{
    //初始lib  void*
    g_ObjCAresLib.init_lib();

    struct ares_options options;
    memset(&options, 0, sizeof(options));
    int optmask = ARES_OPT_SOCK_STATE_CB;
    options.sock_state_cb = Resolver::sock_state_callback;
    options.sock_state_cb_data = this; 
    if(flags)
    {
        options.flags = flags;
        optmask |= ARES_OPT_FLAGS;
    }
    /*if(timeout_ms)
    {
        options.timeout = timeout_ms;
        optmask |= ARES_OPT_TIMEOUTMS;
    }*/
    if(tries)
    {
        options.tries = tries;
        optmask |= ARES_OPT_TRIES;
    }
    if(ndots)
    {
        options.ndots = ndots;
        optmask |= ARES_OPT_NDOTS;
    }
    if(udp_port)
    {
        options.udp_port = udp_port;
        optmask |= ARES_OPT_UDP_PORT;
    }
    if(tcp_port)
    {
        options.tcp_port = tcp_port;
        optmask |= ARES_OPT_TCP_PORT;
    }
    assert(channel_ == nullptr);
    int result = ares_init_options(&channel_, &options, optmask);
    if (result != ARES_SUCCESS)
    {
        TORNADO_LOG_WARN("ares_init_options: %s", ares_strerror(result));
        return -1;
    }
    return 0;
}

void Resolver::sock_state_callback(int socket, int read, int write)
{
    assert(channel_ != nullptr);
    int events = tornado::IOLoop::ERROR;
    if(read)
        events |= tornado::IOLoop::READ;
    if(write)
        events |= tornado::IOLoop::WRITE;

    if(!read && !write && state_)
    {
        TORNADO_LOG_DEBUG_STR("remove handler to ioloop")
        io_loop_->removeHandler(socket);
        state_ = 0;
        return ;
    }

    if(state_ == 0)
    {
        TORNADO_LOG_DEBUG_STR("add handler to ioloop")
        state_ = events;
        io_loop_->addHandler(socket, 
            std::bind(&Resolver::process_fd, this, std::placeholders::_1, std::placeholders::_2), 
            state_);
    }
    if( state_ != events )
    {
        TORNADO_LOG_DEBUG_STR("update handler to ioloop")
        state_ = events;
        io_loop_->updateHandler(socket, state_);
    }
}
   
void Resolver::on_timer(tornado::IOLoop::TimerID tid, int64_t expiration)
{ 
    TORNADO_LOG_DEBUG_STR("timeout");
    assert(tid == timer_);
    if(timer_ != 0)//have clear
    {
        timer_ = 0;
        ares_process_fd(channel_, ARES_SOCKET_BAD, ARES_SOCKET_BAD);
    }
}
    
void Resolver::process_fd(int32_t fd, uint32_t events)
{
    TORNADO_LOG_DEBUG("fd=%d, events=%u", fd, events);
    assert(channel_ != nullptr);
    int read_fd = fd;
    int write_fd = read_fd;
    if(!(events & tornado::IOLoop::READ))
        read_fd = ARES_SOCKET_BAD;
    if(!(events & tornado::IOLoop::WRITE))
        write_fd = ARES_SOCKET_BAD;
    ares_process_fd(channel_, read_fd, write_fd);
}

void Resolver::gethostbyname(Callback cb, const char* name, int family/*=AF_INET*/, int timeout_ms/*=1000*/)
{
    assert(channel_ != nullptr);
    cb_ = std::move(cb);
    //# note that for file lookups still AF_INET can be returned for AF_INET6 request
    ares_gethostbyname(channel_, name, family, Resolver::ares_host_callback, this);
       
    timer_ = io_loop_->addTimeout(timeout_ms, std::bind(&Resolver::on_timer, this, 
        std::placeholders::_1, std::placeholders::_2));
}


void Resolver::sock_state_callback(void *data, int socket, int read, int write)
{
    TORNADO_LOG_DEBUG("Change state fd %d read:%d write:%d", socket, read, write);
    Resolver* ch = reinterpret_cast<Resolver*>(data);
    ch->sock_state_callback(socket, read, write);
}

void Resolver::ares_host_callback(void *arg, int status, int timeouts, struct hostent* host)
{
    Resolver* ch = reinterpret_cast<Resolver*>(arg);
    if(!host || status != ARES_SUCCESS)
    {
        TORNADO_LOG_WARN("Failed to lookup %s\n", ares_strerror(status));
        ch->run_callback(-1, NULL);
        return;
    }
    TORNADO_LOG_DEBUG("Found address name %s", host->h_name);
    Hosts hosts;
    char ip[INET6_ADDRSTRLEN];
    for (int i = 0; host->h_addr_list[i]; ++i) 
    {
        inet_ntop(host->h_addrtype, host->h_addr_list[i], ip, sizeof(ip));
        std::string strIP(ip);
        hosts.push_back( std::move(strIP) );
    }
    ch->run_callback(0, &hosts);
}

void Resolver::run_callback(int err, const Hosts* hosts)
{
    remove_timer();
    if(cb_)
    {
        cb_(err, hosts);
        cb_ = nullptr;
    }
}


}//namespace tornado
 
        