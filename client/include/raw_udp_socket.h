#ifndef RAW_UDP_SOCKET_H_
#define RAW_UDP_SOCKET_H_



#include <iostream>
#include <string>

#include <cstdio>		// for printf(), perror()
#include <cstring>		// for strncpy()
#include <stdlib.h>		// for exit()
#include <signal.h>		// for signal()
#include <sys/socket.h>		// for socket(), recvfrom()
#include <sys/ioctl.h>		// for SIOCGIFFLAGS, SIOCSIFFLAGS
#include <netinet/in.h>		// for htons()
#include <linux/if_ether.h>	// for ETH_P_ALL
#include <linux/if.h>		// for struct ifreq, IFNAMSIZ
#include <linux/if_packet.h>

//-----------------------------------------------------------------------------
class raw_udp_socket
{

//-----------------------------------------------------------------------------
public:

    raw_udp_socket(std::string i_, uint16_t p_) : interface(i_), port(p_)
    {

        // create an unnamed socket for reception of ethernet packets -  PF_PACKET, AF_PACKET, SOCK_RAW, SOCK_DGRAM
        sock = socket( PF_PACKET, SOCK_DGRAM, htons( ETH_P_ALL ) ); 
        if (sock < 0) 
        {
            std::cout << "socket: " << std::to_string(sock) << std::endl;
        }
        
        
        int opt = 1;
        setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    
    
        set_promisc(true);

        /*
        struct sockaddr_in my_addr;
        memset((char*)&my_addr, 0, sizeof(my_addr));
        my_addr.sin_family = AF_PACKET;
        my_addr.sin_port = htons(port);
        my_addr.sin_addr.s_addr = htonl(INADDR_ANY);
        
        int bind_error = bind(sock, (struct sockaddr*)&my_addr, sizeof(my_addr));
        */
        
        struct sockaddr_ll sa;
        memset((char*)&sa, 0, sizeof(sa));
        sa.sll_family = PF_PACKET;
        sa.sll_ifindex = ifindex;
        sa.sll_protocol = htons(ETH_P_ALL);
        
        int bind_error = bind(sock, reinterpret_cast<struct sockaddr *>(&sa), sizeof(sa));
        
        if (bind_error < 0)
        {
            std::cout << "bind error: " << bind_error << std::endl;
            
        }
    }

    ~raw_udp_socket()
    {
        set_promisc(false);
    }


    //-----------------------------------------------------------------------------
    inline int32_t receive_data(uint8_t *buffer, size_t len)
    {
        int32_t	n = recvfrom(sock, buffer, data_size, 0, NULL, NULL);
        return n;
    }


//-----------------------------------------------------------------------------
private:

    //-----------------------------------------------------------------------------
    // internal private variables
    int32_t sock;
    uint16_t port;
    struct ifreq ethreq;
    int32_t ifindex;

    std::string interface;

    size_t data_size = 9000;


    //-----------------------------------------------------------------------------
    inline void set_promisc(bool enable)
    {
        // struct packet_mreq mreq = {0};
        // int action;

        // mreq.mr_ifindex = m_if_index;
        // mreq.mr_type = PACKET_MR_PROMISC;

        // if (enable)
        //     action = PACKET_ADD_MEMBERSHIP;
        // else
        //     action = PACKET_DROP_MEMBERSHIP;

        // if (setsockopt(m_sock, SOL_PACKET, action, &mreq, sizeof(mreq)) != 0)
        //     throw std::system_error(errno, std::system_category(),
        //                             "unable to set interface promisc");

        // enable 'promiscuous mode' for the selected socket interface
        strncpy(ethreq.ifr_name, interface.c_str(), IFNAMSIZ);

        if ( ioctl( sock, SIOCGIFFLAGS, &ethreq ) < 0 )
        { 
            perror( "ioctl: get ifflags" ); 
            exit(1); 
        }

        if (enable)
        	ethreq.ifr_flags |= IFF_PROMISC;                // enable 'promiscuous' mode
        else
            ethreq.ifr_flags &= ~IFF_PROMISC;               // turn off the interface's 'promiscuous' mode
          
        // set the flag
        if ( ioctl( sock, SIOCSIFFLAGS, &ethreq ) < 0 )
        { 
            perror( "ioctl: set ifflags" ); 
            exit(1); 
        }

    }

    // convert interface name to index number, for later use
    void get_index()
    {
        struct ifreq ifr = {0};

        strncpy(ifr.ifr_name, interface.c_str(), IFNAMSIZ);

        if (ioctl(sock, SIOCGIFINDEX, &ifr) != 0)
            throw std::system_error(errno, std::system_category(),
                                    "unable to get interface index");

        ifindex = ifr.ifr_ifindex;
    }

};  // end of class


#endif  // RAW_UDP_SOCKET_H_
