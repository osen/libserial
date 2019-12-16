#include "Stream.h"
#include "Device.h"
#include "Hash.h"

#include <time.h>
#include <stdio.h>

//#define SE_FRAME_BEGIN '^'
#define SE_FRAME_END '$'
#define SE_FRAME_SEP ','

#define SE_TYPE_INITIAL '!'
#define SE_TYPE_REPEAT '.'
#define SE_TYPE_CONFIRM '>'
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
  ref(sstream) id = NULL;
  ref(sstream) hash = NULL;
  vector(unsigned char) tmp = NULL;
  size_t ci = 0;

  id = sstream_new();
  sstream_append_int(id, _(ctx).id);
  tmp = vector_new(unsigned char);

  /*
   * Add id number string
   */
  for(ci = 0; ci < sstream_length(id); ci++)
  {
    vector_push_back(tmp, sstream_at(id, ci));
  }

  /*
   * Add type character
   */
  vector_push_back(tmp, _(ctx).type);

  /*
   * Add payload
   */
  vector_insert(tmp, 1, _(ctx).payload, 0, vector_size(_(ctx).payload));
  hash = SeHash(tmp);
  sstream_str(_(ctx).hash, hash);

  sstream_delete(id);
  vector_delete(tmp);
  sstream_delete(hash);
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
  vector(ref(SeFrame)) framesIn;;
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
  //_SeDebugVector(_(ctx).outgoing);
}

int _SeStreamRetrieveFrame(ref(SeStream) ctx, ref(SeFrame) frame)
{
  vector(unsigned char) incoming = NULL;
  vector(unsigned char) packet = NULL;
  size_t ii = 0;
  int fail = 0;
  ref(sstream) idSeg = NULL;
  vector(unsigned char) payloadSeg = NULL;
  ref(sstream) hashSeg = NULL;

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
    return 0;
  }

  /*
   * <id>,<type>,<payload>,<hash>$
   */
  //_SeDebugVector(packet);

  _(frame).type = SE_TYPE_INVALID;
  idSeg = sstream_new();

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
      sstream_append_char(idSeg, vector_at(packet, ii));
    }
  }

  /*
   * No id segment found.
   */
  if(sstream_length(idSeg) < 1)
  {
    fail = 1;
    goto end;
  }

  _(frame).id = atoi(sstream_cstr(idSeg));

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
  _(frame).type = vector_at(packet, 0);
  vector_erase(packet, 0, 2);

  vector_resize(_(frame).payload, 0);

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
      vector_push_back(_(frame).payload, vector_at(packet, ii));
    }
  }

  hashSeg = sstream_new();

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
      sstream_append_char(hashSeg, vector_at(packet, ii));
    }
  }

  /*
   * No hash segment found.
   */
  if(sstream_length(hashSeg) < 1)
  {
    fail = 1;
    goto end;
  }

  _SeFrameUpdateHash(frame);

  if(strcmp(sstream_cstr(hashSeg), sstream_cstr(_(frame).hash)) != 0)
  {
    fail = 1;
    goto end;
  }

end:
  if(idSeg)
  {
    sstream_delete(idSeg);
  }

  if(hashSeg)
  {
    sstream_delete(hashSeg);
  }

  vector_delete(packet);

  if(fail)
  {
    _(frame).type = SE_TYPE_INVALID;
  }

  return 1;
}

void _SeStreamProcessConfirmation(ref(SeStream) ctx, ref(SeFrame) frame)
{
  size_t fi = 0;
  ref(SeFrame) curr = NULL;

  for(fi = 0; fi < vector_size(_(ctx).frames); fi++)
  {
    curr = vector_at(_(ctx).frames, fi);

    if(_(curr).id == _(frame).id)
    {
      SeFrameDestroy(curr);
      vector_erase(_(ctx).frames, fi, 1);
      fi--;
    }
  }
}

void _SeStreamProcessFrameIn(ref(SeStream) ctx, ref(SeFrame) frame)
{
  ref(SeFrame) response = NULL;
  size_t fi = 0;
  ref(SeFrame) curr = NULL;

  printf("id: %i ", (int)_(frame).id);
  _SeDebugVector(_(frame).payload);

  response = SeFrameCreate();
  _(response).id = _(frame).id;
  _(response).type = SE_TYPE_CONFIRM;
  _SeStreamAddFrame(ctx, response);
  SeFrameDestroy(response);

  for(fi = 0; fi < vector_size(_(ctx).frames); fi++)
  {
    curr = vector_at(_(ctx).frames, fi);

    if(_(curr).id == _(frame).id)
    {
      SeFrameDestroy(frame);

      return;
    }
  }

  vector_push_back(_(ctx).frames, frame);
}

void _SeStreamFlush(ref(SeStream) ctx)
{
  while(SeDeviceReady(_(ctx).dev, SE_MODE_W, 0) &&
    vector_size(_(ctx).outgoing) > 0)
  {
    SeDeviceWrite(_(ctx).dev, _(ctx).outgoing);
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
  frame = SeFrameCreate();

  while(_SeStreamRetrieveFrame(ctx, frame))
  {
    if(_(frame).type == SE_TYPE_INVALID)
    {
      printf("Invalid frame\n");
    }
    else if(_(frame).type == SE_TYPE_INITIAL ||
      _(frame).type == SE_TYPE_REPEAT)
    {
      _SeStreamProcessFrameIn(ctx, frame);
      frame = SeFrameCreate();
    }
    else if(_(frame).type == SE_TYPE_CONFIRM)
    {
      _SeStreamProcessConfirmation(ctx, frame);
    }
    else
    {
      printf("Warning: Unknown frame type\n");
      //abort();
    }
  }

  SeFrameDestroy(frame);
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
  _(rtn).framesIn = vector_new(ref(SeFrame));
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

  for(fi = 0; fi < vector_size(_(ctx).framesIn); fi++)
  {
    SeFrameDestroy(vector_at(_(ctx).framesIn, fi));
  }

  vector_delete(_(ctx).frames);
  vector_delete(_(ctx).framesIn);
  vector_delete(_(ctx).outgoing);
  vector_delete(_(ctx).incoming);
  release(_(ctx).dev);
  release(ctx);
}
