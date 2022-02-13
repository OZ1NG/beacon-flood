#include <pcap.h>
#include <stdbool.h>
#include <stdio.h>
#include "dot11.h"
#include "iwlib.h"
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <thread>
#include <csignal>
#include <list>
#include <iostream>
#include <fstream>
#include <sstream>

using namespace std;

void hexdump(u_char* packet, unsigned int len){
    puts("[TEST Code]");
    for(unsigned int i = 0; i < len; i++){
        if((i % 0x10) == 0){
            puts("");
        }
        printf("%02hhx ", packet[i]);
    }
    puts("\n");
}

void usage() {
    puts("syntax: beacon-flood <interface> <ssid-list-file>");
    puts("sample: beacon-flood mon0 ssid-list.txt");
}

typedef struct {
    char * interface;
    char * file_name;
} Param;

Param param  = {
    .interface = NULL,
    .file_name = NULL
};

bool parse(Param* param, int argc, char* argv[]) {
    if (argc != 3) {
        usage();
        return false;
    }

    param->interface = argv[1];
    param->file_name = argv[2];

    return true;
}

int skfd;
struct iw_range range;
// total channel count : range.num_frequency
// channel info : range.freq[idx].i

int parse_channel(){ // only get channel number
    if(iw_get_range_info(skfd, param.interface, &range) < 0){
        fprintf(stderr, "%-8.16s  no frequency information.1\n\n", param.interface);
        return -2; // iw_get_range_info fail...
    }
    else{
        if(range.num_frequency <= 0){
            fprintf(stderr, "%-8.16s  no frequency information.2\n\n", param.interface);
            return -1; // no channel
        }
    }
    return 0; // complete
}

void change_channel(double freq){
    struct iwreq wrq;
    iw_float2freq(freq, &(wrq.u.freq));
    wrq.u.freq.flags = IW_FREQ_FIXED;
    iw_set_ext(skfd, param.interface, SIOCSIWFREQ, &wrq);
}

double global_channel_info = 0;

int thread_loop = 1;
int * t_change_ch(){ // thread function
    while(1){
        double channel = 1;
        for(int i=0; i < range.num_frequency; i++){
            if(!thread_loop)
                return 0;
            channel = range.freq[i].i;
            change_channel(channel);
            global_channel_info = channel;
            sleep(1);
        }
    }
}

bool end_flag = 0;
void end_handler(int signal){
    end_flag = 1;
}

list<string> essid_list;
int read_file_data(){
    char * line = nullptr;
    size_t len = 0;
    FILE * fd = fopen(param.file_name, "r");
    if (fd == nullptr)
        return EXIT_FAILURE;

    while ((getline(&line, &len, fd)) != -1) {
        line[strlen(line)-1] = 0; // \n -> \x00
        essid_list.push_back(string(line));
    }
    fclose(fd);
    return 0;
}

list<Dot11 *> beacon_packet_list;
int main(int argc, char *argv[])
{
    if (!parse(&param, argc, argv))
        return -1;

    signal(SIGINT, end_handler);

    char errbuf[PCAP_ERRBUF_SIZE];
    pcap_t* pcap = pcap_open_live(param.interface, BUFSIZ, 1, 1000, errbuf);
    if (pcap == NULL) {
        fprintf(stderr, "pcap_open_live(%s) return null - %s\n", param.interface, errbuf);
        return -1;
    }

    // channel parse
    if((skfd = iw_sockets_open()) < 0){
        perror("socket");
        return -1;
    }
    if(parse_channel()){
        iw_sockets_close(skfd);
        return 0;
    }

    // create packets
    read_file_data();
    list<string>::iterator iter = essid_list.begin();
    while(1){
        Dot11 * beacon_flood = new Dot11();
        beacon_flood->set_random_bssid();
        cout << "ESSID : " << *iter << ", BSSID : "; beacon_flood->print_BSSID(); cout << endl;
        beacon_flood->set_ESSID(*iter);
        beacon_flood->create_packet();
        beacon_packet_list.push_back(beacon_flood);
        if(++iter == essid_list.end())
            break;
    }

    // run thread // start channel hopping
    thread thread_id = thread(t_change_ch);

    list<Dot11 *>::iterator iter1 = beacon_packet_list.begin();
    while(1){
        (*iter1)->change_channel(global_channel_info); // change channel
        hexdump((u_char*)(*iter1)->PACKET, (*iter1)->packet_size); // test
        if(pcap_sendpacket(pcap, (u_char*)(*iter1)->PACKET, (*iter1)->packet_size) != 0)
            puts("[*] Send Deauth Packet : Fail");
        //else
        //    puts("[*] Send Deauth Packet : Success"); // test


        //beacon_flood->reset();
        if(end_flag) // SIGINT
            break;
        if(++iter1 == beacon_packet_list.end()){
            iter1 = beacon_packet_list.begin();
            sleep(0.1);
        }
    }

    // end
    thread_loop = 0;
    thread_id.join();
    iw_sockets_close(skfd);

    pcap_close(pcap);
}
