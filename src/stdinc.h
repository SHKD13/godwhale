#ifndef GODWHALE_STDINC_H
#define GODWHALE_STDINC_H

#include "precomp.h"
#include "bonanza6/shogi.h"

#define LOCK_IMPL(obj, lock, flag)       \
    if (bool flag = false) {} else       \
    for (ScopedLock lock(obj); !flag; flag = true)

/** オリジナルのロック定義 */
#define LOCK(obj) \
    LOCK_IMPL(obj, lock_7D0B4924019B, flag_7D0B4924019B)

#define F(fmt) ::boost::format(fmt)

namespace godwhale {

using boost::shared_ptr;
using boost::make_shared;
using boost::weak_ptr;
using boost::enable_shared_from_this;
using boost::lexical_cast;

typedef boost::asio::ip::tcp tcp;
typedef boost::recursive_mutex Mutex;
typedef boost::recursive_mutex::scoped_lock ScopedLock;

enum
{
    PROCE_OK = 0,
    PROCE_ABORT = 1,
    PROCE_CONTINUE = 2,
};

extern tree_t * g_ptree;
extern unsigned int state_node;

} // namespace godwhale

#include "logger.h"
#include "util.h"

#endif
