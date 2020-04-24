//
// Arduino RPC (Remote Procedure Call) library
// Copyright (c) 2020 OpenMV
// written by Larry Bank
//
// project started April, 2020
//

#ifndef __ARDUINO_RPC__
#define __ARDUINO_RPC__

#include <Arduino.h>
#include <SoftwareSerial.h>

//
// The list of registered callback functions for remote procedure calls
// is limited in length by the amount of available RAM. On AVR targets,
// using a std::map or dictionary library would use too much FLASH and RAM
// so we'll use a fixed size list instead. For non-AVR targets, we can allow
// more simultaneous callbacks.
//
// The local buffer size is a stack variable used to receive data packets.
// As of this first version, receiving image data on an Arduino is not
// supported.
//
#ifdef __AVR__
#define MAX_CALLBACKS 16
#define MAX_LOCAL_BUFFER 32
#else
#define MAX_CALLBACKS 32
#define MAX_LOCAL_BUFFER 256
#endif

// Minimum sized buffer needed to process an empty packet
#define MIN_PACKET_SIZE 8

// The address of the I2C slave. This is an arbitrary value that was chosen
// to not interfere with existing devices.
#define I2C_ADDR 0x12

// Callback function prototype
typedef uint32_t (*RPC_CALLBACK)(uint32_t event, uint8_t *data, uint32_t data_len);

//
// Structure holding the list of function IDs matched with their
// corresponding callback function.
//
typedef struct tagrpclist
{
  RPC_CALLBACK pfnCallback;
  uint32_t id;
} RPCLIST;

class RPC
{
  public:
    bool get_packet(uint16_t magic_value, uint8_t *data, uint32_t data_len, int timeout);
    bool put_packet(uint16_t magic_value, uint8_t *data, uint32_t data_len, int timeout);
    uint16_t crc16(uint8_t *data, uint32_t len);
    bool get_bytes(uint8_t *data, uint32_t len, int timeout) {(void)data; (void)len; (void)timeout; return false;}
    bool put_bytes(uint8_t *data, uint32_t len, int timeout) {(void)data; (void)len; (void)timeout; return false;}

    // Prefixes sent at the beginning of every communication packet
    const uint16_t __COMMAND_HEADER_PACKET_MAGIC = 0x1209;
    const uint16_t __COMMAND_DATA_PACKET_MAGIC = 0xABD1;
    const uint16_t __RESULT_HEADER_PACKET_MAGIC = 0x9021;
    const uint16_t __RESULT_DATA_PACKET_MAGIC = 0x1DBA;
private:
    int _put_short_timeout;
    int _get_short_timeout;
    const int _put_long_timeout = 5000;
    const int _get_long_timeout = 5000;


};

class rpc_master : public RPC
{
  public:
    bool put_command(uint32_t cmd, uint8_t *data, uint32_t data_len, int timeout);
    bool get_result(uint8_t *data, uint32_t *data_len, int timeout);
    bool call(uint32_t rpc_id, uint8_t *out_data, uint32_t out_data_len, uint8_t *in_data, uint32_t *in_data_len, int send_timeout, int recv_timeout);
private:
    const int _put_short_timeout_reset = 3;
    const int _get_short_timeout_reset = 3;

};

class rpc_slave : public RPC
{
  public:
    bool register_callback(uint32_t rpc_id, RPC_CALLBACK pfnCB);
    void schedule_callback(RPC_CALLBACK pfnCB);
    uint32_t get_command(uint8_t *data, uint32_t *data_len);
    RPC_CALLBACK find_callback(uint32_t rpc_id);
    void put_result(uint8_t *data, uint32_t data_len);
    void loop();

  private:
    RPCLIST _rpcList[MAX_CALLBACKS]; // list of registered callbacks
    RPC_CALLBACK _schedule_cb = NULL;
    int _rpc_count; // number of registered callback functions
    const int _put_short_timeout_reset = 2;
    const int _get_short_timeout_reset = 2;
};

//
// Derived classes of rpc for different communication types
//
class rpc_i2c_master : public rpc_master
{
  public:
    rpc_i2c_master(int iAddr, unsigned long speed);
    bool get_bytes(uint8_t *data, uint32_t len, int timeout);
    bool put_bytes(uint8_t *data, uint32_t len, int timeout);

  private:
    int _iAddr;
};

class rpc_i2c_slave : public rpc_slave
{
  public:
    rpc_i2c_slave(int iAddr, unsigned long speed);
    bool get_bytes(uint8_t *data, uint32_t len, int timeout);
    bool put_bytes(uint8_t *data, uint32_t len, int timeout);
    static void receive_event(int len);
    static void request_event(void);
    
    static uint32_t _receive_len;
    static uint32_t _response_len;
    static unsigned char _buf[MAX_LOCAL_BUFFER];
};

class rpc_uart_master : public rpc_master
{
  public:
    rpc_uart_master(unsigned long speed);
    bool get_bytes(uint8_t *data, uint32_t len, int timeout);
    bool put_bytes(uint8_t *data, uint32_t len, int timeout);
};

class rpc_uart_slave : public rpc_slave
{
  public:
    rpc_uart_slave(unsigned long speed);
    bool get_bytes(uint8_t *data, uint32_t len, int timeout);
    bool put_bytes(uint8_t *data, uint32_t len, int timeout);
};

class rpc_softuart_master : public rpc_master
{
  public:
    rpc_softuart_master(int pin1, int pin2, unsigned long speed);
    bool get_bytes(uint8_t *data, uint32_t len, int timeout);
    bool put_bytes(uint8_t *data, uint32_t len, int timeout);

  private:
    SoftwareSerial *_sserial;
};

class rpc_softuart_slave : public rpc_slave
{
  public:
    rpc_softuart_slave(int pin1, int pin2, unsigned long speed);
    bool get_bytes(uint8_t *data, uint32_t len, int timeout);
    bool put_bytes(uint8_t *data, uint32_t len, int timeout);

  private:
    SoftwareSerial *_sserial;
};

class rpc_spi_master : public rpc_master
{
  public:
    rpc_spi_master(unsigned long speed);
    bool get_bytes(uint8_t *data, uint32_t len, int timeout);
    bool put_bytes(uint8_t *data, uint32_t len, int timeout);
    
  private:
    unsigned long _speed;
};

class rpc_spi_slave : public rpc_slave
{
  public:
    rpc_spi_slave(unsigned long speed);
    bool get_bytes(uint8_t *data, uint32_t len, int timeout);
    bool put_bytes(uint8_t *data, uint32_t len, int timeout);

  private:
    unsigned long _speed;
};

#endif // __ARDUINO_RPC__

