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
  args_ = strcat(args,",master_clock_rate=184.32e6");
  radio_id_ = 0;
  radio_ss_id_ = 1;
  // FIXME : is it a right idea to make the device3 object in the 
  // constructor. We are creating multiple object of this class
  // usrp_ = uhd::device3::make(args_);
  //
  // // FIXME: at this point uhd_usrp_make would have assigned a usrp_index to uhd_usrp_handle
  // // Therefor following line is added assuming only one USRP is used
  // usrp->usrp_index = 0;

  // radio_ctrl_id_ = uhd::rfnoc::block_id_t(0, "Radio", radio_id_);
  // radio_ctrl_ = usrp_->get_block_ctrl< uhd::rfnoc::radio_ctrl >(radio_ctrl_id_);
  // radio_ctrl_->set_args(radio_args_);
}

extern "C" 
#endif


void suppress_handler(const char *x)
{
  // do nothing
}

// Added by zz4fap
// TODO : Enable support for error log
// static void log_overflow(rf_uhd_handler_t *h) {
//   if (h->uhd_error_handler) {
//     srslte_rf_error_t error;
//     bzero(&error, sizeof(srslte_rf_error_t));
//     error.type = SRSLTE_RF_ERROR_OVERFLOW;
//     h->uhd_error_handler(error);
//   }
// }
// 
// static void log_late(rf_uhd_handler_t *h) {
//   if (h->uhd_error_handler) {
//     srslte_rf_error_t error;
//     bzero(&error, sizeof(srslte_rf_error_t));
//     error.type = SRSLTE_RF_ERROR_LATE;
//     h->uhd_error_handler(error);
//   }
// }
// 
// static void log_underflow(rf_uhd_handler_t *h) {
//   if (h->uhd_error_handler) {
//     srslte_rf_error_t error;
//     bzero(&error, sizeof(srslte_rf_error_t));
//     error.type = SRSLTE_RF_ERROR_UNDERFLOW;
//     h->uhd_error_handler(error);
//   }
// }



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

