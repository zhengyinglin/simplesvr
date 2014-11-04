#ifndef BASE_SHARED_MESSAGE_QUEUE_HPP_
#define BASE_SHARED_MESSAGE_QUEUE_HPP_

#include "boost/interprocess/ipc/message_queue.hpp"
#include <assert.h>

namespace  boostshm
{

class MsgQueue
{
public:
    MsgQueue(const char* pMemName, size_t maxMsgNum, size_t maxMsgSize, bool bCreate = false)
    {
        mq_ = NULL;
        if(!bCreate)
        {
            mq_ = new boost::interprocess::message_queue(boost::interprocess::open_only,
                 pMemName);
        }
        else
        {
            mq_ = new boost::interprocess::message_queue(boost::interprocess::open_or_create,
                 pMemName, maxMsgNum, maxMsgSize);
        }
        //check size
        assert( mq_->get_max_msg() == maxMsgNum && mq_->get_max_msg_size() == maxMsgSize);
    }

    virtual ~MsgQueue()
    {
        delete mq_;
    }

    //If the message queue is full  returns false, otherwise returns true. Throws interprocess_error on error.
    bool send(const char* data, size_t len)
    {
        return mq_->try_send(data, len, 0);
    }

    //If the message queue is empty returns false, otherwise returns true. Throws interprocess_error on error.
    bool receive(char* data, size_t& len)
    {
        size_t data_size = len;
        unsigned priority = 0;
        return mq_->try_receive(data, data_size, len, priority);;
    }

protected:
    boost::interprocess::message_queue*  mq_;
};


 
} // namespace  boostshm

#endif //BASE_SHARED_MESSAGE_QUEUE_HPP_
