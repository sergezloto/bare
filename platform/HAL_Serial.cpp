/*
 *  HAL_Serial.cpp
 *  Embedded
 *
 *  Created by Serge on 09/05/2007.
 *  Copyright 2007 Zorobo. All rights reserved.
 *
 */

#include "HAL_Serial.h"
#include "HAL_VIC.h"
#include "HAL_PinSelect.h"
#include "HAL_Vectors.h"
#include "HAL_Processor.h"

#define USE_FDR 1
/* ------------------------------------------------------------------------------------*/

namespace hal {
  
  /*
   * Line status register
   */
  union LSR {
    struct {
      unsigned receive_data_ready: 1;
      unsigned overrun_error: 1;
      unsigned parity_error: 1;
      unsigned framing_error: 1;
      unsigned break_interrupt: 1;
      unsigned transmitter_holding_register_empty: 1;
      unsigned transmitter_empty: 1;
      unsigned error_on_rx_fifo: 1;
    };
    uint32 value;
  };
  
  union LCR {
    struct {
      unsigned word_length_select: 2;
      unsigned stop_bit_select: 1;
      unsigned parity_enable: 1;
      unsigned parity_select: 2;
      unsigned break_control: 1;
      unsigned divisor_latch_access_bit: 1;
    };
    uint32 value;
  };
  
  union MCR {
    struct {
      unsigned dtr: 1;
      unsigned rts: 1;
    };
    uint32 value;
  };
  
  union MSR {
    struct {
      unsigned delta_cts: 1;
      unsigned delta_dsr: 1;
      unsigned trailing_edge_ring: 1;
      unsigned delta_dcd: 1;
      unsigned cts: 1;
      unsigned dsr: 1;
      unsigned ring: 1;
      unsigned dcd: 1;
    };
    uint32 value;
  };
  
  union FDR {
    struct {
      unsigned div_add_val: 4;
      unsigned mul_val: 4;
    };
    uint32 value;
  };
  
  union FCR {
    struct {
      unsigned enable: 1;
      unsigned rx_reset: 1;
      unsigned tx_reset: 1;
      unsigned : 3; // ignore
      unsigned rx_trigger: 2;  // 0 -> 1 char, 1 -> 4 chars, 2 -> 8 chars, 3 -> 14 chars
    };
    uint32 value;
  };
  
  union IER {
    struct {
      unsigned enable_receive_data_available: 1;
      unsigned enable_thre : 1;
      unsigned enable_rx_line_status: 1;
    };
    uint32 value;
  };
  
  union IIR {
    struct {
      unsigned none_pending: 1;
      unsigned identification: 3;
      unsigned: 3; // ignore
      unsigned fifo_enable: 2;
      unsigned autobaud_success: 1;
      unsigned autobaud_timeout: 1;
    };
    uint32 value;
  };
  
  /**
   * Transmit enable register, for flow control
   */
  union TER {
    struct {
      unsigned: 7; // ignored
      unsigned txen: 1;
    };
    uint32 value;
    enum {
      TRANSMIT = 1 << 7,
      DONT_TRANSMIT = 0
    };
  };
  
  /*
   * Auto bauding register
   */
  union ACR {
    struct {
      unsigned start: 1;
      unsigned mode: 1;
      unsigned auto_restart: 1;
      unsigned: 5; // ignore
      unsigned interrupt_clear: 1;
      unsigned timeout_interrupt_clear: 1;
    };
    uint32 value;
  };
  
  struct uart_register_block {
    union { // 00
      uint8 rbr;
      uint8 thr;
      uint8 dll;
      uint32 pad_0;  // Padding
    };
    union {  // 04
      uint8 dlm;
      IER ier;
    };
    union {  // 08
      IIR iir;
      FCR fcr;
    };
    LCR lcr; // 0C
    MCR mcr; // 10 Only valid for uart1 on lpc21[34]8
    LSR lsr; // 14
    MSR msr; // 18
    union {  // 1C
      uint8 scr;   // Scratch pad register...
      uint32 pad_1;  // Padding
    };
    ACR acr;  // 20
    uint32 pad_2;  // 24
    FDR fdr;  // 28
    uint32 pad_4;  // 2C  // There is a hole in the register file...
    TER ter;  // 30
  };
  
