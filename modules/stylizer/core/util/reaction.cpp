#include "reaction.hpp"

namespace reaction {

    thread_local std::function<void(const NodePtr &)> g_reg_fun = nullptr;
    thread_local std::function<void(const NodePtr &)> g_batch_fun = nullptr;
    thread_local bool g_batch_execute = false;

}