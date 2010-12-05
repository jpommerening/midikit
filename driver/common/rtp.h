#ifndef MIDIKIT_DRIVER_RTP_H
#define MIDIKIT_DRIVER_RTP_H

struct RTPHeader {
  unsigned version         : 2;
  unsigned padding         : 1;
  unsigned extension       : 1;
  unsigned csrc_count      : 4;
  unsigned marker          : 1;
  unsigned payload_type    : 7;
  unsigned sequence_number : 16;
  unsigned long timestamp;
  unsigned long ssrc;
  unsigned long * csrc_list;
};

struct RTPHeaderExtension {
  unsigned short  profile_data;
  unsigned short  length;
  unsigned long * header_extension;
};

#endif
