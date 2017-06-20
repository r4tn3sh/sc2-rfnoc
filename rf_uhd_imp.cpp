#include <uhd.h>
#include <sys/time.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>

#include "srslte/srslte.h"
#include "srslte/rf/rf.h"
#include "uhd_c_api.h"
#include "rf_uhd_imp.h"
#include "rf_uhd_imp.hpp"

//TODO: Not commenting follwing functions result in linker error
//rf_uhd_register_msg_handler_c
//uhd_tx_metadata_set_end
//uhd_tx_metadata_add_time_spec
//uhd_tx_metadata_set_time_spec
//uhd_tx_metadata_set_start
//FIXME : This functions seem to be used in rf_uhd_imp.c as well
//Unsure about the cause of linker error



typedef struct {
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
} rf_uhd_handler_t;

#ifndef __cplusplus
struct RFNoCDevice;
#endif

#ifdef __cplusplus

RFNoCDevice::RFNoCDevice(char* args) {
  args_ = args;
  usrp_ = uhd::device3::make(args_);
  radio_ctrl_id_ = new uhd::rfnoc::block_id_t(0, "Radio", radio_id_);
  // FIXME : radio_ctrl_ seems to be pointer to another pointer, therefore later part of the code needs to be changed accordingly
  radio_ctrl_ = usrp_->get_block_ctrl< uhd::rfnoc::radio_ctrl >(*radio_ctrl_id_);
  radio_ctrl_->set_args(radio_args_);
}

extern "C" 
#endif


void suppress_handler(const char *x)
{
  // do nothing
}

srslte_rf_error_handler_t rfnoc_uhd_error_handler = NULL;

void msg_handler(const char *msg)
{
  srslte_rf_error_t error;
  bzero(&error, sizeof(srslte_rf_error_t));

  if(0 == strcmp(msg, "O")) {
    error.type = srslte_rf_error_t::SRSLTE_RF_ERROR_OVERFLOW;
  } else if(0 == strcmp(msg, "D")) {
    error.type = srslte_rf_error_t::SRSLTE_RF_ERROR_OVERFLOW;
  }else if(0 == strcmp(msg, "U")) {
    error.type = srslte_rf_error_t::SRSLTE_RF_ERROR_UNDERFLOW;
  } else if(0 == strcmp(msg, "L")) {
    error.type = srslte_rf_error_t::SRSLTE_RF_ERROR_LATE;
  }
  if (rfnoc_uhd_error_handler) {
    rfnoc_uhd_error_handler(error);
  }
}

#ifdef __cplusplus
extern "C" 
#endif
void rf_uhd_suppress_stdout(void *h) {
  rf_uhd_register_msg_handler_c(suppress_handler);
}

#ifdef __cplusplus
extern "C" 
#endif
void rf_uhd_register_error_handler(void *notused, srslte_rf_error_handler_t new_handler)
{
  rfnoc_uhd_error_handler = new_handler;
  rf_uhd_register_msg_handler_c(msg_handler);
}

static bool find_string(uhd_string_vector_handle h, const char *str)
{
  char buff[128];
  size_t n;
  uhd_string_vector_size(h, &n);
  for (size_t i=0;i<n;i++) {
    uhd_string_vector_at(h, i, buff, 128);
    if (strstr(buff, str)) {
      return true;
    }
  }
  return false;
}

static bool isLocked(rf_uhd_handler_t *handler, char *sensor_name, uhd_sensor_value_handle *value_h)
{
  bool val_out = false;

  if (sensor_name) {
    uhd_usrp_get_rx_sensor(handler->usrp, sensor_name, 0, value_h);
    uhd_sensor_value_to_bool(*value_h, &val_out);
  } else {
    usleep(500);
    val_out = true;
  }

  return val_out;
}

#ifdef __cplusplus
extern "C" 
#endif
char* rf_uhd_devname(void* h)
{
  //rf_uhd_handler_t *handler = (rf_uhd_handler_t*) h;
  RFNoCDevice *handler = (RFNoCDevice*) h;
  return handler->devname;
}

