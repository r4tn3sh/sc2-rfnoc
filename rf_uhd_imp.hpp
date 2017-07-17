// #include "rf_uhd_imp.h"
#ifdef __cplusplus

#include <uhd/device3.hpp>
#include <uhd/rfnoc/radio_ctrl.hpp>
#include <uhd/rfnoc/source_block_ctrl_base.hpp>

class RFNoCDevice {
public:
  std::string args_;
  std::string radio_args_;
  size_t radio_id_;
  size_t radio_ss_id_;
  size_t radio_chan_;
  size_t radio_ss_chan_;
  uhd::device3::sptr usrp_;

  uhd::rfnoc::block_id_t radio_ctrl_id_;
  uhd::rfnoc::radio_ctrl::sptr radio_ctrl_;
  uhd::rfnoc::block_id_t radio_ss_ctrl_id_;
  uhd::rfnoc::radio_ctrl::sptr radio_ss_ctrl_;

  //uhd::rfnoc::block_id_t ddc_ctrl_id_; // FIXME: It seem you cannot have two block_id_t
  uhd::rfnoc::source_block_ctrl_base::sptr ddc_ctrl_;
  uhd::rfnoc::source_block_ctrl_base::sptr ddc_ss_ctrl_;
  uhd::rfnoc::source_block_ctrl_base::sptr duc_ctrl_;
  uhd::rfnoc::source_block_ctrl_base::sptr specsense_ctrl_;
  
  uhd::rx_streamer::sptr rx_stream_;
  uhd::rx_streamer::sptr rx_ss_stream_;
  uhd::tx_streamer::sptr tx_stream_;
  uhd::rfnoc::graph::sptr rx_graph_;
  uhd::rfnoc::graph::sptr rx_ss_graph_;
  uhd::rfnoc::graph::sptr tx_graph_;

  char *devname;
  uhd_usrp_handle usrp;
  uhd_rx_streamer_handle rx_stream;
  uhd_tx_streamer_handle tx_stream;

  uhd_rx_metadata_handle rx_md, rx_md_first;
  uhd_tx_metadata_handle tx_md;

  uhd_meta_range_handle rx_gain_range;
  size_t rx_nof_samples;
  size_t tx_nof_samples;
  double tx_rate;
  bool dynamic_rate;
  bool has_rssi;
  uhd_sensor_value_handle rssi_value;

  RFNoCDevice(char *);
  //SRSLTE_API int rf_uhd_open(char *args,                                      // TODO
  //                      void **handler);
  //SRSLTE_API char* rf_uhd_devname(void *h);                                   // nothing to change
  //SRSLTE_API int rf_uhd_close(void *h);                                       // dependent on rf_uhd_stop_rx_stream - done
  //SRSLTE_API void rf_uhd_set_tx_cal(void *h, srslte_rf_cal_t *cal);           // empty, nothing to implement
  //SRSLTE_API void rf_uhd_set_rx_cal(void *h, srslte_rf_cal_t *cal);           // empty, nothing to implement
  //SRSLTE_API int rf_uhd_start_rx_stream(void *h);                             // done
  //SRSLTE_API int rf_uhd_start_rx_stream_nsamples(void *h,                     // not implemented in rf_uhd_imp.c
	//					 uint32_t nsamples);
  //SRSLTE_API int rf_uhd_stop_rx_stream(void *h);                              // done
  //SRSLTE_API void rf_uhd_flush_buffer(void *h);                               // dependent on rf_uhd_recv_with_time - TODO
  //SRSLTE_API bool rf_uhd_has_rssi(void *h);                                   // TODO
  //SRSLTE_API float rf_uhd_get_rssi(void *h);                                  // TODO
  //SRSLTE_API bool rf_uhd_rx_wait_lo_locked(void *h);                          // TODO
  //SRSLTE_API void rf_uhd_set_master_clock_rate(void *h,                       // couldn't find related clock function in rfnoc libraries - TODO
	//				       double rate);
  //SRSLTE_API bool rf_uhd_is_master_clock_dynamic(void *h);                    // couldn't find related clock function in rfnoc libraries - TODO
  //SRSLTE_API double rf_uhd_set_rx_srate(void *h,                              // TODO
	//				double freq);
  //SRSLTE_API double rf_uhd_set_rx_gain(void *h,                               // done
	//			       double gain);
  //SRSLTE_API double rf_uhd_get_rx_gain(void *h);                              // done
  //SRSLTE_API double rf_uhd_get_tx_gain(void *h);                              // done
  //SRSLTE_API void rf_uhd_suppress_stdout(void *h);                            // check rf_uhd_register_msg_handler_c in uhd_c_api.cpp - TODO
  //SRSLTE_API void rf_uhd_register_error_handler(void *h, srslte_rf_error_handler_t error_handler);  // check rf_uhd_register_msg_handler_c in uhd_c_api.cpp - TODO
  //SRSLTE_API double rf_uhd_set_rx_freq(void *h,                               // done
	//			       double freq);
  //SRSLTE_API int rf_uhd_recv_with_time(void *h,                               // TODO
	//			       void *data,
	//			       uint32_t nsamples,
	//			       bool blocking,
	//			       time_t *secs,
	//			       double *frac_secs);
  //SRSLTE_API double rf_uhd_set_tx_srate(void *h,                              // TODO
	//				double freq);
  //SRSLTE_API double rf_uhd_set_tx_gain(void *h,                               // done
	//			       double gain);
  //SRSLTE_API double rf_uhd_set_tx_freq(void *h,                               // done
	//			       double freq);
  //SRSLTE_API void rf_uhd_get_time(void *h,                                    // TODO
	//			  time_t *secs,
	//			  double *frac_secs);
  //SRSLTE_API int  rf_uhd_send_timed(void *h,                                  // TODO
	//			    void *data,
	//			    int nsamples,
	//			    time_t secs,
	//			    double frac_secs,
	//			    bool has_time_spec,
	//			    bool blocking,
	//			    bool is_start_of_burst,
	//			    bool is_end_of_burst);
};
#endif
