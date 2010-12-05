
/**
 * @brief MIDI command section
 * The MIDI command section has the following format:
 *
 * <pre>
 *     0                   1                   2                   3
 *     0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
 *    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *    |B|J|Z|P|LEN... |  MIDI list ...                                |
 *    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * </pre>
 *
 * The MIDI command section begins with a variable-length header.
 */
struct RTPMIDICommandSection {

/**
 * @brief Journal (J): 1 bit.
 * If the J header bit is set to 1, a journal section MUST appear after
 * the MIDI command section in the payload.  If the J header bit is set
 * to 0, the payload MUST NOT contain a journal section.
 */
  unsigned char journal;

/**
 * @brief Zero (Z): 1 bit.
 * If the header flag Z is 1, the MIDI list begins with a complete MIDI
 * command (coded in the MIDI Command 0 field, in Figure 3) preceded by
 * a delta time (coded in the Delta Time 0 field). If Z is 0, the Delta
 * Time 0 field is not present in the MIDI list, and the command coded
 * in the MIDI Command 0 field has an implicit delta time of 0.
 */
  unsigned char zero;

/**
 * @brief Phantom (P): 1 bit.
 * If the status octet of the first MIDI command was not present in
 * the source stream (when the source used running status coding) the
 * phantom bit MUST be one. The phantom bit encodes a property of the
 * source stream. The first command in the MIDI list must still begin
 * with a status octet.
 */
  unsigned char phantom;

/**
 * @brief Length (LEN): 4 or 12 bits.
 * The header field LEN codes the number of octets in the MIDI list that
 * follow the header.  If the header flag B is 0, the header is one
 * octet long, and LEN is a 4-bit field, supporting a maximum MIDI list
 * length of 15 octets.
 *
 * If B is 1, the header is two octets long, and LEN is a 12-bit field,
 * supporting a maximum MIDI list length of 4095 octets.  LEN is coded
 * in network byte order (big-endian): the 4 bits of LEN that appear in
 * the first header octet code the most significant 4 bits of the 12-bit
 * LEN value.
 *
 * A LEN value of 0 is legal, and it codes an empty MIDI list.
 */
  unsigned short length;

/**
 * @brief MIDI list.
 *
 * <pre>
 *    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *    |  Delta Time 0     (1-4 octets long, or 0 octets if Z = 1)     |
 *    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *    |  MIDI Command 0   (1 or more octets long)                     |
 *    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *    |  Delta Time 1     (1-4 octets long)                           |
 *    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *    |  MIDI Command 1   (1 or more octets long)                     |
 *    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *    |                              ...                              |
 *    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *    |  Delta Time N     (1-4 octets long)                           |
 *    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *    |  MIDI Command N   (0 or more octets long)                     |
 *    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * </pre>
 */
  struct MIDIMessageList * midi_list;

};


