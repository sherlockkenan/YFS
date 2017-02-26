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


yfs_client::yfs_client(std::string extent_dst, std::string lock_dst)
{
    ec = new extent_client(extent_dst);
	lc = new lock_client(lock_dst);
	lc->acquire(1);
    if (ec->put(1, "") != extent_protocol::OK)
        printf("error init root dir\n"); // XYB: init root dir
	lc->release(1);
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
	lc->acquire(inum);

    if (ec->getattr(inum, a) != extent_protocol::OK) {
		lc->release(inum);
        printf("error getting attr\n");
        return false;
    }
	lc->release(inum);

    if (a.type == extent_protocol::T_FILE) {

        printf("isfile: %lld is a file\n", inum);
        return true;
    } 
    return false;
}
/** Your code here for Lab...
 * You may need to add routines such as
 * readlink, issymlink here to implement symbolic link.
 * 
 * */

int
yfs_client::symlink(const char* link, inum parent, const char* name, inum& ino_out){
	puts("\n\nsymlink\n\n");
	int r = OK;
	bool found;
	std::string parentDir;

	lc->acquire(parent);

	//get parent dir
	ec->get(parent, parentDir);
	found = findNFromP(name, parentDir);
	if(found){
		lc->release(parent);
		r = EXIST;
	}else{
		//add new file to parent dir
		ec->create(extent_protocol::T_SYML, ino_out);
		lc->acquire(ino_out);
		ec->put(ino_out, std::string(link));
		lc->release(ino_out);

		parentDir += filename(ino_out);
		parentDir += "/";
		parentDir += std::string(name);
		parentDir += "/";
		ec->put(parent, parentDir);

		lc->release(parent);

		printf("parent:%s", parentDir.c_str());
		
	}
	return r;
}

int
yfs_client::readlink(inum ino, std::string& link){
	int r = OK;
	printf("\n\n\nreadlink\n\n\n");
	lc->acquire(ino);
	ec->get(ino, link);
	lc->release(ino);
	return r;
}


int yfs_client::getsymlink(inum inum, syminfo &sin){
	int r = OK;
    printf("getsym %016llx\n", inum);
    extent_protocol::attr a;
	lc->acquire(inum);
    if (ec->getattr(inum, a) != extent_protocol::OK) {
        r = IOERR;
        goto release;
    }

    sin.atime = a.atime;
    sin.mtime = a.mtime;
    sin.ctime = a.ctime;
    sin.size = a.size;
    printf("getsim %016llx -> sz %llu\n", inum, sin.size);

release:
	lc->release(inum);
    return r;
}


