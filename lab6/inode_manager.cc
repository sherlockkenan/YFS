#include "inode_manager.h"

// disk layer -----------------------------------------


// block layer -----------------------------------------
//
//
//
//lab5 code 

char decode_half(char data)
{
    char res=0x00;
    char mark=data&0xff;
    char p7=mark&0x01;
    char p6=(mark>>1)&0x01;
    char p5=(mark>>2)&0x01;
    char p4=(mark>>3)&0x01;
    char p3=(mark>>4)&0x01;
    char p2=(mark>>5)&0x01;
    char p1=(mark>>6)&0x01;
    char p1r=p7^p5^p3;
    char p2r=p7^p6^p3;
    char p4r=p7^p6^p5;
    if(p1!=p1r&&p2!=p2r&&p4!=p4r)
    {
        res=res|((~p7)&0x01)|(p6<<1)|(p5<<2)|(p3<<3);
        return res;
    }
    if(p2!=p2r&&p4!=p4r)
    {
        res=res|(p7&0x01)|(((~p6)&0x01)<<1)|(p5<<2)|(p3<<3);
        return res;
    }    
    if(p1!=p1r&&p4!=p4r)
   {
        res=res|(p7&0x01)|(p6<<1)|(((~p5)&0x01)<<2)|(p3<<3);
        return res;
    }            
    if(p1!=p1r&&p2!=p2r)
    {
        res=res|(p7&0x01)|(p6<<1)|(p5<<2)|(((~p3)&0x01)<<3);
        return res;
    }
    res=res|(p7&0x01)|(p6<<1)|(p5<<2)|(p3<<3);

    return res;

}


char decode_help(char data1,char data2)
{
    char finl=0x00;
    finl=finl|(decode_half(data2)&0x0f)|(decode_half(data1)<<4);
    return finl;
}


char encode_help(char data)
{
    char res = 0x00;
    char mark = data & 0X0f;
    res=res|(mark&0x07);
    res= res|((mark<<1) & 0x10);
    char p7=mark&0x01;
    char p6=(mark>>1)&0x01;
    char p5=(mark>>2)&0x01;
    char p3=(mark>>3)&0x01;
    char p1=p7^p5^p3;
    char p2=p7^p6^p3;
    char p4=p7^p6^p5;
    res=res|(p1<<6)|(p2<<5)|(p4<<3);
    return res;
}

void encode(const char* in,char*out,int size){
    int i;  
    for(i=0;i<size;i++){
        uint8_t en=(uint8_t)in[i];
        char first=encode_help((en >> 4) & 0X0f);
        char scend=encode_help(en & 0X0f);
        out[2*i]=first;
        out[2*i+1]=scend;
    }
    
}

void decode(char* in, char*out,int size){
   int i;
    for(i=0;i<size;i+=2){
        char res;
        res =decode_help(in[i],in[i+1]);
        out[i/2]=res;
    }
}


