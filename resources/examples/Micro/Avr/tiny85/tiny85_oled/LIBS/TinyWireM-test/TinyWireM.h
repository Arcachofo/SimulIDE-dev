/*!
 * @file TinyWireM.h
 *
 * TinyWireM.h - a wrapper(+) class for TWI/I2C Master library for the ATtiny on
 *Arduino 1/21/2011 BroHogan -  brohoganx10 at gmail dot com
 *
 * Thanks to 'jkl' for the gcc version of Atmel's USI_TWI_Master code
 * http://www.cs.cmu.edu/~dst/ARTSI/Create/PC%20Comm/
 * I added Atmel's original Device dependant defines section back into
 *USI_TWI_Master.h
 *
 *
 * NOTE! - It's very important to use pullups on the SDA & SCL lines! More so
 *than with the Wire lib.
 *
 * USAGE is modeled after the standard Wire library . . .
 * Put in setup():
 *	TinyWireM.begin(){                               // initialize I2C lib
 * To Send:
 *	TinyWireM.beginTransmission(uint8_t slaveAddr){  // setup slave's
 *address (7 bit address - same as Wire) TinyWireM.send(uint8_t data){ // buffer
 *up bytes to send - can be called multiple times someByte =
 *TinyWireM.endTransmission(){          // actually send the bytes in the buffer
 *	                                                 // returns (optional) 0
 *= sucess or see USI_TWI_Master.h for error codes To Receive: someByte =
 *TinyWireM.requestFrom(uint8_t slaveAddr, uint8_t numBytes){      // reads
 *'numBytes' from slave's address
 *	                                                 // (usage optional)
 *returns 0= success or see USI_TWI_Master.h for error codes someByte =
 *TinyWireM.receive(){                  // returns the next byte in the received
 *buffer - called multiple times someByte = TinyWireM.available(){ // returns
 *the number of unread bytes in the received buffer
 *
 *	TODO:	(by others!)
 *	- merge this class with TinyWireS for master & slave support in one
 *library
 *
 *	This library is free software; you can redistribute it and/or modify it
 *under the terms of the GNU General Public License as published by the Free
 *Software Foundation; either version 2.1 of the License, or any later version.
 * This program is distributed in the hope that it will be useful, but WITHOUT
 *ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 *FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
 *details.
 */

#ifndef TinyWireM_h
#define TinyWireM_h

#include "Arduino.h"
#include <inttypes.h>
#define USI_SEND 0      //!< indicates sending to TWI
#define USI_RCVE 1      //!< indicates receiving from TWI
#define USI_BUF_SIZE 18 //!< bytes in message buffer

/*!
 * @brief class USI_TWI : public Stream
 */
class USI_TWI {
private:
  static uint8_t USI_Buf[];      // holds I2C send and receive data
  static uint8_t USI_BufIdx;     // current number of bytes in the send buff
  static uint8_t USI_LastRead;   // number of bytes read so far
  static uint8_t USI_BytesAvail; // number of bytes requested but not read

public:
  USI_TWI();
  /*!
   * @brief Initialize library for main function
   */
  void begin();
  /*!
   * @brief Setup address & write bit
   * @param slaveAddr Secondary device address
   */
  void beginTransmission(uint8_t);
  /*!
   * @brief Buffers up data to send
   * @param data Data to send
   * @return Returns 1 when successful
   */
  size_t write(uint8_t);
  /*!
   * @brief Sends data
   * @param d Data buffer
   * @param n How much data to send
   * @return Returns 1 when successful
   */
  inline size_t write(uint8_t *d, uint8_t n) {
    uint16_t i;
    for (i = 0; i < n; i++)
      write(d[i]);
    return (size_t)n;
  }
  /*!
   * @brief Sends data
   * @param n Data to send
   * @return Returns 1 when successful
   */
  inline size_t write(unsigned long n) { return write((uint8_t)n); }
  /*!
   * @brief Sends data
   * @param n Data to send
   * @return Returns 1 when successful
   */
  inline size_t write(long n) { return write((uint8_t)n); }
  /*!
   * @brief Sends data
   * @param n Data to send
   * @return Returns 1 when successful
   */
  inline size_t write(unsigned int n) { return write((uint8_t)n); }
  /*!
   * @brief Sends data
   * @param n Data to send
   * @return Returns 1 when successful
   */
  inline size_t write(int n) { return write((uint8_t)n); }
  /*!
   * @brief Buffers up data to send
   * @param b Data to send
   * @return Returns 1 when successful
   */
  void send(uint8_t b) { write(b); }
  /*!
   * @brief Sends data
   * @param d Buffer to send from
   * @param n How much to send from buffer
   * @return Returns 1 when successful
   */
  void send(uint8_t *d, uint8_t n) { write(d, n); }
  /*!
   * @brief Sends data
   * @param n Data to send
   * @return Returns 1 when successful
   */
  void send(int n) { write((uint8_t)n); }
  /*!
   * @brief Ends the transmission
   * @return Returns the error code if there was one
   */
  uint8_t endTransmission();
  /*!
   * @brief Ends the transmission
   * @param stop Buffer to send to end the transmission
   * @return Returns the error code if there was one
   */
  uint8_t endTransmission(uint8_t);
  /*!
   * @brief Setup for receiving from secondary
   * @param slaveAddr Secondary device address
   * @param numBytes How many bytes to request
   * @return Returns 0, if there was an error, returns the error code
   */
  uint8_t requestFrom(uint8_t, uint8_t);
  /*!
   * @brief Returns the bytes received one at a time
   * @return Returns the bytes received one at a time
   */
  int read();
  /*!
   * @brief The bytes available htat haven't been read yet
   * @return Returns how many bytes are available
   */
  int available();
  /*!
   * @brief Returns the top value
   * @return Returns the top value
   */
  int peek(void);
  /*!
   * @brief Flushes the buffer
   */
  void flush(void);
  /*!
   * @brief Receives data from the device
   * @return Returns the data
   */
  uint8_t receive(void) {
    int c = read();
    if (c < 0)
      return 0;
    return c;
  }
  /*!
   * @brief Function for generating a TWI Stop Condition. Used to release
   * the TWI bus
   */
  void end();
};

extern USI_TWI TinyWireM;

#endif
