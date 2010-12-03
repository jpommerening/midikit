
/**
 * @brief Journal section
 * <tt>
 *     0                   1                   2                   3
 *     0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
 *    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *    |S|Y|A|H|TOTCHAN|   Checkpoint Packet Seqnum    | S-journal ... |
 *    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *    |                      Channel journals ...                     |
 *    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * </tt>
 */
struct MIDIRTPJournalSection {

/**
 * @brief Single-packet loss (S): 1 bit.
 * The S (single-packet loss) bit appears in most recovery journal
 * structures, including the recovery journal header. The S bit helps
 * receivers efficiently parse the recovery journal in the common case
 * of the loss of a single packet.
 */
  unsigned char single_loss;

/**
 * @brief System journal (Y): 1 bit.
 * If the Y header bit is set to 1, the system journal appears in the
 * recovery journal, directly following the recovery journal header.
 */
  unsigned char system_journal;

/**
 * @brief Channel journals (A): 1 bit.
 * If the A header bit is set to 1, the recovery journal ends with a
 * list of (TOTCHAN + 1) channel journals (the 4-bit TOTCHAN header
 * field is interpreted as an unsigned integer).
 * If A and Y are both zero, the recovery journal only contains its 3-
 * octet header and is considered to be an "empty" journal.
 */       
  unsigned char channel_journals;

/**
 * @brief Enhanced Chapter C encoding (H): 1 bit.
 * The H bit indicates if MIDI channels in the stream have been
 * configured to use the enhanced Chapter C encoding.
 *
 * By default, the payload format does not use enhanced Chapter C
 * encoding. In this default case, the H bit MUST be set to 0 for all
 * packets in the stream.
 */
  unsigned char enhanced_chapter_encoding;

/**
 * @brief Total number of channels (TOTCHAN): 4 bits.
 * A MIDI channel MAY be represented by (at most) one channel journal
 * in a recovery journal. Channel journals MUST appear in the recovery
 * journal in ascending channel-number order.
 */
  unsigned char total_channels;

/**
 * @brief Checkpoint Packet Seqnum: 16 bits.
 * The 16-bit Checkpoint Packet Seqnum header field codes the sequence
 * number of the checkpoint packet for this journal, in network byte
 * order (big-endian). The choice of the checkpoint packet sets the
 * depth of the checkpoint history for the journal.
 */
  unsigned short checkpoint_packet_seqnum;
};

/**
 * @brief Channel journal
 * <tt>
 *     0                   1                   2                   3
 *     0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
 *    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *    |S| CHAN  |H|      LENGTH       |P|C|M|W|N|E|T|A|  Chapters ... |
 *    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * </tt>
 */

/*
 * @brief System journal
 * <tt>
 *     0                   1                   2                   3
 *     0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
 *    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *    |S|D|V|Q|F|X|      LENGTH       |  System chapters ...          |
 *    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * </tt>
 */