#define N 2
void encode_inode(const char* in,char*out,int size){
   int i;
    char p1,p2,p3,p4,p5,p6,p7,p8;
    for( i=0;i<size;i++){
        p1=(in[i]>>7)&0x01;
        p2=(in[i]>>6)&0x01;
        p3=(in[i]>>5)&0x01;
        p4=(in[i]>>4)&0x01;
        p5=(in[i]>>3)&0x01;
        p6=(in[i]>>2)&0x01;
        p7=(in[i]>>1)&0x01;
        p8=(in[i])&0x01;
        out[N*i] = (p1<<7)|(p1<<6)|(p1<<5)|(p1<<4)|(p1<<3)|(p2<<2)|(p2<<1)|p2;
        out[N*i+1]=(p2<<7)|(p2<<6)|(p3<<5)|(p3<<4)|(p3<<3)|(p3<<2)|(p3<<1)|p4;
        out[N*i+2]=(p4<<7)|(p4<<6)|(p4<<5)|(p4<<4)|(p5<<3)|(p5<<2)|(p5<<1)|p5;
        out[N*i+3]=(p5<<7)|(p6<<6)|(p6<<5)|(p6<<4)|(p6<<3)|(p6<<2)|(p7<<1)|p7;
        out[N*i+4]=(p7<<7)|(p7<<6)|(p7<<5)|(p8<<4)|(p8<<3)|(p8<<2)|(p8<<1)|p8;
       /* out[8*i+0]=(p1<<7)|(p1<<6)|(p1<<5)|(p1<<4)|(p1<<3)|(p1<<2)|(p1<<1)|p1;
        out[8*i+1]=(p2<<7)|(p2<<6)|(p2<<5)|(p2<<4)|(p2<<3)|(p2<<2)|(p2<<1)|p2;
        out[8*i+2]=(p3<<7)|(p3<<6)|(p3<<5)|(p3<<4)|(p3<<3)|(p3<<2)|(p3<<1)|p3;
        out[8*i+3]=(p4<<7)|(p4<<6)|(p4<<5)|(p4<<4)|(p4<<3)|(p4<<2)|(p4<<1)|p4;
        out[8*i+4]=(p5<<7)|(p5<<6)|(p5<<5)|(p5<<4)|(p5<<3)|(p5<<2)|(p5<<1)|p5;
        out[8*i+5]=(p6<<7)|(p6<<6)|(p6<<5)|(p6<<4)|(p6<<3)|(p6<<2)|(p6<<1)|p6;
        out[8*i+6]=(p7<<7)|(p7<<6)|(p7<<5)|(p7<<4)|(p7<<3)|(p7<<2)|(p7<<1)|p7;
        out[8*i+7]=(p8<<7)|(p8<<6)|(p8<<5)|(p8<<4)|(p8<<3)|(p8<<2)|(p8<<1)|p8;*/
    }
   

}

void decode_inode(char* in, char*out,int size){
    
    int i;
    for(i=0;i<size;i+=N){
        int j;
        char ture=0x00;
        char tmp[N];
        for(j=0;j<N*8;j+=N){
            int num0=0,num1=0;
            int k;
            for(k=0;k<N;k++){
                tmp[k]=(in[i+(j+k)/8]>>(7-(j+k)%8))&0x01;
                if(tmp[k]==0x00){
                    num0++;
                }else{
                    num1++;
                }
            }
                if(num0>num1){
                    ture|=(0x00<<(7-j/N));
                }else{
                    ture|=(0x01<<(7-j/N));
                }

            
        }
        out[i/N]=ture;
    };

}





// Allocate a free disk block.
blockid_t
block_manager::alloc_block()
{
  /*;
   * your lab1 code goes here.;
   * note: you should mark the corresponding bit in block bitmap when alloc.
   * you need to think about which block you can start to be allocated.

   *hint: use macro IBLOCK and BBLOCK.
          use bit operation.
          remind yourself of the layout of disk.
   */
    char buf[BLOCK_SIZE];
    blockid_t b=IBLOCK(INODE_NUM,sb.nblocks)+1;
    for(;b<sb.nblocks;b++){
        read_block(BBLOCK(b),buf);
        int num=b%BPB/8;
        char ch=buf[num];
        if(((int)ch & (1 << (7-b%8))) == 0){
			ch |= (1 << (7 - b % 8));
			buf[num]= ch;
			write_block(BBLOCK(b), buf);
			break;
		}
    }

  return b;
}

void
block_manager::free_block(uint32_t id)
{
  /* 
   * your lab1 code goes here.
   * note: you should unmark the corresponding bit in the block bitmap when free.
   */
    char buf[BLOCK_SIZE];
    read_block(BBLOCK(id),buf);
    int num=id%BPB/8;
    char ch=buf[num];
    if(((int)ch & (1 << (7-id%8))) == 0){
		ch ^= (1 << (7 - id%8));
        buf[num]= ch;
		write_block(BBLOCK(id), buf);
    }
}

// The layout of disk should be like this:
// |<-sb->|<-free block bitmap->|<-inode table->|<-data->|
block_manager::block_manager()
{
  d = new disk();

  // format the disk
  sb.size = BLOCK_SIZE * BLOCK_NUM;
  sb.nblocks = BLOCK_NUM;
  sb.ninodes = INODE_NUM;

}

void
block_manager::read_block(uint32_t id, char *buf)
{
 // char tmpbuf1[BLOCK_SIZE];
//    char tmpbuf2[BLOCK_SIZE];
   //d->read_block(id, tmpbuf1);
  // d->read_block(2*id+1,tmpbuf2);
   //decode(tmpbuf1,buf,BLOCK_SIZE);
   //decode(tmpbuf2,buf+BLOCK_SIZE/2,BLOCK_SIZE);
    d->read_block(id, buf);
}

