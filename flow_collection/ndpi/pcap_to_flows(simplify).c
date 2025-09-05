#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <search.h>
#include <sys/types.h>    
#include <sys/stat.h> 
#include <pcap.h>
#include <signal.h>
#include <time.h>
#include "./include/ndpi_main.h"
#include "./include/cJSON.h"//注意路径
#ifdef _WIN32
#include <Windows.h>
#include <Winsock.h>
#include <Winsock2.h>

#else
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#endif // _WIN32

static void setupDetection(void);
static void node_output_flow_info_walker(const void *node, ndpi_VISIT which, int depth, void *user_data);
static void node_proto_guess_walker(const void *node, ndpi_VISIT which, int depth, void *user_data); 
int get_num_applications();
const char *ntos(uint32_t ip);
static int get_app_number(char* label);

int print_on = 0;

int flag=0;
int comma_flag = 0;

static char *_pcap_file = NULL;


static char _pcap_error_buffer[PCAP_ERRBUF_SIZE];
static pcap_t *_pcap_handle = NULL;

static struct ndpi_detection_module_struct *ndpi_struct = NULL;
static u_int32_t detection_tick_resolution = 1000000; //microseconds

static u_int64_t raw_packet_count = 0;
static u_int64_t ip_packet_count = 0;
static u_int64_t total_bytes = 0;
static u_int64_t flow_counter_01 = 0;
static u_int64_t protocol_counter[NDPI_MAX_SUPPORTED_PROTOCOLS + NDPI_MAX_NUM_CUSTOM_PROTOCOLS + 1];
static u_int64_t protocol_counter_bytes[NDPI_MAX_SUPPORTED_PROTOCOLS + NDPI_MAX_NUM_CUSTOM_PROTOCOLS + 1];
static u_int32_t protocol_flows[NDPI_MAX_SUPPORTED_PROTOCOLS + NDPI_MAX_NUM_CUSTOM_PROTOCOLS + 1] = { 0 };
static int num_apps = 0;

static FILE *flow_info_file;
static char *flow_info_file_name;

static char labels[NDPI_MAX_SUPPORTED_PROTOCOLS + NDPI_MAX_NUM_CUSTOM_PROTOCOLS + 1][32];    

static char valid_labels[128][32]={/*"Modbus","FINS","EthernetIP","s7comm"*/};  //用于过滤的白名单，可自行添加
static char invalid_labels[128][32]={};  //用于过滤的黑名单，可自行添加
int using_invalid = 1; // 0代表启用白名单，1代表启用黑名单 

static int del_unknown_flag = 0;
static int app_proto_only_flag = 0;

static int min_number_objects = 30;

static int n_packets = (1<<30);  //当超过多少包时才进行解析？

#define	MAX_NDPI_FLOWS     20000000
#define IA_MAX             10000000

#define pcap_path "test.pcap"
#define json_path "test.json"

// id tracking
typedef struct ndpi_id {
    u_int8_t ip[4];
    struct ndpi_id_struct *ndpi_id;
} ndpi_id_t;


static u_int32_t size_id_struct = 0;

// flow tracking
typedef struct ndpi_flow {
    u_int32_t src_ip;
    u_int32_t dst_ip;
    u_int16_t src_port;
    u_int16_t dst_port;
    u_int32_t first_packet_time_sec;
    u_int32_t first_packet_time_usec;
    u_int8_t detection_completed, protocol;
    struct ndpi_flow_struct *ndpi_flow;
    
    //Flow features
    u_int16_t packets, bytes;
    u_int32_t last_packet_time_sec;
    u_int32_t last_packet_time_usec;
    double d_ia_time;
    double min_ia_time, max_ia_time;
    u_int32_t min_pkt_len, max_pkt_len;
    u_int8_t first_packet;

    // result only, not used for flow identification
    u_int16_t detected_protocol;

    void *src_id, *dst_id;
} ndpi_flow_t;