#ifdef __cplusplus
extern "C" 
#endif
bool rf_uhd_rx_wait_lo_locked(void *h)
{
  RFNoCDevice *handler = (RFNoCDevice*) h;

  uhd_string_vector_handle mb_sensors;
  uhd_string_vector_handle rx_sensors;
  char *sensor_name;
  uhd_sensor_value_handle value_h;
  uhd_string_vector_make(&mb_sensors);
  uhd_string_vector_make(&rx_sensors);
  uhd_sensor_value_make_from_bool(&value_h, "", true, "True", "False");
  uhd_usrp_get_mboard_sensor_names(handler->usrp, 0, &mb_sensors);
  uhd_usrp_get_rx_sensor_names(handler->usrp, 0, &rx_sensors);

  if (find_string(rx_sensors, "lo_locked")) {
    sensor_name = (char *) "lo_locked";
  } else if (find_string(mb_sensors, "ref_locked")) {
    sensor_name = (char *) "ref_locked";
  } else {
    sensor_name = NULL;
  }

  double report = 0.0;
  while (!isLocked((rf_uhd_handler_t *)handler, sensor_name, &value_h) && report < 30.0) {
    report += 0.1;
    usleep(1000);
  }

  bool val = isLocked((rf_uhd_handler_t *)handler, sensor_name, &value_h);

  uhd_string_vector_free(&mb_sensors);
  uhd_string_vector_free(&rx_sensors);
  uhd_sensor_value_free(&value_h);

  return val;
}

void rf_uhd_set_tx_cal(void *h, srslte_rf_cal_t *cal)
{

}

void rf_uhd_set_rx_cal(void *h, srslte_rf_cal_t *cal)
{

}


#ifdef __cplusplus
extern "C" 
#endif
int rf_uhd_start_rx_stream(void *h)
{
  RFNoCDevice *handler = (RFNoCDevice*) h;
  //uhd_stream_cmd_t stream_cmd = {
  //      .stream_mode = UHD_STREAM_MODE_START_CONTINUOUS,
  //      .stream_now = true
  //};
  uhd::stream_cmd_t stream_cmd(uhd::stream_cmd_t::STREAM_MODE_START_CONTINUOUS);
  // stream_cmd.num_samps = size_t(num_requested_samples);
  // stream_cmd.num_samps = size_t(1000); //FIXME : used a constant value !!!
  stream_cmd.stream_now = true;
  stream_cmd.time_spec = uhd::time_spec_t();
  //uhd_rx_streamer_issue_stream_cmd(handler->rx_stream, &stream_cmd);
  handler->rx_stream_->issue_stream_cmd(stream_cmd);
  return 0;
}

#ifdef __cplusplus
extern "C" 
#endif
int rf_uhd_stop_rx_stream(void *h)
{
  RFNoCDevice *handler = (RFNoCDevice*) h;
  //uhd_stream_cmd_t stream_cmd = {
  //      .stream_mode = UHD_STREAM_MODE_STOP_CONTINUOUS,
  //      .stream_now = true
  //};
  uhd::stream_cmd_t stream_cmd(uhd::stream_cmd_t::STREAM_MODE_STOP_CONTINUOUS);
  //uhd_rx_streamer_issue_stream_cmd(handler->rx_stream, &stream_cmd);
  handler->rx_stream_->issue_stream_cmd(stream_cmd);
  return 0;
}

void rf_uhd_flush_buffer(void *h)
{
  int n;
  cf_t tmp[1024];
  do {
    n = rf_uhd_recv_with_time(h, tmp, 1024, 0, NULL, NULL);
  } while (n > 0);
}

#ifdef __cplusplus
extern "C" 
#endif
bool rf_uhd_has_rssi(void *h) {
  RFNoCDevice *handler = (RFNoCDevice*) h;
  return handler->has_rssi;
}

#ifdef __cplusplus
extern "C" 
#endif
bool get_has_rssi(void *h) {
  RFNoCDevice *handler = (RFNoCDevice*) h;
  uhd_string_vector_handle rx_sensors;
  uhd_string_vector_make(&rx_sensors);
  uhd_usrp_get_rx_sensor_names(handler->usrp, 0, &rx_sensors);//FIXME:Error in this
  bool ret = find_string(rx_sensors, "rssi");
  uhd_string_vector_free(&rx_sensors);
  return ret;
}