  volatile uart_register_block &regs_uart0 = *(volatile uart_register_block*) 0xe000c000;
  volatile uart_register_block &regs_uart1 = *(volatile uart_register_block*) 0xe0010000;
  
  /**
   * Select the register area given a port
   */
  static volatile uart_register_block& get_register_block(Serial::Port port) {
    switch (port) {
      case Serial::PORT_0:
        return regs_uart0;
      case Serial::PORT_1:
      default:
        return regs_uart1;
    }
  }
  
  void Serial::configure(Baud baud) {
    volatile uart_register_block &regs = get_register_block(m_port);

    // Disable interrupts
    regs.ier.value = 0;
    
    // Disable fifo, resets all fifo
    regs.fcr.value = 0;
    
#if USE_FDR
    // Try all combinations
    uint32 best_div_add_val = 0, best_mul_val = 1;
    uint32 smallest_error = baud;
    uint32 best_divisor = 0;

    for (uint32 div_add_val = 0; div_add_val < 15; ++div_add_val) {
      for (uint32 mul_val = div_add_val + 1; mul_val <= 15; ++mul_val) {
        const uint32 floor_divisor = PCLK / (16 * baud) * mul_val / (mul_val + div_add_val);
        for (uint32 try_divisor = floor_divisor; try_divisor <= floor_divisor + 1; ++try_divisor) {
          const uint32 baud_rate = PCLK / (16 * try_divisor) * mul_val / (mul_val + div_add_val);
          const uint32 error = baud_rate > static_cast<uint32>(baud) ? baud_rate - baud : baud - baud_rate;
          if (error < smallest_error) {
            smallest_error = error;
            best_divisor = try_divisor;
            best_div_add_val = div_add_val;
            best_mul_val = mul_val;
          }
        }
      }
    }
    const uint32 divisor = best_divisor;
#else
    const uint32 divisor = PCLK / (16 * baud);
#endif
    // Configure UART pins
    switch (m_port) {
      case PORT_0:
        PinSelect::enableUART0();
        break;
      case PORT_1:
      default:
        PinSelect::enableUART1();
        break;
    }
    
    // Setup 8 bits, no parity, 1 stop, bitrate
    const uint32 line_control_setup = /* 8 bits */ bit(0)|bit(1);
    regs.lcr.value = line_control_setup | /* dlab access */bit(7);
#if USE_FDR
    regs.fdr.value = best_mul_val << 4 | best_div_add_val;
#endif
    regs.dll = divisor & 0xff;
    regs.dlm = (divisor >> 8) & 0xff;
    regs.lcr.value = line_control_setup;  // dlab = 0, 8N1
    
    // Enable fifo, reset tx and rx
    regs.fcr.value = /* fifo */bit(0) | 
    /* reset rx */bit(1) | 
    /* reset tx */bit(2) | 
    /* rx trigger 14 chars */bit(6)|bit(7);
  }
  
  void Serial::stop() {
    volatile uart_register_block &regs = get_register_block(m_port);
    // Clear interrupts
    regs.ier.value = 0;
    // Clear fifos
    regs.fcr.value = 0;
  }
  
  void Serial::reset() {
    volatile uart_register_block &regs = get_register_block(m_port);
    stop();
    // Reset the fractional bit rate register
    regs.fdr.value = 0;
  }
  
  void Serial::set_auto_bauding(bool on) {
    volatile uart_register_block &regs = get_register_block(m_port);
    ACR acr;
    if (on) {
      acr.value = bit(0) | /*bit(1) | */ bit(2); // start, mode 0 = length of start bit, autorestart
    } else {
      acr.value = 0;
    }
    //
    // Start auto bauding
    //
    regs.acr.value = acr.value;
  }
  