static u_int32_t size_flow_struct = 0;
static struct ndpi_flow *ndpi_flows_root = NULL;
static u_int32_t ndpi_flow_count = 0;
static u_int32_t valid_flow_count = 0; // 2+ packets in flow

static void *malloc_wrapper(unsigned long size) {
    return malloc(size);
}

static void free_wrapper(void *freeable) {
    free(freeable);
}

static char* ipProto2Name(u_short proto_id) {
    static char proto[8];

    switch(proto_id) {
    case IPPROTO_TCP:
        return("TCP");
        break;
    case IPPROTO_UDP:
        return("UDP");
        break;
    case IPPROTO_ICMP:
        return("ICMP");
        break;
    case 112:
        return("VRRP");
        break;
  }

  printf(proto, sizeof(proto), "%u", proto_id);
  return(proto);
}

static int node_cmp(const void *a, const void *b) {
    struct ndpi_flow *fa = (struct ndpi_flow*)a;
    struct ndpi_flow *fb = (struct ndpi_flow*)b;

    if(fa->src_ip < fb->src_ip) return(-1); else { if(fa->src_ip > fb->src_ip) return(1); }
    if(fa->src_port < fb->src_port) return(-1); else { if(fa->src_port > fb->src_port) return(1); }
    if(fa->dst_ip < fb->dst_ip) return(-1); else { if(fa->dst_ip > fb->dst_ip) return(1); }
    if(fa->dst_port < fb->dst_port) return(-1); else { if(fa->dst_port > fb->dst_port) return(1); }
    if(fa->protocol < fb->protocol) return(-1); else { if(fa->protocol > fb->protocol) return(1); }

  return(0);
}

static void free_ndpi_flow(struct ndpi_flow *flow) {
    if(flow->ndpi_flow) { ndpi_free(flow->ndpi_flow); flow->ndpi_flow = NULL; }
    if(flow->src_id)    { ndpi_free(flow->src_id); flow->src_id = NULL;       }
    if(flow->dst_id)    { ndpi_free(flow->dst_id); flow->dst_id = NULL;       }
}

static int if_valid_label(char *app_name){
    //printf("%s\n",app_name);
    //printf("%d\n",num_apps);
    for(int i=0; i <(sizeof(valid_labels)/32); i++){
        if(strcmp(valid_labels[i], app_name) == 0) {
            return 1;
        }            
    }
    return 0;
}

static int if_invalid_label(char *app_name){
    //printf("%s\n",app_name);
    //printf("%d\n",num_apps);
    for(int i=0; i <(sizeof(invalid_labels)/32); i++){
        if(strcmp(invalid_labels[i], app_name) == 0) {
            return 0;
        }            
    }
    return 1;
}

static int get_app_number(char* label){
    for(int i=0; i < num_apps; i++){
        if(strcmp(labels[i], label) == 0) {
            return i;
        }            
    }
    return 0;
}

const char *ntos(uint32_t ip)
{   
    char *str=(char*)malloc(17);

    unsigned char ip_str[4];
    ip_str[0] = ip & 0xFF;
    ip_str[1] = (ip >> 8) & 0xFF;
    ip_str[2] = (ip >> 16) & 0xFF;
    ip_str[3] = (ip >> 24) & 0xFF;

    snprintf(str, 16, "%d.%d.%d.%d",
                 ip_str[0], ip_str[1], ip_str[2], ip_str[3]);

    return str; 
}