void
block_manager::write_block(uint32_t id, const char *buf)
{
  
  //char tmpbuf1[BLOCK_SIZE];
    //char tmpbuf2[BLOCK_SIZE];
    //encode(buf,tmpbuf1,BLOCK_SIZE/2);
    //encode(buf+BLOCK_SIZE/2,tmpbuf2,BLOCK_SIZE/2);
  //d->write_block(id, tmpbuf1);
  // d->write_block(2*id+1, tmpbuf2);

    d->write_block(id, buf);
}

// inode layer -----------------------------------------

inode_manager::inode_manager()
{
  bm = new block_manager();
  uint32_t root_dir = alloc_inode(extent_protocol::T_DIR);
  if (root_dir != 1) {
    printf("\tim: error! alloc first inode %d, should be 1\n", root_dir);
    exit(0);
  }
}

/* Create a new file.
 * Return its inum. */
uint32_t
inode_manager::alloc_inode(uint32_t type)
{
  /* 
   * your lab1 code goes here.
   * note: the normal inode block should begin from the 2nd inode block.
   * the 1st is used for root_dir, see inode_manager::inode_manager().
    
   * if you get some heap memory, do not forget to free it.
   */
    inode_t *node= (struct inode*)malloc(sizeof(struct inode));
    node->type=type;
    node->size=0;
    time_t currenttime;
    node->ctime = time(&currenttime);
    node->mtime = time(&currenttime);
    node->atime = time(&currenttime);
    //node.atime=
    //node.mtime=
    //node.ctime=
    //{{
    int inum;
    char buf[BLOCK_SIZE];
    for(inum=1;inum<INODE_NUM;inum++){
        bm->read_block(BBLOCK(IBLOCK(inum,bm->sb.nblocks)),buf);
        int num=IBLOCK(inum,bm->sb.nblocks)%BPB/8;
        char ch=buf[num];
        if(((int)ch & (1 << (7-IBLOCK(inum, bm->sb.nblocks)%8))) == 0){
			ch |= (1 << (7 - IBLOCK(inum, bm->sb.nblocks) % 8));
			buf[num]= ch;
			bm->write_block(BBLOCK(IBLOCK(inum, bm->sb.nblocks)), buf);
			break;
		}
    }
    put_inode(inum,node);

    free(node);
    return inum;
}

void
inode_manager::free_inode(uint32_t inum)
{
  /* 
   * your lab1 code goes here.
   * note: you need to check if the inode is already a freed one;
   * if not, clear it, and remember to write back to disk.
   * do not forget to free memory if necessary.
   */
   inode_t* node=get_inode(inum);
   if(node==NULL) return;
    else{
        node->type=0;
        node->size = node->atime = node->ctime  = node->mtime=0;
        put_inode(inum,node); 
        free(node);
    }
}


/* Return an inode structure by inum, NULL otherwise.
 * Caller should release the memory. */
struct inode* 
inode_manager::get_inode(uint32_t inum)
{
  struct inode *ino, *ino_disk;
  char buf[BLOCK_SIZE];
  printf("\tim: get_inode %d\n", inum);
  if (inum < 0 || inum >= INODE_NUM) {
    printf("\tim: inum out of range\n");
    return NULL;
  }
  bm->read_block(IBLOCK(inum, bm->sb.nblocks), buf);
  // printf("%s:%d\n", __FILE__, __LINE__);
  ino_disk = (struct inode*)buf + inum%IPB;
  if (ino_disk->type == 0) {
    printf("\tim: inode not exist\n");
    return NULL;
  }
  ino = (struct inode*)malloc(sizeof(struct inode));
  *ino = *ino_disk;
  return ino;
}
void
inode_manager::put_inode(uint32_t inum, struct inode *ino)
{
  char buf[BLOCK_SIZE];
  struct inode *ino_disk;
  printf("\tim: put_inode %d\n", inum);
  if (ino == NULL)
    return;
  bm->read_block(IBLOCK(inum, bm->sb.nblocks), buf);
  ino_disk = (struct inode*)buf + inum%IPB;
  *ino_disk = *ino;
  bm->write_block(IBLOCK(inum, bm->sb.nblocks), buf);
}