#ifdef __cplusplus
extern "C" 
#endif
float rf_uhd_get_rssi(void *h) {
  RFNoCDevice *handler = (RFNoCDevice*) h;
  if (handler->has_rssi) {
    double val_out;
    uhd_usrp_get_rx_sensor(handler->usrp, "rssi", 0, &handler->rssi_value);
    uhd_sensor_value_to_realnum(handler->rssi_value, &val_out);
    return val_out;
  } else {
    return 0.0;
  }
}

#ifdef __cplusplus
extern "C" 
#endif
int rf_uhd_open(char *args, void **h)
{
  printf("Opening USRP......\n");// << std::endl;
  if (h) {
    *h = NULL;

    //rf_uhd_handler_t *handler = (rf_uhd_handler_t*) malloc(sizeof(rf_uhd_handler_t));
    RFNoCDevice *handler = new RFNoCDevice(args);
    if (!handler) {
      perror("malloc");
      return -1;
    }
    *h = handler;

    /* Set priority to UHD threads */
    //uhd_set_thread_priority(uhd_default_thread_priority, true);

    /* Find available devices */
    //uhd_string_vector_handle devices_str;
    //uhd_string_vector_make(&devices_str);
    //uhd_usrp_find("", &devices_str);

    //char args2[512];

    //handler->dynamic_rate = true;

    // Allow NULL parameter
    //if (args == NULL) {
    //  args = "";
    //}
    handler->devname = NULL;
    // uhd::rfnoc::graph::sptr rx_graph = usrp->create_graph("srslte_rfnoc_rx"); //XXX: replaced by following line by Ratnesh
    uhd::rfnoc::graph::sptr rx_graph = handler->usrp_->create_graph("srslte_rfnoc_rx");

    /* If device type or name not given in args, choose a B200 */
    /*
    if (args[0]=='\0') {
      if (find_string(devices_str, "type=x300") && !strstr(args, "recv_frame_size")) {
        // If B200 is available, use it
        args = "type=b200";
        handler->devname = DEVNAME_B200;
      } else if (find_string(devices_str, "type=x300")) {
        // Else if X300 is available, set master clock rate now (with x310 it can't be changed later)
        args = "type=x300,master_clock_rate=184.32e6";// If no argument is passed we set master clock rate to 184.32 MHz in order to use standard LTE rates, i.e., carrier sepration of 15 KHz.
        //args = "type=x300,master_clock_rate=200e6"; // In case we need longer CP periods, we should set master clock rate to 200 MHz in order to be able to set the sampling rate to 5 MHz and have bigger CPs.
        handler->dynamic_rate = false;
        handler->devname = DEVNAME_X300;
      }
    } else {
      // If args is set and x300 type is specified, make sure master_clock_rate is defined
      if (strstr(args, "type=x300") && !strstr(args, "master_clock_rate")) {
        sprintf(args2, "%s,master_clock_rate=184.32e6",args);
        args = args2;
        handler->dynamic_rate = false;
        handler->devname = DEVNAME_X300;
      } else if (strstr(args, "type=b200")) {
        handler->devname = DEVNAME_B200;
      }
    }
    */

    /* Create UHD handler */
    printf("Opening USRP with args: %s\n", handler->args_.c_str());//XXX: replaced by following line by Ratnesh
    // printf("Opening USRP with args: %s\n", args);
    //uhd_error error = uhd_usrp_make(&handler->usrp, args);
    /*
    if (error) {
      fprintf(stderr, "Error opening UHD: code %d\n", error);
      return -1;
    }
    */

    /*
    if (!handler->devname) {
      char dev_str[1024];
      uhd_usrp_get_mboard_name(handler->usrp, 0, dev_str, 1024);
      if (strstr(dev_str, "B2") || strstr(dev_str, "B2")) {
        handler->devname = DEVNAME_B200;
      } else if (strstr(dev_str, "X3") || strstr(dev_str, "X3")) {
        handler->devname = DEVNAME_X300;
      }
    }
    if (!handler->devname) {
      handler->devname = "uhd_unknown";
    }
    */
    handler->devname = (char *) DEVNAME_X300;
    size_t channel = 0;
    uhd_stream_args_t stream_args = {
      .cpu_format = (char *) "fc32",
      .otw_format = (char *) "sc16",
      .args = (char *) "",
      .channel_list = &channel,
      .n_channels = 1
    };

    // Set external clock reference
    /*
    if (strstr(args, "clock=external")) {
      uhd_usrp_set_clock_source(handler->usrp, "external", 0);
    }
    */

    std::cout<<"Handler created, check rssi"<<std::endl;
    handler->has_rssi = get_has_rssi(handler);
    if (handler->has_rssi) {
      uhd_sensor_value_make_from_realnum(&handler->rssi_value, "rssi", 0, "dBm", "%f");
    }

    /* Initialize rx and tx stremers */
    std::cout<<"Creating streamers"<<std::endl;
    uhd_rx_streamer_make(&handler->rx_stream);
    uhd_error error = uhd_usrp_get_rx_stream(handler->usrp, &stream_args, handler->rx_stream);
    if (error) {
      fprintf(stderr, "Error opening RX stream: %d\n", error);
      return -1;
    }
    uhd_tx_streamer_make(&handler->tx_stream);
    error = uhd_usrp_get_tx_stream(handler->usrp, &stream_args, handler->tx_stream);
    if (error) {
      fprintf(stderr, "Error opening TX stream: %d\n", error);
      return -1;
    }
    std::cout<<"Created streamers"<<std::endl;

    uhd_rx_streamer_max_num_samps(handler->rx_stream, &handler->rx_nof_samples);
    uhd_tx_streamer_max_num_samps(handler->tx_stream, &handler->tx_nof_samples);

    uhd_meta_range_make(&handler->rx_gain_range);
    uhd_usrp_get_rx_gain_range(handler->usrp, "", 0, handler->rx_gain_range);

    // Make metadata objects for RX/TX
    uhd_rx_metadata_make(&handler->rx_md);
    uhd_rx_metadata_make(&handler->rx_md_first);
    uhd_tx_metadata_make(&handler->tx_md, false, 0, 0, false, false);

    return 0;
  } else {
    return SRSLTE_ERROR_INVALID_INPUTS;
  }
}