//1.2.3.1 输出流？？？
static void printFlow(struct ndpi_flow *flow, FILE *file) {
    //解析过滤
    //if (flow->packets < n_packets || valid_label(ndpi_get_proto_name(ndpi_struct, flow->detected_protocol)) == 0 ) { return; }
    if(using_invalid){
        if(if_invalid_label(ndpi_get_proto_name(ndpi_struct, flow->detected_protocol)) == 0){return;} //黑名单判断
    }
    else{
        if(if_valid_label(ndpi_get_proto_name(ndpi_struct, flow->detected_protocol)) == 0){return;}  //白名单判断
    }
    double last_time = (flow->last_packet_time_sec) * detection_tick_resolution + flow->last_packet_time_usec / (1000000 / detection_tick_resolution);
    double first_time = (flow->first_packet_time_sec) * detection_tick_resolution + flow->first_packet_time_usec / (1000000 / detection_tick_resolution);
    double duration = (last_time - first_time)/detection_tick_resolution; 

    const char* src = ntos(flow->src_ip);  
    const char* dst = ntos(flow->dst_ip); 
    char* app_name = ndpi_get_proto_name(ndpi_struct, flow->detected_protocol); 
    int app_num = get_app_number(app_name);
    
    //流量采集输出内容：src/dst_ip/port、持续时间、字节量、内部到达时间、包长度、协议名称
    struct tm first_0001,last_0001;
    unsigned int first_01=(unsigned int)flow->first_packet_time_sec;
    unsigned int last_01=(unsigned int)flow->last_packet_time_sec;
    time_t first_001=(time_t)first_01;
    time_t last_001=(time_t)last_01;

    #ifdef _WIN32
    localtime_s(&first_0001, &first_001);
    localtime_s(&last_0001, &last_001);
    #else
    localtime_r(&first_001, &first_0001);
    localtime_r(&last_001, &last_0001);
    #endif

    //json格式
        if (comma_flag) fprintf(file, ",");
        if (comma_flag) fprintf(file, "\n");
        fprintf(file, 
        "    {\n      \"source_ipport\": \"%s:%u\",\n      \"dest_ipport\": \"%s:%u\",\n      \"first_time\": \"%d-%d-%d-%d:%d:%d\",\n      \"last_time\": \"%d-%d-%d-%d:%d:%d\",\n      \"flow_duration(s)\": %.6f,\n      \"flow_bytes\": %u,\n      \"protocol\": \"%s\"\n    }",

        src,
        ntohs(flow->src_port),
        dst,
        ntohs(flow->dst_port),
        first_0001.tm_year + 1900, first_0001.tm_mon + 1,first_0001.tm_mday, first_0001.tm_hour, first_0001.tm_min, first_0001.tm_sec,
        last_0001.tm_year + 1900, last_0001.tm_mon + 1,last_0001.tm_mday, last_0001.tm_hour, last_0001.tm_min, last_0001.tm_sec,
        duration,
        flow->bytes,
        app_name          
    );
        comma_flag = 1;
       



}

//1.1.1获取数据流
static struct ndpi_flow *get_ndpi_flow(const struct pcap_pkthdr *header, const struct ndpi_iphdr *iph, u_int16_t ipsize) {
    u_int32_t i;
    u_int16_t l4_packet_len;
    struct ndpi_tcphdr *tcph = NULL;
    struct ndpi_udphdr *udph = NULL;
    u_int32_t src_ip;
    u_int32_t dst_ip;
    u_int16_t src_port;
    u_int16_t dst_port;
    struct ndpi_flow flow;
    void *ret;

    if (ipsize < 20) {
        return NULL;
    }

    if ((iph->ihl * 4) > ipsize || ipsize < ntohs(iph->tot_len) || (iph->frag_off & htons(0x1FFF)) != 0) {
        return NULL;
    }

    l4_packet_len = ntohs(iph->tot_len) - (iph->ihl * 4);

    src_ip = iph->saddr;
    dst_ip = iph->daddr;
   
    if (iph->protocol == 6 && l4_packet_len >= 20) {
        // tcp
        tcph = (struct ndpi_tcphdr *) ((u_int8_t *) iph + iph->ihl * 4);
        src_port = tcph->source;
        dst_port = tcph->dest;
       
    } else if (iph->protocol == 17 && l4_packet_len >= 8) {
        // udp
        udph = (struct ndpi_udphdr *) ((u_int8_t *) iph + iph->ihl * 4);
        src_port = udph->source;
        dst_port = udph->dest;
       
    } else {
        // non tcp/udp protocols
        src_port = 0;
        dst_port = 0;
    }