#define MIN(a,b) ((a)<(b) ? (a) : (b))

/* Get all the data of a file by inum. 
 * Return alloced data, should be freed by caller. */
void
inode_manager::read_file(uint32_t inum, char **buf, int *size)
{
  /*
   * your lab1 code goes here.
   * note: read blocks related to inode number inum,
   * and copy them to buf_out
   */


	struct inode *ino = get_inode(inum);
	ino->atime = time(0);
	put_inode(inum, ino);
	*size = ino->size;

    //lab5,inode中的size是实际文件的size的大小，但储存的size比其大
    ino->size=ino->size*N;

	unsigned int nBlock = 0;
	if(ino->size != 0) nBlock = (ino->size-1)/BLOCK_SIZE + 1;

    //lab5
//	*buf_out = (char*)malloc(nBlock * BLOCK_SIZE * sizeof(char*));
	*buf = (char*)malloc(nBlock * BLOCK_SIZE * sizeof(char*));
	char *buf_out = (char*)malloc(nBlock * BLOCK_SIZE * sizeof(char));

	if(nBlock <= NDIRECT){
		for(unsigned int i= 0; i< nBlock; ++i){
			bm->read_block(ino->blocks[i], buf_out + i*BLOCK_SIZE);
		}
	}else{
		for(unsigned int i= 0; i< NDIRECT; ++i){
			bm->read_block(ino->blocks[i], buf_out + i*BLOCK_SIZE);
		}
		blockid_t tmpBlock[NINDIRECT];
		bm->read_block(ino->blocks[NDIRECT], (char*)&tmpBlock);
		for(unsigned int i= NDIRECT; i< nBlock; ++i){
			bm->read_block(tmpBlock[i-NDIRECT], buf_out +i*BLOCK_SIZE);
		}
	}
   
    

    //lab5
   decode(buf_out,*buf,*size*N);
   //*size=*size/2;
   ino->size=ino->size/N;
    
    printf("rrrrrrrrrrrrrrrrrrrrrrrrrrrr read  size:%d inum:%d  pre:%s  decode:%s\n",*size,inum, buf_out,*buf);
	free(ino);
}

