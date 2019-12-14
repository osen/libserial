#include "Stream.h"
#include "Device.h"
#include "stent.h"

#include <time.h>
#include <stdio.h>

#define SE_FRAME_BEGIN '^'
#define SE_FRAME_END '$'
#define SE_FRAME_SEP ','

#define SE_TYPE_INITIAL '!'
#define SE_TYPE_REPEAT '.'

struct SeFrame
{
  size_t id;
  unsigned char type;
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

void _SeStreamAddFrame(ref(SeStream) ctx, ref(SeFrame) frame)
{
  ref(sstream) str = NULL;
  size_t si = 0;

  vector_push_back(_(ctx).outgoing, SE_FRAME_BEGIN);
  vector_push_back(_(ctx).outgoing, SE_FRAME_SEP);
  str = sstream_new();
  sstream_append_int(str, _(frame).id);

  for(si = 0; si < sstream_length(str); si++)
  {
    vector_push_back(_(ctx).outgoing, sstream_at(str, si));
  }

  vector_push_back(_(ctx).outgoing, SE_FRAME_SEP);
  vector_push_back(_(ctx).outgoing, _(frame).type);
  vector_push_back(_(ctx).outgoing, SE_FRAME_SEP);

  vector_insert(_(ctx).outgoing, vector_size(_(ctx).outgoing),
    _(frame).payload, 0, vector_size(_(frame).payload));

  vector_push_back(_(ctx).outgoing, SE_FRAME_SEP);

  for(si = 0; si < sstream_length(_(frame).hash); si++)
  {
    vector_push_back(_(ctx).outgoing, sstream_at(_(frame).hash, si));
  }

  vector_push_back(_(ctx).outgoing, SE_FRAME_SEP);
  vector_push_back(_(ctx).outgoing, SE_FRAME_END);
  sstream_delete(str);

  _(frame).type = SE_TYPE_REPEAT;

  /*
   * ^,34,!,HELO,HASH,$
   */
}

void _SeStreamFlush(ref(SeStream) ctx)
{
  while(SeDeviceReady(_(ctx).dev, SE_MODE_W, 0) &&
    vector_size(_(ctx).outgoing) > 0)
  {
/*
    {
      size_t i = 0;

      printf("%i [", (int)vector_size(_(ctx).outgoing));

      for(i = 0; i < vector_size(_(ctx).outgoing); i++)
      {
        printf("%c", vector_at(_(ctx).outgoing, i));
      }

      printf("]\n");
    }
*/

    //printf("Before: %i\n", (int)vector_size(_(ctx).outgoing));
    SeDeviceWrite(_(ctx).dev, _(ctx).outgoing);
    //printf("After: %i\n", (int)vector_size(_(ctx).outgoing));
  }
}

void _SeStreamProcessIncoming(ref(SeStream) ctx)
{

}

void _SeStreamProcessOutgoing(ref(SeStream) ctx)
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
      _(frame).timestamp = now + SE_PACKET_TIMEOUT;
      _SeStreamAddFrame(ctx, frame);
    }
  }
}

void _SeStreamProcess(ref(SeStream) ctx)
{
  _SeStreamProcessIncoming(ctx);
  _SeStreamProcessOutgoing(ctx);
  _SeStreamFlush(ctx);
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
  _SeStreamProcess(ctx);
}

void SeStreamWrite(ref(SeStream) ctx, vector(unsigned char) buffer)
{
  ref(SeFrame) frame = NULL;

  frame = SeFrameCreate();
  vector_insert(_(frame).payload, 0, buffer, 0, vector_size(buffer));
  sstream_str_cstr(_(frame).hash, "ABC");
  _(frame).id = _(ctx).outId++;
  _(frame).type = SE_TYPE_INITIAL;
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
