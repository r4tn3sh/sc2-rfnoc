#
# Copyright 2013-2015 Software Radio Systems Limited
#
# This file is part of the srsLTE library.
#
# srsLTE is free software: you can redistribute it and/or modify
# it under the terms of the GNU Affero General Public License as
# published by the Free Software Foundation, either version 3 of
# the License, or (at your option) any later version.
#
# srsLTE is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU Affero General Public License for more details.
#
# A copy of the GNU Affero General Public License can be found in
# the LICENSE file in the top-level directory of this distribution
# and at http://www.gnu.org/licenses/.
#

if(RF_FOUND)
  # Include common RF files
  set(SOURCES_RF "")
  list(APPEND SOURCES_RF rf_imp.c rf_utils.c)

  if (UHD_FOUND)
    add_definitions(-DENABLE_UHD)
    #list(APPEND SOURCES_RF rf_uhd_imp.c uhd_c_api.cpp rf_uhd_imp.cpp)
    #list(APPEND SOURCES_RF rf_uhd_imp.c uhd_c_api.cpp)
    list(APPEND SOURCES_RF rf_uhd_imp.cpp uhd_c_api.cpp)
  endif (UHD_FOUND)

  if (BLADERF_FOUND)
    add_definitions(-DENABLE_BLADERF)
    list(APPEND SOURCES_RF rf_blade_imp.c)
  endif (BLADERF_FOUND)

  add_library(srslte_rf OBJECT ${SOURCES_RF})
  SRSLTE_SET_PIC(srslte_rf)
endif(RF_FOUND)
