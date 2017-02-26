#ifndef yfs_client_h
#define yfs_client_h

#include <string>

<<<<<<< HEAD
#include "lock_protocol.h"
#include "lock_client.h"

//#include "yfs_protocol.h"
=======
>>>>>>> lab3
#include "extent_client.h"
#include <vector>


class yfs_client {
  extent_client *ec;
<<<<<<< HEAD
  lock_client *lc;
=======
>>>>>>> lab3
 public:

  typedef unsigned long long inum;
  enum xxstatus { OK, RPCERR, NOENT, IOERR, EXIST };
  typedef int status;

  struct fileinfo {
    unsigned long long size;
    unsigned long atime;
    unsigned long mtime;
    unsigned long ctime;
  };
  struct dirinfo {
    unsigned long atime;
    unsigned long mtime;
    unsigned long ctime;
  };
  struct dirent {
    std::string name;
    yfs_client::inum inum;
  };

 private:
  static std::string filename(inum);
  static inum n2i(std::string);

 public:
<<<<<<< HEAD
  yfs_client(std::string, std::string);
=======
  yfs_client(std::string);
>>>>>>> lab3

  bool isfile(inum);
  bool isdir(inum);

  int getfile(inum, fileinfo &);
  int getdir(inum, dirinfo &);

  int setattr(inum, size_t);
  int lookup(inum, const char *, bool &, inum &);
  int create(inum, const char *, mode_t, inum &);
  int readdir(inum, std::list<dirent> &);
  int write(inum, size_t, off_t, const char *, size_t &);
  int read(inum, size_t, off_t, std::string &);
  int unlink(inum,const char *);
  int mkdir(inum , const char *, mode_t , inum &);
<<<<<<< HEAD
=======
  
  /** you may need to add symbolic link related methods here.*/
>>>>>>> lab3
};

#endif 
