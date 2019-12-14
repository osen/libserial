#include "Stream.h"
#include "Device.h"
#include "stent.h"

#include <time.h>
#include <stdio.h>

struct SeFrame
{
  size_t id;
  int type;
  vector(unsigned char) payload;
  ref(sstream) hash;
  time_t timestamp;
};

ref(SeFrame) SeFrameCreate()
{
  ref(SeFrame) rtn = NULL;

  rtn = allocate(SeFrame);
  _(rtn).payload = vector_new(unsigned char);
  _(rtn).hash = sstream_new();

  return rtn;
}

void SeFrameDestroy(ref(SeFrame) ctx)
{
  vector_delete(_(ctx).payload);
  sstream_delete(_(ctx).hash);
  release(ctx);
}

struct SeStream
{
  size_t outId;
  size_t inId;
  ref(SeDevice) dev;
  vector(ref(SeFrame)) frames;
  vector(unsigned char) outgoing;
  vector(unsigned char) incoming;
};

void _SeStreamProcess(ref(SeStream) ctx)
{
  ref(SeFrame) frame = NULL;
  size_t fi = 0;
  time_t now = 0;

  now = time(NULL);

  for(fi = 0; fi < vector_size(_(ctx).frames); fi++)
  {
    frame = vector_at(_(ctx).frames, fi);

    if(_(frame).timestamp <= now)
    {
      _(frame).timestamp += SE_PACKET_TIMEOUT;
      printf("Send\n");
      // Add frame to outgoing (function?)
      // Write as much of outgoing as possible
    }
  }
}

ref(SeStream) SeStreamOpen(char *path)
{
  ref(SeStream) rtn = NULL;

  rtn = allocate(SeStream);
  _(rtn).dev = SeDeviceOpen(path);
  _(rtn).frames = vector_new(ref(SeFrame));
  _(rtn).outgoing = vector_new(unsigned char);
  _(rtn).incoming = vector_new(unsigned char);

  return rtn;
}

void SeStreamRead(ref(SeStream) ctx, vector(unsigned char) buffer)
{

}

void SeStreamWrite(ref(SeStream) ctx, vector(unsigned char) buffer)
{
  ref(SeFrame) frame = NULL;

  frame = SeFrameCreate();
  vector_insert(_(frame).payload, 0, buffer, 0, vector_size(buffer));
  sstream_str_cstr(_(frame).hash, "ABC");
  _(frame).id = _(ctx).outId++;
  _(frame).type = 1;
  _(frame).timestamp = time(NULL);
  vector_push_back(_(ctx).frames, frame);

  _SeStreamProcess(ctx);
}

void SeStreamClose(ref(SeStream) ctx)
{
  size_t fi = 0;

  for(fi = 0; fi < vector_size(_(ctx).frames); fi++)
  {
    SeFrameDestroy(vector_at(_(ctx).frames, fi));
  }

  vector_delete(_(ctx).frames);
  vector_delete(_(ctx).outgoing);
  vector_delete(_(ctx).incoming);
  release(_(ctx).dev);
  release(ctx);
}