// FIXME : Following may not be useful since device3->radio_ctrl
// operations do not return uhd_error
// char* print_uhd_error(uhd_error error) {
// 
//   char* error_string = NULL;
// 
//   switch(error) {
//     case UHD_ERROR_NONE:
//       error_string = "UHD_ERROR_NONE";
//       break;
//     case UHD_ERROR_INVALID_DEVICE:
//       error_string = "UHD_ERROR_INVALID_DEVICE";
//       break;
//     case UHD_ERROR_INDEX:
//       error_string = "UHD_ERROR_INDEX";
//       break;
//     case UHD_ERROR_KEY:
//       error_string = "UHD_ERROR_KEY";
//       break;
//     case UHD_ERROR_NOT_IMPLEMENTED:
//       error_string = "UHD_ERROR_NOT_IMPLEMENTED";
//       break;
//     case UHD_ERROR_USB:
//       error_string = "UHD_ERROR_USB";
//       break;
//     case UHD_ERROR_IO:
//       error_string = "UHD_ERROR_IO";
//       break;
//     case UHD_ERROR_OS:
//       error_string = "UHD_ERROR_OS";
//       break;
//     case UHD_ERROR_ASSERTION:
//       error_string = "UHD_ERROR_ASSERTION";
//       break;
//     case UHD_ERROR_LOOKUP:
//       error_string = "UHD_ERROR_LOOKUP";
//       break;
//     case UHD_ERROR_TYPE:
//       error_string = "UHD_ERROR_TYPE";
//       break;
//     case UHD_ERROR_VALUE:
//       error_string = "UHD_ERROR_VALUE";
//       break;
//     case UHD_ERROR_RUNTIME:
//       error_string = "UHD_ERROR_RUNTIME";
//       break;
//     case UHD_ERROR_ENVIRONMENT:
//       error_string = "UHD_ERROR_ENVIRONMENT";
//       break;
//     case UHD_ERROR_SYSTEM:
//       error_string = "UHD_ERROR_SYSTEM";
//       break;
//     case UHD_ERROR_EXCEPT:
//       error_string = "UHD_ERROR_EXCEPT";
//       break;
//     case UHD_ERROR_BOOSTEXCEPT:
//       error_string = "UHD_ERROR_BOOSTEXCEPT";
//       break;
//     case UHD_ERROR_STDEXCEPT:
//       error_string = "UHD_ERROR_STDEXCEPT";
//       break;
//     case UHD_ERROR_UNKNOWN:
//       error_string = "UHD_ERROR_UNKNOWN";
//       break;
//     default:
//       error_string = "Unknown error code.....";
//   }
//   return error_string;
// }


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
//FIXME : r4tn3sh : following function relies heavily on uhd_usrp_handler
//for reading the information about sensors.
bool rf_uhd_rx_wait_lo_locked(void *h)
{
  // RFNoCDevice *handler = (RFNoCDevice*) h;

  // uhd_string_vector_handle mb_sensors;
  // uhd_string_vector_handle rx_sensors;
  // char *sensor_name;
  // uhd_sensor_value_handle value_h;
  // uhd_string_vector_make(&mb_sensors);
  // uhd_string_vector_make(&rx_sensors);
  // uhd_sensor_value_make_from_bool(&value_h, "", true, "True", "False");
  // uhd_usrp_get_mboard_sensor_names(handler->usrp, 0, &mb_sensors);
  // uhd_usrp_get_rx_sensor_names(handler->usrp, 0, &rx_sensors);

  // if (find_string(rx_sensors, "lo_locked")) {
  //   sensor_name = (char *) "lo_locked";
  // } else if (find_string(mb_sensors, "ref_locked")) {
  //   sensor_name = (char *) "ref_locked";
  // } else {
  //   sensor_name = NULL;
  // }

  // double report = 0.0;
  // while (!isLocked((rf_uhd_handler_t *)handler, sensor_name, &value_h) && report < 30.0) {
  //   report += 0.1;
  //   usleep(1000);
  // }

  // bool val = isLocked((rf_uhd_handler_t *)handler, sensor_name, &value_h);

  // uhd_string_vector_free(&mb_sensors);
  // uhd_string_vector_free(&rx_sensors);
  // uhd_sensor_value_free(&value_h);

  // TODO: r4tn3sh : find correct solution
  bool val = true;
  usleep(1000);
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
  printf("Flushing begins \n");
  do {
    n = rf_uhd_recv_with_time(h, tmp, 1024, 0, NULL, NULL);
    printf("n=%d\n",n);
  } while (n > 0);
}

#ifdef __cplusplus
extern "C" 
#endif
bool rf_uhd_has_rssi(void *h) {
  //RFNoCDevice *handler = (RFNoCDevice*) h;
  //return handler->has_rssi;
  return false;
}