bool
yfs_client::isdir(inum inum)
{
    // Oops! is this still correct when you implement symlink?
    if(isfile(inum)) return false;
	else{
		extent_protocol::attr a;
		lc->acquire(inum);
		if (ec->getattr(inum, a) != extent_protocol::OK) {
		 	printf("error getting attr\n");
			lc->release(inum);
			return false;
		}
		lc->release(inum);
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
	lc->acquire(inum);
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
	lc->release(inum);
    return r;
}

int
yfs_client::getdir(inum inum, dirinfo &din)
{
    int r = OK;

    printf("getdir %016llx\n", inum);
    extent_protocol::attr a;
	lc->acquire(inum);
    if (ec->getattr(inum, a) != extent_protocol::OK) {
        r = IOERR;
        goto release;
    }
    din.atime = a.atime;
    din.mtime = a.mtime;
    din.ctime = a.ctime;

release:
	lc->release(inum);
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
	//printf("\n\n\nsetattr:\nsize:%d\n", size);
    /*
     * your lab2 code goes here.
     * note: get the content of inode ino, and modify its content
     * according to the size (<, =, or >) content length.
     */
	std::string tmpBuf;
	lc->acquire(ino);
	ec->get(ino, tmpBuf);
	if(tmpBuf.length() > size)
		tmpBuf.erase(size);
	else if(tmpBuf.length() < size)
		tmpBuf.resize(size);
	ec->put(ino, tmpBuf);
	//printf("acturalSize:%d\n\n\n", tmpBuf.size());
	lc->release(ino);
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
	bool found;
	std::string parentDir;

	lc->acquire(parent);

	//get parent dir
	ec->get(parent, parentDir);
	found = findNFromP(name, parentDir);
	if(found){
		lc->release(parent);
		r = EXIST;
	}else{
		//add new file to parent dir
		ec->create(extent_protocol::T_FILE, ino_out);
		parentDir += filename(ino_out);
		parentDir += "/";
		parentDir += std::string(name);
		parentDir += "/";
		ec->put(parent, parentDir);

		lc->release(parent);
		//printf("\n\n\ninum:%d\nparentDirSize:%d\n\n\n", ino_out, parentDir.size());
	}
    return r;
}

int
yfs_client::mkdir(inum parent, const char *name, mode_t mode, inum &ino_out)
{
    int r = OK;

    /*
     * your lab2 code goes here.
     * note: lookup is what you need to check if directory exist;
     * after create file or dir, you must remember to modify the parent information.
     */
	bool found;
	std::string parentDir;

	lc->acquire(parent);

	//get parent dir
	ec->get(parent, parentDir);
	found = findNFromP(name, parentDir);
	if(found){
		lc->release(parent);
		r = EXIST;
	}else{
		//add new file to parent dir
		ec->create(extent_protocol::T_DIR, ino_out);
		parentDir += filename(ino_out);
		parentDir += "/";
		parentDir += std::string(name);
		parentDir += "/";
		ec->put(parent, parentDir);

		lc->release(parent);
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
	std::list<dirent> dirList;
	r = readdir(parent, dirList);
	std::list<dirent>::iterator iter;
	for(iter = dirList.begin(); iter != dirList.end(); ++iter){
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
	std::string dirStr;
	lc->acquire(dir);
	ec->get(dir, dirStr);
	lc->release(dir);
	s2d(dirStr, list);
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
	lc->acquire(ino);
	ec->get(ino, tmpBuf);
	lc->release(ino);
	if(off > tmpBuf.length()) data = "";
	else data = tmpBuf.substr(off, size);
	//printf("\n\n\nread:\nino:%d\noff:%d\nrequest length:%d\nactual length:%d\n%s\n\n\n", ino,off,size,data.length(),data.c_str());
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
	std::string tmpBuf;
	//data too small, fill with \0
//	std::string dataStr = std::string(data);
//	if(size > dataStr.size())
//		dataStr.resize(size, '\0');
	std::string dataStr = std::string(data, size);

	lc->acquire(ino);

	ec->get(ino, tmpBuf);
	int contentLength = tmpBuf.length();
	bytes_written = 0;
	if(off > contentLength){
		tmpBuf.resize(off);
		tmpBuf += dataStr;
		if(tmpBuf.size() > off+size)
			tmpBuf.erase(off+size);
		bytes_written = size + off - contentLength;
	}else if(off+size >= contentLength){
		if(off < tmpBuf.size())
			tmpBuf.erase(off);
		tmpBuf += dataStr;
		if(tmpBuf.size() > off+size)
			tmpBuf.erase(off+size);
		bytes_written = size;
	}else{
		bytes_written = size;
		tmpBuf.replace(off, size, dataStr.substr(0,size));
	}
	ec->put(ino, tmpBuf);

	lc->release(ino);
	//printf("\n\n\nwrite:\nino:%d\noff:%d\nrequest length:%d\nactual length:%dlength after:%d\n%s\n%s\n\n\n", ino,off,size,bytes_written,tmpBuf.length(),data, tmpBuf.c_str());
    return r;
}

int yfs_client::unlink(inum parent,const char *name)
{
    int r = OK;

    /*
     * your lab2 code goes here.
     * note: you should remove the file using ec->remove,
     * and update the parent directory content.
     */
	bool found;
	yfs_client::inum ino;
	r = lookup(parent, name, found, ino);
	if(found){
		std::string parentDir, inoStr;
		lc->acquire(parent);
		ec->get(parent, parentDir);
		
		int pOld = 0, pNew = 0;
		while(true){
			//ino
			pNew = parentDir.find('/', pOld);
			inoStr = parentDir.substr(pOld, pNew-pOld);
			if(n2i(inoStr) == ino){
				//found, remove it from parentDir string
				pNew = parentDir.find('/', pNew+1);
				parentDir.erase(pOld, pNew-pOld+1);
				break;
			}
			pOld = pNew+1;
			//file name
			pNew = parentDir.find('/', pOld);
			pOld = pNew+1;
		}

		ec->put(parent, parentDir);
		lc->release(parent);

		lc->acquire(ino);
		ec->remove(ino);
		lc->release(ino);
	}else
		r = NOENT;
    return r;
}


void yfs_client::s2d(const std::string& dirStr, std::list<dirent>& dir){
	int pOld = 0, pNew = 0;
	inum ino;
	std::string inoStr, filename;
	dirent tmpDirent;
	while(true){
		//ino
		pNew = dirStr.find('/', pOld);
		if(pNew == -1) break;
		inoStr = dirStr.substr(pOld, pNew-pOld);
		ino = n2i(inoStr);
		pOld = pNew+1;
		//file name
		pNew = dirStr.find('/', pOld);
		filename = dirStr.substr(pOld, pNew-pOld);
		pOld = pNew+1;
		//push_back
		tmpDirent.inum = ino;
		tmpDirent.name = filename;
		dir.push_back(tmpDirent);
	}
}

bool yfs_client::findNFromP(const char* name, const std::string& parentDir){
	bool found = false;
	std::list<dirent> dirList;
	s2d(parentDir, dirList);

	std::list<dirent>::iterator iter;
	for(iter = dirList.begin(); iter != dirList.end(); ++iter){
		if(iter->name == std::string(name)){
			found = true;
			break;
		}
	}
	return found;
}
