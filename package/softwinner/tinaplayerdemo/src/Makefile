Target = tinaplayerdemo

LoadFlags +=   -ltinaplayer -lxplayer -lcdc_vdecoder -lcdc_adecoder -lnormal_audio -lcdc_sdecoder -lcdc_base -lcdc_ve -lcdc_memory \
               -lcdx_parser  -lplayer -lcdx_stream -lcdx_base  -lpostprocess \
               -law_plugin  -lpthread -ldl -lstdc++ -lrt -lm -lc -lz -lasound

$(Target): tinaplayerdemo.cpp
	$(CXX) -o $@ $^ $(CFLAGS) $(LDFLAGS) $(LoadFlags)

