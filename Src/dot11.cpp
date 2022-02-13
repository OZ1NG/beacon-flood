#include "dot11.h"
#include <cstring>
#include <random>
#include <stdio.h>
#define BEACON_PACKET_LENGTH 0x34

Dot11::Dot11()
{

}

void Dot11::create_packet()
{
    this->packet_size = BEACON_PACKET_LENGTH + this->mbf.ESSID.tag_length+2 + this->mbf.channel.tag_length+2 + this->mbf.vender.tag_length+2 + sizeof(FCS);
    this->PACKET = new char[this->packet_size];
    memcpy(this->PACKET, &this->beacon_packet, BEACON_PACKET_LENGTH); // copy template

    // copy essid tag
    memcpy(this->PACKET+BEACON_PACKET_LENGTH, &this->mbf.ESSID.tag_number, 1);
    memcpy(this->PACKET+BEACON_PACKET_LENGTH+1, &this->mbf.ESSID.tag_length, 1);
    memcpy(this->PACKET+BEACON_PACKET_LENGTH+2, this->mbf.ESSID.data, this->mbf.ESSID.tag_length);

    // copy channel tag
    memcpy(this->PACKET+BEACON_PACKET_LENGTH+this->mbf.ESSID.tag_length+2, &this->mbf.channel, this->mbf.channel.tag_length+2);

    // copy vender tag
    memcpy(this->PACKET+BEACON_PACKET_LENGTH+this->mbf.ESSID.tag_length+2 + this->mbf.channel.tag_length+2, &this->mbf.vender, this->mbf.vender.tag_length+2);

    // copy FCS
    memcpy(this->PACKET+BEACON_PACKET_LENGTH+this->mbf.ESSID.tag_length+2 + this->mbf.channel.tag_length+2 + this->mbf.vender.tag_length+2, this->FCS, sizeof(FCS));

}

void Dot11::set_ESSID(string essid)
{
    this->ESSID = essid;
    this->mbf.ESSID.tag_number = SSID;
    this->mbf.ESSID.tag_length = essid.length();
    this->mbf.ESSID.data = (u_char *)this->ESSID.c_str();
}

void Dot11::reset()
{
    memset(this->PACKET, 0, this->packet_size);
    delete this->PACKET;
}

void Dot11::change_channel(double channel)
{
    this->PACKET[BEACON_PACKET_LENGTH+this->mbf.ESSID.tag_length+2+2] = channel;
}

void Dot11::print_BSSID()
{
    printf("%02hhx:%02hhx:%02hhx:%02hhx:%02hhx:%02hhx",
            this->beacon_packet.mfs.BSSID[0],
            this->beacon_packet.mfs.BSSID[1],
            this->beacon_packet.mfs.BSSID[2],
            this->beacon_packet.mfs.BSSID[3],
            this->beacon_packet.mfs.BSSID[4],
            this->beacon_packet.mfs.BSSID[5]
            );
}

void Dot11::set_random_bssid()
{
    u_char random_bssid[6];
    // init
    random_device rd;
    mt19937 gen(rd());

    // set range
    uniform_int_distribution<u_char> dis(0, 0xff); // 0~0xff

    // create random_bssid
    for(int i=0; i < 6; i++){
        random_bssid[i] = dis(gen);
    }

    // set bssid
    memcpy(this->beacon_packet.mfs.BSSID, random_bssid, 6);
    memcpy(this->beacon_packet.mfs.src_addr, random_bssid, 6);
}