    flow.protocol = iph->protocol;
    flow.src_ip = src_ip;
    flow.dst_ip = dst_ip;
    flow.src_port = src_port;
    flow.dst_port = dst_port;
    flow.first_packet_time_sec = header->ts.tv_sec;
    flow.first_packet_time_usec = header->ts.tv_usec;

  

    ret = ndpi_tfind(&flow, (void*)&ndpi_flows_root, node_cmp);

    if(ret == NULL) {
        if (ndpi_flow_count == MAX_NDPI_FLOWS) {
            printf("ERROR: maximum flow count (%u) has been exceeded\n", MAX_NDPI_FLOWS);
            exit(-1);
        } else {
            struct ndpi_flow *newflow = (struct ndpi_flow*)malloc(sizeof(struct ndpi_flow));

            if(newflow == NULL) {
        	    printf("[NDPI] %s(1): not enough memory\n", __FUNCTION__);
        	    return(NULL);
            }

            memset(newflow, 0, sizeof(struct ndpi_flow));
            newflow->protocol = iph->protocol;
            newflow->src_ip = src_ip, newflow->dst_ip = dst_ip;
            newflow->src_port = src_port, newflow->dst_port = dst_port;
            newflow->first_packet_time_sec = header->ts.tv_sec;
            newflow->first_packet_time_usec = header->ts.tv_usec;
            newflow->last_packet_time_sec = header->ts.tv_sec;
            newflow->last_packet_time_usec = header->ts.tv_usec;
            newflow->d_ia_time = 0;
            newflow->min_ia_time = IA_MAX;
            newflow->max_ia_time = 0;
            newflow->min_pkt_len = header->len;
            newflow->max_pkt_len = header->len;
            newflow->first_packet = 1; 
            newflow->packets = 0;
            newflow->bytes = 0;         

            ndpi_tsearch(newflow, (void**)&ndpi_flows_root, node_cmp); /* Add */

            ndpi_flow_count += 1;

            //printFlow(newflow);
            return(newflow);
        }
    } else{
        return *(struct ndpi_flow**)ret;
    }
}

//1.1.2获取内部到达时间
static double get_inter_arrival_time(u_int32_t last_packet_time_sec, u_int32_t last_packet_time_usec, u_int32_t new_packet_time_sec, u_int32_t new_packet_time_usec) {
    u_int64_t last_time = ((uint64_t) last_packet_time_sec) * detection_tick_resolution + last_packet_time_usec / (1000000 / detection_tick_resolution);
    u_int64_t new_time = ((uint64_t) new_packet_time_sec) * detection_tick_resolution + new_packet_time_usec / (1000000 / detection_tick_resolution);
    double time = (double)(new_time - last_time);
    return time; 
}

//1.2.1 统计包
static void node_proto_guess_walker(const void *node, ndpi_VISIT which, int depth, void *user_data) {
    struct ndpi_flow *flow = *(struct ndpi_flow**)node;
    if((which == preorder) || (which == leaf)) { /* Avoid walking the same node multiple times */
        if (flow->packets >= n_packets ) {
            protocol_counter[flow->detected_protocol]       += flow->packets;
            protocol_counter_bytes[flow->detected_protocol] += flow->bytes;
            protocol_flows[flow->detected_protocol]++;    
            //valid_flow_count++;
        }
    }    
}

//1.2.2 获取协议名称
int get_num_applications(){
    int num_apps = 0;  
    int label_i = 0;  
    for(int i=0; i < NDPI_MAX_SUPPORTED_PROTOCOLS + NDPI_MAX_NUM_CUSTOM_PROTOCOLS; i++){
        if(protocol_flows[i] >= min_number_objects) {
            num_apps++;
            valid_flow_count+=protocol_flows[i];
            strcpy(labels[label_i] ,ndpi_get_proto_name(ndpi_struct, i));
            label_i++;            
        }
    }
    return num_apps;
}

