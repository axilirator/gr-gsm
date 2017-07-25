/* -*- c++ -*- */
/*
 * @file
 * @author Vadim Yanitskiy <axilirator@gmail.com>
 * @section LICENSE
 *
 * Gr-gsm is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3, or (at your option)
 * any later version.
 *
 * Gr-gsm is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with gr-gsm; see the file COPYING.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street,
 * Boston, MA 02110-1301, USA.
 */

#ifndef INCLUDED_GSM_CONTROL_CHANNELS_DECODER_IMPL_H
#define INCLUDED_GSM_CONTROL_CHANNELS_DECODER_IMPL_H

#include <grgsm/decoding/control_channels_decoder.h>

extern "C" {
  #include <osmocom/coding/gsm0503_coding.h>
  #include <osmocom/core/bits.h>
}

#define GSM_BURST_PL_LEN    116
#define GSM_MACBLOCK_LEN    23

namespace gr {
  namespace gsm {

    class control_channels_decoder_impl : public control_channels_decoder
    {
    private:
      /* Burst count (N/4) */
      uint8_t d_burst_cnt;
      /* Burst buffer */
      sbit_t d_burst_buf[GSM_BURST_PL_LEN * 4];
      /* GSMTAP header of first burst from sequence */
      struct gsmtap_hdr *d_header;

      /* Performs GSM 05.03 decoding using libosmocoding */
      int decode(void);

    public:
      control_channels_decoder_impl(void);
      ~control_channels_decoder_impl(void);
      void collect(pmt::pmt_t msg);
    };

  }
}

#endif
