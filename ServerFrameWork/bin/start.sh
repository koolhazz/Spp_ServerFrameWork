#!/bin/bash
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:.
./spp_ctrl ../etc/spp_common.xml ../etc/spp_ctrl.xml 3 2 1 frame_ctrl_flag
