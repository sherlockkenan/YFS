// RPC stubs for clients to talk to extent_server

#include "extent_client.h"
#include <sstream>
#include <iostream>
#include <stdio.h>
#include <unistd.h>
#include <time.h>

extent_client::extent_client(std::string dst)
{
  sockaddr_in dstsock;
  make_sockaddr(dst.c_str(), &dstsock);
  cl = new rpcc(dstsock);
  if (cl->bind() != 0) {
    printf("extent_client: bind failed\n");
  }
}

// a demo to show how to use RPC
extent_protocol::status
extent_client::getattr(extent_protocol::extentid_t eid, 
		       extent_protocol::attr &attr)
{
  extent_protocol::status ret = extent_protocol::OK;
   
  ret = cl->call(extent_protocol::getattr, eid, attr);
  printf ("in the cl getattr mode=%o,eid=%d\n",attr.mode,eid);
  return ret;
}

extent_protocol::status
extent_client::setattr(extent_protocol::extentid_t eid, unsigned int mode,unsigned short uid,unsigned short gid)
{
  extent_protocol::status ret = extent_protocol::OK;
  printf ("in the cl mode=%o\n,uid:%d",mode,uid);
  extent_protocol::attr att;
  att.mode=mode;
  att.uid=uid;
  att.gid=gid;
  ret = cl->call(extent_protocol::setattr, eid, att,eid);
  return ret;
}

extent_protocol::status
extent_client::create(uint32_t type,unsigned int mode,unsigned short uid,unsigned short gid, extent_protocol::extentid_t &id)
{
  extent_protocol::status ret = extent_protocol::OK;
  // Your lab3 code goes here
  
  extent_protocol::attr att;
  att.mode=mode;
  att.uid=uid;
  att.gid=gid;
  ret = cl->call(extent_protocol::create,type,att,id);
  return ret;
}

extent_protocol::status
extent_client::get(extent_protocol::extentid_t eid, std::string &buf)
{
  extent_protocol::status ret = extent_protocol::OK;
  // Your lab3 code goes here
  ret = cl->call(extent_protocol::get, eid,buf);
  return ret;
}

extent_protocol::status
extent_client::put(extent_protocol::extentid_t eid, std::string buf)
{
  extent_protocol::status ret = extent_protocol::OK;
  // Your lab3 code goes here
  ret = cl->call(extent_protocol::put, eid, buf,ret);
  return ret;
}

extent_protocol::status
extent_client::remove(extent_protocol::extentid_t eid)
{
  extent_protocol::status ret = extent_protocol::OK;
  // Your lab3 code goes here
  ret = cl->call(extent_protocol::remove, eid,ret);
  return ret;
}


extent_protocol::status
extent_client::cleanAllNodes(extent_protocol::extentid_t eid){
  extent_protocol::status ret = extent_protocol::OK;
  ret = cl->call(extent_protocol::cleanAllNodes, eid, ret);
  return ret;
}

extent_protocol::status
extent_client::set_ino(extent_protocol::extentid_t eid, uint32_t type, std::string buf){
  extent_protocol::status ret = extent_protocol::OK;
  ret = cl->call(extent_protocol::set_ino, eid, type, buf, ret);
  return ret;
}