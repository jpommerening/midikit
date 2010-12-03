#ifndef MIDIKIT_DRIVER_RTP_H
#define MIDIKIT_DRIVER_RTP_H

/**
 * @brief RTP header
 * The RTP header has the following format:
 *
 * <tt>
 *     0                   1                   2                   3
 *     0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
 *    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *    | V |P|X|  CC   |M|     PT      |        Sequence number        |
 *    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *    |                           Timestamp                           |
 *    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *    |                             SSRC                              |
 *    +=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+
 *    |            contributing source (CSRC) identifiers             |
 *    |                             ....                              |
 *    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * </tt>
 *
 * The first twelve octets are present in every RTP packet, while the
 * list of CSRC identifiers is present only when inserted by a mixer.
 */
struct RTPHeader {

/**
 * @brief Version (V): 2 bits.
 * The version field identifies the version of RTP. The version we use
 * and the one specified by RFC3550 is two (2). (The value 1 is used by
 * the first draft version of RTP and the value 0 is used by the
 * protocol initially implemented in the "vat" audio tool.)
 */
  unsigned char version;

/**
 * @brief Padding (P): 1 bit.
 * If the padding bit is set, the packet contains one or more additional
 * padding octets at the end which are not part of the payload. The last
 * octet of the padding contains a count of how many padding octets
 * should be ignored, including itself.  Padding may be needed by some
 * encryption algorithms with fixed block sizes or for carrying several
 * RTP packets in a lower-layer protocol data unit.
 */
  unsigned char padding;

/**
 * @brief Extension (X): 1 bit.
 * If the extension bit is set, the fixed header MUST be followed by
 * exactly one header extension, with a format defined by
 * RTPHeaderExtension.
 */
  unsigned char extension;

/**
 * @brief CSRC count (CC): 4 bits.
 * The CSRC count contains the number of CSRC identifiers that follow
 * the fixed header.
 */
  unsigned char csrc_count;

/**
 * @brief Marker (M): 1 bit.
 * The interpretation of the marker is defined by a profile.  It is
 * intended to allow significant events such as frame boundaries to be
 * marked in the packet stream. A profile MAY define additional marker
 * bits or specify that there is no marker bit by changing the number
 * of bits in the payload type field.
 */
  unsigned char marker;

/**
 * @brief Payload type (PT): 7 bits.
 * This field identifies the format of the RTP payload and determines
 * its interpretation by the application.  A profile MAY specify a
 * default static mapping of payload type codes to payload formats.
 */
  unsigned char payload_type;

/**
 * @brief Sequence number: 16 bits.
 * The sequence number increments by one for each RTP data packet sent,
 * and may be used by the receiver to detect packet loss and to restore
 * packet sequence. The initial value of the sequence number SHOULD be
 * random (unpredictable) to make known-plaintext attacks on encryption
 * more difficult, even if the source itself does not encrypt, because
 * the packets may flow through a translator that does.
 */
  unsigned short sequence_number;

/**
 * @brief Timestamp: 32 bits.
 * The timestamp reflects the sampling instant of the first octet in
 * the RTP data packet. The sampling instant MUST be derived from a
 * clock that increments monotonically and linearly in time to allow
 * synchronization and jitter calculations.
 */
  unsigned long timestamp;

/**
 * @brief SSRC: 32 bits.
 * The SSRC field identifies the synchronization source.  This
 * identifier SHOULD be chosen randomly, with the intent that no two
 * synchronization sources within the same RTP session will have the
 * same SSRC identifier.
 */
  unsigned long ssrc;

/**
 * @brief CSRC list: 0 to 15 items, 32 bits each.
 * The CSRC list identifies the contributing sources for the payload
 * contained in this packet. The number of identifiers is given by the
 * CC field. If there are more than 15 contributing sources, only 15 can
 * can be identified. CSRC identifiers are inserted by mixers, using the
 * SSRC identifiers of contributing sources. For example, for audio
 * packets the SSRC identifiers of all sources that were mixed together
 * to create a packet are listed, allowing correct talker indication at
 * the receiver.
 */
  unsigned long * csrc_list;

};

/**
 * @brief RTP header extension
 * The RTP header extension has the following format:
 *
 * <tt>
 *     0                   1                   2                   3
 *     0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
 *    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *    |      defined by profile       |           length              |
 *    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *    |                        header extension                       |
 *    |                             ....                              |
 *    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * </tt>
 *
 * An extension mechanism is provided to allow individual implementations
 * to experiment with new payload-format-independent functions that
 * require additional information to be carried in the RTP data packet
 * header. This mechanism is designed so that the header extension may
 * be ignored by other interoperating implementations that have not been
 * extended.
 */
struct RTPHeaderExtension {

/**
 * @brief Defined by profile.
 * To allow multiple interoperating implementations to each experiment
 * independently with different header extensions, or to allow a
 * particular implementation to experiment with more than one type of
 * header extension, the first 16 bits of the header extension are left
 * open for distinguishing identifiers or parameters.  The format of
 * these 16 bits is to be defined by the profile specification under
 * which the implementations are operating.  This RTP specification does
 * not define any header extensions itself.
 */
  unsigned short  profile_data;

/**
 * @brief Length: 16 bits.
 * The length field counts the number of 32-bit words in the extension,
 * excluding the four-octet extension header (therefore zero is a valid
 * length).
 */
  unsigned short  length;

/**
 * @brief Header extension.
 */
  unsigned long * header_extension;
};

#endif
