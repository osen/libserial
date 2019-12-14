#include "Stream.h"
#include "Device.h"
#include "stent.h"

#include <time.h>
#include <stdio.h>

//#define SE_FRAME_BEGIN '^'
#define SE_FRAME_END '$'
#define SE_FRAME_SEP ','

#define SE_TYPE_INITIAL '!'
#define SE_TYPE_REPEAT '.'
#define SE_TYPE_INVALID '?'

void _SeDebugVector(vector(unsigned char) in)
{
  size_t i = 0;

  printf("%i [", (int)vector_size(in));

  for(i = 0; i < vector_size(in); i++)
  {
    printf("%c", vector_at(in, i));
  }

  printf("]\n");
}

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
  _(rtn).type = SE_TYPE_INVALID;
  _(rtn).payload = vector_new(unsigned char);
  _(rtn).hash = sstream_new();

  return rtn;
}

void _SeFrameUpdateHash(ref(SeFrame) ctx)
{
  sstream_str_cstr(_(ctx).hash, "ABC");
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

  //vector_push_back(_(ctx).outgoing, SE_FRAME_BEGIN);
  //vector_push_back(_(ctx).outgoing, SE_FRAME_SEP);
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
  _SeFrameUpdateHash(frame);

  for(si = 0; si < sstream_length(_(frame).hash); si++)
  {
    vector_push_back(_(ctx).outgoing, sstream_at(_(frame).hash, si));
  }

  vector_push_back(_(ctx).outgoing, SE_FRAME_END);
  sstream_delete(str);

  _(frame).type = SE_TYPE_REPEAT;

  /*
   * 34,!,HELO,HASH$
   */
}

ref(SeFrame) _SeStreamRetrieveFrame(ref(SeStream) ctx)
{
  vector(unsigned char) incoming = NULL;
  vector(unsigned char) packet = NULL;
  size_t ii = 0;
  ref(SeFrame) rtn = NULL;
  int fail = 0;
  vector(unsigned char) idSeg = NULL;
  vector(unsigned char) payloadSeg = NULL;
  vector(unsigned char) hashSeg = NULL;

  incoming = _(ctx).incoming;

  for(ii = 0; ii < vector_size(incoming); ii++)
  {
    if(vector_at(incoming, ii) == SE_FRAME_END)
    {
      packet = vector_new(unsigned char);
      vector_insert(packet, 0, incoming, 0, ii + 1);
      vector_erase(incoming, 0, ii + 1);
      break;
    }
  }

  if(packet == NULL)
  {
    return NULL;
  }

  /*
   * <id>,<type>,<payload>,<hash>$
   */

  //_SeDebugVector(packet);

  rtn = SeFrameCreate();
  idSeg = vector_new(unsigned char);

  /*
   * Store up to first ',' as the id segment.
   * Remove these from the packet.
   */
  for(ii = 0; ii < vector_size(packet); ii++)
  {
    if(vector_at(packet, ii) == ',')
    {
      vector_erase(packet, 0, ii + 1);
      break;
    }
    else
    {
      vector_push_back(idSeg, vector_at(packet, ii));
    }
  }

  /*
   * No id segment found.
   */
  if(vector_size(idSeg) < 1)
  {
    fail = 1;
    goto end;
  }

  /*
   * Packet not large enough to store the type and sep.
   */
  if(vector_size(packet) < 2)
  {
    fail = 1;
    goto end;
  }

  /*
   * Store the type and remove both it and sep.
   */
  _(rtn).type = vector_at(packet, 0);
  vector_erase(packet, 0, 2);

  payloadSeg = vector_new(unsigned char);

  /*
   * Store up to next ',' as the payload segment.
   * Remove these from the packet.
   */
  for(ii = 0; ii < vector_size(packet); ii++)
  {
    if(vector_at(packet, ii) == ',')
    {
      vector_erase(packet, 0, ii + 1);
      break;
    }
    else
    {
      vector_push_back(payloadSeg, vector_at(packet, ii));
    }
  }

  hashSeg = vector_new(unsigned char);

  /*
   * Store up to end '$' as the hash segment.
   * Remove these from the packet.
   */
  for(ii = 0; ii < vector_size(packet); ii++)
  {
    if(vector_at(packet, ii) == '$')
    {
      vector_erase(packet, 0, ii + 1);
      break;
    }
    else
    {
      vector_push_back(hashSeg, vector_at(packet, ii));
    }
  }

  /*
   * No hash segment found.
   */
  if(vector_size(hashSeg) < 1)
  {
    fail = 1;
    goto end;
  }

  //printf("Payload and hash obtained\n");
  _SeDebugVector(payloadSeg);

end:
  if(idSeg)
  {
    vector_delete(idSeg);
  }

  if(payloadSeg)
  {
    vector_delete(payloadSeg);
  }

  if(hashSeg)
  {
    vector_delete(hashSeg);
  }

  vector_delete(packet);

  if(fail)
  {
    _(rtn).type = SE_TYPE_INVALID;
  }

  return rtn;
}

void _SeStreamFlush(ref(SeStream) ctx)
{
  while(SeDeviceReady(_(ctx).dev, SE_MODE_W, 0) &&
    vector_size(_(ctx).outgoing) > 0)
  {
    //_SeDebugVector(_(ctx).outgoing);
    //printf("Before: %i\n", (int)vector_size(_(ctx).outgoing));
    SeDeviceWrite(_(ctx).dev, _(ctx).outgoing);
    //_SeDebugVector(_(ctx).outgoing);
    //printf("After: %i\n", (int)vector_size(_(ctx).outgoing));
  }
}

void _SeStreamProcessIncoming(ref(SeStream) ctx)
{
  vector(unsigned char) buffer = NULL;
  size_t bi = 0;
  ref(SeFrame) frame = NULL;

  buffer = vector_new(unsigned char);

  while(SeDeviceReady(_(ctx).dev, SE_MODE_R, 0))
  {
    vector_resize(buffer, 1024);
    SeDeviceRead(_(ctx).dev, buffer);

    vector_insert(_(ctx).incoming, vector_size(_(ctx).incoming),
      buffer, 0, vector_size(buffer));
  }

  vector_delete(buffer);
  //_SeDebugVector(_(ctx).incoming);

  while(1)
  {
    frame = _SeStreamRetrieveFrame(ctx);

    if(!frame)
    {
      break;
    }

    if(_(frame).type == SE_TYPE_INVALID)
    {
      printf("Invalid frame\n");
    }

    SeFrameDestroy(frame);
  }
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
