#!/bin/bash

gst-launch-1.0 -v filesrc location=~/Видео/bt.ts ! tsdemux ! queue ! mpegvideoparse ! avdec_mpegvideo ! identity ! videoanalysis ! identity ! xvimagesink
