#ifndef PACKET_H
#define PACKET_H


#include <stdint.h>
#include <sys/uio.h>
#include <stdbool.h>


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
  NO_END     = 0xFFF6, // No 'end of packet' ID
  DUP_PACK   = 0xFFF7  // This was a duplicate (already received before)
} reject_code;


// Helper type for packet_info
typedef union {
  struct {
    sequence_num seq_num;
    payload_len len;
    void* payload;
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


// Build an iovec list out of packet info (to pass to send(), etc.)  
size_t build_iovec_list(packet_info pi, struct iovec* iovec_list);


#endif // PACKET_H
