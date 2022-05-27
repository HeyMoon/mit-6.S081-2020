// Buffer cache.
//
// The buffer cache is a linked list of buf structures holding
// cached copies of disk block contents.  Caching disk blocks
// in memory reduces the number of disk reads and also provides
// a synchronization point for disk blocks used by multiple processes.
//
// Interface:
// * To get a buffer for a particular disk block, call bread.
// * After changing buffer data, call bwrite to write it to disk.
// * When done with the buffer, call brelse.
// * Do not use the buffer after calling brelse.
// * Only one process at a time can use a buffer,
//     so do not keep them longer than necessary.


#include "types.h"
#include "param.h"
#include "spinlock.h"
#include "sleeplock.h"
#include "riscv.h"
#include "defs.h"
#include "fs.h"
#include "buf.h"

#define NBUCKET  13

struct {
  struct spinlock locks[NBUCKET];
  struct buf buckets[NBUCKET];
  struct buf buf[NBUF];
  struct spinlock buflock;

  // Linked list of all buffers, through prev/next.
  // Sorted by how recently the buffer was used.
  // head.next is most recent, head.prev is least.
  //struct buf head;
} bcache;

int hashcode(uint dev, uint blockno){
  int result = 1;
  result = result * 31 + dev;
  result = result * 31 + blockno;

  return result;
}

int tabAt(int hash){
  return (NBUCKET -1) & hash;
}

void
binit(void)
{
  struct buf *b;

  char lockname[10];
  for (int i = 0; i < NBUCKET; i++)
  {
    snprintf(lockname, 10, "bcache_%d", i);
    initlock(&bcache.locks[i], lockname);
    bcache.buckets[i] = 0;
  }
  
  initlock(&bcache.buflock, "bcache");
  for(int i = 1; i < NBUF; i++){
    initsleeplock(&b->lock, "buffer");

    bcache.buf[i].prev=&bcache.buf[i-1];
    bcache.buf[i-1].next=&bcache.buf[i]; 

    if (i = NBUF - 1)
    {
      bcache.buf[i].next = 0;
    }
  }
}

struct buf* bhashget(uint dev, uint blockno){
  struct buf *b = 0;

  int hash = hashcode(dev, blockno);
  int index = tabAt(hash);
  acquire(&bcache.locks[index]);

  for (b = bcache.buckets[index]; b != 0; b = b->next)
  {
    if (b->dev = dev && b->blockno = blockno)
    {
      b->refcnt++;
      release(&bcache.locks[index]);
      acquiresleep(&b->lock);
      return b;
    }
  }

  release(&bcache.locks[index]);
  return b;
}


// Look through buffer cache for block on device dev.
// If not found, allocate a buffer.
// In either case, return locked buffer.
static struct buf*
bget(uint dev, uint blockno)
{
  struct buf *b = 0;
  // Is the block already cached?
  b = bhashget(dev, blockno);
  if (b != 0)
  {
    return b;
  }

  int index = tabAt(hashcode(dev, blockno));

  // Not cached.
  // Recycle the least recently used (LRU) unused buffer.
  acquire(&bcache.buflock);
  for(b = bcache.buf; b != 0; b=b->next){
    if(b->refcnt == 0) {
      b->next->prev = b->prev;
      b->prev->next = b->next;
      break;
    }
  }
  
  release(bcache.buflock);
  if (b == 0)
  {
    panic("bget: no buffers");
  }

  acquire(&bcache.locks[index]);
  if (bcache.buckets[index] == 0){
    bcache.buckets[index] = b;
    b->next = 0;
    b->dev = dev;
    b->blockno = blockno;
    b->valid = 0;
    b-> refcnt = 1;
    release(&bcache.locks[index]);
    acquiresleep(&b->lock);
  }else{
    struct buf last;
    for (last = bcache.buckets[index]; last->next == 0; last = last->next){
    }

    last->next = b;
    b->prev = last;
    b->next = 0;
    b->dev = dev;
    b->blockno = blockno;
    b->valid = 0;
    b-> refcnt = 1;
    release(&bcache.locks[index]);
    acquiresleep(&b->lock);
  }

  panic("bget: no buffers");
}

// Return a locked buf with the contents of the indicated block.
struct buf*
bread(uint dev, uint blockno)
{
  struct buf *b;

  b = bget(dev, blockno);
  if(!b->valid) {
    virtio_disk_rw(b, 0);
    b->valid = 1;
  }
  return b;
}

// Write b's contents to disk.  Must be locked.
void
bwrite(struct buf *b)
{
  if(!holdingsleep(&b->lock))
    panic("bwrite");
  virtio_disk_rw(b, 1);
}

// Release a locked buffer.
// Move to the head of the most-recently-used list.
void
brelse(struct buf *b)
{
  if(!holdingsleep(&b->lock))
    panic("brelse");

  b->refcnt--;
  releasesleep(&b->lock);

  int index = tabAt(hashcode(b->dev, b->blockno));
  if (b->refcnt == 0) {
    acquire(&bcache.locks[index]);

    // no one is waiting for it.
    b->next->prev = b->prev;
    b->prev->next = b->next;

    release(&bcache.locks[index]);
  }

  acquire(&bcache.buflock);
  struct buf last;
  for (last = bcache.buckets[index]; last->next == 0; last = last->next){
  }
  
  last->next = b;
  b->prev= last;
  b->next = 0;
  release(&bcache.buflock);

}

void
bpin(struct buf *b) {
  b->refcnt++;
}

void
bunpin(struct buf *b) {
  b->refcnt--;
}