  bool Serial::get_auto_bauding() const {
    volatile const uart_register_block &regs = get_register_block(m_port);
    return regs.acr.start;
  }
  
  
  void Serial::flush() {
    volatile const uart_register_block &regs = get_register_block(m_port);
    switch (m_interrupt_mode) {
    case INTERRUPT_ENABLED:
        while (!(m_send_buffer.is_empty() && regs.lsr.transmitter_empty))
          ;
        break;
      case INTERRUPT_DISABLED:
        while (!regs.lsr.transmitter_empty)
          ;
        break;
    }
  }

  uint32 Serial::write(const uint8* bytes, uint32 size) {
    volatile uart_register_block &regs = get_register_block(m_port);

    uint32 bytes_written = 0;
    
    switch (m_interrupt_mode) {
      case INTERRUPT_DISABLED:
        if (regs.lsr.transmitter_holding_register_empty) {
          uint32 sz = util::min(TX_FIFO_SIZE, size);
          while (sz--) {
            regs.thr = bytes[bytes_written++];
          }          
        }
        break;
      case INTERRUPT_ENABLED:
      {
        //
        // If the send buffer and hw fifo are empty, we can send some bytes right away!
        //
        if (m_send_buffer.is_empty()) {
          if (regs.lsr.transmitter_holding_register_empty) {
            uint32 sz = util::min(TX_FIFO_SIZE, size);
            while (sz--) {
              regs.thr = bytes[bytes_written++];
            }          
          }
        }

        //
        // Stuff bytes into the send buffer
        //
        while (bytes_written < size && !m_send_buffer.is_full()) {
          m_send_buffer.put(bytes[bytes_written++]);  

          //
          // We have stored bytes in the send buffer
          // so prepare for sending data via interrupt
          //
          regs.ier.enable_thre = 1;
        }
      }
        break;
    }
    
    return bytes_written;
  }
  
  uint32 Serial::read(uint8 *bytes, uint32 count) {
    uint32 bytes_read = 0;

    volatile uart_register_block &regs = get_register_block(m_port);
    
    switch (m_interrupt_mode) {
    case INTERRUPT_DISABLED:
      while (regs.lsr.receive_data_ready && bytes_read < count) {
        const uint8 byte = regs.rbr;
        
        //
        // Check for XON/XOFF
        //
        if ( byte == XOFF && m_flow_control == FLOW_CONTROL_XON_OFF ) {
          regs.ter.value = TER::DONT_TRANSMIT;
          continue;
        } else if ( byte == XON && m_flow_control == FLOW_CONTROL_XON_OFF ) {
          regs.ter.value = TER::TRANSMIT;
          continue;
        }
        
        // Append the character
        bytes[bytes_read++] = byte;
      }
      break;
    case INTERRUPT_ENABLED: 
        //
        // In interrupt mode, all reads go through the receive buffer
        //
        while (!m_receive_buffer.is_empty() && bytes_read < count) {
          bytes[bytes_read++] = m_receive_buffer.get();
        }
      break;
    }
    
    return bytes_read;
  }


  void Serial::set_flow_control(FlowControl flow_control) {
    volatile uart_register_block &regs = get_register_block(m_port);

    m_flow_control = flow_control;
    
    if (flow_control == FLOW_CONTROL_NONE) {
      // Make sure we are in the transmit mode!
      regs.ter.value = TER::TRANSMIT;
    }
  }
  
