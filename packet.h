#ifndef PACKET_H
#define PACKET_H


#include <stdint.h>
#include <sys/uio.h>
#include <stdbool.h>


// Get the size of the range of an unsigned type
// XXX: taken from http://stackoverflow.com/questions/2053843/min-and-max-value-of-data-type-in-c
#define urange(t) ((((0x1ULL << ((sizeof(t) * 8ULL) - 1ULL)) - 1ULL) | \
                    (0xFULL << ((sizeof(t) * 8ULL) - 4ULL))) + 1)



//
// Constants 
//


// FIXME: should these be defined elsewhere/differently???
extern const unsigned MAX_IOVEC_LEN;


extern const uint16_t PACKET_START; // Start of packet identifier
extern const uint16_t PACKET_END;   // End of packet identifier


typedef uint8_t client_id;    // [0,255]
typedef uint8_t payload_len;  // [0,255]
typedef uint8_t sequence_num; // [0,255]


typedef enum {     // This packet communicates...
  DATA   = 0xFFF1, // data
  ACK    = 0xFFF2, // acknowledgement
  REJECT = 0xFFF3  // rejection
} packet_type;


typedef enum {         // The received packet was rejected because...
  OUT_OF_SEQ = 0xFFF4, // Out of sequence
  BAD_LEN    = 0xFFF5, // Length field mismatches payload length
  NO_END     = 0xFFF6, // No 'end of packet' OR 'start of packet' ID
  DUP_PACK   = 0xFFF7, // This was a duplicate (already received before)
  BAD_TYPE   = 0xFFF8  // XXX: This is extra, but it seemed necessary
} reject_code;


// Helper type for packet_info
typedef union {
  struct {
    sequence_num seq_num;
    payload_len len;
    void const* payload;
  } data_info;

  // XXX: Kept these as structs to keep access/naming consistent, i.e.
  // pi.ack.recvd_seq_num instead of pi.recvd_seq_num, in which case it's not
  // clear that pi holds info for an ACK packet
  struct {
    sequence_num recvd_seq_num;
  } ack_info;

  struct {
    reject_code code;
    sequence_num recvd_seq_num;
  } reject_info; 
} content;


/// XXX: packet_type must be consistent with cont's contents.
typedef struct {
  packet_type type;
  client_id id;
  content cont;
} packet_info;


//
// Functions
//


//
// XXX: For well-formed packets, flatten() and interpret_packet() should be
// inverses, i.e.
//
// flatten(pi, buf);
// interpret_packet(buf, pi);
//
// and
//
// interpret_packet(buf, pi);
// flatten(pi, buf);
// 
// Should do nothing to pi or buf.
//


// Flatten and copy a packet_info into a buffer for sending.
// Return value: the size of the flattened packet, in bytes
size_t flatten(packet_info const* pi, void* buf, size_t size);


// Interpret a raw buffer as a packet. As much info is filled out as possible
// even if the packet overall is malformed.
// Return value: Zero if everything was OK, or the reject_code (uncasted)
// otherwise.
// POSTCONDITION: Argument 'pi' is set to the appropriate info if 0 is returned.
// XXX: the 'data' field inside the packet info is left pointing to inside
// the given buffer, so any data must be processed before the buffers is
// reused.
// XXX: The sequence number parameter is only used if the processed packet is
// of type DATA
int interpret_packet(void const* buf, packet_info* pi, size_t size);


// Wrapper for use with busy_wait_until()
// TODO: docs
bool try_sendto(void* args, void* retval);


#endif // PACKET_H