#ifdef __cplusplus
extern "C" 
#endif
int rf_uhd_close(void *h)
{
  rf_uhd_stop_rx_stream(h);

  /** Something else to close the USRP?? */
  return 0;
}

#ifdef __cplusplus
extern "C" 
#endif
void rf_uhd_set_master_clock_rate(void *h, double rate) {
  RFNoCDevice *handler = (RFNoCDevice*) h;
  if (handler->dynamic_rate) {
    uhd_usrp_set_master_clock_rate(handler->usrp, rate, 0);
  }
}

#ifdef __cplusplus
extern "C" 
#endif
bool rf_uhd_is_master_clock_dynamic(void *h) {
  RFNoCDevice *handler = (RFNoCDevice*) h;
  return handler->dynamic_rate;
}

#ifdef __cplusplus
extern "C" 
#endif
double rf_uhd_set_rx_srate(void *h, double freq)
{
  printf("rf_uhd_set_rx_srate - freq: %f\n",freq);
  RFNoCDevice *handler = (RFNoCDevice*) h;
  uhd_usrp_set_rx_rate(handler->usrp, freq, 0);
  uhd_usrp_get_rx_rate(handler->usrp, 0, &freq);
  return freq;
}

#ifdef __cplusplus
extern "C" 
#endif
double rf_uhd_set_tx_srate(void *h, double freq)
{
  RFNoCDevice *handler = (RFNoCDevice*) h;
  uhd_usrp_set_tx_rate(handler->usrp, freq, 0);
  uhd_usrp_get_tx_rate(handler->usrp, 0, &freq);
  handler->tx_rate = freq;
  return freq;
}

