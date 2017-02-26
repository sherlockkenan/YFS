#include "inode_manager.h"
// disk layer -----------------------------------------
disk::disk()
{
  bzero(blocks, sizeof(blocks));
}
void disk::read_block(blockid_t id, char *buf)
{
  /*
   *your lab1 code goes here.
   *if id is smaller than 0 or larger than BLOCK_NUM 
   *or buf is null, just return.
   *put the content of target block into buf.
   *hint: use memcpy
  */
    if(id<0||id>BLOCK_NUM||buf==NULL)
       return;
    memcpy(buf,blocks[id],BLOCK_SIZE);
}
void disk::write_block(blockid_t id, const char *buf)
{
  /*
   *your lab1 code goes here.
   *hint: just like read_block
  */

    if(buf==NULL)
       bzero(blocks[id],BLOCK_SIZE);
    if(id<0||id>BLOCK_NUM||buf==NULL)
       return;
   // if(strlen(buf)>sizeof(blocks[id])){
   //     
   // }
    memcpy(blocks[id],buf,BLOCK_SIZE);
}
// block layer -----------------------------------------
// Allocate a free disk block.
blockid_t
block_manager::alloc_block()
{
  /*
   * your lab1 code goes here.
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

   // d->write_block(id,0);
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
  d->read_block(id, buf);
}
void
block_manager::write_block(uint32_t id, const char *buf)
{
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
    //node.atime=
    //node.mtime=
    //node.ctime=
    //
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
   * note: you need to check if the inode is already a freed one;mZnot, clear it, and remember to write back to disk.
   * do not forget to free memory if necessary.
   */
   inode_t* node=get_inode(inum);
   if(node==NULL) return;
    else{
        node->type=0;
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
inode_manager::read_file(uint32_t inum, char **buf_out, int *size)
{
  /*
   * your lab1 code goes here.
   * note: read blocks related to inode number inum,
   * and copy them to buf_out
   */
    if(inum<0||buf_out==NULL)
        return;
    inode_t* node=get_inode(inum);
    *size=node->size;

    //node->atime=
    //put_inode(inum,node);
    int num=(node->size+BLOCK_SIZE-1)/BLOCK_SIZE;
    
  //  printf("debug1\n");
   // printf("%d\n",*size); 
    char buf[BLOCK_SIZE];
     char* buf_tmp = (char*)malloc(*size);
   
    int i;
    for(i=0;i<MIN(num,NDIRECT)-1;i++){
        bm->read_block(node->blocks[i],buf);
        memcpy(buf_tmp+i*BLOCK_SIZE,buf,BLOCK_SIZE);
    } 
    if(i==(num-1)){ 
        bm->read_block(node->blocks[i],buf);
        memcpy(buf_tmp+i*BLOCK_SIZE,buf,*size-i*BLOCK_SIZE);
    }else if(i==(NDIRECT-1)){ 
        bm->read_block(node->blocks[i],buf);
        memcpy(buf_tmp+i*BLOCK_SIZE,buf,BLOCK_SIZE);
    }

   // printf("debug2\n");
    if(num>NDIRECT)
    {
        char indirect[BLOCK_SIZE];
        bm->read_block(node->blocks[NDIRECT],indirect);
        uint32_t* dir=(uint32_t*)indirect;
        int j;
        for(j=0;j<num-NDIRECT-1;j++){
            bm->read_block(dir[j],buf);
            memcpy(buf_tmp+(NDIRECT+j)*BLOCK_SIZE,buf,BLOCK_SIZE);
        }
        if(j==(num-NDIRECT-1)){
            bm->read_block(dir[j],buf);
            memcpy(buf_tmp+(NDIRECT+j)*BLOCK_SIZE,buf,*size-BLOCK_SIZE*(NDIRECT+j));
        }
    }

    *buf_out=buf_tmp;
   // printf("%s\n",*buf_out);
   // printf("debug3\n");
    return;
}
/* alloc/free blocks if needed */
void
inode_manager::write_file(uint32_t inum, const char *buf, int size)
{
  /*
   * your lab1 code goes here.
   * note: write buf to blocks of inode inum.
   * you need to consider the situation when the size of buf 
   * is larger or smaller than the size of original inode.
   * you should free some blocks if necessary.
   */
   // printf("debug4\n");
   // printf("%d\n",size);
    if(inum<0||buf==NULL)
        return ;
    inode_t* node=get_inode(inum);
    uint32_t orgnum=(node->size+BLOCK_SIZE-1)/BLOCK_SIZE;
    
    uint32_t curnum=(size+BLOCK_SIZE-1)/BLOCK_SIZE;
//printf("%d %d\n",orgnum,curnum);
    //char buf[BLOCK_SIZE];

   // printf("debug5\n");
    uint32_t i, id, *ip;
    char blk[BLOCK_SIZE];
    id = node->blocks[NDIRECT];
    ip = (uint32_t*)blk;
    if(orgnum < curnum){
        if(orgnum > NDIRECT){
            bm->read_block(id, blk);
            for(i=orgnum; i<curnum; i++)
                ip[i-NDIRECT] = bm->alloc_block();
            bm->write_block(id, blk); 
        }
        else if(curnum > NDIRECT){
                for(i=orgnum; i<NDIRECT; i++)
                    node->blocks[i] = bm->alloc_block();
                id = bm->alloc_block();
                node->blocks[NDIRECT] = id;
                for(i=0; i<curnum-NDIRECT; i++)
                    ip[i] = bm->alloc_block();
                bm->write_block(id, blk);
                    
        }
        else{
            for(i=orgnum; i<curnum; i++)
                node->blocks[i] = bm->alloc_block();
                    
        }
            
    }
    else if(orgnum > curnum){
        if(curnum > NDIRECT){
            bm->read_block(id, blk);
            for(i=curnum; i<orgnum; i++)
                bm->free_block(ip[i-NDIRECT]);
                    
        }
        else if(orgnum > NDIRECT){
                bm->read_block(id, blk);
                for(i=curnum; i<NDIRECT; i++)
                    bm->free_block(node->blocks[i]);
                for(i=0; i<curnum-NDIRECT; i++)
                    bm->free_block(ip[i]);
                    
        }
        else{
            for(i=curnum; i<orgnum; i++)
                bm->free_block(node->blocks[i]);
                    
        }
    }
    
   // printf("debug6\n");
    char tmp[BLOCK_SIZE];
    for(i=0; i<MIN(curnum, NDIRECT); i++){
            memcpy(tmp, buf+i*BLOCK_SIZE, BLOCK_SIZE);
            bm->write_block(node->blocks[i], tmp);
        
    }

    if(curnum > NDIRECT){
        bm->read_block(node->blocks[NDIRECT], blk);
        for(; i<curnum; i++){
            memcpy(tmp, buf+i*BLOCK_SIZE, BLOCK_SIZE);
            bm->write_block(ip[i-NDIRECT], tmp);    
        }
            
    }

  // char test[BLOCK_SIZE];
//   bm->read_block(ip[31],test);
 //  printf("%s\n",test);
    node->size=size;
    put_inode(inum, node);

    free(node);

}
void
inode_manager::getattr(uint32_t inum, extent_protocol::attr &a)
{
  /*
   * your lab1 code goes here.
   * note: get the attributes of inode inum.
   * you can refer to "struct attr" in extent_protocol.h
   */
    struct inode* node=get_inode(inum);

    //printf("%d\n",node->type);
    if (node==NULL) return;
    a.type=node->type;
    a.atime=node->atime;
    a.ctime=node->ctime;
    a.mtime=node->mtime;
    a.size=node->size;
    free(node);
}
void
inode_manager::remove_file(uint32_t inum)
{
  /*
   * your lab1 code goes here
   * note: you need to consider about both the data block and inode of the file
   * do not forget to free memory if necessary.
   */

   inode_t* node=get_inode(inum);
    if(node==NULL) return;
    else{
        free_inode(inum);
        uint32_t num=(node->size+BLOCK_SIZE-1)/BLOCK_SIZE;
        for(uint32_t i=0;i<MIN(num,NDIRECT);i++){
            bm->free_block(node->blocks[i]);
        }
        if(num>NDIRECT){
            char indirect[BLOCK_SIZE];
            bm->read_block(node->blocks[NDIRECT],indirect);
            uint32_t* dir=(uint32_t*)indirect;
            for(uint32_t j=0;j<num-NDIRECT;j++){
                bm->free_block(dir[j]);
            }
            bm->free_block(node->blocks[NDIRECT]);
            
        }
    }
}