  void Serial::set_interrupt_mode(InterruptMode mode) {
    //
    // Do not do unnecessary work
    //
    if (mode == m_interrupt_mode)
      return;
    
    //
    // Suspend interrupts while fiddling with the IER and VIC vectors
    //
    NoInterrupt disable_interrupts;
    
    volatile uart_register_block &regs = get_register_block(m_port);
    switch (mode) {
      case INTERRUPT_ENABLED:
      {
        //
        // Switching to interrupt mode
        //
        IER ier;
        ier.value = ( /* RBR */ bit(0) 
                     /* | THRE bit(1) */ 
                     /* | RX LSR bit(2) */
                     );
        
        //
        // make the VIC handle it
        //
        switch (m_port) {
          case PORT_0:
            VIC::enable(VIC::UART0, VEC_UART0, *this);
            break;
          case PORT_1:
            VIC::enable(VIC::UART1, VEC_UART1, *this);
            break;
        }
        regs.ier.value = ier.value;
      }
        break;
        
      case INTERRUPT_DISABLED:
        //
        // Non interrupt driven mode. We may lose bytes already in the send or receive buffers
        //
        regs.ier.value = 0;
        
        if (m_port == PORT_0)
          VIC::disable(VIC::UART0, VEC_UART0);
        else
          VIC::disable(VIC::UART1, VEC_UART1);
        
        m_send_buffer.reset();
        m_receive_buffer.reset();
        break;
    }
    
    m_interrupt_mode = mode;
  }
  
  void Serial::handle_irq() {
    volatile uart_register_block &regs = get_register_block(m_port);
    IIR iir; iir.value = regs.iir.value;
    
    if (iir.none_pending) {
      // Why did we get here then?
    } else {
      switch (iir.identification) {
        case 1:
#if SER_DEBUG
          m_nb_write_interrupts++;
#endif
          // transmit register empty. Send as much as we can
          if (regs.lsr.transmitter_holding_register_empty) {
            uint32 max_bytes_sent = TX_FIFO_SIZE;
            while (--max_bytes_sent && !m_send_buffer.is_empty()) {
              regs.thr = m_send_buffer.get();                      
            }            
          }
          //
          // Are we really done?
          //
          if (m_send_buffer.is_empty()) {
            // Yes, we have no business interrupting if we don't have anything to send!
            regs.ier.enable_thre = 0;
          }
          break;
          
        case 2:
          // Receive data available above set threshold
          // Clear by reading RBR
          
          /* FALLTHROUGH */
          
        case 6:
#if SER_DEBUG
          m_nb_read_interrupts++;
#endif
          // Receive character time out interrupt. Bytes are available in the receive fifo
          // Clear by reading RBR
          while (regs.lsr.receive_data_ready) {
            const uint8 byte = regs.rbr;
            
            //
            // Check for XON/XOFF.
            //
            if ( byte == XOFF && m_flow_control == FLOW_CONTROL_XON_OFF ) {
              regs.ter.value = TER::DONT_TRANSMIT;
            } else if ( byte == XON && m_flow_control == FLOW_CONTROL_XON_OFF ) {
              regs.ter.value = TER::TRANSMIT;
            } else if (!m_receive_buffer.is_full()) {
              // TODO: If the receive buffer is at some threshold, send an XOFF
              m_receive_buffer.put(byte);
            } else {
              // TODO: Here the receive buffer is full. We should take note of this overrun
            }
          }
          break;
        
        case 3:
          // Receive line status change
          // Can be overrun, parity, framing errors or break.
          // Clear by reading the LSR
          // TODO: Check LSR and mark status
#if SER_DEBUG
          LSR lsr; lsr.value = regs.lsr.value;  
#endif
          // TODO: MANAGE OVERRUN, PARITY, FRAMING, BREAK!
#if SER_DEBUG
          if (lsr.framing_error)
            m_nb_framing_errors++;
          if (lsr.parity_error)
            m_nb_parity_errors++;
          if (lsr.overrun_error)
            m_nb_overrun_errors++;
          if (lsr.error_on_rx_fifo)
            m_nb_error_on_rx_fifo++;
#endif
          break;

        default:
          break;
      }
    }      
  }
}
