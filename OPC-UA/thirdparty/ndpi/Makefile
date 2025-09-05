all: pcap_to_flows

ifneq ($(OS),Windows_NT)
pcap_to_flows:
	gcc pcap_to_flows.c -o pcap.exe -I ./include/ ./lib/libndpi.a -lpcap -lm
else
pcap_to_flows:
	gcc pcap_to_flows.c -o pcap.exe -I ./include/ ./lib/libndpi.lib ./lib/wpcap.lib -lws2_32
endif

clean:
	rm -f pcap_to_flows *.o *~
