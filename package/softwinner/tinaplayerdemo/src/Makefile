Target = tinaplayerdemo

LoadFlags =   -ltinaplayer -lxplayer -lvdecoder -ladecoder -lsdecoder -lnormal_audio -lpostprocess \
               -lcdx_parser  -lplayer -lcdx_stream -lcdx_base -lVE -lMemAdapter \
               -law_plugin -lcrypto -lssl  -lpthread -ldl -lstdc++ -lrt -lasound

$(Target): tinaplayerdemo.cpp
	$(CXX) -o $@ $^ $(CFLAGS) $(LDFLAGS) $(LoadFlags)

