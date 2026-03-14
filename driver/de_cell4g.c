#include "de_cell4g.h"

int hard4g_init(char *net_name)
{
        int retry_count = 5;
        int success = 1;

        while (retry_count-- && success)
        {
                //system("ifconfig eth2 up");
                //system("/home/root/g2020/mokuai/4g/quectel-CM -6 -s ctnet &");
                
                //system("/home/root/g2020/mokuai/4g/ppp/ppp/quectel-pppd.sh /dev/ttyUSB5 "" ctnet@mycdma.cn vnet.mobi");
                system("/home/root/g2020/mokuai/4g/ppp/ppp/quectel-pppd.sh /dev/ttyUSB5 "" "" vnet.mobi &");
                sleep(8);
                if (cell4g_detect(net_name) == 0)
                        success = 0;
        }

        if (success == 0)
                return 0;
        else
        {
                //system("killall quectel-CM");
                system("/home/root/g2020/mokuai/4g/ppp/ppp/quectel-ppp-kill");
                return -1;
        }
}

int cell4g_detect(char *net_name)
{

        system("route add -net 0.0.0.0 netmask 0.0.0.0 dev ppp0");
        int sock_fd;
        struct sockaddr_in my_addr;
        struct ifreq ifr;
        char ipaddr[20];

        /**/ /* Get socket file descriptor */
        if ((sock_fd = socket(PF_INET, SOCK_DGRAM, 0)) == -1)
        {
                perror("socket");
                return -1;
        }

        /**/ /* Get IP Address */
        strncpy(ifr.ifr_name, net_name, IF_NAMESIZE);
        ifr.ifr_name[IFNAMSIZ - 1] = '\0';

        if (ioctl(sock_fd, SIOCGIFADDR, &ifr) < 0)
        {
                printf(":No Such Device %s\n", net_name);
                return -1;
        }

        memcpy(&my_addr, &ifr.ifr_addr, sizeof(my_addr));
        strcpy(ipaddr, inet_ntoa(my_addr.sin_addr));
        printf("Network addresss: %s\r\n", ipaddr);
        if (strlen(ipaddr) == 0){
                printf("ip is not vaild\r\n");
                return -1;
        }
                
        close(sock_fd);
        return 0;
}

//返回0 服务器ip及端口号可ping通 返回1 服务器ip及端口号不可达
int cell4g_ip_port_check(char *remote_ip, int remote_port)
{
        int sockfd, num, ret;
        char buf[100];

        struct hostent *he;
        struct sockaddr_in server, peer;
        char temp_str[100];

        // LAN设备配置
        sockfd = socket(AF_INET, SOCK_STREAM, 0);
        if (sockfd < 0)
        {
                printf("Connect to the server error\n");
                return -1;
        }
        else
        {
                struct sockaddr_in svraddr; //绑定地址(ip和端口号)
                memset(&svraddr, 0, sizeof(svraddr));
                svraddr.sin_family = AF_INET;
                svraddr.sin_port = htons(remote_port);
                inet_pton(AF_INET, remote_ip, &svraddr.sin_addr);

                int ret = connect(sockfd, (struct sockaddr *)&svraddr, sizeof(svraddr));
                if (ret < 0)
                {
                        close(ret);
                        return -1;
                }
                else{
                    char buf[]="Device login\n";
                    write(sockfd, buf, sizeof(buf));
                }
        }
        printf("Connect server success\r\n");
        return sockfd;
}

int cell4g_open()
{
        if (hard4g_init("ppp0") == -1)
                return -1;
        //printf("check pass\r\n");

        //    if(cell4g_detect("wwp0s20f0u11i4")) return -1;
        return cell4g_ip_port_check("106.52.84.156", 8888);
}
