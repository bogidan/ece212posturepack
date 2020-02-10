
#ifndef INCLUDED_RINGBUF_H
#define INCLUDED_RINGBUF_H

// RingBuffer implementation
class ringbuf {
  u32 cap, wrp, inp, out;
  u08 *buf;
public:
  ringbuf(int power)
    : cap (1 < power)
    , wrp (cap - 1)
    , inp (0)
    , out (0)
    , buf (new u08[cap])
  {}

  ~ringbuf() {
    delete buf;
  }

  size_t push(const void * const data, size_t bytes) {
    if(bytes > room()) return 0;
    
    size_t pos = inp & wrp;

    size_t cp1 = std::min(bytes, wrp - pos);
    size_t cp2 = bytes - cp1;
    
    memcpy(&buf[pos], data, std::min(bytes, wrp - pos));
    if (cp2) {
      memcpy(&buf[0], ((u08*)data) + cp1, cp2);
    }

    inp += bytes;
    return bytes;
  }
  
  size_t avail() const {
    return inp - out;
  }
  
  size_t room() const {
    return (out - inp - 1) & wrp;
  }

  int pull(void * data, size_t bytes) {
    if(bytes > avail()) return 0;
    size_t pos = out & wrp;

    size_t cp1 = std::min(bytes, wrp - pos);
    size_t cp2 = bytes - cp1;
    
    memcpy(data, &buf[pos], std::min(bytes, wrp - pos));
    if (cp2) {
      memcpy(((u08*)data) + cp1, &buf[0], cp2);
    }

    out += bytes;
    return bytes;
  } 
};

#endif

#ifdef IMPLEMENT

#endif