#ifdef __cplusplus
extern "C" 
#endif
bool get_has_rssi(void *h) {
  //RFNoCDevice *handler = (RFNoCDevice*) h;
  //uhd_string_vector_handle rx_sensors;
  //uhd_string_vector_make(&rx_sensors);
  //std::cout << "Getting sensor name "<<handler->usrp << " " << &rx_sensors<<std::endl;
  //uhd_usrp_get_rx_sensor_names(handler->usrp, 0, &rx_sensors);//FIXME:Error in this
  //bool ret = find_string(rx_sensors, "rssi");
  //uhd_string_vector_free(&rx_sensors);
  //return ret;
  return false;
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
    uhd::device_addr_t rx_stream_args_args;
    uhd::device_addr_t rx_ss_stream_args_args;
    uhd::device_addr_t tx_stream_args_args;
    // ------------------------------------------------------------
    // XXX : r4tn3sh : moved the USRP object creation here, in order to 
    // avoid creating multiple usrp objects from constructor 
    std::cout << boost::format("<---------Opening USRP with args: %s")% handler->args_.c_str()<< std::endl;
    handler->usrp_ = uhd::device3::make(handler->args_);
    handler->radio_chan_ = 0;
    handler->radio_ss_chan_ = 1;
    handler->radio_ctrl_id_ = uhd::rfnoc::block_id_t(0, "Radio", handler->radio_id_);
    handler->radio_ctrl_ = handler->usrp_->get_block_ctrl< uhd::rfnoc::radio_ctrl >(handler->radio_ctrl_id_);
    handler->radio_ctrl_->set_args(handler->radio_args_);//TODO:radio_args is empty now
    handler->radio_ctrl_->set_rate(184320000);
    std::cout << boost::format("<---------Opening USRP with args: %s is successful, rate = %f")% handler->args_.c_str() % handler->radio_ctrl_->get_rate()<< std::endl;

    handler->rx_graph_ = handler->usrp_->create_graph("srslte_rfnoc_rx");
    handler->rx_ss_graph_ = handler->usrp_->create_graph("specsense_rfnoc_rx");
    handler->tx_graph_ = handler->usrp_->create_graph("srslte_rfnoc_tx");

    // ------------------------------------------------------------
    // XXX : r4tn3sh : DDC and other rfnoc blocks are configured here
    // NOTE: list of all args for rfnoc blocks can be found in xml file at uhd/host/include/uhd/rfnoc/blocks/

    // handler->ddc_ctrl_id_ = uhd::rfnoc::block_id_t(0, "DDC", 0);
    std::string ddc_id("0/DDC_0");
    std::string ddc_ss_id("0/DDC_1");
    std::string duc_id("0/DUC_0");

    handler->ddc_ctrl_ = handler->usrp_->get_block_ctrl<uhd::rfnoc::source_block_ctrl_base>(ddc_id);
    handler->ddc_ss_ctrl_ = handler->usrp_->get_block_ctrl<uhd::rfnoc::source_block_ctrl_base>(ddc_ss_id);
    handler->duc_ctrl_ = handler->usrp_->get_block_ctrl<uhd::rfnoc::source_block_ctrl_base>(duc_id);

    std::string ddc_args = "input_rate=184320000.0,output_rate=30720000.0";
    handler->ddc_ctrl_->set_args(uhd::device_addr_t(ddc_args));

    std::string ddc_ss_args = "input_rate=184320000.0,output_rate=92160000.0";
    handler->ddc_ss_ctrl_->set_args(uhd::device_addr_t(ddc_ss_args));

    std::string duc_args = "input_rate=30720000.0,output_rate=184320000.0";
    handler->duc_ctrl_->set_args(uhd::device_addr_t(duc_args));

    // ------------------------------------------------------------
    // XXX : r4tn3sh : spec_sense
    std::string specsense_id("0/SpecSense_0");
    handler->specsense_ctrl_ = handler->usrp_->get_block_ctrl<uhd::rfnoc::source_block_ctrl_base>(specsense_id);
    handler->radio_ss_ctrl_id_ = uhd::rfnoc::block_id_t(0, "Radio", handler->radio_ss_id_);
    handler->radio_ss_ctrl_ = handler->usrp_->get_block_ctrl< uhd::rfnoc::radio_ctrl >(handler->radio_ss_ctrl_id_);

    // ------------------------------------------------------------
    std::cout << "Connecting " << handler->radio_ctrl_id_ << " ==> " << handler->ddc_ctrl_->get_block_id() << std::endl;    
    handler->rx_graph_->connect(handler->radio_ctrl_id_, handler->radio_chan_, handler->ddc_ctrl_->get_block_id(), uhd::rfnoc::ANY_PORT);
    rx_stream_args_args["block_id"] = handler->ddc_ctrl_->get_block_id().to_string();//FIXME:r4tn3sh: currently DDC is the endpoint
    std::cout << "<---------Using DDC ID : " << handler->ddc_ctrl_->get_block_id().to_string() << std::endl;

    std::cout << "Connecting " << handler->radio_ss_ctrl_id_ << " ==> " << handler->ddc_ss_ctrl_->get_block_id() << std::endl;    
    handler->rx_ss_graph_->connect(handler->radio_ss_ctrl_id_, handler->radio_chan_, handler->ddc_ss_ctrl_->get_block_id(), uhd::rfnoc::ANY_PORT);
    std::cout << "Connecting " << handler->ddc_ss_ctrl_->get_block_id() << " ==> " <<  handler->specsense_ctrl_->get_block_id() << std::endl;
    handler->rx_ss_graph_->connect( handler->ddc_ss_ctrl_->get_block_id(), uhd::rfnoc::ANY_PORT,  handler->specsense_ctrl_->get_block_id(), uhd::rfnoc::ANY_PORT);
    rx_ss_stream_args_args["block_id"] = handler->specsense_ctrl_->get_block_id().to_string();//FIXME:r4tn3sh: currently spec_sense is the endpoint
    std::cout << "<---------Using DDC ID : " << handler->ddc_ss_ctrl_->get_block_id().to_string() << std::endl;


    std::cout << "Connecting " << handler->duc_ctrl_->get_block_id() << " ==> " << handler->radio_ctrl_id_ << std::endl;    
    handler->tx_graph_->connect(handler->duc_ctrl_->get_block_id(), uhd::rfnoc::ANY_PORT, handler->radio_ctrl_id_, handler->radio_chan_);
    tx_stream_args_args["block_id"] = handler->duc_ctrl_->get_block_id().to_string();//FIXME:r4tn3sh: currently DUC is the endpoint
    std::cout << "<---------Using DUC ID : " << handler->duc_ctrl_->get_block_id().to_string() << std::endl;
    // ------------------------------------------------------------
    // TODO: if using DDC or other blocks, then comment following lines
    // std::cout << "<---------Using Block ID : " << handler->radio_ctrl_id_.to_string() << std::endl;
    // stream_args_args["block_id"] = handler->radio_ctrl_id_.to_string();


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
    // size_t channel = 0; //FIXME: not used
    std::string streamargs="";
    uhd::stream_args_t rx_stream_args("fc32","sc16");
    rx_stream_args.args = rx_stream_args_args;
    uhd::stream_args_t rx_ss_stream_args("fc32","sc16");
    rx_ss_stream_args.args = rx_ss_stream_args_args;
    uhd::stream_args_t tx_stream_args("fc32","sc16");
    tx_stream_args.args = tx_stream_args_args;

    // Set external clock reference
    /*
    if (strstr(args, "clock=external")) {
      uhd_usrp_set_clock_source(handler->usrp, "external", 0);
    }
    */

    // //FIXME:following function results in error. But do we need this? commenting!
    // std::cout<<"<---------Handler created with devname "<< handler->devname << ", check rssi"<<std::endl;
    // handler->has_rssi = get_has_rssi(handler);
    // if (handler->has_rssi) {
    //   uhd_sensor_value_make_from_realnum(&handler->rssi_value, "rssi", 0, "dBm", "%f");
    // }
    // -------------------------------------------------------------------
    uhd::rx_streamer::sptr rx_stream = handler->usrp_->get_rx_stream(rx_stream_args);
    handler->rx_stream_ = rx_stream;
    std::cout << "<---------Rx streamer created"<< std::endl;
    handler->rx_nof_samples = handler->rx_stream_->get_max_num_samps();
    std::cout << boost::format("<---------Max samples obtained for rx buffer is %d") % handler->rx_nof_samples << std::endl;

    uhd::rx_streamer::sptr rx_ss_stream = handler->usrp_->get_rx_stream(rx_ss_stream_args);
    handler->rx_ss_stream_ = rx_ss_stream;
    std::cout << "<---------Rx SS streamer created"<< std::endl;
    // handler->rx_nof_samples = handler->rx_stream_->get_max_num_samps();
    // std::cout << boost::format("<---------Max samples obtained for rx buffer is %d") % handler->rx_nof_samples << std::endl;

    uhd::tx_streamer::sptr tx_stream = handler->usrp_->get_tx_stream(tx_stream_args);
    handler->tx_stream_ = tx_stream;
    std::cout << "<---------Tx streamer created"<< std::endl;
    handler->tx_nof_samples = handler->tx_stream_->get_max_num_samps();
    std::cout << boost::format("<---------Max samples obtained for tx buffer is %d") % handler->tx_nof_samples << std::endl;

    // uhd_rx_streamer_max_num_samps(handler->rx_stream, &handler->rx_nof_samples);
    // uhd_tx_streamer_max_num_samps(handler->tx_stream, &handler->tx_nof_samples);

    //uhd_meta_range_make(&handler->rx_gain_range);
    //uhd_usrp_get_rx_gain_range(handler->usrp, "", 0, handler->rx_gain_range);

    // // Make metadata objects for RX/TX
    // uhd_rx_metadata_make(&handler->rx_md);
    // uhd_rx_metadata_make(&handler->rx_md_first);
    // uhd_tx_metadata_make(&handler->tx_md, false, 0, 0, false, false);

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
//XXX : r4tn3sh -------------------------------------
#ifdef __cplusplus
extern "C"
#endif
double rf_uhd_set_ss_srate(void *h, double rate)
{
  RFNoCDevice  *handler = (RFNoCDevice *) h;
  handler->ddc_ss_ctrl_->set_arg("output_rate", rate, 0);
  rate = (double)handler->ddc_ss_ctrl_->get_arg<double>("output_rate");
  std::cout << boost::format("<---------setting rx sampling rate : %f ") % rate << std::endl;
  return rate;
}
#ifdef __cplusplus
extern "C"
#endif
int rf_uhd_set_ss_avg_size(void *h, int avg_size)
{
  RFNoCDevice  *handler = (RFNoCDevice *) h;
  handler->ddc_ctrl_->set_arg("avg_size", avg_size, 0);
  avg_size = (int)handler->ddc_ctrl_->get_arg<int>("avg_size");
  std::cout << boost::format("<---------Avg size set to  : %d ") % avg_size << std::endl;
  return avg_size;
}
// --------------------------------------------------

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
double rf_uhd_set_rx_srate(void *h, double rate)
{
  RFNoCDevice  *handler = (RFNoCDevice *) h;
  handler->ddc_ctrl_->set_arg("output_rate", rate, 0);
  rate = (double)handler->ddc_ctrl_->get_arg<double>("output_rate");
  std::cout << boost::format("<---------setting rx sampling rate : %f ") % rate << std::endl;
  return rate;
}

#ifdef __cplusplus
extern "C" 
#endif
double rf_uhd_set_tx_srate(void *h, double rate)
{
  RFNoCDevice *handler = (RFNoCDevice*) h;
  std::cout << "<---------Setting Tx sampling rate : "<<rate<< std::endl;
  handler->duc_ctrl_->set_arg("input_rate", rate, 0);
  rate = (double)handler->duc_ctrl_->get_arg<double>("input_rate");
  handler->tx_rate = rate;
  std::cout << boost::format("<---------Setting Tx sampling rate : %f ") % rate << std::endl;
  return rate;
}

#ifdef __cplusplus
extern "C" 
#endif
double rf_uhd_set_rx_gain(void *h, double gain)
{
  RFNoCDevice *handler = (RFNoCDevice*) h;
  std::cout << "<---------USRP with chan : "<< handler->radio_chan_<<std::endl;//
  std::cout << "<---------Setting Rx gain : "<< gain<< std::endl;
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
  std::cout << "<---------Setting Tx gain : "<< gain<< std::endl;
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
  std::cout << "<---------Getting Rx gain"<< std::endl;
  double gain;
  gain = handler->radio_ctrl_->get_rx_gain(handler->radio_chan_);
  return gain;
}

#ifdef __cplusplus
extern "C" 
#endif
double rf_uhd_get_tx_gain(void *h)
{
  RFNoCDevice *handler = (RFNoCDevice*) h;
  std::cout << "<---------Getting Tx gain"<< std::endl;
  double gain;
  gain = handler->radio_ctrl_->get_tx_gain(handler->radio_chan_);
  return gain;
}

#ifdef __cplusplus
extern "C" 
#endif
double rf_uhd_set_rx_freq(void *h, double freq)
{
  RFNoCDevice *handler = (RFNoCDevice*) h;
  uhd::tune_request_t tune_request(freq);
  std::cout << "<---------Setting Rx freq : "<< freq << std::endl;
  handler->radio_ctrl_->set_rx_frequency(freq, handler->radio_chan_);
  freq = handler->radio_ctrl_->get_rx_frequency(handler->radio_chan_);
  return freq;
}

#ifdef __cplusplus
extern "C" 
#endif
double rf_uhd_set_tx_freq(void *h, double freq)
{
  RFNoCDevice *handler = (RFNoCDevice*) h;
  std::cout << "<---------Setting Tx freq : "<< freq<< std::endl;
  handler->radio_ctrl_->set_tx_frequency(freq, handler->radio_chan_);
  freq = handler->radio_ctrl_->get_tx_frequency(handler->radio_chan_);
  return freq;
}


#ifdef __cplusplus
extern "C" 
#endif
void rf_uhd_get_time(void *h, time_t *secs, double *frac_secs) {
  RFNoCDevice *handler = (RFNoCDevice*) h;
  const uhd::time_spec_t t = handler->radio_ctrl_->get_time_now();
  // TODO: copy the value to secs and frac_secs
  // uhd_usrp_get_time_now(handler->usrp, 0, secs, frac_secs);
  *secs = t.get_full_secs();
  *frac_secs = t.get_frac_secs();
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
  // uhd_rx_metadata_handle *md = &handler->rx_md_first;
  uhd::rx_metadata_t md;
  cf_t *data_c = (cf_t*) data;
  int trials = 0;
  if (blocking) {
    unsigned int n = 0;
    do {
      // zz4fap: increase the number of samples read.
      // size_t rx_samples = handler->rx_nof_samples;
      size_t rx_samples = SCATTER_SAMPLES_TO_READ;

#if SCATTER_DEBUG_MODE
      //zz4fap: DEBUG: REMOVER!!!!
      if(data == NULL) {
        printf("zz4fap: data == NULL!!!!!!!!!!!!!!!\n");
        exit(-1);
      }

      if(md == NULL) {
        printf("zz4fap: md == NULL!!!!!!!!!!!!!!!\n");
        exit(-1);
      }

      if(rx_samples <= 0) {
        printf("zz4fap: Menor ou negativo - handler->rx_nof_samples: %d !!!!!!!!!!!!!!!\n",rx_samples);
        exit(-1);
      }

      if(nsamples <= 0) {
        printf("zz4fap: Menor ou negativo - nsamples: %d !!!!!!!!!!!!!!!\n",nsamples);
        exit(-1);
      }
      //zz4fap: DEBUG: REMOVER!!!!
#endif

      if (rx_samples > nsamples - n) {
        rx_samples = nsamples - n;
      }
      // void *buff = (void*) &data_c[n];
      // void **buffs_ptr = (void**) &buff;
#if SCATTER_DEBUG_MODE
      //zz4fap: DEBUG: REMOVER!!!!
      if(rx_samples <= 0) {
        printf("zz4fap: Menor ou negativo - handler->rx_nof_samples: %d !!!!!!!!!!!!!!!\n",rx_samples);
        exit(-1);
      }

      if(buffs_ptr == NULL) {
        printf("zz4fap: buffs_ptr == NULL !!!!!!!!!!!!!!!\n");
        exit(-1);
      }
      //zz4fap: DEBUG: REMOVER!!!!
#endif
      // XXX : r4tn3sh : following code needs to be replaced for rx_streamer
      // uhd_error error = uhd_rx_streamer_recv(handler->rx_stream, buffs_ptr,
      //     rx_samples, md, 5.0, false, &rxd_samples);
      rxd_samples = handler->rx_stream_->recv(&data_c[n], rx_samples, md, 5.0, false);

#if SCATTER_DEBUG_MODE
      if(rxd_samples <= 0) {
        printf("zz4fap: Received no samples from UHD, nsamples: %d - rxd_samples: %d - n: %d - handler->rx_nof_samples: %d - rx_samples: %d !!!!!!!!!!!!!!!!!!!!!!!!!!!!\n", nsamples, rxd_samples, n, handler->rx_nof_samples, rx_samples); //zz4fap: DEBUG: REMOVER!!!!
      }
#endif
      if (md.error_code == uhd::rx_metadata_t::ERROR_CODE_TIMEOUT) {
        break;
      }

      if (md.error_code != uhd::rx_metadata_t::ERROR_CODE_NONE){
        std::string error = str(boost::format("Receiver error: %s") % md.strerror());
        throw std::runtime_error(error);
      }
      // if (error) {
      //   fprintf(stderr, "Error receiving from UHD: %d\n", error);
      //   return -1;
      // }
      // md = &handler->rx_md;
      n += rxd_samples;
      trials++;
#if SCATTER_DEBUG_MODE
      // uhd_rx_metadata_error_code_t error_code;
      // uhd_rx_metadata_error_code(*md, &error_code);
      // if (error_code == UHD_RX_METADATA_ERROR_CODE_OVERFLOW) {
      //   log_overflow(handler);
      // } else if (error_code == UHD_RX_METADATA_ERROR_CODE_LATE_COMMAND) {
      //   log_late(handler);
      // } else if (error_code != UHD_RX_METADATA_ERROR_CODE_NONE) {
      //   fprintf(stderr, "Error code 0x%x was returned during streaming. Aborting.\n", error_code);
      // }
#endif

    } while (n < nsamples && trials < 100);
  } else {
    rxd_samples = handler->rx_stream_->recv(&data_c[0], nsamples, md, 0.0, false);
    return md.error_code;
  }
  if (secs && frac_secs) {
    *secs = md.time_spec.get_full_secs();
    *frac_secs = md.time_spec.get_frac_secs();
    // uhd_rx_metadata_time_spec(handler->rx_md_first, secs, frac_secs);
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
  // rf_uhd_handler_t* handler = (rf_uhd_handler_t*) h;
  RFNoCDevice *handler = (RFNoCDevice*) h;
  uhd::tx_metadata_t md;

  size_t txd_samples;
  if (has_time_spec) {
    // uhd_tx_metadata_set_time_spec(&handler->tx_md, secs, frac_secs);
    md.time_spec = uhd::time_spec_t(secs, frac_secs);
  }
  int trials = 0;
  cf_t *data_c = (cf_t*) data;
  if (blocking) {
    unsigned int n = 0;
    do {
      size_t tx_samples = handler->tx_nof_samples;

      // First packet is start of burst if so defined, others are never
      if (n == 0) {
        md.start_of_burst = is_start_of_burst;
        // uhd_tx_metadata_set_start(&handler->tx_md, is_start_of_burst);
      } else {
        md.start_of_burst = false;
        // uhd_tx_metadata_set_start(&handler->tx_md, false);
      }

      // middle packets are never end of burst, last one as defined
      if (nsamples - n > tx_samples) {
        md.end_of_burst = false;
        // uhd_tx_metadata_set_end(&handler->tx_md, false);
      } else {
        tx_samples = nsamples - n;
        md.end_of_burst = is_end_of_burst;
        // uhd_tx_metadata_set_end(&handler->tx_md, is_end_of_burst);
      }

      // void *buff = (void*) &data_c[n];
      // const void **buffs_ptr = (const void**) &buff;
      txd_samples = handler->tx_stream_->send(&data_c[n], tx_samples, md, 3.0);
      // uhd_error error = uhd_tx_streamer_send(handler->tx_stream, buffs_ptr,
      //     tx_samples, &handler->tx_md, 3.0, &txd_samples);
      // if (error) {
      //   fprintf(stderr, "Error sending to UHD: %d\n", error);
      //   return -1;
      // }

      // Increase time spec
      // uhd_tx_metadata_add_time_spec(&handler->tx_md, txd_samples/handler->tx_rate);
      md.time_spec += txd_samples/handler->tx_rate;

      n += txd_samples;
      trials++;
    } while (n < (unsigned int) nsamples && trials < 100);
    return nsamples;
  } else {
    md.start_of_burst = is_start_of_burst;
    md.end_of_burst = is_end_of_burst;
    txd_samples = handler->tx_stream_->send(&data_c[0], nsamples, md, 0.0);
    return 0;
    // const void **buffs_ptr = (const void**) &data;
    // uhd_tx_metadata_set_start(&handler->tx_md, is_start_of_burst);
    // uhd_tx_metadata_set_end(&handler->tx_md, is_end_of_burst);
    // return uhd_tx_streamer_send(handler->tx_stream, buffs_ptr, nsamples, &handler->tx_md, 0.0, &txd_samples);
  }
}
