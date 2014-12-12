/**
 ***All Rights Reserved
 *
 *author zhengyinglin (979762787@qq.com)
 *date 2014-11-26
*/

//https://github.com/facebook/folly/blame/master/folly/ScopeGuard.h

#ifndef _UTIL_COMM_SCOPE_GUARD_H_
#define _UTIL_COMM_SCOPE_GUARD_H_
#include <type_traits>
#include "boost/optional.hpp"

namespace util
{

namespace comm
{


template <typename FunctionType>
class ScopeGuardImpl
{
public:
    explicit ScopeGuardImpl(FunctionType&& fn):
        function_(std::move(fn)), dismissed_(false)
    {
    }

    ScopeGuardImpl(ScopeGuardImpl&& other):
        function_(std::move(other.function_)),
        dismissed_(other.dismissed_) 
    {
        other.dismissed_ = true;
    }

    ~ScopeGuardImpl() noexcept 
    {
        if (!dismissed_) 
        {
            execute();
        }
    }

    void dismiss() noexcept 
    {
        dismissed_ = true;
    }

private:
    void* operator new(size_t) = delete;
    void execute() noexcept { function_(); }

private:
  FunctionType function_;
  bool dismissed_;
};


template <typename FunctionType> inline
ScopeGuardImpl<typename std::decay<FunctionType>::type>  makeGuard(FunctionType&& fn)
{
    return ScopeGuardImpl<typename std::decay<FunctionType>::type>( std::forward<FunctionType>(fn) );
}





}//namespace comm

}//namespace util

#endif //_UTIL_COMM_SCOPE_GUARD_H_