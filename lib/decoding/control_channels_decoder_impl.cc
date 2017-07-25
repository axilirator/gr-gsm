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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <gnuradio/io_signature.h>
#include <grgsm/gsmtap.h>
#include "control_channels_decoder_impl.h"

namespace gr {
  namespace gsm {

    /* The public constructor */
    control_channels_decoder::sptr
    control_channels_decoder::make()
    {
      return gnuradio::get_initial_sptr
        (new control_channels_decoder_impl());
    }

    /* The private constructor */
    control_channels_decoder_impl::control_channels_decoder_impl(void)
    : gr::block("control_channels_decoder",
        gr::io_signature::make(0, 0, 0),
        gr::io_signature::make(0, 0, 0)),
      d_burst_cnt(0)
    {
      /* Register IN / OUT ports */
      message_port_register_in(pmt::mp("bursts"));
      message_port_register_out(pmt::mp("msgs"));

      /* Setup input handler */
      set_msg_handler(pmt::mp("bursts"),
        boost::bind(&control_channels_decoder_impl::collect, this, _1));
    }

    control_channels_decoder_impl::~control_channels_decoder_impl(void)
    {
      /* Do nothing */
    }

    void
    control_channels_decoder_impl::collect(pmt::pmt_t msg)
    {
      pmt::pmt_t blob = pmt::cdr(msg);

      /* Extract bits {0..1} from message */
      uint8_t *burst = (uint8_t *)
        (pmt::blob_data(blob)) + sizeof(gsmtap_hdr);

      /**
       * Copy burst to buffer of 4 bursts,
       * and convert to sbits by the way
       */
      for (int i = 0; i < 58; i++) {
        int offset = d_burst_cnt * GSM_BURST_PL_LEN;
        d_burst_buf[offset + i] = burst[i + 3] ? -127 : 127;
        d_burst_buf[offset + 58 + i] = burst[i + 87] ? -127 : 127;
      }

      /* Increase burst counter */
      d_burst_cnt++;

      /* Store GSMTAP header of first burst */
      if (d_burst_cnt == 1)
        d_header = (gsmtap_hdr *) pmt::blob_data(blob);

      /* If we have all 4/4 bursts collected */
      if (d_burst_cnt == 4) {
        /* Flush counter */
        d_burst_cnt = 0;

        /* Attempt to decode bursts */
        decode();
      }
    }

    int
    control_channels_decoder_impl::decode(void)
    {
      uint8_t l2_msg[GSM_MACBLOCK_LEN];
      int n_errors, n_bits_total, rc;

      /* Attempt to decode stored bursts */
      rc = gsm0503_xcch_decode(l2_msg, d_burst_buf,
        &n_errors, &n_bits_total);
      if (rc)
        return rc;

      /* Compose a new message */
      uint8_t msg[sizeof(gsmtap_hdr) + GSM_MACBLOCK_LEN];

      /* Fill in stored GSMTAP header and the payload */
      memcpy(msg, d_header, sizeof(gsmtap_hdr));
      memcpy(msg + sizeof(gsmtap_hdr), l2_msg, GSM_MACBLOCK_LEN);

      /* Change payload type in header */
      ((gsmtap_hdr*) msg)->type = GSMTAP_TYPE_UM;

      /* Create a pmt blob */
      pmt::pmt_t blob = pmt::make_blob(msg,
        sizeof(gsmtap_hdr) + GSM_MACBLOCK_LEN);
      pmt::pmt_t msg_out = pmt::cons(pmt::PMT_NIL, blob);

      /* Send to output */
      message_port_pub(pmt::mp("msgs"), msg_out);

      return 0;
    }

  }
}
