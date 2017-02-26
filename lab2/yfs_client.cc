// yfs client.  implements FS operations using extent and lock server
#include "yfs_client.h"
#include "extent_client.h"
#include <sstream>
#include <iostream>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

yfs_client::yfs_client()
{
    ec = new extent_client();

}

yfs_client::yfs_client(std::string extent_dst, std::string lock_dst)
{
    ec = new extent_client();
    if (ec->put(1, "") != extent_protocol::OK)
        printf("error init root dir\n"); // XYB: init root dir
}

yfs_client::inum
yfs_client::n2i(std::string n)
{
    std::istringstream ist(n);
    unsigned long long finum;
    ist >> finum;
    return finum;
}

std::string
yfs_client::filename(inum inum)
{
    std::ostringstream ost;
    ost << inum;
    return ost.str();
}

bool
yfs_client::isfile(inum inum)
{
    extent_protocol::attr a;

    if (ec->getattr(inum, a) != extent_protocol::OK) {
        printf("error getting attr\n");
        return false;
    }

    if (a.type == extent_protocol::T_FILE) {
        printf("isfile: %lld is a file\n", inum);
        return true;
    } 
    printf("isfile: %lld is a dir\n", inum);
    return false;
}
/** Your code here for Lab...
 * You may need to add routines such as
 * readlink, issymlink here to implement symbolic link.
 * 
 * */


int yfs_client::symlink(const char* symlin, inum parent, const char* name, inum& ino_out){
    
    int r=OK;

    bool found=true;
    lookup(parent,name,found,ino_out);
        if(found){
            r=EXIST;
            return r;
        }
    ec->create(extent_protocol::T_SYML,ino_out);
    std::string file_name;

    if(ec->get(parent,file_name)!=extent_protocol::OK){
                printf("get error \n");
        r=IOERR;
        return r;
    }
    file_name = file_name + filename(ino_out)+"/"+std::string(name)+"/";
    if (ec->put(parent, file_name) != extent_protocol::OK) {
        printf("put error \n");
        return IOERR;
                        
    }
    
    ec->put(ino_out,std::string(symlin));
    return r;
  }




int yfs_client::readlink(inum ino, std::string& link){
    
    int r=OK;
    if(ec->get(ino,link)!=extent_protocol::OK){
        return IOERR;
    }
    return r;
}




int yfs_client::getsymlink(inum ino, syminfo & syminf){
	int r = OK;

    extent_protocol::attr a;
    if (ec->getattr(ino, a) != extent_protocol::OK) {
        r = IOERR;
        return r;
    }

    syminf.atime = a.atime;
    syminf.mtime = a.mtime;
    syminf.ctime = a.ctime;
    syminf.size = a.size;

    return r;
    
}

bool
yfs_client::isdir(inum inum)
{
    // Oops! is this still correct when you implement symlink?
   // return ! isfile(inum);
    if(isfile(inum)) return false;
	else{
		extent_protocol::attr a;
		if (ec->getattr(inum, a) != extent_protocol::OK) {
		 	printf("error getting attr\n");
			return false;
		}
		if (a.type == extent_protocol::T_DIR) {
			printf("isdir: %lld is a dir\n", inum);
			return true;
		}else{
			printf("issym: %lld is a sym\n", inum);
			return false;
		}
	}
}

int
yfs_client::getfile(inum inum, fileinfo &fin)
{
    int r = OK;

    printf("getfile %016llx\n", inum);
    extent_protocol::attr a;
    if (ec->getattr(inum, a) != extent_protocol::OK) {
        r = IOERR;
        goto release;
    }

    fin.atime = a.atime;
    fin.mtime = a.mtime;
    fin.ctime = a.ctime;
    fin.size = a.size;
    printf("getfile %016llx -> sz %llu\n", inum, fin.size);

release:
    return r;
}

int
yfs_client::getdir(inum inum, dirinfo &din)
{
    int r = OK;

    printf("getdir %016llx\n", inum);
    extent_protocol::attr a;
    if (ec->getattr(inum, a) != extent_protocol::OK) {
        r = IOERR;
        goto release;
    }
    din.atime = a.atime;
    din.mtime = a.mtime;
    din.ctime = a.ctime;

release:
    return r;
}


#define EXT_RPC(xx) do { \
    if ((xx) != extent_protocol::OK) { \
        printf("EXT_RPC Error: %s:%d \n", __FILE__, __LINE__); \
        r = IOERR; \
        goto release; \
    } \
} while (0)

// Only support set size of attr
int
yfs_client::setattr(inum ino, size_t size)
{
    int r = OK;
    std::string buf;
    extent_protocol::attr attribute;
    if((ec->getattr(ino,attribute))!=extent_protocol::OK)
    {
        printf("error to get attr in setattr %d\n",attribute.size);
       return IOERR;
    }

    ec->get(ino,buf);

    if(size<=attribute.size){
        buf =buf.substr(0,size);
    }else{
        buf += std::string(size - attribute.size, '\0');
    
    }

    ec->put(ino,buf);

    /*
     * your lab2 code goes here.
     * note: get the content of inode ino, and modify its content
     * according to the size (<, =, or >) content length.
     */

    return r;
}

