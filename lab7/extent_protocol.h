// extent wire protocol

#ifndef extent_protocol_h
#define extent_protocol_h

#include "rpc.h"

class extent_protocol {
 public:
  typedef int status;
  typedef unsigned long long extentid_t;
  enum xxstatus { OK, RPCERR, NOENT, IOERR };
  enum rpc_numbers {
    put = 0x6001,
    get,
    getattr,
    setattr,
    remove,
    create,
    cleanAllNodes,
    set_ino
  };

  enum types {
    T_DIR = 1,
    T_FILE,
    T_SYML
  };

  struct attr {
    uint32_t type;
    unsigned int atime;
    unsigned int mtime;
    unsigned int ctime;
    unsigned int size;
    unsigned int mode;
    unsigned short uid;
    unsigned short gid;
  };
};

/*inline unmarshall &
operator>>(unmarshall &u, unsigned long& a)
{
  u >> a;

  return u;
}

inline marshall &
operator<<(marshall &m, unsigned long a)
{
  m << a;
 
  return m;
}*/

inline unmarshall &
operator>>(unmarshall &u, extent_protocol::attr &a)
{
  u >> a.type;
  u >> a.atime;
  u >> a.mtime;
  u >> a.ctime;
  u >> a.size;
  u >> a.mode;
  u >> a.uid;
  u >> a.gid;
  return u;
}

inline marshall &
operator<<(marshall &m, extent_protocol::attr a)
{
  m << a.type;
  m << a.atime;
  m << a.mtime;
  m << a.ctime;
  m << a.size;
  m << a.mode;
  m << a.uid;
  m << a.gid;
  return m;
}

#endif 