//1.2.3 输出流？？？
static void node_output_flow_info_walker(const void *node, ndpi_VISIT which, int depth, void *user_data) {
    struct ndpi_flow *flow = *(struct ndpi_flow**)node;
    if (flow_info_file != NULL){
        if ((which == preorder) || (which == leaf)) {
           printFlow(flow, flow_info_file);

        }
        
    }else {printf("Invalid file stream!\n"); exit(-1);}
       
}

//1.1解析包
static unsigned int packet_processing(const u_int64_t time, const struct pcap_pkthdr *header, const struct ndpi_iphdr *iph, u_int16_t ipsize, u_int16_t rawsize) {
    //struct ndpi_id_struct *src, *dst;
    struct ndpi_flow_input_info *input_info;
    struct ndpi_flow *flow;
    struct ndpi_flow_struct *ndpi_flow = NULL;
    u_int16_t protocol = 0;
    u_int16_t frag_off = ntohs(iph->frag_off);
    double ia_time;

    flow = get_ndpi_flow(header, iph, ipsize);
    //printf("%d\n",flow->packets);
    if (flow != NULL){
        //if(flow->packets < n_packets) {  
    
            ndpi_flow = flow->ndpi_flow;
            flow->packets++, flow->bytes += rawsize;
            //src = flow->src_id, dst = flow->dst_id;
            ia_time = get_inter_arrival_time(flow->last_packet_time_sec, flow->last_packet_time_usec, header->ts.tv_sec, header->ts.tv_usec);
            flow->d_ia_time+=ia_time;
            
            if(flow->first_packet != 1 ) {
                if(ia_time < flow->min_ia_time){ flow->min_ia_time = ia_time; }
                if(ia_time > flow->max_ia_time){ flow->max_ia_time = ia_time; }
            }
            flow->first_packet = 0;
            
            if(rawsize < flow->min_pkt_len){flow->min_pkt_len = rawsize;}
            if(rawsize > flow->max_pkt_len){flow->max_pkt_len = rawsize;}
            
            flow->last_packet_time_sec = header->ts.tv_sec;
            flow->last_packet_time_usec = header->ts.tv_usec;           
        //}        
    } else {
        return 0;
    }
  
    ip_packet_count++;
    total_bytes += rawsize;

    if(flow->detection_completed != 1){
        // here the actual detection is performed
        ndpi_protocol detected = ndpi_detection_process_packet(ndpi_struct, ndpi_flow, (uint8_t *) iph, ipsize, time, input_info);
        protocol = detected.app_protocol;
        // this function actually detect protocols
        if(protocol==0){
            detected = ndpi_guess_undetected_protocol_v4(ndpi_struct, ndpi_flow, flow->protocol,ntohl(flow->src_ip), ntohs(flow->src_port), ntohl(flow->dst_ip), ntohs(flow->dst_port));
            if(detected.app_protocol!=detected.master_protocol){        
                protocol = detected.app_protocol;
            }          
        }
        flow->detected_protocol = protocol;

    

        if (flow->detected_protocol != NDPI_PROTOCOL_UNKNOWN) {
            flow->detection_completed = 1;
            free_ndpi_flow(flow);        
        }
    }

  return 0;
}

//1.2输出结果
static void printResults(void) {
    
    ndpi_twalk(ndpi_flows_root, node_proto_guess_walker, NULL);
    num_apps = get_num_applications();
    //printf("%d\n",num_apps);
    flow_info_file = fopen(flow_info_file_name, "wb");
    //fprintf(flow_info_file, "%i,10,%i\n", valid_flow_count, num_apps);    
    for(int i=0; i < num_apps; i++){
        fprintf(flow_info_file,"%s,",labels[i]);
    }    
   
    fprintf(flow_info_file, "{\n  \"FlowCollection\": [\n");
    comma_flag = 0;
    ndpi_twalk(ndpi_flows_root, node_output_flow_info_walker, NULL);
    fprintf(flow_info_file, "\n  ]\n}");
    fclose(flow_info_file);
    
    
}