/* alloc/free blocks if needed */
void
inode_manager::write_file(uint32_t inum, const char *buf1, int size)
{
  /*
   * your lab1 code goes here.
   * note: write buf to blocks of inode inum.
   * you need to consider the situation when the size of buf 
   * is larger or smaller than the size of original inode.
   * you should free some blocks if necessary.
   */
    //lab5
    if(size==20000){
        printf("10000000000000000000000000000000\n");
    }
    char*buf=(char*)malloc(N*size*sizeof(char*));
    encode(buf1,buf,size);
    printf("rrrrrrrrrrrrrrrrrrrrrrrrrrrr write size:%d inum:%d  pre:%s  encode:%s\n", size,inum, buf1,buf);
    size=size*N;
    
    
	struct inode *ino = get_inode(inum);

    //lab5
    ino->size=ino->size*N;

	ino->atime = time(0);
	ino->mtime = time(0);
	ino->ctime = time(0);
	unsigned int nOldBlock = 0;
	unsigned int nNewBlock = 0;
	if(ino->size != 0) nOldBlock = (ino->size - 1)/BLOCK_SIZE + 1;
	if(size != 0) nNewBlock = (size - 1)/BLOCK_SIZE + 1;
    
    //lab5
	ino->size = size/N;
   
    if(nNewBlock > MAXFILE) {
        return;
    }
	//alloc new memery if necessary
	if(nNewBlock > nOldBlock){
		if(nNewBlock <= NDIRECT){
			for(unsigned int i= nOldBlock; i< nNewBlock; ++i){
				ino->blocks[i] = bm->alloc_block();
			}
		}else if(nOldBlock > NDIRECT){
			blockid_t tmpBlock[NINDIRECT];
			bm->read_block(ino->blocks[NDIRECT], (char*)tmpBlock);
			for(unsigned int i= nOldBlock; i< nNewBlock; ++i){
				tmpBlock[i-NDIRECT] = bm->alloc_block();
			}
			bm->write_block(ino->blocks[NDIRECT], (const char*)tmpBlock);
		}else{
			for(unsigned int i= nOldBlock; i< NDIRECT; ++i){
				ino->blocks[i] = bm->alloc_block();
			}
			ino->blocks[NDIRECT] = bm->alloc_block();
			blockid_t tmpBlock[NINDIRECT];
			for(unsigned int i= NDIRECT; i< nNewBlock; ++i){
				tmpBlock[i-NDIRECT] = bm->alloc_block();
			}
			bm->write_block(ino->blocks[NDIRECT], (const char*)tmpBlock);
		}
	}
	//copy
	if(nNewBlock <= NDIRECT){
		for(unsigned int i= 0; i< nNewBlock; ++i){
			bm->write_block(ino->blocks[i], buf+BLOCK_SIZE*i);
		}
	}else{
		for(unsigned int i= 0; i< NDIRECT; ++i){
			bm->write_block(ino->blocks[i], buf+BLOCK_SIZE*i);
		}
		blockid_t tmpBlock[NINDIRECT];
		bm->read_block(ino->blocks[NDIRECT], (char*)tmpBlock);
		for(unsigned int i= NDIRECT; i< nNewBlock; ++i){
			bm->write_block(tmpBlock[i-NDIRECT], buf+BLOCK_SIZE*i);
		}
	}
	//free if necessary
	if(nNewBlock < nOldBlock){
		if(nOldBlock <= NDIRECT){
			for(unsigned int i= nNewBlock; i< nOldBlock; ++i){
				bm->free_block(ino->blocks[i]);
			}
		}else if(nNewBlock > NDIRECT){
			blockid_t tmpBlock[NINDIRECT];
			bm->read_block(ino->blocks[NDIRECT], (char*)tmpBlock);
			for(unsigned int i= nNewBlock; i< nOldBlock; ++i){
				bm->free_block(tmpBlock[i-NDIRECT]);
			}
			bm->write_block(ino->blocks[NDIRECT], (const char*)tmpBlock);
		}else{
			for(unsigned int i= nNewBlock; i< NDIRECT; ++i){
				bm->free_block(ino->blocks[i]);
			}
			blockid_t tmpBlock[NINDIRECT];
			bm->read_block(ino->blocks[NDIRECT], (char*)tmpBlock);
			for(unsigned int i= NDIRECT; i< nOldBlock; ++i){
				bm->free_block(tmpBlock[i-NDIRECT]);
			}
			bm->free_block(ino->blocks[NDIRECT]);
		}
	}
	put_inode(inum, ino);
	free(ino);
}

void
inode_manager::getattr(uint32_t inum, extent_protocol::attr &a)
{
  /*
   * your lab1 code goes here.
   * note: get the attributes of inode inum.
   * you can refer to "struct attr" in extent_protocol.h
   */
	struct inode *ino = get_inode(inum);
	if(ino == NULL) return;
	a.type  = ino->type;
	a.atime = ino->atime;
	a.mtime = ino->mtime;
	a.ctime = ino->ctime;
	a.size  = ino->size;
	
	free(ino);
}

void
inode_manager::remove_file(uint32_t inum)
{
  /*
   * your lab1 code goes here
   * note: you need to consider about both the data block and inode of the file
   * do not forget to free memory if necessary.
   */
   struct inode *ino = get_inode(inum);
	unsigned int nBlock = 0;
	if(ino->size != 0) nBlock = (ino->size-1)/BLOCK_SIZE+1;
	if(nBlock <= NDIRECT){
		for(unsigned int i= 0; i< nBlock; ++i){
			bm->free_block(ino->blocks[i]);
		}
	}else{
		for(unsigned int i= 0; i< NDIRECT; ++i){
			bm->free_block(ino->blocks[i]);
		}
		blockid_t tmpBlock[NINDIRECT];
		bm->read_block(ino->blocks[NDIRECT], (char*)&tmpBlock);
		for(unsigned int i= NDIRECT; i< nBlock; ++i){
			bm->free_block(tmpBlock[i-NDIRECT]);
		}
		bm->free_block(ino->blocks[NDIRECT]);
	}
	free_inode(inum);
	free(ino);
}



