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


#include <openssl/bio.h>
#include <openssl/err.h>
#include <openssl/pem.h>
#include <openssl/x509.h>

yfs_client::yfs_client()
{
  ec = NULL;
  lc = NULL;
}


std::vector<Log> logfile;

yfs_client::yfs_client(std::string extent_dst, std::string lock_dst, const char* cert_file)
{
    //lab6
    pthread_mutex_init(&log_mutex, NULL);
    yfs_version=0;

    ec = new extent_client(extent_dst);
	lc = new lock_client(lock_dst);
	/* filestat st;
    st.mode=0777;
    st.uid=uid;
    st.gid=gid;*/
    unsigned short my_uid;
    verify(cert_file,&my_uid);
    uid=my_uid;
    gid=uid;

    if (ec->put(1, "") != extent_protocol::OK)
        printf("error init root dir\n"); // XYB: init root dir
	//lc->release(1);
    ec->setattr(1,0777,uid,gid);
    Log log;
    log.op=ROOT;
    write_log(log);

    

}

int
yfs_client::verify(const char* name, unsigned short *uid)
{


    X509* cert = X509_new();
    BIO* bio_cert;
    if((bio_cert=BIO_new_file(name, "r")) == NULL){
        return ERRPEM;
    }
    PEM_read_bio_X509(bio_cert, &cert, NULL, NULL);

   
    char*subject_line = X509_NAME_oneline(X509_get_subject_name(cert), 0, 0);
    char*issuer_line = X509_NAME_oneline(X509_get_issuer_name(cert), 0, 0);
    ASN1_TIME* time=X509_get_notAfter(cert);
    
    char subject_cn[100],issuer_cn[20];

    sscanf(subject_line,"/%*[^/]/%*[^/]/%*[^/]/%*[^/]/%*[^/]/%*[^=]=%[^/]",subject_cn);

    sscanf(issuer_line,"/%*[^/]/%*[^/]/%*[^/]/%*[^/]/%*[^/]/%*[^=]=%[^/]",issuer_cn);
    
  	int ret = OK;
    if(strcmp(subject_line,"")==0){

        return ERRPEM;
    }
    
    if(*time->data=='1'){
        return ECTIM;
    }
  
    if(strcmp(issuer_cn,"LAB6")!=0){
        return EINVA;
    }

   if(strcmp(subject_cn,"root")==0){
        // printf("iam here\n");
        *uid=0;
    }
    if(strcmp(subject_cn,"user1")==0){
        *uid=1003;
    }
    if(strcmp(subject_cn,"user2")==0){
        *uid=1004;
    }
   if(strcmp(subject_cn,"user3")==0){
        *uid=1005;
    }
	return ret;
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
	////lc->acquire(inum);

    if (ec->getattr(inum, a) != extent_protocol::OK) {
		//lc->release(inum);
        printf("error getting attr\n");
        return false;
    }
	////lc->release(inum);

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
yfs_client::symlink(const char* symlin, inum parent, const char* name, inum& ino_out){

	puts("\n\nsymlink\n\n");
	int r = OK;
    //lc->acquire(parent);

    bool found=true;
  /*  lookup(parent,name,found,ino_out);
        if(found){
            r=EXIST;
            //lc->release(parent);
            return r;
        }*/

    ec->create(extent_protocol::T_SYML,0,uid,gid,ino_out);
    std::string file_name;

    if(ec->get(parent,file_name)!=extent_protocol::OK){
                printf("get error \n");
        r=IOERR;
            //lc->release(parent);
        return r;
    }
    file_name = file_name + filename(ino_out)+"/"+std::string(name)+"/";
    if (ec->put(parent, file_name) != extent_protocol::OK) {
        printf("put error \n");
            //lc->release(parent);
        return IOERR;
                        
    }

    Log log;
    log.op=SYMLINK;
    log.ino=ino_out;
    log.parent=parent;
    log.dir=file_name;
    log.content=std::string(symlin);
    write_log(log);

    ec->put(ino_out,std::string(symlin));
     //lc->release(parent);
	return r;
}

int
yfs_client::readlink(inum ino, std::string& link){
	int r = OK;
	printf("\n\n\nreadlink\n\n\n");
	////lc->acquire(ino);
    if(ec->get(ino,link)!=extent_protocol::OK){
	  //  //lc->release(ino);
        return IOERR;
    }
	return r;
}


int yfs_client::getsymlink(inum ino, syminfo &syminf){
	int r = OK;
    printf("getsym %016llx\n", ino);
    extent_protocol::attr a;
    //lc->acquire(ino);
    if (ec->getattr(ino, a) != extent_protocol::OK) {
        r = IOERR;
	//lc->release(ino);
        return r;
    }

    syminf.atime = a.atime;
    syminf.mtime = a.mtime;
    syminf.ctime = a.ctime;
    syminf.size = a.size;
    syminf.uid=a.uid;
    syminf.gid=a.gid;
	//lc->release(ino);
    return r;
}


bool
yfs_client::isdir(inum inum)
{
    // Oops! is this still correct when you implement symlink?
    if(isfile(inum)) return false;
	else{
		extent_protocol::attr a;
		////lc->acquire(inum);
		if (ec->getattr(inum, a) != extent_protocol::OK) {
		 	printf("error getting attr\n");
			//lc->release(inum);
			return false;
		}
		////lc->release(inum);
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
	////lc->acquire(inum);
    if (ec->getattr(inum, a) != extent_protocol::OK) {
        r = IOERR;
        goto release;
    }

    fin.atime = a.atime;
    fin.mtime = a.mtime;
    fin.ctime = a.ctime;
    fin.size = a.size;
    fin.mode=a.mode;
    fin.uid=a.uid;
    fin.gid=a.gid;
    printf("getfile %016llx -> sz %llu\n", inum, fin.size);

release:
	////lc->release(inum);
    return r;
}

int
yfs_client::getdir(inum inum, dirinfo &din)
{
    int r = OK;

    printf("getdir %016llx\n", inum);
    extent_protocol::attr a;
	////lc->acquire(inum);
    if (ec->getattr(inum, a) != extent_protocol::OK) {
        r = IOERR;
        goto release;
    }
    din.atime = a.atime;
    din.mtime = a.mtime;
    din.ctime = a.ctime;
    din.mode=a.mode;
    din.uid=a.uid;
    din.gid=a.gid;

release:
	////lc->release(inum);
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
yfs_client::setattr(inum ino, filestat st, unsigned long size)
{
    int r = OK;
	//printf("\n\n\nsetattr:\nsize:%d\n", size);
    /*
     * your lab2 code goes here.
     * note: get the content of inode ino, and modify its content
     * according to the size (<, =, or >) content length.
     */
	//lc->acquire(ino);
    std::string buf;
    extent_protocol::attr attribute;

    if((ec->getattr(ino,attribute))!=extent_protocol::OK)
    {
        printf("error to get attr in setattr %d\n",attribute.size);
	   //lc->release(ino);
       return IOERR;
    }

    ec->get(ino,buf);

    if(size<=attribute.size){
        buf =buf.substr(0,size);
    }else{
        buf += std::string(size - attribute.size, '\0');
    
    }
    ec->put(ino,buf);
    //lc->release(ino);
    attribute.size=size;
    //fileinfo fin;
    //getfile(ino,fin);
    printf("hhhhhhhhhhh,userid:%d,fileownerid :%d,,mode%o,change mode %o,change userid:%d\n",uid,attribute.uid,attribute.mode,st.mode,st.uid);
    if( attribute.mode!=st.mode){
        if(uid!=0){
         if(uid!=attribute.uid){
            return NOPEM;
         }
        attribute.mode=st.mode;
        int i=0;
        ec->setattr(ino,attribute.mode,attribute.uid,attribute.gid);
         ec->getattr(ino,attribute);
        printf("jjjjjjjjjjjjjjjjjjjjjjj%o\n",attribute.mode);
      }
    }
    
    if(st.uid==1003||st.uid==1004||st.uid==1005){
        if(attribute.uid!=st.uid||attribute.gid!=st.gid){
            printf("i am not here\n");
            if(uid!=0){
                return NOPEM;
            }
            attribute.uid=st.uid;
            attribute.gid=st.gid;
            printf("jjjjjjjjjjjjjjjjjjjjjjjj%d\n",attribute.uid);
            ec->setattr(ino,attribute.mode,attribute.uid,attribute.gid);
            ec->getattr(ino,attribute);
             printf("jjjjjjjjjjjjjjjjjjjjjjj%d\n",attribute.uid);

        }
    } 
   
    
  
    //ec->getattr(ino,att);
    //printf("%o\n",att.mode );
    
    return r;
}
int
yfs_client::create(inum parent, const char *name, mode_t mode, inum &ino_out)
{
    int r = OK;
    /*

     * your lab2 code goes here.
     * note: lookup is what you need to check if file exist;v
     * after create file or dir, you must remember to modify the parent infomation.
     */

  
    /*dirinfo din;
    getdir(parent,din);
    unsigned long mod=din.mode;
   printf("kkkkkkkkkkkkkkkkkkmode:%o,uid:%d,diruid:%d\n",din.mode,uid,din.uid);

    if(din.uid==0){

    }else if(din.uid==uid){
     
        if((mod>>7&0x1)==0){
            printf("here no write permission\n");
        
            return NOPEM;
        }
  

    }else if(din.gid+1==gid||din.gid==gid){
            if((mod>>4&0x1)==0){
            return NOPEM;
            }
    }else {
         if((mod>>1&0x1)==0){
            return NOPEM;
        }
    }*/

    
	bool found;
	std::string file_name;

	//lc->acquire(parent);

    if(ec->get(parent,file_name)!=extent_protocol::OK){
                printf("get error \n");
        r=IOERR;
		//lc->release(parent);
        return r;
    }
	//found = findNFromP(name, file_name);
   /* lookup(parent,name,found,ino_out);
    if(found){
        r=EXIST;
		//lc->release(parent);
        return r;
    }*/
    ec->create(extent_protocol::T_FILE,mode,uid,gid,ino_out);

    file_name = file_name + filename(ino_out)+"/"+std::string(name)+"/";
    if (ec->put(parent, file_name) != extent_protocol::OK) {
        printf("put error \n");
		//lc->release(parent);
        return IOERR;
                        
    }

    Log log;
    log.op=CREATE;
    log.ino=ino_out;
    log.parent=parent;
    log.dir=file_name;
    
    write_log(log);

	//lc->release(parent);
    return r;
}

int
yfs_client::mkdir(inum parent, const char *name, mode_t mode, inum &ino_out)
{
    int r = OK;


    printf("i am here\n");
    dirinfo din;
    getdir(parent,din);
    unsigned long mod=din.mode;  

    if(din.uid==0){

    }else if(din.uid==uid){
      
        if((mod>>7&0x1)==0){
          
            return NOPEM;
        }      

    }else if(din.gid+1==gid||din.gid==gid){
            if((mod>>4&0x1)==0){
            return NOPEM;
            }
    }else {
         if((mod>>1&0x1)==0){
            return NOPEM;
        }
    }
    /*
     * your lab2 code goes here.
     * note: lookup is what you need to check if directory exist;
     * after create file or dir, you must remember to modify the parent information.
     */
	bool found;
	std::string file_name;

	//lc->acquire(parent);

    if(ec->get(parent,file_name)!=extent_protocol::OK){
                printf("get error \n");
        r=IOERR;
		//lc->release(parent);
        return r;
    }
   /* lookup(parent,name,found,ino_out);
	//found = findNFromP(name, file_name);
    if(found){
        r=EXIST;
		//lc->release(parent);
        return r;
    }*/
   // printf(".....................................%d\n", mode);
    ec->create(extent_protocol::T_DIR,mode,uid,gid,ino_out);
    //printf(".....................................%d\n", mode);

    file_name = file_name + filename(ino_out)+"/"+std::string(name)+"/";
    if (ec->put(parent, file_name) != extent_protocol::OK) {
        printf("put error \n");
		//lc->release(parent);
        return IOERR;
                        
    }

    Log log;
    log.op=MKDIR;
    log.ino=ino_out;
    log.parent=parent;
    log.dir=file_name;
    write_log(log);

	//lc->release(parent);

    return r;
}
int
yfs_client::readdir_help(inum dir, std::list<dirent> &list)
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
    ////lc->acquire(dir);
    ec->get(dir, dirstr);
    ////lc->release(dir);
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
	r = readdir_help(parent, dirList);
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

    dirinfo din;
    getdir(dir,din);  
     printf("i am in read dir,mod: %o,uid:%d\n",din.mode,din.uid);
    unsigned long mod=din.mode;
    if(din.uid==0){

    }else if(din.uid==uid){
        if((mod>>8&0x1)==0){
            return NOPEM;
        }

    }else if(din.gid+1==gid){
            if((mod>>5&0x1)==0){
            return NOPEM;
            }
    }else {
         if((mod>>2&0x1)==0){
            return NOPEM;
        }
    }

	std::string dirstr, filename, inostr;
	int ino;
	 dirent diren;
	////lc->acquire(dir);
	ec->get(dir, dirstr);
	////lc->release(dir);
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
    fileinfo fin;
    getfile(ino,fin);
    unsigned long mod=fin.mode;
    if(fin.uid==0){

    }else if(fin.uid==uid){
        if((mod>>8&0x1)==0){
            return NOPEM;
        }

    }else if(fin.gid+1==gid){
            if((mod>>5&0x1)==0){
            return NOPEM;
            }
    }else {
         if((mod>>2&0x1)==0){
            return NOPEM;
        }
    }

    
    
	std::string Buf;
	//lc->acquire(ino);
	ec->get(ino, Buf);
	//lc->release(ino);
	if(off > Buf.length()) data = "";
	else data = Buf.substr(off, size);
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
  ///  printf("tttttttttttttttttttttttttt%s %d %d",data,size,off);
    if(isfile(ino)){
    fileinfo fin;
    getfile(ino,fin);
    unsigned long mod=fin.mode;
    printf("%o\n", mod);
    printf("file user id :%d,user id,%d\n", fin.uid,uid);

    if(fin.uid==0){

    }else if(fin.uid==uid){
        printf("i am here\n");
        if((mod>>7&0x1)==0){
            printf("hhhhhhhhh.........ture\n");
            //printf("%d\n",EACCES );
            return NOPEM;
        }
         printf("hhhhhhhhh.........error\n");

    }else if(fin.gid+1==gid||fin.gid==gid){
            if((mod>>4&0x1)==0){
            return NOPEM;
            }
    }else {
         if((mod>>1&0x1)==0){
            return NOPEM;
        }
    }
   }



   
	//lc->acquire(ino);
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

    Log log;
    log.op=WRITE;
    log.ino=ino;
    log.content=buf;
    write_log(log);

   // printf("kkkkkkkkkkkkkkkkk%s,%d",buf.c_str(),org_len);
	ec->put(ino, buf);

	//lc->release(ino);
    return r;
}

void yfs_client::To_version(std::vector<Log> logs){
    //recover to version due to logs
  /*  pthread_mutex_lock(&log_mutex);
    ec->cleanAllNodes(1);   
    int logCnt = logs.size();
    int current_version=0;
  
  
    for(int i= 0; i< logCnt; ++i){
        if(current_version>yfs_version){
            break;
        }

        switch(logs[i].op){
        case ROOT:
            ec->set_ino(1, extent_protocol::T_DIR, std::string(""));
            break;
        case MKDIR:
            ec->set_ino(logs[i].ino, extent_protocol::T_DIR, std::string(""));
            ec->set_ino(logs[i].parent, extent_protocol::T_DIR, logs[i].dir);
            break;
        case CREATE:
            ec->set_ino(logs[i].ino, extent_protocol::T_FILE, std::string(""));
            ec->set_ino(logs[i].parent, extent_protocol::T_DIR, logs[i].dir);
            break;
        case WRITE:
            printf("write function, the content%s\n",logs[i].content.c_str());
            ec->put(logs[i].ino, logs[i].content);
            break;
        case SYMLINK:
            ec->set_ino(logs[i].ino, extent_protocol::T_SYML, logs[i].content);
            ec->set_ino(logs[i].parent, extent_protocol::T_DIR, logs[i].dir);
            break;
        case UNLINK:
            ec->set_ino(logs[i].ino, 0, std::string(""));
            ec->set_ino(logs[i].parent, extent_protocol::T_DIR, logs[i].dir);
            break;
        case CHECKPOINT:
            current_version++;
            break;
        default:
            break;
        }
    }
    pthread_mutex_unlock(&log_mutex);*/
}



int yfs_client::unlink(inum parent,const char *name)
{
    int r = OK;

    /*
     * your lab2 code goes here.
     * note: you should remove the file using ec->remove,
     * and update the parent directory content.
     */
	std::list<dirent> dir;
	bool found;
	inum ino;
	/*r = lookup(parent, name, found, ino);
	if(!found){
        return NOENT;
	}*/
    //lc->acquire(parent);
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


    Log log;
    log.op=UNLINK;
    log.ino=ino;
    log.parent=parent;
    log.dir=parentstr;
    write_log(log);


	ec->put(parent, parentstr);
    //lc->release(parent);
    //lc->acquire(ino);
	ec->remove(ino);
    //lc->release(ino);
    return r;
}

void yfs_client::write_log(Log log){
    pthread_mutex_lock(&log_mutex);
    logfile.push_back(log);
    pthread_mutex_unlock(&log_mutex);
}

void yfs_client::commit(){
    Log log;
    log.op=CHECKPOINT;
    write_log(log);
    yfs_version++;
    std::cout<<"myversion "<<yfs_version<<std::endl;

}
void yfs_client::rollback(){
    
    yfs_version--;
    std::cout<<"myversion "<<yfs_version+1<<" to "<<yfs_version<<std::endl;
    printf("kkkkkkkkkkkkkkkkkkkkkkk\n");
    
    printf("the logfile size%d\n",logfile.size());
    To_version(logfile);

}
void yfs_client::next_version(){

    yfs_version++;
    To_version(logfile);

}