#ifdef __cplusplus
extern "C" 
#endif
double rf_uhd_set_rx_gain(void *h, double gain)
{
  RFNoCDevice *handler = (RFNoCDevice*) h;
  //uhd_usrp_set_rx_gain(handler->usrp, gain, 0, "");
  //uhd_usrp_get_rx_gain(handler->usrp, 0, "", &gain);
  // XXX : Following part of the code needed to be modified becase of 
  // radio_ctrl_ being pointer of pointer
  handler->radio_ctrl_->set_rx_gain(gain, handler->radio_chan_);
  gain = handler->radio_ctrl_->get_rx_gain(handler->radio_chan_);
  return gain;
}

#ifdef __cplusplus
extern "C" 
#endif
double rf_uhd_set_tx_gain(void *h, double gain)
{
  RFNoCDevice *handler = (RFNoCDevice*) h;
  //uhd_usrp_set_tx_gain(handler->usrp, gain, 0, "");
  //uhd_usrp_get_tx_gain(handler->usrp, 0, "", &gain);
  handler->radio_ctrl_->set_tx_gain(gain, handler->radio_chan_);
  gain = handler->radio_ctrl_->get_tx_gain(handler->radio_chan_);
  return gain;
}

#ifdef __cplusplus
extern "C" 
#endif
double rf_uhd_get_rx_gain(void *h)
{
  RFNoCDevice *handler = (RFNoCDevice*) h;
  double gain;
  //uhd_usrp_get_rx_gain(handler->usrp, 0, "", &gain);
  gain = handler->radio_ctrl_->get_rx_gain(handler->radio_chan_);
  return gain;
}

#ifdef __cplusplus
extern "C" 
#endif
double rf_uhd_get_tx_gain(void *h)
{
  RFNoCDevice *handler = (RFNoCDevice*) h;
  double gain;
  //uhd_usrp_get_tx_gain(handler->usrp, 0, "", &gain);
  gain = handler->radio_ctrl_->get_tx_gain(handler->radio_chan_);
  return gain;
}

#ifdef __cplusplus
extern "C" 
#endif
double rf_uhd_set_rx_freq(void *h, double freq)
{
  //uhd_tune_request_t tune_request = {
  //    .target_freq = freq,
  //    .rf_freq_policy = UHD_TUNE_REQUEST_POLICY_AUTO,
  //    .dsp_freq_policy = UHD_TUNE_REQUEST_POLICY_AUTO,
  //};
  //uhd_tune_result_t tune_result;
  RFNoCDevice *handler = (RFNoCDevice*) h;
  //uhd_usrp_set_rx_freq(handler->usrp, &tune_request, 0, &tune_result);
  //uhd_usrp_get_rx_freq(handler->usrp, 0, &freq);
  handler->radio_ctrl_->set_rx_frequency(freq, handler->radio_chan_);
  freq = handler->radio_ctrl_->get_rx_frequency(handler->radio_chan_);
  return freq;
}

#ifdef __cplusplus
extern "C" 
#endif
double rf_uhd_set_tx_freq(void *h, double freq)
{
  //uhd_tune_request_t tune_request = {
  //    .target_freq = freq,
  //    .rf_freq_policy = UHD_TUNE_REQUEST_POLICY_AUTO,
  //    .dsp_freq_policy = UHD_TUNE_REQUEST_POLICY_AUTO,
  //};
  //uhd_tune_result_t tune_result;
  RFNoCDevice *handler = (RFNoCDevice*) h;
  //uhd_usrp_set_tx_freq(handler->usrp, &tune_request, 0, &tune_result);
  //uhd_usrp_get_tx_freq(handler->usrp, 0, &freq);
  handler->radio_ctrl_->set_tx_frequency(freq, handler->radio_chan_);
  freq = handler->radio_ctrl_->get_tx_frequency(handler->radio_chan_);
  return freq;
}


#ifdef __cplusplus
extern "C" 
#endif
void rf_uhd_get_time(void *h, time_t *secs, double *frac_secs) {
  RFNoCDevice *handler = (RFNoCDevice*) h;
  uhd_usrp_get_time_now(handler->usrp, 0, secs, frac_secs);
}

