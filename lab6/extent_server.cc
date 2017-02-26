// the extent server implementation

#include "extent_server.h"
#include <sstream>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

extent_server::extent_server() 
{
  im = new inode_manager();
}

int extent_server::create(uint32_t type, extent_protocol::extentid_t &id)
{
  // alloc a new inode and return inum
  printf("extent_server: create inode\n");
  id = im->alloc_inode(type);

  return extent_protocol::OK;
}

int extent_server::put(extent_protocol::extentid_t id, std::string buf, int &)
{
  id &= 0x7fffffff;
  
  const char * cbuf = buf.c_str();
  int size = buf.size();
  im->write_file(id, cbuf, size);
  
  return extent_protocol::OK;
}

int extent_server::get(extent_protocol::extentid_t id, std::string &buf)
{
  printf("extent_server: get %lld\n", id);

  id &= 0x7fffffff;

  int size = 0;
  char *cbuf = NULL;

  im->read_file(id, &cbuf, &size);
  if (size == 0)
    buf = "";
  else {
    buf.assign(cbuf, size);
    free(cbuf);
  }

  return extent_protocol::OK;
}

int extent_server::getattr(extent_protocol::extentid_t id, extent_protocol::attr &a)
{
  printf("extent_server: getattr %lld\n", id);

  id &= 0x7fffffff;
  
  extent_protocol::attr attr;
  memset(&attr, 0, sizeof(attr));
  im->getattr(id, attr);
  a = attr;

  return extent_protocol::OK;
}

int extent_server::remove(extent_protocol::extentid_t id, int &)
{
  printf("extent_server: write %lld\n", id);

  id &= 0x7fffffff;
  im->remove_file(id);
 
  return extent_protocol::OK;
}



int extent_server::cleanAllNodes(extent_protocol::extentid_t id, int &){
  printf("i am here debug !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
  struct inode *ino = new inode;
    ino->type = 0;
  ino->size = 0;
  ino->atime = 0;
  ino->mtime = 0;
  ino->ctime = 0;
  for(int i= 1; i<= INODE_NUM; ++i)
    im->put_inode(i, ino);
  delete ino;
  return extent_protocol::OK;
}

int extent_server::set_ino(extent_protocol::extentid_t id, uint32_t type, std::string buf, int &){
  struct inode *ino = new inode;
    ino->type = type;
  ino->size = 0;
  ino->atime = 0;
  ino->mtime = 0;
  ino->ctime = 0;
  im->put_inode(id, ino);
  delete ino;
  if(type != 0){
    const char *cbuf = buf.c_str();
    int buf_size = buf.size();
    im->write_file(id, cbuf, buf_size);
  }
  return extent_protocol::OK;
}