int
yfs_client::create(inum parent, const char *name, mode_t mode, inum &ino_out)
{
    int r = OK;
    /*
     * your lab2 code goes here.
     * note: lookup is what you need to check if file exist;
     * after create file or dir, you must remember to modify the parent infomation.
     */
    
    bool found=true;
    lookup(parent,name,found,ino_out);
        if(found){
            r=EXIST;
            return r;
        }
    ec->create(extent_protocol::T_FILE,ino_out);
    std::string file_name;

    if(ec->get(parent,file_name)!=extent_protocol::OK){
                printf("get error \n");
        r=IOERR;
        return r;
    }
    file_name = file_name + filename(ino_out)+"/"+std::string(name)+"/";
    if (ec->put(parent, file_name) != extent_protocol::OK) {
        printf("put error \n");
        return IOERR;
                        
    }

       // file_name+='/0';
    

    

    return r;
}

int
yfs_client::mkdir(inum parent, const char *name, mode_t mode, inum &ino_out)
{
    int r = OK;

    /*
     * your lab2 code goes here.
     * note: lookup is what you need to check if directory exist;
     * after create file or dir, you must remember to modify the parent infomation.
     */
    bool found=true;
    lookup(parent,name,found,ino_out);
        if(found){
            r=EXIST;
            return r;
        }
    ec->create(extent_protocol::T_DIR,ino_out);
    std::string file_name;

    if(ec->get(parent,file_name)!=extent_protocol::OK){
                printf("get error \n");
        r=IOERR;
        return r;
    }
    file_name = file_name + filename(ino_out)+"/"+std::string(name)+"/";
    if (ec->put(parent, file_name) != extent_protocol::OK) {
        printf("put error \n");
        return IOERR;
                        
    }
    return r;

}

int
yfs_client::lookup(inum parent, const char *name, bool &found, inum &ino_out)
{
    int r = OK;

    /*
     * your lab2 code goes here.
     * note: lookup file from parent dir according to name;
     * you should design the format of directory content.
     */

	found = false;
	std::list<dirent> dir;
    if(readdir(parent, dir)!=OK){
        r=IOERR;
        return r;
    }
	std::list<dirent>::iterator iter;
	for(iter = dir.begin(); iter != dir.end(); ++iter){
		if(iter->name == std::string(name)){
			found = true;
			ino_out = iter->inum;
			break;
		}
	}
    return r;
}

int
yfs_client::readdir(inum dir, std::list<dirent> &list)
{
    int r = OK;

    /*
     * your lab2 code goes here.
     * note: you should parse the dirctory content using your defined format,
     * and push the dirents to the list.
     */

	std::string dirstr, filename, inostr;
	int ino;
	 dirent diren;
    if(!isdir(dir)){
        r=IOERR;
        return r;
    }
	ec->get(dir, dirstr);
    int inopos=0,namepos=-1;
    while((inopos=dirstr.find('/',namepos+1))!=-1){
        inostr=dirstr.substr(namepos+1,inopos-namepos-1);
        ino=n2i(inostr);
        if(ino==0) break;
        namepos=dirstr.find('/',inopos+1);
        filename=dirstr.substr(inopos+1,namepos-inopos-1);
        
        diren.inum=ino;
        diren.name=filename;
        list.push_back(diren); 
        
    }

    return r;
}

int
yfs_client::read(inum ino, size_t size, off_t off, std::string &data)
{
    int r = OK;

    /*
     * your lab2 code goes here.
     * note: read using ec->get().
     */
	std::string tmpBuf;
	ec->get(ino, tmpBuf);
	if(off > tmpBuf.length()) data = "";
	else data = tmpBuf.substr(off, size);

    return r;
}

int
yfs_client::write(inum ino, size_t size, off_t off, const char *data,
        size_t &bytes_written)
{
    int r = OK;

    /*
     * your lab2 code goes here.
     * note: write using ec->put().
     * when off > length of original file, fill the holes with '\0'.
     */
    std::string buf,data_str;
    ec->get(ino,buf);
    data_str=std::string(data,size);
    int org_len=buf.length();
    if(off+size<org_len){
		bytes_written = size;
		buf.replace(off, size, data_str.substr(0,size));
        
    }else if(off<=org_len){

		buf.erase(off);
		buf+= data_str;
		bytes_written = size;
	}else{

		buf.resize(off,'\0');
		buf += data_str;
		bytes_written =size;
    }

	ec->put(ino, buf);
    return r;
}

int yfs_client::unlink(inum parent,const char *name)
{
    int r = OK;

    /*
     * your lab2 code goes here.
     * note: you should rmove the file using ec->remove,
     * and update the parent directory content.
     */
	std::list<dirent> dir;
	bool found;
	inum ino;
	r = lookup(parent, name, found, ino);
	if(!found){
        return NOENT;
	}
	ec->remove(ino);
	std::string parentstr = "";
	std::list<dirent>::iterator iter;
	std::string nameStr = std::string(name);
	readdir(parent, dir);
	for(iter = dir.begin(); iter != dir.end(); ++iter){
		if(iter->name == nameStr){
			continue;
		}
		parentstr += filename(iter->inum);
		parentstr += "/";
		parentstr += iter->name;
		parentstr += "/";
	}
	ec->put(parent, parentstr);

    return r;
}