//0.开始探测
static void setupDetection(void) {
    u_int32_t i;
    NDPI_PROTOCOL_BITMASK all;

    // init global detection structure
    ndpi_struct = ndpi_init_detection_module(0);
    //printf("%s",ndpi_struct.);
    if (ndpi_struct == NULL) {
        printf("ERROR: global structure initialization failed\n");
        exit(-1);
    }
    // enable all protocols
    NDPI_BITMASK_SET_ALL(all);
    ndpi_set_protocol_detection_bitmask2(ndpi_struct, &all);

    // allocate memory for id and flow tracking
    //size_id_struct = ndpi_detection_get_sizeof_ndpi_id_struct();
    size_flow_struct = ndpi_detection_get_sizeof_ndpi_flow_struct();

    // clear memory for results
    memset(protocol_counter, 0, sizeof(protocol_counter));
    memset(protocol_counter_bytes, 0, sizeof(protocol_counter_bytes));
    memset(protocol_flows, 0, sizeof(protocol_flows));

    raw_packet_count = ip_packet_count = total_bytes = 0;
    ndpi_flow_count = 0;
}

//1.入口函数
// executed for each packet in the pcap file
static void pcap_packet_callback(u_char * args, const struct pcap_pkthdr *header, const u_char * packet) {
    //printf("pcap_packet_callback\n");
    const struct ndpi_ethhdr *ethernet = (struct ndpi_ethhdr *) packet;
    struct ndpi_iphdr *iph = (struct ndpi_iphdr *) &packet[sizeof(struct ndpi_ethhdr)];
    u_int64_t time;
    static u_int64_t lasttime = 0;
    u_int16_t type, ip_offset;

    raw_packet_count++;

    time = ((uint64_t) header->ts.tv_sec) * detection_tick_resolution + header->ts.tv_usec / (1000000 / detection_tick_resolution);

    type = ethernet->h_proto;

    if (type != 8 || iph->version != 4) {
       // printf("WARNING: only IPv4 packets are supported\n");
        return;
    }

    ip_offset = sizeof(struct ndpi_ethhdr);
    
    // process the packet
    packet_processing(time, header, iph, header->len - ip_offset, header->len);
    if(flag) printResults();
  
}