#ifdef __cplusplus
extern "C" 
#endif
int rf_uhd_recv_with_time(void *h,
                    void *data,
                    uint32_t nsamples,
                    bool blocking,
                    time_t *secs,
                    double *frac_secs)
{

  RFNoCDevice *handler = (RFNoCDevice*) h;
  size_t rxd_samples;
  uhd_rx_metadata_handle *md = &handler->rx_md_first;
  int trials = 0;
  if (blocking) {
    unsigned int n = 0;
    cf_t *data_c = (cf_t*) data;
    do {
      size_t rx_samples = handler->rx_nof_samples;

      if (rx_samples > nsamples - n) {
        rx_samples = nsamples - n;
      }
      void *buff = (void*) &data_c[n];
      void **buffs_ptr = (void**) &buff;
      uhd_error error = uhd_rx_streamer_recv(handler->rx_stream, buffs_ptr,
                                             rx_samples, md, 5.0, false, &rxd_samples);

      if (error) {
        fprintf(stderr, "Error receiving from UHD: %d\n", error);
        return -1;
      }
      md = &handler->rx_md;
      n += rxd_samples;
      trials++;
    } while (n < nsamples && trials < 100);
  } else {
    void **buffs_ptr = (void**) &data;
    return uhd_rx_streamer_recv(handler->rx_stream, buffs_ptr,
                                             nsamples, md, 0.0, false, &rxd_samples);
  }
  if (secs && frac_secs) {
    uhd_rx_metadata_time_spec(handler->rx_md_first, secs, frac_secs);
  }
  return nsamples;
}

#ifdef __cplusplus
extern "C" 
#endif
int rf_uhd_send_timed(void *h,
                     void *data,
                     int nsamples,
                     time_t secs,
                     double frac_secs,
                     bool has_time_spec,
                     bool blocking,
                     bool is_start_of_burst,
                     bool is_end_of_burst)
{
  rf_uhd_handler_t* handler = (rf_uhd_handler_t*) h;

  size_t txd_samples;
  if (has_time_spec) {
    uhd_tx_metadata_set_time_spec(&handler->tx_md, secs, frac_secs);
  }
  int trials = 0;
  if (blocking) {
    unsigned int n = 0;
    cf_t *data_c = (cf_t*) data;
    do {
      size_t tx_samples = handler->tx_nof_samples;

      // First packet is start of burst if so defined, others are never
      if (n == 0) {
        uhd_tx_metadata_set_start(&handler->tx_md, is_start_of_burst);
      } else {
        uhd_tx_metadata_set_start(&handler->tx_md, false);
      }

      // middle packets are never end of burst, last one as defined
      if (nsamples - n > tx_samples) {
        uhd_tx_metadata_set_end(&handler->tx_md, false);
      } else {
        tx_samples = nsamples - n;
        uhd_tx_metadata_set_end(&handler->tx_md, is_end_of_burst);
      }

      void *buff = (void*) &data_c[n];
      const void **buffs_ptr = (const void**) &buff;
      uhd_error error = uhd_tx_streamer_send(handler->tx_stream, buffs_ptr,
                                             tx_samples, &handler->tx_md, 3.0, &txd_samples);
      if (error) {
        fprintf(stderr, "Error sending to UHD: %d\n", error);
        return -1;
      }
      // Increase time spec
      uhd_tx_metadata_add_time_spec(&handler->tx_md, txd_samples/handler->tx_rate);
      n += txd_samples;
      trials++;
    } while (n < nsamples && trials < 100);
    return nsamples;
  } else {
    const void **buffs_ptr = (const void**) &data;
    uhd_tx_metadata_set_start(&handler->tx_md, is_start_of_burst);
    uhd_tx_metadata_set_end(&handler->tx_md, is_end_of_burst);
    return uhd_tx_streamer_send(handler->tx_stream, buffs_ptr, nsamples, &handler->tx_md, 0.0, &txd_samples);
  }
}