int main(int argc, char* argv[]) {
   
   	int devs_num=0;
	char devs_name[64][64];
    //char devs_disc[64][64];
	char buf[1024];
	pcap_if_t *allDev;
    //查找所有设备
	pcap_findalldevs_ex(PCAP_SRC_IF_STRING, NULL ,&allDev, buf);
  
	for (pcap_if_t *pdev = allDev; pdev; pdev=pdev->next)
	{
	    strcpy(devs_name[devs_num],pdev->name);
	    devs_num++;
	}
	pcap_freealldevs(allDev);    
    if(argc==1){     

     int op;//设备名对应数字
     char *dev;  //设备名
     char errbuf[PCAP_ERRBUF_SIZE] = {}; //PCAP_ERRBUF_SIZE在pcap.h中已经定义
     bpf_u_int32 netp, maskp;  //网络号和掩码
     pcap_t *handle;          //数据包捕获描述字
     pcap_dumper_t* out_pcap; //记录日志
     struct bpf_program *fp;   
     //char *filter_str = "port 80";  //过滤条件
    	//加载配置文件
    // 配置文件打开文件
char currentDir[260];
char json_position[260];
GetCurrentDirectory(260, currentDir);
//printf("current directory:%s\n",currentDir);
if((strstr(currentDir,"ModBus")==NULL)&&(strstr(currentDir,"MTConnect")==NULL)&&(strstr(currentDir,"OPC-UA")==NULL)){
    strcpy(json_position,"conf.json");
} else {
    strcpy(json_position,"../../../../flow_collection/ndpi/conf.json");
}
FILE* file = fopen(json_position, "r");//配置文件路径
if (file == NULL) {
	printf("Open file fail!\n");
	return 0;
}

// 获得文件大小
struct stat statbuf;
stat(json_position, &statbuf);
int fileSize = statbuf.st_size;

// 分配符合文件大小的内存
char *jsonStr = (char *)malloc(sizeof(char) * fileSize + 1);
memset(jsonStr, 0, fileSize + 1);
// 读取文件中的json字符串
int size = fread(jsonStr, sizeof(char), fileSize, file);
if (size == 0) {
	printf("读取文件失败！\n");
	fclose(file);
	return 0;
}
fclose(file);
// 将读取到的json字符串转换成json变量指针
cJSON *root = cJSON_Parse(jsonStr);
if (!root) {
	printf("Error before: [%s]\n", cJSON_GetErrorPtr());
	free(jsonStr);
	return 0;
}
free(jsonStr);
cJSON *item = NULL;
op = cJSON_GetObjectItem(root, "Network_interface")->valueint;//选择的网卡
//printf("%d\n",op);

using_invalid=cJSON_GetObjectItem(root, "using_invalid")->valueint;//启用黑白名单，默认为黑名单，黑名单为空
if(using_invalid==0) //开启白名单
{
item = cJSON_GetObjectItem(root, "valid_labels");

if (item) {
		cJSON *arr = item->child;
        int i=0;
		while (arr) {
			if (arr->type == cJSON_String) {
				//char *v_str = arr->valuestring;
				strcpy(valid_labels[i++],arr->valuestring);
			}
			arr = arr->next;
		}
	}
}
else if(using_invalid==1)//开启黑名单
{
item = cJSON_GetObjectItem(root, "invalid_labels");
int i=0;
if(item){
       cJSON *arr = item->child;
		while (arr) {
			if (arr->type == cJSON_String) {
				strcpy(valid_labels[i++], arr->valuestring);
			}
			arr = arr->next;
		}
	}
}
 
    if(op==0){
        if((dev = pcap_lookupdev(errbuf)) == NULL){
         printf("lookupdev failed:%s\n", errbuf);
         exit(1);
     }
    }
    else if(op>0&&op<=devs_num){
        dev=devs_name[op-1];
    }
   // printf("Device:%s\n", dev);
     
    /*Get the network number and mask of the network device*/
     if(pcap_lookupnet(dev, &netp, &maskp, errbuf) == -1){
         printf("%s\n", errbuf);
         exit(1);
     }
     
    /*Open network device*/
     if((handle = pcap_open_live(dev, 65535, 1, 0, errbuf)) == NULL){
         printf("%s\n", errbuf);
         exit(1);
     }
     
    setupDetection();
    printf("start detection...\n");

    flow_info_file_name = json_path;
    out_pcap = pcap_dump_open(handle,pcap_path);
    flag=1;
    if(pcap_loop(handle, -1, pcap_packet_callback, (u_char*)out_pcap)==-1){
        printf("pcap_loop error...\n");
          pcap_close(handle);
         pcap_dump_flush(out_pcap); //刷新缓冲区
         pcap_dump_close(out_pcap); //关闭资源
    };

    }
    
    else if(argc==3){
    pcap_t *handle; //store the "device" (from tcpdump)

    handle = pcap_open_offline(argv[1],NULL);

    if(handle==NULL){
       printf("Couldn't open the file %s\n", argv[1]);
       return(-1);
    } 
       setupDetection();
       pcap_loop(handle, -1, pcap_packet_callback, NULL);
       pcap_close(handle);
       flow_info_file_name = argv[2];
       printResults();
    }

return 0;
}
